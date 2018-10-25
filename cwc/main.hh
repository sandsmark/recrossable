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

#ifndef CWC_MAIN_HH
#define CWC_MAIN_HH

#define MAXWORDLEN 32
#define DEFAULT_DICT_FILE "/usr/share/dict/words"

#include <string>

class error {
    std::string errmsg;
public:
    error(const std::string &msg) : errmsg(msg) {}
    const std::string &what() { return errmsg; }
};

struct setup_s {
    typedef enum { simple_format, ascii_format } output_format_t;
    typedef enum { prefixwalker, floodwalker } walker_t;
    typedef enum { btreedict, letterdict } dict_t;
    typedef enum { noformat, generalgrid, squaregrid } gridformat_t;
    output_format_t output_format;
    walker_t walkertype;
    dict_t dictstyle;
    bool verbose, showsteps, showallsteps;
    std::string dictfile, gridfile, svgfile;
    gridformat_t gridformat;
    bool benchdict;
    bool setseed;
    int seed;
    bool debuginfo;
};

extern setup_s setup;

#endif // CWC_MAIN_HH
