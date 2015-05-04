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
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "Server.h"
#include "../shared.h"


/*
 *  constant initialization
 *
 */

const string Server::LOGFILE = "server.log";


/*
 *  external globals
 *
 */

extern int errno;


Server::Server() throw(runtime_error) : m_socket(-1), m_maxSocket(-1)
{
    // clear the logfile
    m_log.open(LOGFILE.c_str());
    m_log.close();

    zeroSocketSets(true);

    // open the socket
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(-1 == m_socket) {
        error("Could not create local socket", true);
        throw runtime_error("socket");
    }

    // try to let the socket re-use addresses
    int v = 1;
    if(-1 == setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v)))
        error("Could not set socket to re-use addresses, continuing anyway", true);

    struct sockaddr_in l_addr;
    memset(&l_addr, 0, sizeof(l_addr));
    l_addr.sin_family      = AF_INET;
    l_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    l_addr.sin_port        = htons(shared::PORT);

    // name the socket
    int r = bind(m_socket, (struct sockaddr*)&l_addr, sizeof(l_addr));
    if(-1 == r) {
        error("Could not name local socket", true);
        throw runtime_error("bind");
    }

    // listen on the socket
    r = listen(m_socket, MAX_CONNECTIONS);
    if(-1 == r) {
        error("Could not listen on local socket", true);
        throw runtime_error("listen");
    }

    FD_SET(m_socket, &m_masterSet);
    m_maxSocket = m_socket;
}


Server::~Server() throw()
{
    closeAllConnections();
    if(-1 != m_socket) {
        if(-1 == shutdown(m_socket, 2))
            if(ENOTCONN != errno)
                error("There was an error shutting down the local socket", true);
        if(-1 == close(m_socket))
            error("There was an error shutting down the local socket", true);
    }

    // make sure and close the log
    m_log.close();
}


void Server::run() throw(runtime_error)
{
    while(true) {
        zeroSocketSets();
        m_readSet   = m_masterSet;
        m_exceptSet = m_masterSet;

        // select on the socket
        int r = select(m_maxSocket+1, &m_readSet, NULL, &m_exceptSet, NULL);
        if(-1 == r) {
            error("Could not select on local socket", true);
            throw runtime_error("select");
        } else if(0 == r) {
            usleep(1);
            continue;
        }

        // check for new connections
        int rs = -1;
        if(FD_ISSET(m_socket, &m_readSet))
            rs = acceptConnection();

        // handle current connections
        handleConnections(rs);
        usleep(1);
    }
}


const int Server::acceptConnection()
{
    int rs = -1;
    struct sockaddr_in r_addr;
    socklen_t len = sizeof(r_addr);
    memset(&r_addr, 0, sizeof(r_addr));

    // accept the connection
    print("Accepting new connection... ");
    rs = accept(m_socket, (struct sockaddr*)&r_addr, &len);
    if(-1 == rs) {
        error("failed - accept()", true);
        return -1;
    }

    // get the client's handle
    string handle;
    if(!getHandle(rs, &handle)) {
        sendMessage(rs, "/invalid");
        if(-1 == close(rs))
            error("There was an error closing remote socket", true);
        return -1;
    }
    print(handle + "... ");

    // send the client the list of users
    if(!sendUserList(rs)) {
        error("There was an error sending data to the client", true);
        if(-1 == close(rs))
            error("There was an error closing remote socket", true);
        return -1;
    }

    // make the new socket non-blocking
    if(-1 == fcntl(rs, F_SETFL, O_NONBLOCK)) {
        error("failed - fcntl()", true);
        sendMessage(rs, "/disconnect");
        if(-1 == close(rs))
            error("There was an error closing remote socket", true);
        return -1;
    }

    // find an empty slot and add the connection
    for(unsigned int i=0; i<MAX_CONNECTIONS; i++) {
        if(!m_connections[i].connected()) {
            m_connections[i] = Connection(rs, handle);
            break;
        }
    }

    // save the new socket in the master set and update the max socket
    FD_SET(rs, &m_masterSet);
    if(rs >= m_maxSocket)
        m_maxSocket = rs;
    print("success\n");

    // tell the other clients and send the user a list of connections and return the socket
    broadcast("server: " + handle + " has connected", rs);
    return rs;
}


const bool Server::getHandle(unsigned int socket, string* handle)
{
    if(NULL == handle)
        return false;

    char buffer[shared::MAX_BUFFER];
    memset(buffer, 0, shared::MAX_BUFFER);
    int r = recv(socket, buffer, shared::MAX_BUFFER, 0);
    if(r <= 0) {
        error("failed - recv()", true);
        return false;
    }
    *handle = string(buffer);

    // verify the client's handle
    if(!verifyHandle(*handle)) {
        error("failed - requested handle, " + *handle + ", already in use\n");
        return false;
    }
    return true;
}


const bool Server::verifyHandle(const string& handle)
{
    for(unsigned int i=0; i<MAX_CONNECTIONS; i++)
        if(m_connections[i].getHandle() == handle)
            return false;
    return true;
}


void Server::handleConnections(const int except)
{
    for(unsigned int i=0; i<MAX_CONNECTIONS; i++) {
        if(m_connections[i].connected() && (int)m_connections[i].getSocket() != except) {   // avoid dealing with the currently added connection
            // remove clients in the except set
            if(FD_ISSET(m_connections[i].getSocket(), &m_exceptSet)) {
                error("Client found in except set\n");
                closeConnection(m_connections[i].getSocket());
                continue;
            }

            // read from the read set
            if(FD_ISSET(m_connections[i].getSocket(), &m_readSet))
                readConnection(m_connections[i].getSocket());
        }
    }
}


void Server::readConnection(const unsigned int socket)
{
    char buffer[shared::MAX_BUFFER];
    memset(buffer, 0, shared::MAX_BUFFER);

    // recieve from the connection
    int r = recv(socket, buffer, shared::MAX_BUFFER, 0);
    if(r <= 0) {
        if(EAGAIN == errno)
            return;
        error("Could not recv data from client", true);
        closeConnection(socket);
        return;
    }
    string b(buffer);

    // if we got the disconnect message, we need to clear them out
    if(b == "/disconnect") {
        for(unsigned int i=0; i<MAX_CONNECTIONS; i++) {
            if(m_connections[i].getSocket() == socket) {
print("found it\n");
                b = "server: " + m_connections[i].getHandle() + " has disconnected";
                break;
            }
        }
        closeConnection(socket, false);
    } else
        print(b + "\n");
    broadcast(b, socket);
}


const bool Server::sendMessage(const unsigned int socket, const string& message)
{
    int sent  = 0;
    unsigned int total = 0;
    while(total < message.length()) {
        string sendable = message.substr(total);
        sent = send(socket, sendable.c_str(), sendable.length(), 0);
        if(-1 == sent)
            return false;
        total += sent;
    }
    return true;
}


const bool Server::sendUserList(const unsigned int socket)
{
    string userlist = "userlist: ";
    bool added = false;
    for(unsigned int i=0; i<MAX_CONNECTIONS; i++) {
        if(m_connections[i].connected()) {
            userlist += m_connections[i].getHandle() + ", ";
            added = true;
        }
    }
    if(added)
        userlist.erase(userlist.length()-2);
    else
        userlist += "There are no other users";
    return sendMessage(socket, userlist);
}


void Server::broadcast(const string& message, const unsigned int except)
{
    for(unsigned int i=0; i<MAX_CONNECTIONS; i++) {
        if(m_connections[i].connected() && m_connections[i].getSocket() != except) {
            if(!sendMessage(m_connections[i].getSocket(), message))
                error("Could not send data to client", true);
        }
    }
}


void Server::closeConnection(const unsigned int socket, const bool notify)
{
    // find and close the connection, and also check for the max socket while we're here
    m_maxSocket = m_socket;
    for(unsigned int i=0; i<MAX_CONNECTIONS; i++) {
        if(m_connections[i].getSocket() == socket) {
            print("Closing " + m_connections[i].getHandle() + "'s connection\n");

            // notify the client
            if(notify) {
                if(!sendMessage(socket, "/disconnect"))
                    error("Could not send data to client", true);
            }

            // clear the client's connection
            FD_CLR(socket, &m_masterSet);
            if(-1 == close(socket))
                error("There was an error closing remote socket", true);
            m_connections[i] = Connection();
        } else {
            if((int)m_connections[i].getSocket() > m_maxSocket)
                m_maxSocket = m_connections[i].getSocket();
        }
    }
}


void Server::closeAllConnections() throw()
{
    print("Closing all connections\n");
    for(unsigned int i=0; i<MAX_CONNECTIONS; i++) {
        if(m_connections[i].connected()) {
            // notify the client
            if(!sendMessage(m_connections[i].getSocket(), "/disconnect"))
                error("Could not send data to client", true);

            // clear the client's connection
            FD_CLR(m_connections[i].getSocket(), &m_masterSet);
            if(-1 == close(m_connections[i].getSocket()))
                error("There was an error closing remote socket", true);
            m_connections[i] = Connection();
        }
    }
    m_maxSocket = m_socket;
}


void Server::print(const string& message, const bool doLog)
{
    cout << message << flush;
    if(doLog)
        log(message);
}


void Server::error(const string& message, const bool perr)
{
    // print to the screen
    if(perr)
        perror(message.c_str());
    else
        cerr << message;

    // print to the log
    log(message);
}


void Server::log(const string& message)
{
    // open the log
    m_log.open(LOGFILE.c_str(), ios::app);

    // log only if it opened
    if(m_log.is_open())
        m_log << message;

    // close the log
    m_log.close();
}


void Server::zeroSocketSets(const bool master)
{
    if(master)
        FD_ZERO(&m_masterSet);
    FD_ZERO(&m_readSet);
    FD_ZERO(&m_exceptSet);
}
