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


#include "CLArgs.h"


void CLArgs::registerVar(const std::string& name, const std::string& value)
{
    m_argMap[name] = value;
}


const bool CLArgs::peakVar(const std::string& name)
{
    std::map<std::string, std::string>::iterator iterator;
    iterator = m_argMap.find(name);
    if(iterator != m_argMap.end())
        return true;
    return false;
}


const std::string& CLArgs::findVar(const std::string& name) throw(NoSuchVarException)
{
    std::map<std::string, std::string>::iterator iterator;
    iterator = m_argMap.find(name);
    if(iterator != m_argMap.end())
        return iterator->second;
    throw NoSuchVarException();
}


void CLArgs::unregisterVar(const std::string& name)
{
    m_argMap.erase(name);
}
