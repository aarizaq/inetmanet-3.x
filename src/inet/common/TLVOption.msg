//
// Copyright (C) 2015 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// @author Zoltan Bojthe
//


cplusplus {{
#include "INETDefs.h"
}}

namespace inet;

class TLVOptionBase extends cObject
{
    short type = -1;
    short length = 0;   // total length of option
}

class TLVOptionRaw extends TLVOptionBase
{
    char bytes[];
}

class TLVOptions extends cObject
{
    TLVOptionBase *tlvOption[] @owned;
}

cplusplus(TLVOptions) {{
  public:
    /*
     * Removes option from optionVector and returns option when removed, otherwise returns nullptr.
     */
    TLVOptionBase *dropTlvOption(TLVOptionBase *option);

    /*
     * Removes all options or first only option where type is the specified type.
     */
    void deleteOptionByType(int type, bool firstOnly = true);

    /*
     * Calculate and returns the total length of all stored options in bytes
     */
    virtual int getLength() const;

    /*
     * Find the first option with specified type. Search started with element at firstPos.
     * Returns the position of found option, or return -1 if not found
     */
    int findByType(short int type, int firstPos=0) const;
}}


