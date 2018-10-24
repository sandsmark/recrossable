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

#include <iostream>

using std::cout;
using std::endl;

#include "symbol.hh"

#include <stdlib.h>

//////////////////////////////////////////////////////////////////////
// class symbol

char symbol::alphabet[32];
symbol symbol::alphindex[256];

int symbol::symballoc = 0;
symbol symbol::outside;
symbol symbol::none;
symbol symbol::empty;

symbol symbol::symbolbit(symbolset ss) {
    symbol s;
    for (symbolset i=1,n=0; i; i<<=1,n++) {
        if (ss&i)
            s.symb = n;
    }
    return s;
}

symbol symbol::alloc(char ch) {
    symbol s;
    if (symballoc >= 32) {
        for (int i=0; i<32;i++) cout << alphabet[i]; cout << endl;
        cout << ch << ' ' << int(ch) << endl;
        throw error("Too many symbols");
    }

    s.symb = symballoc;
    symballoc++;
    alphabet[(int)s.symb] = ch;
    alphindex[(unsigned char)ch].symb = s.symb;
    return s;
}

void symbol::buildindex() {
    for (int i=0;i<32;i++)
        alphabet[i] = UNDEF;
    for (int i=0;i<256;i++)
        alphindex[i].symb = UNDEF;
    symballoc = 0;
    none = symbol::alloc('/');
    empty = symbol::alloc('+');
    outside = symbol::alloc(' ');
}

symbol::symbol(char ch) {
    symb = alphindex[(unsigned char)ch].symb;
    if (symb == UNDEF)
        *this = symbol::alloc(ch);
}

symbolset pickbit(symbolset &ss) {
    int a[32], n = 0;
    for (int i=1; i; i<<=1) {
        if (ss & i)
            a[n++] = i;
    }
    if (n==0) return 0;
    symbolset bit = a[rand()%n];
    ss &= ~bit;
    return bit;
}

int wordlen(symbol *st) {
    int n = 0;
    while (st[n] != symbol::outside) n++;
    return n;
}

ostream &operator <<(ostream &os, symbol *s) {
    while (*s != symbol::outside) {
        os << *s;
        s++;
    }
    return os;
}

int numones(symbolset ss) {
    int n = 0;
    for (int i=1; i; i <<= 1) {
        if (ss&i)
            n++;
    }
    return n;
}

int symbol::numalpha() {
    int n = 0;
    for (int i=0; i<32; i++)
        if (isalpha(alphabet[i]))
            n++;
    return n;
}
