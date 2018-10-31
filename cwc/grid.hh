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

#ifndef CWC_GRID_HH
#define CWC_GRID_HH

#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <sstream>
#include "symbol.hh"
#include "dict.hh"

class Cell;
class WordBlock;
class Grid;
struct WordRef {
    int pos;
    WordBlock *wbl;
};

class Cell {
    std::vector<WordRef> wbl; int wbl_size;
    int attempts;
    Symbol symb;
    Symbol preferred;
    bool locked;
public:
    static Cell outside_cell;

    Cell(Symbol s = Symbol::empty); // ctor
    void setsymbol(const Symbol &s);

    // structural methods
    void addword(WordBlock *w, int pos);
    int numwords() { return wbl_size; }
    WordBlock &getwordblock(int wordno) { return *wbl[wordno].wbl; }
    int getpos(int wordno) { return wbl[wordno].pos; }
    void clearwords() { wbl.clear(); }

    Symbol getsymbol() { return symb; }
    Symbol getpreferred() { return preferred; }

    bool haspreferred() { return preferred != Symbol::none; }
    void usepreferred();
    void remove(); // remove from grid (make outsider)
    void clear(bool setpreferred = true); // make empty

    bool isoutside();
    bool isinside() { return !isoutside(); }

    bool isempty() {
        return symb == Symbol::empty;
    }

    bool isfilled();

    int getattempts() { return attempts; }

    bool islocked() { return locked; }
    void lock() { locked = true; }

    SymbolSet findpossible(Dict &d);

    friend std::ostream&operator<<(std::ostream &os, Cell &c);

    void dumpwords() {
        std::cout << "got " << numwords() << " words." << std::endl;
    }

    // statistics

    int dependencydegree(int level);

    // Build a mapping from cell numbers to clue numbers:
    std::unordered_map<int,int> celltoclue();

    std::string tostring();
    std::string touppercasestring();
};

struct Coord {
    int x, y;
public:
    Coord(int _x = 0, int _y = 0);
    bool operator==(const Coord &c) const { return ((x==c.x)&&(y==c.y)); }
};

std::ostream & operator<<(std::ostream &os, Coord &c);

class ClueNumbering {
public:
    std::unordered_set<int> cells;
    std::set<int> clues;
    std::unordered_map<int,std::string> cluetoanswer;
    std::unordered_map<int,std::string> celltoanswer;
    void dump(std::ostream &os);
};

class Answers {
public:
    std::unordered_map<int,int> celltoclue;
    ClueNumbering across;
    ClueNumbering down;
    void dump(std::ostream &os);
};

class Grid {
protected:
    std::vector<Cell> cls; int cls_size;
    std::vector<WordBlock*> wbl;
    void init_grid(int w, int h);

public:
    bool verbose;
    int w, h;
    Grid(int width = 4, int height = 4);

    inline Cell &cellno(int n) {
        if ((n < 0)||(n >= cls_size))
            return Cell::outside_cell;
        return cls[n];
    }

    inline int cellnofromxy(int x, int y) {
        return y*w + x;
    }

    inline Cell &cellat(int x, int y) {
        if ((x < 0)||(x >= w)||(y < 0)||(y>=h))
            return Cell::outside_cell;
        return cls[y*w + x];
    }

    Cell &cellat(Coord &c) {
        return cellat(c.x, c.y);
    }

    Cell &operator()(int x, int y) { return cellat(x, y); }
    Cell &operator()(Coord &c) { return cellat(c); }
    Cell &operator()(int p) { return cellno(p); }

    void load_template(std::istream &stream);
    void load_template(const std::string &filename);
    void load(const std::string &fn);
    void load(std::istream &stream);
    void buildwords();

    void dump(std::ostream &os, Answers * an);
    void dump_ascii(std::ostream &os, Answers * an);

    void dump_ggrid(std::ostream &os);

    void lock();

    int getempty();

    // statistics

    float interlockdegree();
    float density();
    float attemptaverage();
    int numopen();
    int numcells() { return cls.size(); }
    double dependencydegree(int level);
    int celldependencies(int cellno, int level);

    // build clue numbering
    Answers getanswers();
};


struct CellRef {
    int cellno;
    Grid *g;
    CellRef(int no, Grid &gr) : cellno(no), g(&gr) {}
    Cell *operator ->() { return &g->cellno(cellno); }
    Cell *ptr() { return &g->cellno(cellno); }
};


class WordBlock {
    std::vector<CellRef> cls; int cls_size;
public:
    WordBlock();
    void addcell(int n, Grid &gr) {
        cls.push_back(CellRef(n, gr));
        cls_size++;
    }
    int length() { return cls_size; }
    void getword(Symbol *);
    int getcellno(int pos) {
        if ((pos < 0)||(pos >= cls_size)) throw error("Bug");
        return cls[pos].cellno;
    }
    Cell &getcell(int pos) {
        if ((pos < 0)||(pos >= cls_size)) return Cell::outside_cell;
        return *cls[pos].ptr();
    }
};

#endif // CWC_GRID_HH
