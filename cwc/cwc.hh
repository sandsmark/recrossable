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
#include "grid.hh"

#include <map>
#include <list>

//////////////////////////////////////////////////////////////////////

class Backtracker;

class Walker {
protected:
    std::vector<int> cellno;
    int current;
    Grid &g;

    bool current_oneof(int *no, int n);
    int limit;
    bool inited;

public:
    Walker(Grid &thegrid);
    virtual ~Walker() {}
    void backTo(int dest);
    void backToOneOf(int dest[], int n);
    void backToOneOf(Backtracker &bt);
    int getCurrent() { return current; }
    Cell &currentCell();
    void forward();
    void backward(bool savepreferred = false);
    int stepCount() { return cellno.size() + 1; }

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


class PrefixWalker : public Walker {
public:
    PrefixWalker(Grid &g) : Walker(g) {}
protected:
    virtual void step_forward();
};

class FloodWalker : public Walker {
public:
    FloodWalker(Grid &g);
protected:
    void step_forward();
};

//////////////////////////////////////////////////////////////////////

class Backtracker {
protected:
    Grid &g;
public:
    Backtracker(Grid &thegrid);
    virtual ~Backtracker() {}
    // upon a dead end, this method will track back to
    // a cell where a new solution should be tried.
    virtual void backtrack(Walker &w) =  0;
    virtual bool stopHere(int p) = 0;
};

class NaiveBacktracker : public Backtracker {
public:
    NaiveBacktracker(Grid &thegrid) : Backtracker(thegrid) {}
    void backtrack(Walker &w) override;
    bool stopHere(int /*p*/) override { return true; }
};

class SmartBacktracker : public Backtracker {
    // first = pos, second = bt point.
    // nb: pair<> is sorted on the first element (according to STL doc).
    typedef std::pair<int, int> cpair;
    std::list<cpair> bt_points;
public:
    SmartBacktracker(Grid &thegrid) : Backtracker(thegrid) {}
    void backtrack(Walker &w) override;
    bool stopHere(int p) override;
};

class Compiler {
protected:
    int numcells;
    int numalpha;
    double rejected;
    Grid &g;
    Walker &w;
    Backtracker &bt;
    Dict &d;
    bool compile_rest(double rejected = 0);
public:
    Compiler(Grid &thegrid, Walker &thewalker, Backtracker &thebacktracker, Dict &thedict);
    bool compile();

    bool verbose, findall, showsteps;
    double getRejected() { return rejected; }
};

void dodictbench();
int dictbench(Dict &d);

#endif // CWC_CWC_HH
