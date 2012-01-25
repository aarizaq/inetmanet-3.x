//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file StringConvert.h
 * @author Stephan Krause
 */

#include <cexception.h>
#include <iostream>
#include <sstream>
#include <string>

template <class t> t convertString( const std::string& str) {
    std::istringstream s(str);
    t buf;
    if (!(s >> buf)) {
        throw( cException("convertString: parsing of string %s failed", str.c_str()) );
    }
    return buf;
}

template <class t> std::string convertToString( const t& val) {
    std::ostringstream s;
    if (!(s << val)) {
        throw( cException("convertToString: cannot convert value to string") );
    }
    return s.str();
}

