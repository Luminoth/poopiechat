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


#if !defined CLIENT_H
#define CLIENT_H


#include <string>
#include <queue>
#include <fstream>

#include <pthread.h>

#include "../shared.h"
#include "Module.h"
#include "consoleui/main.h"


class Client
{
private:
    /*
     *  private contants
     *
     */

    static const std::string LOGFILE;

public:
    /*
     *  public constructors/destructors
     *
     */

    explicit Client(Module& uiModule);
    ~Client() throw();

public:
    /*
     *  public methods
     *
     */

    // starts the client threads
    const bool startThreads();

    // ends the threads
    const bool endThreads() throw();

    // disconnects from the server
    void disconnect() throw();

    // adds a string to the send queue
    void addToSendQueue(const std::string& message);

    // prints a message to the console (set doLog to false to prevent logging)
    void print(const shared::byte type, const std::string& message, const bool doLog=true);

    // logs a message to the logfile
    void log(const std::string& message);

public:
    /*
     *  inline public methods
     *
     */

    // sets the client's handle
    inline void setHandle(const std::string& handle) { if(!m_handle.empty()) m_handle = handle; }

    // gets the client's handle
    inline const std::string& handle() const { return m_handle; }

    // checks if the client is connected to a server
    inline const bool connected() const { return m_connected; }

private:
    /*
     *  private methods
     *
     */

    const bool sendMessage(const std::string& message);
    void error(const std::string& message, const bool strerr=false);

private:
    /*
     *  private friends
     *
     */

    friend void* sendThread(void* arg);
    friend void* recvThread(void* arg);
    friend void defaultPrintFunc(const shared::byte type, const std::string& message);

private:
    /*
     *  private members
     *
     */

    // connection vars
    int  m_socket;
    bool m_connected;
    std::string m_server;

    // misc. vars
    std::queue<std::string> m_sendQueue;
    std::string m_handle;
    std::ofstream m_log;

    // synchronization vars
    pthread_mutex_t m_sendMutex;    // protects the send queue
    pthread_mutex_t m_logMutex;     // protects output to the log file
    pthread_mutex_t m_printMutex;   // protects output to the screen

    // thread vars
    pthread_t m_sendThread;
    pthread_t m_recvThread;
    bool m_sendStarted;
    bool m_recvStarted;

    // handle to the function used to print messages
    UIPRINT m_printFunc;

private:
    /*
     *  private constructors
     *
     */

    inline Client() {}
};


#endif
