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
#include <cstring>
#include <iostream>
using namespace std;

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "Client.h"


/*
 *  constant initialization
 *
 */

const string Client::LOGFILE = "client.log";


/*
 *  external globals
 *
 */

extern int errno;


Client::Client(Module& uiModule) : m_socket(-1), m_connected(false), m_sendStarted(false), m_recvStarted(false), m_printFunc(NULL)
{
    // clear the logfile
    m_log.open(LOGFILE.c_str());
    m_log.close();

    // get the UI print function
    m_printFunc = (UIPRINT)uiModule.getSymbol("print");
    if(NULL == m_printFunc) {
        m_printFunc = defaultPrintFunc;
        error("Unable to get print function, using stdout - " + uiModule.lastError() + "\n");
    } else {
        // get the server to connect to from the UI
        UIGETSERVER p_getServer = (UIGETSERVER)uiModule.getSymbol("getServer");
        if(NULL == p_getServer) {
            error("Unable to get getServer function - " + uiModule.lastError() + "\n");
            exit(1);
        }
        m_server = p_getServer();
    }
}


Client::~Client() throw()
{
    endThreads();
    disconnect();
    m_log.close();
}


const bool Client::startThreads()
{
    // initialize our mutexes
    if(0 != pthread_mutex_init(&m_sendMutex, NULL)) {
        cerr << "Could not initialize send queue mutex" << endl;
        return false;
    }
    if(0 != pthread_mutex_init(&m_printMutex, NULL)) {
        cerr << "Could not initialize print mutex" << endl;
        endThreads();
        return false;
    }
    if(0 != pthread_mutex_init(&m_logMutex, NULL)) {
        cerr << "Could not initialize log mutex" << endl;
        endThreads();
        return false;
    }

    // start the send thread
    if(!m_sendStarted) {
        if(0 != pthread_create(&m_sendThread, NULL, sendThread, this)) {
            error("Could not create send thread\n");
            endThreads();
            return false;
        }
    }
    m_sendStarted = true;

    // wait for us to connect to a server (the thread will kill the whole program if it can't connect)
    while(!m_connected)
        usleep(1);

    // start the recv thread
    if(!m_recvStarted) {
        if(0 != pthread_create(&m_recvThread, NULL, recvThread, this)) {
            error("Could not create recv thread\n");
            endThreads();
            return false;
        }
    }
    m_recvStarted = true;
    return true;
}


const bool Client::endThreads() throw()
{
    bool noerr = true;

    // cancel the recv thread and wait for it to end
    if(m_recvStarted) {
        if(0 != pthread_cancel(m_recvThread)) {
            error("Could not cancel recv thread\n");
            noerr = false;
        }
        if(0 != pthread_join(m_recvThread, NULL)) {
            error("Could not join recv thread\n");
            noerr = false;
        }
    }
    m_recvStarted = false;

     // cancel the send thread and wait for it to end
    if(m_sendStarted) {
        if(0 != pthread_cancel(m_sendThread)) {
            error("Could not cancel send thread\n");
            noerr = false;
        }
        if(0 != pthread_join(m_sendThread, NULL)) {
            error("Could not join send thread\n");
            noerr = false;
        }
    }
    m_sendStarted = false;
    m_connected = false;

    // destroy the mutexes
    if(0 != pthread_mutex_destroy(&m_logMutex)) {
        cerr << "Could not destroy log mutex" << endl;
        noerr = false;
    }
    if(0 != pthread_mutex_destroy(&m_printMutex)) {
        cerr << "Could not destroy print mutex" << endl;
        noerr = false;
    }
    if(0 != pthread_mutex_destroy(&m_sendMutex)) {
        cerr << "Could not destroy send mutex" << endl;
        noerr = false;
    }

    return noerr;
}


void Client::disconnect() throw()
{
    // send the disconnect message
    print(PRINT_CONNECTION, "\nDisconnecting\n");
    pthread_mutex_lock(&m_sendMutex);
    sendMessage("/disconnect");
    pthread_mutex_unlock(&m_sendMutex);

    // close the socket
    if(-1 != m_socket) {
        if(-1 == shutdown(m_socket, 2))
            if(ENOTCONN != errno)
                error("There was an error shutting down the socket", true);
        if(-1 == close(m_socket))
            error("There was an error shutting down the socket", true);
    }
}


void Client::addToSendQueue(const string& message)
{
    pthread_mutex_lock(&m_sendMutex);
    m_sendQueue.push(message);
    pthread_mutex_unlock(&m_sendMutex);
}


const bool Client::sendMessage(const string& message)
{
    int sent  = 0;
    unsigned int total = 0;
    while(total < message.length()) {
        string sendable = message.substr(total);
        sent = send(m_socket, sendable.c_str(), sendable.length(), 0);
        if(-1 == sent)
            return false;
        total += sent;
    }
    return true;
}


void Client::print(const byte type, const string& message, const bool doLog)
{
    // print to the screen
    pthread_mutex_lock(&m_printMutex);
    m_printFunc(type, message);
    pthread_mutex_unlock(&m_printMutex);

    if(doLog)
        log(message);
}


void Client::log(const string& message)
{
    pthread_mutex_lock(&m_logMutex);
    // open the log
    m_log.open(LOGFILE.c_str(), ios::app);

    // log only if it opened
    if(m_log.is_open())
        m_log << message;

    // close the log
    m_log.close();
    pthread_mutex_unlock(&m_logMutex);
}


void Client::error(const string& message, const bool strerr)
{
    string& t = (string&)message;

    // print the error
    pthread_mutex_lock(&m_printMutex);
    if(strerr)
        t += ": " + string(strerror(errno)) + "\n";
    m_printFunc(PRINT_ERROR, t);
    pthread_mutex_unlock(&m_printMutex);

    // print to the log
    log("Error - " + t);
}


void defaultPrintFunc(const byte type, const string& message)
{
    unsigned int pos = 0;
    switch(type)
    {
    case PRINT_ERROR:
        cout << "\n\033[1;34;40m" << message << "\033[00m\n";
        break;
    case PRINT_CONNECTION:
        cout << "\033[1;34;40m" << message << "\033[00m";
        break;
    case PRINT_MESSAGE:
    case PRINT_SERVER:
        pos = message.find(':');
        if(string::npos != pos)
            cout << "\n\033[1;32;40m" << message.substr(0, pos) << "\033[00m" << message.substr(pos, message.length()) << "\n";
        else
            cout << message;
        break;
    default:
        cout << message;
    }
    cout << flush;
}


void* sendThread(void* arg)
{
    // extract the client
    if(NULL == arg) {
        cerr << "\033[1;34;40msendThread() requires a Client argument!\033[00m" << endl;
        exit(1);
    }
    Client* t = (Client*)arg;

    // make sure we enable thread cancelling and defer it
    if(0 != pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)) {
        t->error("Could not set cancel state\n");
        exit(1);
    }
    if(0 != pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL)) {
        t->error("Could not set cancel type\n");
        exit(1);
    }

    t->log("Attempting connection to server at " + t->m_server + "\n");

    // setup the socket address
    hostent* h = NULL;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(shared::PORT);

    // resolve the hostname/ip
    h = gethostbyname(t->m_server.c_str());
    if(NULL == h) {
        t->error("Could not get host by name", true);
        exit(1);
    }
    addr.sin_addr = *((in_addr*)h->h_addr_list[0]);

    // create the socket
    t->m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(-1 == t->m_socket) {
        t->error("Could not create socket", true);
        exit(1);
    }

    // connect to the server
    t->print(PRINT_NORMAL, "Connecting... please wait... ");
    int r = connect(t->m_socket, (struct sockaddr*)&addr, sizeof(struct sockaddr));
    if(-1 == r) {
        t->error("failed", true);
        exit(1);
    }

    // send the server our handle
    if(!t->sendMessage(t->m_handle)) {
        t->error("Could not send handle", true);
        exit(1);
    }

    // wait for the server's reply
    char buffer[shared::MAX_BUFFER];
    memset(buffer, 0, shared::MAX_BUFFER);
    r = recv(t->m_socket, buffer, shared::MAX_BUFFER, 0);
    if(r <= 0) {
        t->error("Could not recv data from server", true);
        exit(1);
    }

    // see if we've been accepted
    if(0 == strcmp("/invalid", buffer)) {
        t->error("failed - handle already in use\n");
        exit(1);
    } else if(0 == strcmp("/disconnect", buffer)) {
        t->error("failed - server is full\n");
        exit(1);
    }

    // make the socket non-blocking
    if(-1 == fcntl(t->m_socket, F_SETFL, O_NONBLOCK)) {
        t->error("Could not set socket to be non-blocking", true);
        exit(1);
    }
    t->print(PRINT_NORMAL, "success\n\n\033[1;32;40m" + string(buffer) + "\033[00m\n\n");
    t->m_connected = true;

    while(true) {
        // it's okay to exit now
        pthread_testcancel();

        pthread_mutex_lock(&t->m_sendMutex);
        if(!t->m_sendQueue.empty()) {
            // get the top of the queue
            string toSend = t->m_sendQueue.front();
            t->m_sendQueue.pop();
            pthread_mutex_unlock(&t->m_sendMutex);

            // send the string to the server
            if(!t->sendMessage(t->m_handle + ": " + toSend)) {
                t->error("Could not send data to server", true);
                exit(1);
            }
        } else
            pthread_mutex_unlock(&t->m_sendMutex);
        usleep(1);
    }
    pthread_exit(0);
}


void* recvThread(void* arg)
{
    // extract the client
    if(NULL == arg) {
        cerr << "\033[1;34;40mrecvThread() requires a Client argument!\033[00m" << endl;
        exit(1);
    }
    Client* t = (Client*)arg;

    char buffer[shared::MAX_BUFFER];
    while(true) {
        // it's okay to exit now
        pthread_testcancel();

        memset(buffer, 0, shared::MAX_BUFFER);
        int r = recv(t->m_socket, buffer, shared::MAX_BUFFER, 0);
        if(r <= 0) {
            if(EAGAIN == errno) {
                usleep(1);
                continue;
            } else {
                t->error("Could not recv data from server", true);
                exit(1);
            }
        }
        if(0 == strcmp("/disconnect", buffer)) {
            t->print(PRINT_CONNECTION, "\nServer disconnected\n");
            exit(1);
        }

        // output what we've recieved
        t->print(PRINT_MESSAGE, "\n" + string(buffer) + "\n");
        usleep(1);
    }
    pthread_exit(0);
}
