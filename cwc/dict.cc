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

#include <string>
#include <fstream>
#include <iostream>

#include <string.h>

#include "symbol.hh"
#include "dict.hh"

//////////////////////////////////////////////////////////////////////
// class symbollink

int SymbolLink::instancecount = 0;

SymbolLink::SymbolLink() : symb(Symbol::outside), target(0), next(0) {
    instancecount++;
}

SymbolLink *SymbolLink::addlink(Symbol s) {
    SymbolLink *sl = new SymbolLink();
    sl->symb = s;
    sl->next = target;
    target = sl;
    return target;
}

void SymbolLink::addword(Symbol *str, int n) {
    if (n == 0) return;
    if (!isalpha(str[0]))
        throw error("!!!");
    SymbolLink *sl = getlink(str[0]);
    if (sl == 0)
        sl = addlink(str[0]);
    sl->addword(str+1, n-1);
}

SymbolLink *SymbolLink::getlink(Symbol s) {
    for (SymbolLink *sl = target; sl != 0; sl = sl->next) {
        if (sl->symb == s) return sl;
    }
    return 0;
}

bool SymbolLink::findpossible(Symbol *s, int len,
                              int pos, SymbolSet &ss) {
    if ((target == 0)&&(len==0)) {
        if (pos == 0)
            ss |= symb.getsymbolset();
        return true;
    }
    if ((target==0)||(len==0))
        return false;

    if (s[0] == Symbol::empty) {
        // search each subtree and OR the result.
        bool atallany = false;
        for (SymbolLink *sl = target; sl != 0; sl = sl->next) {
            bool any = sl->findpossible(s+1, len-1, pos-1, ss);
            if (any) {
                atallany = true;
                if (pos==0)
                    ss |= sl->symb.getsymbolset();
            }
        }
        return atallany;
    } else {
        // search specific subtree
        SymbolLink *sl = getlink(s[0]);
        if (sl == 0)
            return false;
        else {
            if (sl->findpossible(s+1, len-1, pos-1, ss)) {
                if (pos == 0)
                    ss |= sl->symb.getsymbolset();
                return true;
            }
            return false;
        }
    }
}

void SymbolLink::dump(char *prefix, int len) {
    if (target == 0) {
        std::cout << prefix << symb << std::endl;
        return;
    }

    if (prefix == 0) {
        prefix = new char[256];
        prefix[0] = '\0';
        for (SymbolLink *sl = target; sl; sl = sl->next) {
            sl->dump(prefix, len);
        }
        delete[] prefix;
    } else {
        prefix[len++] = symb; prefix[len] = '\0';
        for (SymbolLink *sl = target; sl; sl = sl->next) {
            sl->dump(prefix, len);
        }
        prefix[--len] = '\0';
    }
}
//////////////////////////////////////////////////////////////////////
// dict

Dict::Dict() {
}

Dict::~Dict() {
}

//////////////////////////////////////////////////////////////////////
// btree_dict

BtreeDict::BtreeDict() : primary() {
}

void BtreeDict::addword(Symbol *str, int n) {
    primary[n].addword(str, n);
}

int BtreeDict::size() {
    return SymbolLink::instancecount;
}

void BtreeDict::load(const std::string &fn) {
    std::cout << "Loading wordlist and building dictionary... " << std::flush;
    bool chset[256];
    for (int i=0;i<256;i++) chset[i] = false;

    std::ifstream f(fn.c_str());
    if (!f.is_open()) throw error("Failed to open dictionary file");
    char sz[256];
    int wordcount = 0, wordsused = 0;
    while (f.getline(sz, 256), !f.eof()) {
        wordcount++;
        int wlen = strlen(sz);
        if (sz[wlen-1]=='\n')
            sz[--wlen] = '\0';

        bool ok = true;
        for (int i=0;i<wlen;i++) {
            sz[i] = tolower(sz[i]);
            if (!isalpha(sz[i])) {
                ok = false;
            }
        }
        if (ok) {
            Symbol *symbs = new Symbol[wlen];
            for (int i=0;i<wlen;i++) {
                symbs[i] = sz[i];
                chset[(unsigned char)sz[i]] = true;
            }
            addword(symbs, wlen);
            wordsused++;
            delete[] symbs;
        } else {
            // cout << "rejecting " << sz << endl;
        }

    }
    for (int i=0;i<256;i++) {
        if (chset[i]) {
            Symbol s[1];
            s[0] = i;
            addword(s, 1);
        }
    }
    std::cout << "ok" << std::endl;
    std::cout << wordsused << " of " << wordcount << " words used." << std::endl;
}

SymbolSet BtreeDict::findpossible(Symbol *s, int len, int pos) {
    SymbolSet ss = 0;
    primary[len].findpossible(s, len, pos, ss);
    return ss;
}

void BtreeDict::dump(int len) {
    primary[len].dump();
}
