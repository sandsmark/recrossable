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

#ifndef CWC_CWC_HH
#define CWC_CWC_HH

#include "main.hh"

#include <map>
#include <list>

using std::pair;
using std::list;

//////////////////////////////////////////////////////////////////////

class backtracker;

class walker {
protected:
    vector<int> cellno;
    int current;
    grid &g;

    bool current_oneof(int *no, int n);
    int limit;
    bool inited;

public:
    walker(grid &thegrid);
    virtual ~walker() {}
    void backto(int dest);
    void backto_oneof(int dest[], int n);
    void backto_oneof(backtracker &bt);
    int getcurrent() { return current; }
    cell &currentcell();
    void forward();
    void backward(bool savepreferred = false);
    int stepno() { return cellno.size() + 1; }

protected:
    /**
     * new walkers _must_ implement step_forward(). init() and findnext()
     * _may_ be redefined, but that is not necessary.
     */

    /**
     * find the _next_ cell to fill. Stepping into outside cells is
     * allowed in what case the walker will simple make another step
     * until we are inside.
     */
    virtual void step_forward() = 0;
    /**
     * inits the walker to an initial position. The default method calls
     * findnext();
     */
    virtual void init();
    /**
     * find the first free cell in the grid.
     */
    virtual void findnext();
public:
    bool moresteps();
};


class prefix_walker : public walker {
public:
    prefix_walker(grid &g) : walker(g) {};
protected:
    virtual void step_forward();
};

class flood_walker : public walker {
public:
    flood_walker(grid &g);
protected:
    void step_forward();
};

//////////////////////////////////////////////////////////////////////

class backtracker {
protected:
    grid &g;
public:
    backtracker(grid &thegrid);
    virtual ~backtracker() {}
    // upon a dead end, this method will track back to
    // a cell where a new solution should be tried.
    virtual void backtrack(walker &w) =  0;
    virtual bool stophere(int p) = 0;
};

class naive_backtracker : public backtracker {
public:
    naive_backtracker(grid &thegrid) : backtracker(thegrid) {}
    void backtrack(walker &w);
    bool stophere(coord &c) { return true; }
};

class smart_backtracker : public backtracker {
    // first = pos, second = bt point.
    // nb: pair<> is sorted on the first element (according to STL doc).
    typedef pair<int, int> cpair;
    list<cpair> bt_points;
public:
    smart_backtracker(grid &thegrid) : backtracker(thegrid) {}
    void backtrack(walker &w);
    bool stophere(int p);
};

class compiler {
protected:
    int numcells;
    int numalpha;
    double rejected;
    grid &g;
    walker &w;
    backtracker &bt;
    dict &d;
    bool compile_rest(double rejected = 0);
public:
    compiler(grid &thegrid, walker &thewalker, backtracker &thebacktracker, dict &thedict);
    void compile();

    bool verbose, findall, showsteps;
    double getrejected() { return rejected; }
};

void dodictbench();
int dictbench(dict &d);

#endif // CWC_CWC_HH
