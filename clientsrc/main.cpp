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
#include "Module.h"
#include "Client.h"
#include "consoleui/main.h"
#include "CLArgs.h"


/*
 *  external globals
 *
 */

extern char* optarg;


/*
 *  globals
 *
 */

Client* g_client = NULL;
UIQUIT  g_uiQuit = NULL;
CLArgs g_clArgs;


void communicationLink(const string& message)
{
    // add the message to the client send queue
    if(NULL != g_client)
        g_client->addToSendQueue(message);
}


inline void printUsage(const string& prog)
{
    cout << "Usage: " << prog << " -u ui" << endl;
}


int main(int argc, char* argv[])
{
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

    // process command line arguments
    if(!registerArguments(argc, argv)) {
        printUsage(argv[0]);
        cleanup();
        return 1;
    }

    // make sure we have a UI library to use
    if(!g_clArgs.peakVar("ui")) {
        printUsage(argv[0]);
        cleanup();
        return 1;
    }

    // open the UI library
    Module uiModule(g_clArgs.findVar("ui"));
    if(!uiModule.open()) {
        cerr << "Unable to open UI library, " << g_clArgs.findVar("ui") << " - " << uiModule.lastError() << endl;
        cleanup();
        return 1;
    }

    // create the UI
    UICONSTRUCT p_construct = NULL;
    p_construct = (UICONSTRUCT)uiModule.getSymbol("construct");
    if(NULL == p_construct) {
        cerr << "Unable to get UI construct function - " << uiModule.lastError() << endl;
        cleanup();
        return 1;
    }
    p_construct(communicationLink);

    // get the user's handle from the UI
    UIGETHANDLE p_getHandle = NULL;
    p_getHandle = (UIGETHANDLE)uiModule.getSymbol("getHandle");
    if(NULL == p_getHandle) {
        cerr << "Unable to get UI getHandle function - " << uiModule.lastError() << endl;
        cleanup();
        return 1;
    }
    string handle = p_getHandle();

    // create the client network interface
    g_client = new Client(uiModule);
    if(NULL == g_client) {
        cerr << "Could not create client object" << endl;
        cleanup();
        return 1;
    }
    g_client->setHandle(handle);

    // start the client threads
    if(!g_client->startThreads()){
        cleanup();
        return 1;
    }

    // get the UI run function
    UIRUN p_run = NULL;
    p_run = (UIRUN)uiModule.getSymbol("run");
    if(NULL == p_run) {
        cerr << "Unable to get the UI run function - " << uiModule.lastError() << endl;
        cleanup();
        return 1;
    }

    // get the UI quit function
    g_uiQuit = (UIQUIT)uiModule.getSymbol("quit");
    if(NULL == g_uiQuit) {
        cerr << "Unable to get the UI quit function - " << uiModule.lastError() << endl;
        cleanup();
        return 1;
    }

    // start the UI
    p_run();

    // make sure we don't leave a mess
    cleanup();
    return 0;
}


void cleanup()
{
    // kill the client network interface
    if(NULL != g_client) {
        g_client->endThreads();
        delete g_client;
    }
    g_client = NULL;

    // make sure the UI quits
    if(NULL != g_uiQuit)
        g_uiQuit();
}


const bool registerArguments(const int argc, char* const argv[])
{
    int opt = getopt(argc, argv, "u:");
    while(opt != -1) {
        switch(opt)
        {
        case 'u':
            g_clArgs.registerVar("ui", optarg);
            break;
        case ':':
        case '?':
        default:
            return false;
        }
        opt = getopt(argc, argv, "u:");
    }
    return true;
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
