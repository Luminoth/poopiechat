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


#if !defined CLARGS_H
#define CLARGS_H


#include <stdexcept>
#include <map>
#include <string>


class CLArgs
{
public:
    /*
     *  public exceptions
     *
     */

    class NoSuchVarException : public exception
    {
    public:
        NoSuchVarException(const std::string& what="Variable does not exist") : _what(what) {}
        virtual const char* what() const { return _what.c_str(); }
    private:
        std::string _what;
    };

public:
    /*
     *  public constructors/destructors
     *
     */

    CLArgs() {}

public:
    /*
     *  public methods
     *
     */

    // registers a variable based on its name and its value
    void registerVar(const std::string& name, const std::string& value);

    // checks to see if a variable exists
    const bool peakVar(const std::string& name);

    // finds the variable with name and returns its value
    const std::string& findVar(const std::string& name) throw(NoSuchVarException);

    // unregisters the variable with name
    void unregisterVar(const std::string& name);

private:
    /*
     *  private members
     *
     */

    std::map<std::string, std::string> m_argMap;
};


#endif
