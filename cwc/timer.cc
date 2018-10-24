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

#include <sys/times.h>
#include <time.h>
#include "timer.hh"

timer::timer() : elapsed(0), starttime(0), running(false) {
}

clock_t timer::getptime() {
    struct tms t;
    times(&t);
    return t.tms_utime;
}

void timer::start() {
    if (running)
        return;
    starttime = getptime();
    running = true;
}

void timer::stop() {
    if (!running)
        return;
    clock_t stop_time = getptime();
    elapsed += stop_time - starttime;
    running = false;
}

void timer::reset() {
    elapsed = 0;
    starttime = getptime();
}

clock_t timer::getticks() {
    clock_t t = elapsed;
    if (running)
        t += getptime() - starttime;
    return t;
}

int timer::getmsecs() {
    return (getticks() * 1000) / CLOCKS_PER_SEC;
}
