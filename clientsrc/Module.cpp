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


#include "Module.h"


Module::Module() : m_handle(NULL)
{
}


Module::Module(const std::string& filename) : m_filename(filename), m_handle(NULL)
{
}


Module::~Module() throw()
{
    close();
}


const bool Module::open(const int flags)
{
    if(isOpen())
        return true;

    // open the library
    m_handle = dlopen(m_filename.c_str(), flags);
    if(NULL == m_handle) {
        saveLastError();
        return false;
    }
    return true;
}


const bool Module::open(const std::string& filename, const int flags)
{
    if(m_filename != filename)
        close();

    // save the filename
    m_filename = filename;
    return open(flags);
}


const bool Module::close() throw()
{
    if(!isOpen())
        return true;

    bool noerr = true;
    if(0 != dlclose(m_handle)) {
        saveLastError();
        noerr = false;
    }
    m_handle = NULL;
    return noerr;
}


void* Module::getSymbol(const std::string& symbol) const
{
    return dlsym(m_handle, symbol.c_str());
}


void Module::saveLastError()
{
    m_lastError = dlerror();
}
