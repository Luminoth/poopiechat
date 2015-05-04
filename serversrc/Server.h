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


#if !defined CSERVER_H
#define CSERVER_H


#include <stdexcept>
#include <string>
#include <fstream>

#include <sys/types.h>


class Server
{
private:
    /*
     *  private inner classes
     *
     */

    class Connection
    {
    public:
        inline Connection() : m_socket(0) {}
        inline Connection(const unsigned int socket, const std::string& handle) : m_socket(socket), m_handle(handle) {}

    public:
        inline const unsigned int getSocket() const { return m_socket; }
        inline const std::string& getHandle() const { return m_handle; }
        inline const bool connected() const { return (m_socket != 0); }

    private:
        unsigned int m_socket;
        std::string m_handle;
    };

private:
    /*
     *  private constants
     *
     */

    enum { MAX_CONNECTIONS=5 };

    // the server logfile
    static const std::string LOGFILE;

public:
    /*
     *  public constructors/destructors
     *
     */

    // this sets up the socket, so don't create globally
    Server() throw(std::runtime_error);

    // this kills the socket, et al.
    virtual ~Server() throw();

public:
    /*
     *  public methods
     *
     */

    // runs the server (accepts connections, talks to clients, whatever)
    void run() throw(std::runtime_error);

private:
    /*
     *  private methods
     *
     */

    // accepts a new connection, returns the new socket or -1 on error
    const int acceptConnection();

    // gets the client's handle, puts the handle into handle param if successful
    const bool getHandle(unsigned int socket, std::string* handle);

    // returns true if the handle isn't already in use
    const bool verifyHandle(const std::string& handle);

    // handles all connected clients (recvs them, etc), excludes except socket
    void handleConnections(const int except);

    // reads data from socket
    void readConnection(const unsigned int socket);

    // sends a message to socket
    const bool sendMessage(const unsigned int socket, const std::string& message);

    // sends the list of users to socket
    const bool sendUserList(const unsigned int socket);

    // broadcasts a message to all connections excluding except
    void broadcast(const std::string& message, const unsigned int except);

    // closes the connection at socket
    void closeConnection(const unsigned int socket, const bool notify=true);

    // closes all the connections
    void closeAllConnections() throw();

    // prints a message to the console (set log to false to prevent logging)
    void print(const std::string& message, const bool doLog=true);

    // prints error messages (set perr to true to use perror() for printing)
    void error(const std::string& message, const bool perr=false);

    // prints a message to the logfile
    void log(const std::string& message);

    // zeros the socket sets, if doMaster is true, the master set is zeroed as well
    void zeroSocketSets(const bool master=false);

private:
    /*
     *  private members
     *
     */

    // socket sets
    fd_set m_masterSet;
    fd_set m_readSet;
    fd_set m_exceptSet;

    // log file
    std::ofstream m_log;

    // connection vars
    int m_socket;
    int m_maxSocket;
    Connection m_connections[MAX_CONNECTIONS];
};


#endif //CSERVER_H
