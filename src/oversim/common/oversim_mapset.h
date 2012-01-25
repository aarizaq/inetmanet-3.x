//
// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

/**
 * @file oversim_mapset.h
 * @author Ingmar Baumgart
 */

#ifndef OVERSIM_MAPSET_H_
#define OVERSIM_MAPSET_H_

#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#define HAVE_GCC_TR1 1
#endif

#if defined(HAVE_GCC_TR1)

#include <tr1/unordered_map>
#include <tr1/unordered_set>

#define UNORDERED_MAP std::tr1::unordered_map
#define UNORDERED_SET std::tr1::unordered_set
#define HASH_NAMESPACE std::tr1

#elif defined(HAVE_MSVC_TR1)

#include <tr1/unordered_map>
#include <tr1/unordered_set>

#define UNORDERED_MAP std::tr1::unordered_map
#define UNORDERED_SET std::tr1::unordered_set
#define HASH_NAMESPACE std::tr1

#elif _MSC_VER

#include <hash_set>
#include <hash_map>

#define UNORDERED_MAP stdext::hash_map
#define UNORDERED_SET stdext::hash_set
#define HASH_NAMESPACE stdext

#else

#include <ext/hash_map>
#include <ext/hash_set>

#define UNORDERED_MAP __gnu_cxx::hash_map
#define UNORDERED_SET __gnu_cxx::hash_set
#define HASH_NAMESPACE __gnu_cxx

//#else // boost
//#include "boost/unordered_map.hpp"
//
//namespace std { namespace tr1 {
//
//  using boost::unordered_map;
//  using boost::unordered_set;
//  using boost::hash;
//
//}}

#endif
#endif
