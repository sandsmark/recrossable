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

#ifndef CWC_DICT_HH
#define CWC_DICT_HH

#include "symbol.hh"

//////////////////////////////////////////////////////////////////////

struct SymbolLink {
    Symbol symb;
    static int instancecount;
    SymbolLink *target, *next;
    SymbolLink *getlink(Symbol);
    SymbolLink();
    SymbolLink *addlink(Symbol);
    void addword(Symbol *, int);
    bool findpossible(Symbol *s, int len, int pos, SymbolSet &ss);
    void dump(char *prefix = 0, int len = 0);
};

class Dict {
public:
    Dict();
    virtual ~Dict();

    virtual void load(const std::string &fn) = 0;
    virtual SymbolSet findpossible(Symbol *s, int len, int pos) = 0;
};

class BtreeDict : public Dict {
    SymbolLink primary[MAXWORDLEN];
public:
    BtreeDict();
    void addWord(Symbol *, int);
    void load(const std::string &fn);
    int size();
    SymbolSet findpossible(Symbol *s, int len, int pos);
    void dump(int len);
};

#endif // CWC_DICT_HH

