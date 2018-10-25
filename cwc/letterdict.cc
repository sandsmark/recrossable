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
#include <algorithm>
#include <iostream>

#include "letterdict.hh"

/*
                         1   2   3   4   5
   word length         +---+---+---+---+--
                       |   |   | O |   |
                       +---+---+-+-+---+---
                                 |
          position               V 1   2   3   4
                                 +---+---+---+---+--
                                 |   | O |   |   |
                                 +---+-|-+---+---+---
                                       |
                                       V a   b   c   d   e
                                       +---+---+---+---+--
                                       |   | O |   |   |
                                       +---+-|-+---+---+---

*/


//////////////////////////////////////////////////////////////////////
// letterdict

LetterDict::LetterDict() : p(0), all(0) {
}

template<class T>
T **newptrarray(int n) {
    T **p = new T*[n];
    for (int i=0;i<n;i++) p[i] = 0;
    return p;
}

void LetterDict::addword(Symbol *st, int wordi) {
    if (p == 0)
        p = newptrarray<intvec**>(MAXWORDLEN);
    if (all == 0)
        all = newptrarray<SymbolSet>(MAXWORDLEN);

    int wlen = wordlen(st);
    if (p[wlen] == 0)
        p[wlen] = newptrarray<intvec*>(wlen);

    if (all[wlen] == 0) {
        all[wlen] = new SymbolSet[wlen];
        for (int i=0; i<wlen; i++) all[wlen][i] = 0;
    }

    // for each position in the word
    for (int pos=0; pos<wlen; pos++) {
        if (p[wlen][pos] == 0)
            p[wlen][pos] = newptrarray<intvec>(32);
        int chval = st[pos].symbvalue();
        if (p[wlen][pos][chval] == 0)
            p[wlen][pos][chval] = new intvec;
        p[wlen][pos][chval]->push_back(wordi);

        all[wlen][pos] |= st[pos].getsymbolset();

    } // pointer hell :-)
}

LetterDict::intvec LetterDict::emptyvec;

LetterDict::intvec *LetterDict::getintvec(int len, int pos, Symbol s) {
    if (p[len] == 0) return &emptyvec;
    if (p[len][pos] == 0) return &emptyvec;
    int chval = s.symbvalue();
    if (p[len][pos][chval] == 0) return &emptyvec;
    return p[len][pos][chval];
}

SymbolSet LetterDict::findpossible(Symbol *s, int len, int pos) {
    if (len == 1) return wl->allalpha;

    intvec *chpset[len];
    int nsets = 0;

    for (int i=0;i<len;i++)
        if (s[i] != Symbol::empty)
            chpset[nsets++] = getintvec(len, i, s[i]);

    // cout << nsets << " sets\n";
    if (nsets == 0) {
        if (all[len] == 0)
            return 0;
        // dumpset(all[len][pos]);
        return all[len][pos];
    }

    SymbolSet ss = 0;

    intvec::iterator it[nsets];
    for (int i=0;i<nsets; i++) {
        it[i] = chpset[i]->begin();
        if (it[i] == chpset[i]->end())
            goto done;
    }

    // find intersection among sets

    while (1) {
        // check if all iterators point to same word
        bool allequal = true;
        for (int i = 0; i < nsets-1; i++)
            allequal &= (*it[i] == *it[i+1]);
        if (allequal) {
            // for (int i=0; i<nsets; i++)
            // cout << (*wl)[*it[i]] << ' ';
            // cout << endl;
            int wnum = *it[0];
            ss |= (*wl)[wnum][pos].getsymbolset();

            for (int i=0;i<nsets;i++) {
                it[i]++;
                if (it[i] == chpset[i]->end())
                    goto done;
            }

        } else {

            // increment the smallest iterator
            int sm = 0;
            for (int i = 1; i < nsets; i++)
                if (*it[i] < *it[sm])
                    sm = i;
            it[sm]++;
            if (it[sm] == chpset[sm]->end())
                break;
        }
    }
  done:
    // dumpset(ss);

    return ss;
}

void LetterDict::load(const std::string &fn) {
    std::cout << "Loading wordlist and building dictionary... " << std::flush;

    wl = new WordList();
    wl->load(fn);

    int nwords = wl->numwords();
    for (int i=0; i<nwords; i++)
        addword((*wl)[i], i);

    std::cout << "ok" << std::endl;
}
