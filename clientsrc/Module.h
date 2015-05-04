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


#if !defined MODULE_H
#define MODULE_H


#include <string>

#include <dlfcn.h>


class Module
{
public:
    /*
     *  public constructors/destructors
     *
     */

    Module();
    explicit Module(const string& filename);    // doesn't open the module
    ~Module() throw();

public:
    /*
     *  public methods
     *
     */

    // opens the module
    const bool open(const int flags=RTLD_LAZY);

    // opens the specified module
    const bool open(const std::string& filename, const int flags=RTLD_LAZY);

    // closes the module
    const bool close() throw();

    // gets a symbol from the module
    void* getSymbol(const std::string& symbol) const;

public:
    /*
     *  public inline methods
     *
     */

    // checks if the module is open
    inline const bool isOpen() const { return m_handle != NULL; }

    // returns the last error that occured with a module
    inline const std::string& lastError() const { return m_lastError; }

public:
    /*
     *  public operators
     *
     */

    inline operator bool() const { return isOpen(); }
    inline const bool operator !() const { return !isOpen(); }

private:
    /*
     *  private methods
     *
     */

    void saveLastError();

private:
    /*
     *  private members
     *
     */

    std::string m_filename;
    void* m_handle;
    std::string m_lastError;
};


#endif
