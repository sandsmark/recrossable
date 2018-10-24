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

using std::ostream;

typedef unsigned long symbolset;

#define UNDEF 0x7f

class symbol {
    char symb;
    static symbol alphindex[256];
    static char searchchar(char);
    static int symballoc;
public:
    static char alphabet[32];
    static symbol outside, empty, none;
    static symbol alloc(char ch = UNDEF);
    static symbol symbolbit(symbolset); // named constructor
    //  static symbol special();

    symbol() : symb(UNDEF) {}
    symbol(char ch);

    inline symbolset getsymbolset();
    inline operator char();
    inline bool operator == (symbol const &s) const;

    static void buildindex();
    int symbvalue() { return int(symb); }
    static int numalpha();
};

symbolset symbol::getsymbolset() {
    return 1 << symb;
}

bool symbol::operator==(symbol const &s) const {
    return symb == s.symb;
}

symbol::operator char() {
    if (symb == UNDEF) throw error("action on undefined symbol");
    return alphabet[(int)symb];
}

symbolset pickbit(symbolset &ss);

//////////////////////////////////////////////////////////////////////

void dumpset(symbolset ss);
void dumpsymbollist(symbol *s, int n);
ostream &operator <<(ostream &os, symbol *s);

int wordlen(symbol *st);

int numones(symbolset ss);

#endif // CWC_SYMBOL_HH
