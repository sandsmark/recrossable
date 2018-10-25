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

#include "symbol.hh"

#include <stdlib.h>

//////////////////////////////////////////////////////////////////////
// class symbol

char Symbol::alphabet[32];
Symbol Symbol::alphindex[256];

int Symbol::symballoc = 0;
Symbol Symbol::outside;
Symbol Symbol::none;
Symbol Symbol::empty;

Symbol Symbol::symbolbit(SymbolSet ss) {
    Symbol s;
    for (SymbolSet i=1,n=0; i; i<<=1,n++) {
        if (ss&i)
            s.symb = n;
    }
    return s;
}

Symbol Symbol::alloc(char ch) {
    Symbol s;
    if (symballoc >= 32) {
        for (int i=0; i<32;i++) std::cout << alphabet[i];
        std::cout << std::endl;
        std::cout << ch << ' ' << int(ch) << std::endl;
        throw error("Too many symbols");
    }

    s.symb = symballoc;
    symballoc++;
    alphabet[(int)s.symb] = ch;
    alphindex[(unsigned char)ch].symb = s.symb;
    return s;
}

void Symbol::buildindex() {
    for (int i=0;i<32;i++)
        alphabet[i] = UNDEF;
    for (int i=0;i<256;i++)
        alphindex[i].symb = UNDEF;
    symballoc = 0;
    none = Symbol::alloc('/');
    empty = Symbol::alloc('+');
    outside = Symbol::alloc(' ');
}

Symbol::Symbol(char ch) {
    symb = alphindex[(unsigned char)ch].symb;
    if (symb == UNDEF)
        *this = Symbol::alloc(ch);
}

SymbolSet pickbit(SymbolSet &ss) {
    int a[32], n = 0;
    for (int i=1; i; i<<=1) {
        if (ss & i)
            a[n++] = i;
    }
    if (n==0) return 0;
    SymbolSet bit = a[rand()%n];
    ss &= ~bit;
    return bit;
}

int wordlen(Symbol *st) {
    int n = 0;
    while (st[n] != Symbol::outside) n++;
    return n;
}

std::ostream &operator <<(std::ostream &os, Symbol *s) {
    while (*s != Symbol::outside) {
        os << *s;
        s++;
    }
    return os;
}

int numones(SymbolSet ss) {
    int n = 0;
    for (int i=1; i; i <<= 1) {
        if (ss&i)
            n++;
    }
    return n;
}

int Symbol::numalpha() {
    int n = 0;
    for (int i=0; i<32; i++)
        if (isalpha(alphabet[i]))
            n++;
    return n;
}
