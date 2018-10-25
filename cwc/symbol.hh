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

#ifndef CWC_SYMBOL_HH
#define CWC_SYMBOL_HH

#include "main.hh"

typedef unsigned long SymbolSet;

#define UNDEF 0x7f

class Symbol {
    char symb;
    static Symbol alphindex[256];
    static char searchchar(char);
    static int symballoc;
public:
    static char alphabet[32];
    static Symbol outside, empty, none;
    static Symbol alloc(char ch = UNDEF);
    static Symbol symbolbit(SymbolSet); // named constructor
    //  static symbol special();

    Symbol() : symb(UNDEF) {}
    Symbol(char ch);

    inline SymbolSet getsymbolset();
    inline operator char();
    inline bool operator == (Symbol const &s) const;

    static void buildindex();
    int symbvalue() { return int(symb); }
    static int numalpha();
};

SymbolSet Symbol::getsymbolset() {
    return 1 << symb;
}

bool Symbol::operator==(Symbol const &s) const {
    return symb == s.symb;
}

Symbol::operator char() {
    if (symb == UNDEF) throw error("action on undefined symbol");
    return alphabet[(int)symb];
}

SymbolSet pickbit(SymbolSet &ss);

//////////////////////////////////////////////////////////////////////

void dumpset(SymbolSet ss);
void dumpsymbollist(Symbol *s, int n);
std::ostream &operator <<(std::ostream &os, Symbol *s);

int wordlen(Symbol *st);

int numones(SymbolSet ss);

#endif // CWC_SYMBOL_HH
