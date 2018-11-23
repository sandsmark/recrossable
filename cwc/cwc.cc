/**
 * cwc - a crossword compiler.
 *
 * Copyright (C) 1999, 2000, 2001, 2002 Lars Christensen, 2008 Mark Longair
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 **/

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <locale.h>
#include <iostream>
#include <sys/time.h>

#include <fstream>
#include <string>
#include <math.h>
#include <stdlib.h>

#include <set>
#include <vector>
#include <list>

#include "timer.hh"
#include "symbol.hh"
#include "dict.hh"
#include "letterdict.hh"
#include "grid.hh"

#include "cwc.hh"

//////////////////////////////////////////////////////////////////////
// class walker

Walker::Walker(Grid &thegrid) : current(0), g(thegrid) {
    limit = thegrid.getempty();
    inited = false;
}

Cell &Walker::currentCell() {
    if (!inited) throw error("walker not inited");
    return g.cellno(current);
}

void Walker::backTo(int c) {
    while (current != c)
        backward();
}

bool Walker::current_oneof(int no[], int n) {
    for (int i=0;i<n;i++) {
        if (no[i] == current)
            return true;
    }
    return false;
}

void Walker::backToOneOf(int no[], int n) {
    while (!current_oneof(no, n))
        backward();
}

void Walker::backToOneOf(Backtracker &bt) {
    backward(false); // dont save current
    while (!bt.stopHere(current))
        backward(true); // save all we skip
}

void Walker::forward() {
    if (inited) {
        cellno.push_back(current);
        do step_forward(); while (!g.cellno(current).isempty());
    } else {
        init();
        inited = true;
    }
}

void Walker::backward(bool savepreferred) {
    if (!g.cellno(current).isoutside())
        g.cellno(current).clear(savepreferred);
    current = cellno.back();
    cellno.pop_back();
}

bool Walker::moresteps() {
    return (cellno.size() + 1) < unsigned(limit);
}

void Walker::findnext() {
    int ncells = g.numcells();
    for (int i = 0; i < ncells; i++) {
        std::cout << i << " " << g.cellno(i).getsymbol().symbvalue() << std::endl;
        if (g.cellno(i).isempty()) {
            current = i;
            return;
        }
    }
    throw error("No empty cells");
}

void Walker::init() {
    findnext();
}

//////////////////////////////////////////////////////////////////////
// class prefix_walker

void PrefixWalker::step_forward() {
    current++; // not correct now
}

//////////////////////////////////////////////////////////////////////
// class flood_walker

FloodWalker::FloodWalker(Grid &g) : Walker(g) {
}

void FloodWalker::step_forward() {
    std::vector<int>::iterator i;
    for (i = cellno.begin(); i != cellno.end(); i++) {
        int cno = *i;

        int nwords = g.cellno(cno).numwords();
        for (int w = 0; w < nwords; w++) {
            Cell &thecell = g.cellno(cno);
            WordBlock &wb = thecell.getwordblock(w);
            int pos = thecell.getpos(w);
            int len = wb.length();

            if (pos > 0) {
                int cellbefore = wb.getcellno(pos-1);
                if (g.cellno(cellbefore).isempty()) {
                    current = cellbefore;
                    return;
                }
            }
            if (pos < len-1) {
                int cellafter = wb.getcellno(pos + 1);
                if (g.cellno(cellafter).isempty()) {
                    current = cellafter;
                    return;
                }
            }
        }
    }
    // at this point: no adjacent cells found
    findnext();
}

//////////////////////////////////////////////////////////////////////
// class backtracker

Backtracker::Backtracker(Grid &thegrid) : g(thegrid) {
}

//////////////////////////////////////////////////////////////////////
// class naive_backtracker

// The naive backtracker simply backs up to the previously
// filled cell.

void NaiveBacktracker::backtrack(Walker &w) {
    w.backward(false);
}

//////////////////////////////////////////////////////////////////////
// class smart_backtracker
//
// the smart backtracker steps back to the previously filled cell
// that is within reach from the current cell

void SmartBacktracker::backtrack(Walker &w) {
    // search up
    int cpos = w.stepCount();

    // initially, forget all bt points that are dependend on cells behind
    // current point.
    std::list<cpair>::iterator next;
    for (std::list<cpair>::iterator i = bt_points.begin();
         i != bt_points.end();
         i = next) {
        next = i; next++;
        if ((*i).first <= cpos) {
            if (setup.debuginfo) std::cout << "removing " << (*i).second << "(from " << (*i).first << ")" << std::endl;
            bt_points.erase(i);
        }
    }

    int cno = w.getCurrent();

    Cell &c = g.cellno(cno);
    int nwords = c.numwords();
    for (int wno = 0; wno < nwords; wno++) {
        WordBlock &wb = c.getwordblock(wno);
        int len = wb.length();

        int pos = c.getpos(wno);
        for (int p = 0; p < len; p++) {
            if ((p!=pos)&&(wb.getcell(p).isfilled()))
                bt_points.push_back(cpair(cpos, wb.getcellno(p)));
        }

    }

    if (setup.debuginfo) {
        std::cout << "BTSET:" << std::endl;
        for (std::list<cpair>::iterator i = bt_points.begin(); i != bt_points.end(); i++)
            std::cout << " (" << (*i).first << ',' << (*i).second << ")";
        std::cout << std::endl;
    }

    w.backToOneOf(*this);
}

bool SmartBacktracker::stopHere(int p) {
    for (std::list<cpair>::iterator i = bt_points.begin();
         i != bt_points.end();
         i++) {
        if ((*i).second == p) {
            bt_points.erase(i); // dont stop here again
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////
// compiler
//
// Cross word compiler logic

Compiler::Compiler(Grid &thegrid, Walker &thewalker,
                   Backtracker &thebacktracker, Dict &thedict)
    : g(thegrid), w(thewalker), bt(thebacktracker), d(thedict) {
    g.verbose = verbose = false;
    findall = false;
}

#define success true
#define failure false

// upon failure, walker is backed up to some cell
// the reclevel trying to compute this cell catches it
// and others will return.

Timer dtimer;

bool Compiler::compile_rest(double rejected) {
    int c = w.getCurrent();
    if (verbose)
        std::cout << "attempting to find solution for " << c << std::endl;
    SymbolSet ss = g(c).findpossible(d);
    int npossible = numones(ss);
    rejected += (numalpha-double(npossible)) * pow(numalpha, numcells - w.stepCount());
    if (verbose)
        dumpset(ss);

    SymbolSet bit;
    // use preferred if any
    if (g(c).haspreferred()) {
        Symbol s = g(c).getpreferred();
        SymbolSet ss2 = s.getsymbolset();
        if (ss2 & ss) {
            bit = ss2;
            ss &= ~bit; // remove bit from set
        }
        else
            bit = pickbit(ss);
    } else
        bit = pickbit(ss);
    for (; bit; bit=pickbit(ss)) {
        Symbol s = Symbol::symbolbit(bit);
        g(c).setsymbol(s);
        if (w.moresteps()) {
            w.forward();
            if (compile_rest(rejected) == success) return success;
            if (w.getCurrent() != c) return failure; // catch if ==
            // cout << "continue at " << c << endl;
            rejected += pow(numalpha, numcells - w.stepCount());
        } else {
            this->rejected = rejected;
            return success;
        }
        g(c).setsymbol(Symbol::empty);
    }
    if (w.stepCount() > 1) {
        bt.backtrack(w);
        int cur = w.getCurrent();
        if (verbose)
            std::cout << "return to " << cur << " from " << c << std::endl;
    }
    return failure;
}

bool Compiler::compile() {
    dtimer.reset(); dtimer.start();
    w.forward();
    numcells = g.numopen();
    numalpha = Symbol::numalpha();
    return compile_rest();
}

//////////////////////////////////////////////////////////////////////
// main

void dumpset(SymbolSet ss) {
    int i,n;
    std::cout << '{';
    for (i=1, n = 0; i; i<<=1, n++) {
        if (ss&i)
            std::cout << Symbol::alphabet[n];
    }
    std::cout << '}' << std::endl;
}

void dumpsymbollist(Symbol *s, int n) {
    for (int i=0;i<n;i++) {
        if (s[i] == Symbol::empty)
            std::cout << '-';
        else
            std::cout << s[i];
    }
    std::cout << std::endl;
}

setup_s setup = {
    setup.ascii_format,
    setup.floodwalker,
    setup.letterdict,
    false,
    false,
    false,
    DEFAULT_DICT_FILE,
    "",
    "",
    setup.noformat,
    false,
    false,
    0,
    false,
};

#if 0
int main_cwc(int argc, char *argv[]) {
    if (parseparameters(argc, argv) == -1) exit(EXIT_FAILURE);
    random_init(setup);
    symbol::buildindex();

    if (setlocale(LC_CTYPE, "") == 0)
        cout << "Failed to set locale" << endl;

    try {

        if (setup.benchdict) {
            dodictbench();
            exit(EXIT_SUCCESS);
        }

        dict *d = 0;
        if (setup.dictstyle==setup.btreedict) {
            cout << "Using binary tree index" << endl;
            d = new btree_dict();
        } else if (setup.dictstyle==setup.letterdict) {
            cout << "Using letter index" << endl;
            d = new letterdict();
        }
        d->load(setup.dictfile);

        grid g;
        if (setup.gridformat == setup.generalgrid)
            g.load(setup.gridfile);
        else if (setup.gridformat == setup.squaregrid)
            g.load_template(setup.gridfile);
        // g.dump_ggrid(cout);
        int nopen = g.numopen();
        double space = pow(symbol::numalpha(), nopen);
        cout << nopen << " cells to be filled. " << space << " possible fillings." << endl;
        walker *w;
        if (setup.walkertype==setup.prefixwalker) {
            w = new prefix_walker(g);
            puts("Using prefix walking heuristics");
        } else if (setup.walkertype==setup.floodwalker) {
            w = new flood_walker(g);
            puts("Using flood walking heuristics");
        } else {
            puts("Internal error");
            exit(EXIT_FAILURE);
        }

        cout << "Degree of interlock: " << g.interlockdegree()*100 << "%" << endl;
        double depdeg1 = g.dependencydegree(1);
        double depdeg2 = g.dependencydegree(2);
        cout << "Degree of dependency: " << depdeg1 << '(' << (depdeg1*100.0/nopen) << "%)" << endl;
        cout << "Degree of 2nd level dependency: " << depdeg2 << '(' << (depdeg2*100.0/nopen) << "%)" << endl;

        smart_backtracker bt(g);

        compiler c(g, *w, bt, *d);
        c.verbose = setup.verbose;
        c.showsteps = setup.showsteps;
        timer t; t.start();
        c.compile();
        t.stop();

        if (g.w == 0) {
            g.dump(cout,setup.output_format,0);
            if( setup.svgfile.length() > 0 ) {
                cout << "Warning: can't currently generate SVG files ";
                cout << "for non-grid crosswords, so ";
                cout << "ignoring '-s " << setup.svgfile << "'" << endl;
            }
        } else {
            answers an = g.getanswers();
            g.dump(cout, setup.output_format, &an);
            cout << endl;
            an.dump(cout);

            if( setup.svgfile.length() > 0 ) {
                ofstream ofs(setup.svgfile.c_str());
                draw_to_svg(ofs, g, &an);
                ofs.close();
                cout << "Wrote SVG to: '" << setup.svgfile << "'" << endl;
            }
        }
        cout << "Attempt average: " << g.attemptaverage() << endl;
        cout << "Compilation time: " << t.getmsecs() << " msecs" << endl;

        double searched = c.getrejected();
        cout << searched << " solutions searched. " << (searched*100/space) << "% of search space." << endl;
    } catch (error e) {
        cout << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}
#endif
