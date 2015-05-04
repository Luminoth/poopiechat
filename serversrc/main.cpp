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


#include <cstdio>
#include <stdexcept>
#include <iostream>
#include <string>
using namespace std;

#include <unistd.h>
#include <signal.h>

#include "main.h"
#include "Server.h"


/*
 *  globals
 *
 */

// the server is global so we can destroy it properly
Server* g_server = NULL;


int main(int argc, char* argv[])
{
    cout << endl
        << "**************************************" << endl
        << "*                                    *" << endl
        << "*      poopieserver starting up      *" << endl
        << "* brought to you by Energon Software *" << endl
        << "*  and the great downward spiral...  *" << endl
        << "*                                    *" << endl
        << "**************************************" << endl
         << endl;

    // set signal handlers
    struct sigaction sigAction;
    sigAction.sa_handler = sighandler;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    if(-1 == sigaction(SIGINT, &sigAction, NULL))
        perror("Unable to set SIGINT handler, continuing anyway");
    if(-1 == sigaction(SIGSEGV, &sigAction, NULL))
        perror("Unable to set SIGSEGV handler, continuting anyway");
    if(-1 == sigaction(SIGPIPE, &sigAction, NULL))
        perror("Unable to set SIGPIPE handler, continuing anyway");

    // set the default terminate handler
    set_terminate(defaultTerminate);

    // create the server object
    g_server = new Server();
    if(NULL == g_server) {
        perror("Could not create server object");
        return 1;
    }

    // initialize the network
/*    if(!g_server->initNetwork()) {
        cleanup();
        return 1;
    }*/

    // let the server run
    g_server->run();

/*    // get user input
    string input;
    while(true) {
        g_client->print(g_client->getHandle() + "# ", false);
        getline(cin, input);
        if(input == "exit" || input == "quit") {
            g_client->disconnect();
            break;
        } else if(!input.empty())
            g_client->addToSendQueue(input);
        usleep(1);
    }*/

    // make sure we don't leave a mess
    cleanup();
    return 0;
}


void cleanup()
{
    if(NULL != g_server) {
        delete g_server;
        g_server = NULL;
    }
}


void sighandler(int signum)
{
    switch(signum)
    {
    case SIGINT:
        cout << "Caught SIGINT, exiting" << endl;
        cleanup();
        exit(0);
    case SIGSEGV:
        cerr << "Memory overrun (Segmentation fault) detected, exiting" << endl;
        cleanup();
        exit(1);
    case SIGPIPE:
        cerr << "Broken pipe detected, exiting" << endl;
        cleanup();
        exit(1);
    default:
        cerr << "Unknown signal caught, continuing anyway" << endl;
    };
}


void defaultTerminate()
{
    cerr << "Unhandled exception caught, exiting" << endl;
    cleanup();
    exit(1);
}
