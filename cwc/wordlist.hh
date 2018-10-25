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

#ifndef CWC_WORDLIST_HH
#define CWC_WORDLIST_HH

#include <vector>
#include "symbol.hh"

/**
 * the wordlist is a container class for the words loaded from
 * a file. Words are referenced by a integer index. The words
 * are sorted.
 */

class WordList
{
    static const int chunksize = 8192;

public:
    SymbolSet allalpha;
    WordList();
    void load(const std::string &fn);
    void addWord(const std::string &word);
    int numwords() {
        return widx.size();
    }

    Symbol *operator[](int i) {
        return widx[i];
    }

protected:
    std::vector<Symbol*> widx;
    bool wordok(const std::string &st);
    int nwords;

    Symbol *chunk = nullptr;
    int chunkused = 0;
};

#endif // CWC_WORDLIST_HH
