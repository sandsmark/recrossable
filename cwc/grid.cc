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
#include <stdio.h>
#include <sstream>
#include <algorithm>

#include "grid.hh"

using std::ifstream;
using std::ostringstream;
using std::transform;

//////////////////////////////////////////////////////////////////////
// wordblock

wordblock::wordblock() {
    cls_size = 0;
}

void wordblock::getword(symbol *s) {
    int i, len = cls.size();
    for (i = 0; i < len; i++)
        s[i] = cls[i]->getsymbol();
}

//////////////////////////////////////////////////////////////////////
// class cell

cell cell::outside_cell;

cell::cell(symbol s) :
    wbl_size(0), attempts(0), symb(s), preferred(symbol::none), locked(false) {
}

void cell::addword(wordblock *w, int pos) {
    struct wordref wr = {pos, w};
    wbl.push_back(wr);
    wbl_size++;
}

void cell::setsymbol(const symbol &s) {
    if (locked)
        throw error("Attempt to set symbol in locked cell");
    if (!(s == symbol::empty) && !(s == symbol::outside))
        attempts++;
    symb = s;
}

void cell::remove() {
    symb = symbol::outside;
}

void cell::clear(bool setpreferred) {
    if (setpreferred)
        preferred = symb;
    else
        preferred = symbol::none;
    symb = symbol::empty;
}

ostream &operator << (ostream &os, cell &c) {
    return os << c.symb;
}

bool cell::isoutside() {
    return symb == symbol::outside;
}

bool cell::isfilled() {
    return (symb!=symbol::empty)&&(symb!=symbol::outside);
}

string cell::tostring() {
    ostringstream s;
    s << symb;
    return s.str();
}

string cell::touppercasestring() {
    string s = tostring();
    transform(s.begin(), s.end(), s.begin(), toupper);
    return s;
}

//////////////////////////////////////////////////////////////////////
// class coord

coord::coord(int _x, int _y) : x(_x), y(_y) {
}

ostream &operator << (ostream &os, coord &c) {
    return os << c.x << ',' << c.y;
}

//////////////////////////////////////////////////////////////////////
// class grid

grid::grid(int width, int height)
    : cls(0), cls_size(0), verbose(false) {
    init_grid(width, height);
    cellno(-1).setsymbol(symbol::outside);
    buildwords();
}

void grid::init_grid(int w, int h) {
    this->w = w;
    this->h = h;
    cls.clear();
    for (int i = 0; i < w*h; i++)
        cls.push_back(cell());
    cls_size = cls.size();
}

/**
void grid::get_template_h(const coord &c, symbol *s, int &len, int &pos) {
    coord i = c;
    // find first
    while (!cellat(i.x-1, i.y).isoutside())
        i.x--;
    pos = c.x - i.x;
    len = 0;
    while (!cellat(i).isoutside()) {
        s[len] = cellat(i).getsymbol();
        i.x++; len++;
    }
}

void grid::get_template_v(const coord &c, symbol *s, int &len, int &pos) {
    coord i = c;
    // find first
    while (!cellat(i.x, i.y-1).isoutside())
        i.y--;
    pos = c.y - i.y;
    len = 0;
    while (!cellat(i).isoutside()) {
        s[len] = cellat(i).getsymbol();
        i.y++; len++;
    }
}
**/

symbolset cell::findpossible(dict &d) {
    int nwords = numwords();
    if (nwords == 0) throw error("Bugger");

    symbolset ss = ~0;

    for (int i = 0; i < nwords; i++) {

        int pos = getpos(i);
        wordblock &wb = getwordblock(i);
        int len = wb.length();

        symbol word[len+1]; word[len] = symbol::outside;

        wb.getword(word);

        ss &= d.findpossible(word, len, pos); // intersect solutions
        if (setup.verbose) {
            cout << "vertical: "; dumpsymbollist(word, len);
            dumpset(ss);
        }
    }

    return ss;
}


void grid::load_template(const string &filename) {
    ifstream tf(filename.c_str());
    if (!tf.is_open()) throw error("Failed to open pattern file");
    string istr;
    getline(tf, istr);
    w = h = 1;
    sscanf(istr.c_str(), "%d %d", &w, &h);
    if ((w==0)||(h==0))
        throw error("Grid dimensions must be at least 1");
    init_grid(w, h);

    for (int y=0;y<h;y++) {
        getline(tf,istr);
        if (tf.eof()) throw error("Not enough lines in input file");
        // if (istr.length() < unsigned(w+1)) throw error("Line to short in input file");
        for (int x=0;x<w;x++) {
            if (unsigned(x) >= istr.length()) {
                cellat(x, y).remove();
                continue;
            }

            char ch = istr[x];
            if (ch == '+')
                cellat(x, y).clear();
            else if (ch == ' ' )
                cellat(x, y).remove();
            else if (isalpha(ch))
                cellat(x, y).setsymbol(tolower(ch));
            else
                throw error("Invalid character in input file");
        }
    }
    buildwords();
    lock();
}

void grid::load(const string &fn) {
    cls.clear();
    wbl.clear();
    w = h = 0;

    ifstream f(fn.c_str());
    if (!f.is_open()) throw error("Failed to open file");
    string ln;

    while (!f.eof()) {
        getline(f, ln);
        const char *st = ln.c_str();

        wordblock *wb = new wordblock();
        int pos = 0;
        while (*st != '\0') {
            while (*st&&(!isdigit(*st))) st++;
            if (*st == '\0') break;
            int a = atoi(st);
            while (cls.size() <= unsigned(a))
                cls.push_back(cell(symbol::outside));
            cls_size = cls.size();
            cls[a].setsymbol(symbol::empty);
            wb->addcell(a, *this);
            cls[a].addword(wb, pos); pos++;
            while (*st && (isdigit(*st))) st++;
        }
        if (wb->length())
            wbl.push_back(wb);
        else
            delete wb;
    }
    lock();
}

/**
 * builds the words/cell structures when we use a square grid formation
 */

void grid::buildwords() {
    wbl.clear();
    for (int n = 0; n < numcells(); n++)
        cls[n].clearwords();

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int cno = cellnofromxy(x, y);
            wordblock *w = 0;
            int pos = 0;
            while (cellat(x, y).isinside()) {
                if (w==0) { w = new wordblock(); wbl.push_back(w); }
                w->addcell(cno, *this);
                cellno(cno).addword(w, pos);
                pos++;
                x++;
                cno = cellnofromxy(x, y);
            }
        }
    }
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            wordblock *w = 0;
            int pos = 0;
            while (cellat(x, y).isinside()) {
                if (w == 0) { w = new wordblock(); wbl.push_back(w); }
                w->addcell(cellnofromxy(x, y), *this);
                cellat(x, y).addword(w, pos);
                pos++; y++;
            }
        }
    }
}

void grid::dump_ggrid(ostream &os) {
    bool first = true;
    for (vector<wordblock*>::iterator i = wbl.begin(); i != wbl.end(); i++) {
        int wlen = (*i)->length();
        for (int p = 0; p < wlen; p++) {
            if (!first) cout << ' ';
            os << (*i)->getcellno(p);
            first = false;
        }
        os << endl; first = true;
    }
}

char bold[] = "\x1b[1m";
char normal[] = "\x1b[0m";

void grid::dump_ascii(ostream &os,answers * an) {
    /* If we're given answers as well, then print the entries two
       high, possibly with a clue number in the top row. */
    ostringstream vertbarstream;
    vertbarstream << '+';
    for (int i=0;i<w;i++) vertbarstream << "---+";
    string vertbar = vertbarstream.str();

    os << vertbar << endl;
    for (int y=0;y<h;y++) {
        if (an) {
            os << "|";
            for (int x=0; x<w; x++) {
                cell &c = cellat(x,y);
                if (c.isoutside())
                    os << "XXX|";
                else {
                    int cellnumber = y*w + x;
                    if( an->celltoclue.find(cellnumber) != an->celltoclue.end() ) {
                        int cluenumber = an->celltoclue[cellnumber];
                        char cluenumberstring[4];
                        snprintf(cluenumberstring,4,"%-4d",cluenumber);
                        cluenumberstring[3] = 0;
                        os << cluenumberstring << "|";
                    } else
                        os << "   " << "|";
                }
            }
            os << endl;
        }
        // Now draw the rows with the letters:
        os << "|";
        for (int x=0; x<w; x++) {
            cell &c = cellat(x,y);
            if (c.isoutside())
                os << "XXX|";
            else
                os << ' ' <<  c << " |";
        }
        os << endl;
        os << vertbar << endl;
    }
}

void grid::dump_svg(ostream &os, answers * an, float cell_side_mm, bool draw_solutions, bool draw_clue_numbers, float top_left_x_mm, float top_left_y_mm) {

    float scale_defaults = cell_side_mm / 6.5;

    for( int x = 0; x < w; ++x ) {
        for( int y = 0; y < h; ++y ) {
            cell &c = cellat(x,y);
            bool light = ! c.isoutside();
            int cluenumber = -1;
            if (draw_clue_numbers) {
                int cellnumber = y*w + x;
                if( an->celltoclue.find(cellnumber) != an->celltoclue.end() )
                    cluenumber = an->celltoclue[cellnumber];
            }
            draw_cell_svg(os, light, cluenumber, x, y, w, scale_defaults, top_left_x_mm, top_left_y_mm, false, "" );
        }
    }

    if (draw_solutions) {
        for( int x = 0; x < w; ++x ) {
            for( int y = 0; y < h; ++y ) {
                cell &c = cellat(x,y);
                bool light = ! c.isoutside();
                if (!light)
                    continue;
                int cluenumber = -1;
                int cellnumber = y*w + x;
                if( an->celltoclue.find(cellnumber) != an->celltoclue.end() )
                    cluenumber = an->celltoclue[cellnumber];
                draw_cell_svg(os, light, cluenumber, x, y, w, scale_defaults, top_left_x_mm, top_left_y_mm, true, c.touppercasestring() );
            }
        }
    }
}

void grid::draw_cell_svg(ostream &os,
                         bool light,
                         int cluenumber,
                         int gridx,
                         int gridy,
                         int gridwidth,
                         float scale_defaults,
                         float top_left_x_mm,
                         float top_left_y_mm,
                         bool drawletter,
                         const string & letter) {

    float fontsizecluenumber = 4 * scale_defaults;
    float fontsizeletter = 9 * scale_defaults;

    float cell_width = 6.5 * scale_defaults;
    float cell_height = 6.5 * scale_defaults;

    float stroke_width = 0.15 * scale_defaults;

    int cellnumber = gridy * gridwidth + gridx;

    string fillcolour = light ? "#ffffff" : "#b3b3b3";

    float realx = top_left_x_mm + gridx * cell_width;
    float realy = top_left_y_mm + gridy * cell_height;

    float letterrectwidth = 0.823 * cell_width;
    float letterrectheight = 0.76923 * cell_height;

    float letterxoffset = (cell_width - letterrectwidth) / 2;
    float letteryoffset = (2 * (cell_height - letterrectheight)) / 3;

    if (drawletter) {

        os << "    <flowRoot" << endl;
        os << "       xml:space=\"preserve\"" << endl;
        os << "       id=\"letterflow" << cellnumber << "\">";
        os << "<flowRegion" << endl;
        os << "         id=\"letterflowregion" << cellnumber << "\">";
        os << "<rect" << endl;
        os << "           id=\"letterrect" << cellnumber << "\"" << endl;
        os << "           width=\"" << letterrectwidth << "mm\"" << endl;
        os << "           height=\"" << letterrectheight << "mm\"" << endl;
        os << "           x=\"" << (realx + letterxoffset) << "mm\"" << endl;
        os << "           y=\"" << (realy + letteryoffset) << "mm\"/>";
        os << "</flowRegion>";
        os << "<flowPara" << endl;
        os << "         id=\"letterflowpara" << cellnumber << "\"" << endl;
        os << "         style=\"font-size:" << fontsizeletter << "pt;text-align:center;text-anchor:middle\">" << letter << "</flowPara></flowRoot>";
        os << endl;

    } else {

        os << "    <rect" << endl;
        os << "        width=\"" << cell_width << "mm\"" << endl;
        os << "        height=\"" << cell_height << "mm\"" << endl;
        os << "        x=\"" << realx << "mm\"" << endl;
        os << "        y=\"" << realy << "mm\"" << endl;
        os << "        style=\"fill:" << fillcolour << ";fill-opacity:1;stroke:#000000;stroke-width:" << stroke_width <<
            "mm;stroke-miterlimit:4;stroke-dasharray:none;stroke-dashoffset:0;stroke-opacity:1\"" << endl;
        os << "        id=\"cell" << cellnumber << "\"/>" << endl;

        if (light) {
            if (cluenumber>0) {
                // Then also draw the clue number, slightly offset:

                float numberx = realx + (0.3 * scale_defaults);
                float numbery = realy + (1.6 * scale_defaults);

                os << "    <text" << endl;
                os << "       x=\"" << numberx << "mm\"" << endl;
                os << "       y=\"" << numbery << "mm\"" << endl;
                os << "       style=\"font-size:" << fontsizecluenumber << "pt;font-style:normal;font-weight:bold;fill:#000000;fill-opacity:1;stroke:none;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1;font-family:Bitstream Charter\"" << endl;
                os << "       id=\"cluenumberincell" << cellnumber << "\"" << endl;
                os << "       xml:space=\"preserve\"><tspan" << endl;
                os << "         x=\"" << numberx << "mm\"" << endl;
                os << "         y=\"" << numbery << "mm\"" << endl;
                os << "         style=\"font-size:" << fontsizecluenumber << "pt\"" << endl;
                os << "         id=\"cluenumberincell" << cellnumber << "b\">" << cluenumber << "</tspan></text>" << endl;
            }
        }
    }
}

void grid::dump_simple(ostream &os) {
    for (int y=0;y<h;y++) {
        for (int x=0;x<w;x++) {
            os << cellat(x, y);
        }
        os << endl;
    }
}

void grid::dump(ostream &os, setup_s::output_format_t fmt, answers * an) {
    if (w == 0) {
        for (int i = 0; unsigned(i) < wbl.size(); i++) {
            int len = wbl[i]->length();
            symbol *s = new symbol[len + 1];
            s[len] = symbol::outside;
            wbl[i]->getword(s);
            os << s << ' ';
            delete[] s;

            os << '(';
            for (int p = 0; p < len; p++) {
                if (p) os << ',';
                os << wbl[i]->getcellno(p);
            }
            os << ')' << endl;
        }
        return;
    }

    if (fmt == setup.ascii_format)
        dump_ascii(os,an);
    else if (fmt == setup.simple_format)
        dump_simple(os);
}

answers grid::getanswers() {
    answers c;
    set<int> startcells;
    /* Index the start cells, and decide which are across and down
       clues: */
    vector<wordblock*>::iterator i;
    for (i=wbl.begin();
         i!=wbl.end();
         ++i) {
        wordblock * wb = *i;
        int l=wb->length();
        if (l>1) {
            // Then it's a real clue. Guess if it's across or down:
            int firstcell=wb->getcellno(0);
            int secondcell=wb->getcellno(1);
            bool across = (firstcell + 1) == secondcell;
            // Get the answer as a string:
            char * c_str=new char[l+1];
            symbol *symbols = new symbol[l+1];
            symbols[l] = symbol::outside;
            wb->getword(symbols);
            for(int k=0;k<l;++k) {
                c_str[k] = (char)symbols[k];
            }
            c_str[l] = 0;
            string string_version(c_str);
            delete [] symbols;
            delete [] c_str;
            // Add those:
            if (across) {
                c.across.cells.insert(firstcell);
                c.across.celltoanswer[firstcell] = string_version;
            } else {
                c.down.cells.insert(firstcell);
                c.down.celltoanswer[firstcell] = string_version;
            }
            startcells.insert(firstcell);
        }
    }
    /* Number the start cells: */
    int cluenumber=1;
    set<int>::iterator s;
    for(s=startcells.begin();
        s!=startcells.end();
        ++s) {
        c.celltoclue[*s] = cluenumber;
        if( c.across.cells.find(*s) != c.across.cells.end() ) {
            c.across.cluetoanswer[cluenumber] = c.across.celltoanswer[*s];
            c.across.clues.insert(cluenumber);
        }
        if( c.down.cells.find(*s) != c.down.cells.end() ) {
            c.down.cluetoanswer[cluenumber] = c.down.celltoanswer[*s];
            c.down.clues.insert(cluenumber);
        }
        ++ cluenumber;
    }
    return c;
}

float grid::interlockdegree() {
    int interlocked = 0, total = 0;
    for (int y=0; y<h; y++) {
        for (int x=0;x<w;x++) {
            if (cellat(x, y).isinside()) {
                bool vertuse = cellat(x, y-1).isinside() || cellat(x, y+1).isinside();
                bool horizuse = cellat(x-1, y).isinside() || cellat(x+1, y).isinside();
                if (vertuse && horizuse)
                    interlocked++;
                total++;
            }
        }
    }
    return float(interlocked) / float(total);
}

float grid::attemptaverage() {
    int sum = 0, n = 0;
    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            if (cellat(x, y).isinside()) {
                sum += cellat(x, y).getattempts();
                n++;
            }
        }
    }
    return sum / float(n);
}

double grid::dependencydegree(int level) {
    int n = numcells(), ncell = 0;
    int d = 0;
    for (int i = 0; i < n; i++) {
        if (cellno(i).isinside()) {
            d += celldependencies(i, level);
            ncell++;
        }
    }
    return double(d) / double(ncell);
}

int grid::celldependencies(int cno, int level) {
    set<int> cnums;
    cnums.insert(cno);

    while (level--) {
        set<int> newnums;
        for (set<int>::iterator i = cnums.begin(); i != cnums.end(); i++) {
            int clno = *i;
            int words = cellno(clno).numwords();
            for (int w = 0; w < words; w++) {
                wordblock &wb = cellno(clno).getwordblock(w);
                int wlen = wb.length();
                for (int j = 0; j < wlen; j++)
                    newnums.insert(wb.getcellno(j));
            }
        }
        for (set<int>::iterator i = newnums.begin(); i != newnums.end(); i++)
            cnums.insert(*i);
    }
    return cnums.size();
}

void grid::lock() {
    int n = numcells();
    for (int i = 0; i < n; i++)
        if (!cellno(i).isempty())
            cellno(i).lock();
}

int grid::getempty() {
    int n = 0;
    int ncells = numcells();
    for (int i = 0 ; i< ncells; i++)
        if (cellno(i).isempty())
            n++;
    return n;
}

int grid::numopen() {
    int n=0;
    int ncells = numcells();
    for (int i = 0; i < ncells; i++)
        if (!cls[i].islocked())
            n++;
    return n;
}

void cluenumbering::dump(ostream &os) {
    set<int>::iterator a;
    for(a=clues.begin();
        a!=clues.end();
        ++a) {
        int cluenumber=*a;
        string answer=cluetoanswer[cluenumber];
        os << cluenumber << ". " << answer << endl;
    }
}

void cluenumbering::dump_to_svg(ostream &os, const string & id_prefix, const string & font_style_string) {
    set<int>::iterator a;
    for(a=clues.begin();
        a!=clues.end();
        ++a) {
        int cluenumber=*a;
        string answer=cluetoanswer[cluenumber];
        os << "<flowPara id=\"clue-" << id_prefix << "\" ";
        os << "style=\"" << font_style_string << "\">";
        string s(answer);
        transform(s.begin(), s.end(), s.begin(), toupper);
        os << "<flowSpan id=\"clue-number-span-" << id_prefix << "\" ";
        os << " style=\"font-weight:bold;" << font_style_string << "\">";
        os << cluenumber << "</flowSpan>. [" << s << "]</flowPara>";
    }
}

void answers::dump(ostream &os) {
    os << "Across:" << endl;
    across.dump(os);
    os << endl << "Down:" << endl;
    down.dump(os);
}

void answers::dump_to_svg(ostream &os, const string & font_style_string) {
    os << "<flowPara id=\"across-heading\" style=\"font-weight:bold;" << font_style_string << "\">Across:</flowPara>";
    os << "<flowPara id=\"empty-down-spacer\" style=\"" << font_style_string << "\"></flowPara>";
    across.dump_to_svg(os, "across", font_style_string);
    os << "<flowPara id=\"empty-across-spacer-before\" style=\"" << font_style_string << "\"></flowPara>";
    os << "<flowPara id=\"down-heading\" style=\"font-weight:bold;" << font_style_string << "\">Down:</flowPara>";
    os << "<flowPara id=\"empty-across-spacer\" style=\"" << font_style_string << "\"></flowPara>";
    down.dump_to_svg(os, "down", font_style_string);
}
