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

#include <fstream>
#include <stdio.h>
#include <sstream>
#include <algorithm>

#include "grid.hh"

//////////////////////////////////////////////////////////////////////
// wordblock

WordBlock::WordBlock() {
    cls_size = 0;
}

void WordBlock::getword(Symbol *s) {
    int i, len = cls.size();
    for (i = 0; i < len; i++)
        s[i] = cls[i]->getsymbol();
}

//////////////////////////////////////////////////////////////////////
// class cell

Cell Cell::outside_cell;

Cell::Cell(Symbol s) :
    wbl_size(0), attempts(0), symb(s), preferred(Symbol::none), locked(false) {
}

void Cell::addword(WordBlock *w, int pos) {
    struct WordRef wr = {pos, w};
    wbl.push_back(wr);
    wbl_size++;
}

void Cell::setsymbol(const Symbol &s) {
    if (locked)
        throw error("Attempt to set symbol in locked cell");
    if (!(s == Symbol::empty) && !(s == Symbol::outside))
        attempts++;
    symb = s;
}

void Cell::remove() {
    symb = Symbol::outside;
}

void Cell::clear(bool setpreferred) {
    if (setpreferred)
        preferred = symb;
    else
        preferred = Symbol::none;
    symb = Symbol::empty;
}

std::ostream &operator << (std::ostream &os, Cell &c) {
    return os << c.symb;
}

bool Cell::isoutside() {
    return symb == Symbol::outside;
}

bool Cell::isfilled() {
    return (symb!=Symbol::empty)&&(symb!=Symbol::outside);
}

std::string Cell::tostring() {
    std::ostringstream s;
    s << symb;
    return s.str();
}

std::string Cell::touppercasestring() {
    std::string s = tostring();
    transform(s.begin(), s.end(), s.begin(), toupper);
    return s;
}

//////////////////////////////////////////////////////////////////////
// class coord

coord::coord(int _x, int _y) : x(_x), y(_y) {
}

std::ostream &operator << (std::ostream &os, coord &c) {
    return os << c.x << ',' << c.y;
}

//////////////////////////////////////////////////////////////////////
// class grid

Grid::Grid(int width, int height)
    : cls(0), cls_size(0), verbose(false) {
    init_grid(width, height);
    cellno(-1).setsymbol(Symbol::outside);
    buildwords();
}

void Grid::init_grid(int w, int h) {
    this->w = w;
    this->h = h;
    cls.clear();
    for (int i = 0; i < w*h; i++)
        cls.push_back(Cell());
    cls_size = cls.size();
}

/**
void grid::get_template_h(const coord &c, symbol *s, int &len, int &pos) {
    coord i = c;
    // find first
    while (!cellat(i.x-1, i.y).isoutside())
        i.x--;
    pos = c.x - i.x;
    len = 0;
    while (!cellat(i).isoutside()) {
        s[len] = cellat(i).getsymbol();
        i.x++; len++;
    }
}

void grid::get_template_v(const coord &c, symbol *s, int &len, int &pos) {
    coord i = c;
    // find first
    while (!cellat(i.x, i.y-1).isoutside())
        i.y--;
    pos = c.y - i.y;
    len = 0;
    while (!cellat(i).isoutside()) {
        s[len] = cellat(i).getsymbol();
        i.y++; len++;
    }
}
**/

SymbolSet Cell::findpossible(Dict &d) {
    int nwords = numwords();
    if (nwords == 0) throw error("Bugger");

    SymbolSet ss = ~0;

    for (int i = 0; i < nwords; i++) {

        int pos = getpos(i);
        WordBlock &wb = getwordblock(i);
        int len = wb.length();

        Symbol word[len+1]; word[len] = Symbol::outside;

        wb.getword(word);

        ss &= d.findpossible(word, len, pos); // intersect solutions
//        if (setup.verbose) {
//            cout << "vertical: "; dumpsymbollist(word, len);
//            dumpset(ss);
//        }
    }

    return ss;
}


void Grid::load_template(const std::string &filename) {
    std::ifstream tf(filename.c_str());
    if (!tf.is_open()) throw error("Failed to open pattern file");
    std::string istr;
    getline(tf, istr);
    w = h = 1;
    sscanf(istr.c_str(), "%d %d", &w, &h);
    if ((w==0)||(h==0))
        throw error("Grid dimensions must be at least 1");
    init_grid(w, h);

    for (int y=0;y<h;y++) {
        getline(tf,istr);
        if (tf.eof()) throw error("Not enough lines in input file");
        // if (istr.length() < unsigned(w+1)) throw error("Line to short in input file");
        for (int x=0;x<w;x++) {
            if (unsigned(x) >= istr.length()) {
                cellat(x, y).remove();
                continue;
            }

            char ch = istr[x];
            if (ch == '+')
                cellat(x, y).clear();
            else if (ch == ' ' )
                cellat(x, y).remove();
            else if (isalpha(ch))
                cellat(x, y).setsymbol(tolower(ch));
            else
                throw error("Invalid character in input file");
        }
    }
    buildwords();
    lock();
}

void Grid::load(const std::string &fn) {
    cls.clear();
    wbl.clear();
    w = h = 0;

    std::ifstream f(fn.c_str());
    if (!f.is_open()) throw error("Failed to open file");
    std::string ln;

    while (!f.eof()) {
        getline(f, ln);
        const char *st = ln.c_str();

        WordBlock *wb = new WordBlock();
        int pos = 0;
        while (*st != '\0') {
            while (*st&&(!isdigit(*st))) st++;
            if (*st == '\0') break;
            int a = atoi(st);
            while (cls.size() <= unsigned(a))
                cls.push_back(Cell(Symbol::outside));
            cls_size = cls.size();
            cls[a].setsymbol(Symbol::empty);
            wb->addcell(a, *this);
            cls[a].addword(wb, pos); pos++;
            while (*st && (isdigit(*st))) st++;
        }
        if (wb->length())
            wbl.push_back(wb);
        else
            delete wb;
    }
    lock();
}

/**
 * builds the words/cell structures when we use a square grid formation
 */

void Grid::buildwords() {
    wbl.clear();
    for (int n = 0; n < numcells(); n++)
        cls[n].clearwords();

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int cno = cellnofromxy(x, y);
            WordBlock *w = 0;
            int pos = 0;
            while (cellat(x, y).isinside()) {
                if (w==0) { w = new WordBlock(); wbl.push_back(w); }
                w->addcell(cno, *this);
                cellno(cno).addword(w, pos);
                pos++;
                x++;
                cno = cellnofromxy(x, y);
            }
        }
    }
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            WordBlock *w = 0;
            int pos = 0;
            while (cellat(x, y).isinside()) {
                if (w == 0) { w = new WordBlock(); wbl.push_back(w); }
                w->addcell(cellnofromxy(x, y), *this);
                cellat(x, y).addword(w, pos);
                pos++; y++;
            }
        }
    }
}

void Grid::dump_ggrid(std::ostream &os) {
    bool first = true;
    for (std::vector<WordBlock*>::iterator i = wbl.begin(); i != wbl.end(); i++) {
        int wlen = (*i)->length();
        for (int p = 0; p < wlen; p++) {
            if (!first) std::cout << ' ';
            os << (*i)->getcellno(p);
            first = false;
        }
        os << std::endl; first = true;
    }
}

char bold[] = "\x1b[1m";
char normal[] = "\x1b[0m";

void Grid::dump_ascii(std::ostream &os,Answers * an) {
    /* If we're given answers as well, then print the entries two
       high, possibly with a clue number in the top row. */
    std::ostringstream vertbarstream;
    vertbarstream << '+';
    for (int i=0;i<w;i++) vertbarstream << "---+";
    std::string vertbar = vertbarstream.str();

    os << vertbar << std::endl;
    for (int y=0;y<h;y++) {
        if (an) {
            os << "|";
            for (int x=0; x<w; x++) {
                Cell &c = cellat(x,y);
                if (c.isoutside())
                    os << "XXX|";
                else {
                    int cellnumber = y*w + x;
                    if( an->celltoclue.find(cellnumber) != an->celltoclue.end() ) {
                        int cluenumber = an->celltoclue[cellnumber];
                        char cluenumberstring[4];
                        snprintf(cluenumberstring,4,"%-4d",cluenumber);
                        cluenumberstring[3] = 0;
                        os << cluenumberstring << "|";
                    } else
                        os << "   " << "|";
                }
            }
            os << std::endl;
        }
        // Now draw the rows with the letters:
        os << "|";
        for (int x=0; x<w; x++) {
            Cell &c = cellat(x,y);
            if (c.isoutside())
                os << "XXX|";
            else
                os << ' ' <<  c << " |";
        }
        os << std::endl;
        os << vertbar << std::endl;
    }
}

void Grid::dump(std::ostream &os, Answers * an) {
    if (w == 0) {
        for (int i = 0; unsigned(i) < wbl.size(); i++) {
            int len = wbl[i]->length();
            Symbol *s = new Symbol[len + 1];
            s[len] = Symbol::outside;
            wbl[i]->getword(s);
            os << s << ' ';
            delete[] s;

            os << '(';
            for (int p = 0; p < len; p++) {
                if (p) os << ',';
                os << wbl[i]->getcellno(p);
            }
            os << ')' << std::endl;
        }
        return;
    }

    dump_ascii(os,an);
}

Answers Grid::getanswers() {
    Answers c;
    std::unordered_set<int> startcells;
    /* Index the start cells, and decide which are across and down
       clues: */
    std::vector<WordBlock*>::iterator i;
    for (i=wbl.begin();
         i!=wbl.end();
         ++i) {
        WordBlock * wb = *i;
        int l=wb->length();
        if (l>1) {
            // Then it's a real clue. Guess if it's across or down:
            int firstcell=wb->getcellno(0);
            int secondcell=wb->getcellno(1);
            bool across = (firstcell + 1) == secondcell;
            // Get the answer as a string:
            char * c_str=new char[l+1];
            Symbol *symbols = new Symbol[l+1];
            symbols[l] = Symbol::outside;
            wb->getword(symbols);
            for(int k=0;k<l;++k) {
                c_str[k] = (char)symbols[k];
            }
            c_str[l] = 0;
            std::string string_version(c_str);
            delete [] symbols;
            delete [] c_str;
            // Add those:
            if (across) {
                c.across.cells.insert(firstcell);
                c.across.celltoanswer[firstcell] = string_version;
            } else {
                c.down.cells.insert(firstcell);
                c.down.celltoanswer[firstcell] = string_version;
            }
            startcells.insert(firstcell);
        }
    }
    /* Number the start cells: */
    int cluenumber=1;
    std::unordered_set<int>::iterator s;
    for(s=startcells.begin();
        s!=startcells.end();
        ++s) {
        c.celltoclue[*s] = cluenumber;
        if( c.across.cells.find(*s) != c.across.cells.end() ) {
            c.across.cluetoanswer[cluenumber] = c.across.celltoanswer[*s];
            c.across.clues.insert(cluenumber);
        }
        if( c.down.cells.find(*s) != c.down.cells.end() ) {
            c.down.cluetoanswer[cluenumber] = c.down.celltoanswer[*s];
            c.down.clues.insert(cluenumber);
        }
        ++ cluenumber;
    }
    return c;
}

float Grid::interlockdegree() {
    int interlocked = 0, total = 0;
    for (int y=0; y<h; y++) {
        for (int x=0;x<w;x++) {
            if (cellat(x, y).isinside()) {
                bool vertuse = cellat(x, y-1).isinside() || cellat(x, y+1).isinside();
                bool horizuse = cellat(x-1, y).isinside() || cellat(x+1, y).isinside();
                if (vertuse && horizuse)
                    interlocked++;
                total++;
            }
        }
    }
    return float(interlocked) / float(total);
}

float Grid::attemptaverage() {
    int sum = 0, n = 0;
    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            if (cellat(x, y).isinside()) {
                sum += cellat(x, y).getattempts();
                n++;
            }
        }
    }
    return sum / float(n);
}

double Grid::dependencydegree(int level) {
    int n = numcells(), ncell = 0;
    int d = 0;
    for (int i = 0; i < n; i++) {
        if (cellno(i).isinside()) {
            d += celldependencies(i, level);
            ncell++;
        }
    }
    return double(d) / double(ncell);
}

int Grid::celldependencies(int cno, int level) {
    std::unordered_set<int> cnums;
    cnums.insert(cno);

    while (level--) {
        std::unordered_set<int> newnums;
        for (std::unordered_set<int>::iterator i = cnums.begin(); i != cnums.end(); i++) {
            int clno = *i;
            int words = cellno(clno).numwords();
            for (int w = 0; w < words; w++) {
                WordBlock &wb = cellno(clno).getwordblock(w);
                int wlen = wb.length();
                for (int j = 0; j < wlen; j++)
                    newnums.insert(wb.getcellno(j));
            }
        }
        for (std::unordered_set<int>::iterator i = newnums.begin(); i != newnums.end(); i++)
            cnums.insert(*i);
    }
    return cnums.size();
}

void Grid::lock() {
    int n = numcells();
    for (int i = 0; i < n; i++)
        if (!cellno(i).isempty())
            cellno(i).lock();
}

int Grid::getempty() {
    int n = 0;
    int ncells = numcells();
    for (int i = 0 ; i< ncells; i++)
        if (cellno(i).isempty())
            n++;
    return n;
}

int Grid::numopen() {
    int n=0;
    int ncells = numcells();
    for (int i = 0; i < ncells; i++)
        if (!cls[i].islocked())
            n++;
    return n;
}

void ClueNumbering::dump(std::ostream &os) {
    std::unordered_set<int>::iterator a;
    for(a=clues.begin();
        a!=clues.end();
        ++a) {
        int cluenumber=*a;
        std::string answer=cluetoanswer[cluenumber];
        os << cluenumber << ". " << answer << std::endl;
    }
}

void Answers::dump(std::ostream &os) {
    os << "Across:" << std::endl;
    across.dump(os);
    os << std::endl << "Down:" << std::endl;
    down.dump(os);
}
