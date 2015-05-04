/*
==========
Copyright 2002 Energon Software

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
==========
*/


#if !defined SHARED_H
#define SHARED_H


namespace shared {
    /*
     *  constants
     *
     */

    const unsigned short PORT       = 7779;
    const unsigned int   MAX_BUFFER = 2048;

    /*
     *  macros
     *
     */

    // something to make life a bit easier
    #define FOO() do{}while(0)


    /*
     *  typedefs
     *
     */

    typedef unsigned char byte;
    typedef unsigned short word;
}


#endif
