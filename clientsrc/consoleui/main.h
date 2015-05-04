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


#if !defined UI_MAIN_H
#define UI_MAIN_H


#include <string>
using std::string;


// we don't really wanna include the shared header for this...
typedef unsigned char byte;


/*
 *  constants
 *
 */

// print flags
const byte PRINT_NORMAL     = 0x00; // normal message
const byte PRINT_ERROR      = 0x01; // error message
const byte PRINT_CONNECTION = 0x02; // connection related message
const byte PRINT_MESSAGE    = 0x03; // message from a user
const byte PRINT_SERVER     = 0x04; // message from the server


/*
 *  typedefs
 *
 */

// function pointers
typedef void(*COMMLINK)(const string&);
typedef void(*UICONSTRUCT)(const COMMLINK);
typedef void(*UIRUN)(void);
typedef void(*UIPRINT)(const byte, const string&);
typedef const string&(*UIGETHANDLE)(void);
typedef const string&(*UIGETSERVER)(void);
typedef void(*UIQUIT)(void);


/*
 *  prototypes
 *
 */

// these functions are also used by the network layer
extern "C" {
    // constructs the UI
    void construct(const COMMLINK communicationLink);

    // runs the UI
    void run();

    // prints a message from the network layer
    void print(const byte type, const string& message);

    // gets the user's handle
    const string& getHandle();

    // gets the server to connect to
    const string& getServer();

    // tells the ui to quit
    void quit();
}

// sends a message to the network layer
const bool send(const string& message);


#endif
