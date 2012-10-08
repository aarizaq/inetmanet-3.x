//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef SCTPSEQNUMBERS_H
#define SCTPSEQNUMBERS_H

#include <omnetpp.h>


/**
* Compare TSNs
*/
inline bool tsnLt (const uint32 tsn1, const uint32 tsn2) {
   return ((int32)(tsn1-tsn2) < 0);
}

inline bool tsnLe (const uint32 tsn1, const uint32 tsn2) {
   return ((int32)(tsn1-tsn2) <= 0);
}

inline bool tsnGe (const uint32 tsn1, const uint32 tsn2) {
   return ((int32)(tsn1-tsn2) >= 0);
}

inline bool tsnGt (const uint32 tsn1, const uint32 tsn2) {
   return ((int32)(tsn1-tsn2) > 0);
}

inline bool tsnBetween (const uint32 tsn1, const uint32 midtsn, const uint32 tsn2) {
   return ((tsn2-tsn1) >= (midtsn-tsn1));
}


/**
* Compare SSNs
*/
inline bool ssnGt (const uint16 ssn1, const uint16 ssn2) {
   return ((int16)(ssn1-ssn2) > 0);
}

#endif
