//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef SECURITYKEYS_H_
#define SECURITYKEYS_H_

class SecurityKeys
{
    public:
        typedef struct { std::vector<uint64_t> buf; int len;} key256;
        typedef struct { std::vector<uint64_t> buf; int len;} nonce;
        typedef struct { std::vector<uint64_t> buf; int len;} mic;
        typedef struct { std::vector<uint64_t> buf; int len;} key128;
        typedef struct { std::vector<uint64_t> buf; int len;} key384;
        typedef struct { std::vector<uint64_t> buf; int len;} key64;
        typedef uint64_t  unit64_t_;
};

#endif /* SECURITYKEYS_H_ */
