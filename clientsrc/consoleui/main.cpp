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

#include <iostream>

#include "main.h"


/*
 *  globals
 *
 */

COMMLINK g_communicationLink = NULL;
bool     g_quit = false;
string   g_handle;
string   g_server;


void construct(const COMMLINK communicationLink)
{
    g_communicationLink = communicationLink;

    // get the user's handle
    do{
        std::cout << "What is your handle? " << std::flush;
        getline(std::cin, g_handle);
    } while(g_handle.empty());

    // get the server to connect to
    do {
        std::cout << "What server would you like to connect to? " << std::flush;
        getline(std::cin, g_server);
    } while(g_server.empty());
}


void run()
{
    if(NULL == g_communicationLink || g_handle.empty() || g_server.empty()) {
        print(PRINT_ERROR, "run() requires construct() to have been called");
        return;
    }

    // get user input until we're told to quit
    string input;
    while(!g_quit) {
        print(PRINT_NORMAL, g_handle  + "# ");
        getline(std::cin, input);

        // handle special inputs
        if(input == "exit" || input == "quit") {
            print(PRINT_CONNECTION, "Disconnecting");
            g_communicationLink("/disconnect");
            quit();
        } else if(!input.empty())   // only send if there's something to send
            send(input);
    }
}


const bool send(const string& message)
{
    if(NULL == g_communicationLink)
        return false;
    g_communicationLink(message);
    return true;
}


void print(const byte type, const string& message)
{
    unsigned int pos = 0;
    switch(type)
    {
    case PRINT_ERROR:
        std::cout << "\n\033[1;34;40m" << message << "\033[00m\n";
        break;
    case PRINT_CONNECTION:
        std::cout << "\033[1;34;40m" << message << "\033[00m";
        break;
    case PRINT_MESSAGE:
    case PRINT_SERVER:
        pos = message.find(':');
        if(string::npos != pos)
            std::cout << "\n\033[1;32;40m" << message.substr(0, pos) << "\033[00m" << message.substr(pos, message.length()) << "\n";
        else
            std::cout << message;
        break;
    default:
        std::cout << message;
    }
    std::cout << std::flush;
//    std::cout << "\n" << g_handle << "# " << std::flush;
}


const string& getHandle()
{
    return g_handle;
}


const string& getServer()
{
    return g_server;
}


void quit()
{
    g_quit = true;
}
