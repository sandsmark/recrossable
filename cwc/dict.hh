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

//////////////////////////////////////////////////////////////////////

struct symbollink {
    symbol symb;
    static int instancecount;
    symbollink *target, *next;
    symbollink *getlink(symbol);
    symbollink();
    symbollink *addlink(symbol);
    void addword(symbol *, int);
    bool findpossible(symbol *s, int len, int pos, symbolset &ss);
    void dump(char *prefix = 0, int len = 0);
};

class dict {
public:
    dict();
    virtual ~dict();

    virtual void load(const string &fn) = 0;
    virtual symbolset findpossible(symbol *s, int len, int pos) = 0;
};

class btree_dict : public dict {
    symbollink primary[MAXWORDLEN];
public:
    btree_dict();
    void addword(symbol *, int);
    void load(const string &fn);
    int size();
    symbolset findpossible(symbol *s, int len, int pos);
    void dump(int len);
};

#endif // CWC_DICT_HH

