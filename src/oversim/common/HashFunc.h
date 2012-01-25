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
 * @file
 * @author Stephan Krause
 */

#ifndef __HASHFUNC
#define __HASHFUNC

#include <oversim_mapset.h>
#include <oversim_byteswap.h>

#include <IPvXAddress.h>
#include <TransportAddress.h>

#if defined(HAVE_GCC_TR1) || defined(HAVE_MSVC_TR1)
namespace std { namespace tr1 {
#else
namespace __gnu_cxx {
#endif

/**
 * defines a hash function for IPvXAddress
 */
template<> struct hash<IPvXAddress> : std::unary_function<IPvXAddress, std::size_t>
{
    /**
     * hash function for IPvXaddress
     *
     * @param addr the IPvXAddress to hash
     * @return the hashed IPvXAddress
     */
    std::size_t operator()(const IPvXAddress& addr) const
    {
        if (addr.isIPv6()) {
            return (bswap_32(addr.get6().words()[0])) ^
                   (bswap_32(addr.get6().words()[1])) ^
                   (bswap_32(addr.get6().words()[2])) ^
                   (bswap_32(addr.get6().words()[3]));
        } else {
            return bswap_32(addr.get4().getInt());
        }
    }
};


/**
 * defines a hash function for TransportAddress
 */
template<> struct hash<TransportAddress> : std::unary_function<TransportAddress, std::size_t>
{
    /**
     * hash function for TransportAddress
     *
     * @param addr the TransportAddress to hash
     * @return the hashed TransportAddress
     */
    std::size_t operator()(const TransportAddress& addr) const
    {
        if (addr.getIp().isIPv6()) {
            return (((bswap_32(addr.getIp().get6().words()[0])) ^
                     (bswap_32(addr.getIp().get6().words()[1])) ^
                     (bswap_32(addr.getIp().get6().words()[2])) ^
                     (bswap_32(addr.getIp().get6().words()[3]))) ^
                      addr.getPort());
        } else {
            return ((bswap_32(addr.getIp().get4().getInt())) ^
                    addr.getPort());
        }
    }
};

}
#if defined(HAVE_GCC_TR1) || defined(HAVE_MSVC_TR1)
}
#endif

#endif
