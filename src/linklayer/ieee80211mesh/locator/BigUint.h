/**
 * Copyright (c) 2012 Alfonso Ariza
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the author be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * The origin of this software must not be misrepresented; you must not claim
 * that you wrote the original software. If you use this software in a
 * product, an acknowledgment in the product documentation would be
 * appreciated but is not required. Altered source versions must be plainly
 * marked as such, and must not be misrepresented as being the original
 * software. This notice may not be removed or altered from any source
 * distribution.
 */

#ifndef __INET_BIGUINT_H
#define __INET_BIGUINT_H

#include <vector>
#include <INETDefs.h>


#define DEFAULTSIZE 4

class BigUint
{
    // Binary correct representation of signed 128bit integer
  private:
    std::vector<uint64_t> value;
    unsigned int defaultSize;

    //inline BigUint (const unsigned __int64 & a, const unsigned __int64 & b)
//            : lo (a), hi (b) {}
    inline BigUint(const uint64_t *a, unsigned int size)
    {
        BigUint(a,size,DEFAULTSIZE);
    }

    inline BigUint(std::vector<uint64_t> v)
    {
        value = v;
        defaultSize = value.size();
    }

    inline BigUint(const uint64_t *a, unsigned int size,unsigned int defaultS)
    {
        value.clear();
        if (size>defaultS)
            size = defaultS;
        defaultSize = defaultS;
        for (unsigned int i = 0; i<size;i++)
            value.push_back(a[i]);
        for (unsigned int i = size; i<defaultSize;i++)
            value.push_back(0);
    }

    inline void clear()
    {
        std::fill(value.begin(), value.end(), 0);
    }
    inline void resizeTo(const BigUint &b)
    {
        if (b.value.size()>value.size())
        {
            for (unsigned int i = defaultSize;i<b.value.size();i++)
                value.push_back(0);
            defaultSize = b.defaultSize;
        }
    }

  protected:
    // Some global operator functions must be friends
    friend bool operator<(const BigUint&, const BigUint&);
    //friend bool operator >  (const BigUint&, const BigUint&);
    friend bool operator==(const BigUint&, const uint32_t&);
    friend bool operator==(const BigUint&, const int32_t&);
    friend bool operator==(const BigUint&, const uint64_t&);
    friend bool operator==(const BigUint&, const int64_t&);

    friend bool operator==(const uint32_t&, const BigUint&);
    friend bool operator==(const int32_t&, const BigUint&);
    friend bool operator==(const uint64_t&, const BigUint&);
    friend bool operator==(const int64_t&, const BigUint&);

    friend bool operator==(const BigUint&, const BigUint&);
    friend bool operator||(const BigUint&, const BigUint&);
    friend bool operator&&(const BigUint&, const BigUint&);

    friend bool operator!=(const BigUint&, const uint32_t&);
    friend bool operator!=(const BigUint&, const int32_t&);
    friend bool operator!=(const BigUint&, const uint64_t&);
    friend bool operator!=(const BigUint&, const int64_t&);

    friend bool operator!=(const uint32_t&, const BigUint&);
    friend bool operator!=(const int32_t&, const BigUint&);
    friend bool operator!=(const uint64_t&, const BigUint&);
    friend bool operator!=(const int64_t&, const BigUint&);
    friend std::ostream& operator<<(std::ostream& os, const BigUint& );

#ifdef __GNUC__
//            friend BigUint operator <? (const BigUint&, const BigUint&);
//            friend BigUint operator >? (const BigUint&, const BigUint&);
#endif
  public:
    void set( const char *sz);

    // Constructors
    BigUint()
    {
        defaultSize = DEFAULTSIZE;
        value.resize(defaultSize);
        clear();
    }

    BigUint(const BigUint& a) : value(a.value), defaultSize(a.defaultSize) {}

    // Note: int / unsigned int operators conflict with other integer types with at least GCC and MSVC
    // inline BigUint (const unsigned int & a) : lo (a), hi (0ull) {}
    // inline BigUint (const int & a) : lo (a), hi (0ull) {}

    //   inline BigUint (const unsigned __int64 & a) : lo (a), hi (0ull) {}
    BigUint(const int32_t& a)
    {
        defaultSize = DEFAULTSIZE;
        value.resize(defaultSize);
        clear();
        value[0] = a;
    }

    BigUint(const uint32_t& a)
    {
        defaultSize = DEFAULTSIZE;
        value.resize(defaultSize);
        clear();
        value[0] = a;
    }

    BigUint(const int64_t& a)
    {
        defaultSize = DEFAULTSIZE;
        value.resize(defaultSize);
        clear();
        value[0] = a;
    }

    BigUint(const uint64_t& a)
    {
        defaultSize = DEFAULTSIZE;
        value.resize(defaultSize);
        clear();
        value[0] = a;
    }

    BigUint(const float a);
    BigUint(const double& a);
    BigUint(const long double& a);

    BigUint(const char *sz) {set(sz);}

    // TODO: Consider creation of operator= to eliminate
    //       the need of intermediate objects during assignments.
    BigUint& operator=(const BigUint& other)
    {
        value = other.value; defaultSize = other.defaultSize; return *this;
    }

    // Note: int / unsigned int operators conflict with other integer types with at least GCC and MSVC
    // BigUint& operator = (const int &a) {lo = a; hi = 0; return *this;}
    // BigUint& operator = (const unsigned int &a) {lo = a; hi = 0; return *this;}

    BigUint& operator=(const int32_t& a)
    {
        if (value.empty())
        {
            defaultSize = DEFAULTSIZE;
            value.resize(defaultSize);
        }
        if (value.size() != defaultSize)
            value.resize(defaultSize);
        clear();
        value[0] = a;
        return *this;
    }
    BigUint& operator=(const uint32_t& a)
    {
        if (value.empty())
        {
            defaultSize = DEFAULTSIZE;
            value.resize(defaultSize);
        }
        if (value.size() != defaultSize)
            value.resize(defaultSize);
        clear();
        value[0] = a;
        return *this;
    }
    BigUint& operator=(const int64_t& a)
    {
        if (value.empty())
        {
            defaultSize = DEFAULTSIZE;
            value.resize(defaultSize);
        }
        if (value.size() != defaultSize)
            value.resize(defaultSize);
        clear();
        value[0] = a;
        return *this;
    }
    BigUint& operator=(const uint64_t& a)
    {
        if (value.empty())
        {
            defaultSize = DEFAULTSIZE;
            value.resize(defaultSize);
        }
        if (value.size() != defaultSize)
            value.resize(defaultSize);
        clear();
        value[0] = a;
        return *this;
    }

    BigUint& operator=( const char *sz) {set(sz); return *this;}
    BigUint& operator=(const float& a);
    BigUint& operator=(const double& a);
    BigUint& operator=(const long double& a);

    // Operators
    bool operator!() const
    {
        bool v = value[0];
        for (unsigned int i = 1;i<value.size();i++)
            v = v || value[i];
        return !v;
    }
    BigUint operator-() const;
    BigUint operator ~ () const;
    BigUint& operator++();
    BigUint& operator--();
    BigUint operator++(int);
    BigUint operator--(int);

    BigUint& operator+=(const BigUint& b);
    BigUint& operator*=(const BigUint& b);

    BigUint& operator>>=(unsigned int n);
    BigUint& operator<<=(unsigned int n);
    BigUint& operator|=(const BigUint& b)
    {
        resizeTo(b);
        for (unsigned int i = 0;i<b.value.size();i++)
            value[i] |= b.value[i];
        return *this;
    }


    BigUint& operator&=(const BigUint& b)
    {
        resizeTo(b);
        for (unsigned int i = 0;i<b.value.size();i++)
            value[i] &= b.value[i];
        return *this;
    }

    BigUint& operator^=(const BigUint& b)
    {
        if (b.value.size()>value.size())
        {

            for (unsigned int i = defaultSize;i<b.value.size();i++)
                value.push_back(0);
            defaultSize = DEFAULTSIZE;
        }
        for (unsigned int i = 0;i<b.value.size();i++)
            value[i] ^= b.value[i];
        return *this;
    }


    // Inline simple operators
    inline const BigUint& operator+() const { return *this; }

    // Rest of inline operators
    inline BigUint& operator-=(const BigUint& b)
    {
        return *this += (-b);
    }
    inline BigUint& operator/=(const BigUint& b)
    {
        BigUint dummy;
        *this = this->div(b, dummy);
        return *this;
    }
    inline BigUint& operator%=(const BigUint& b)
    {
        this->div(b, *this);
        return *this;
    }

    // Common methods
    unsigned int toUint() const {return (unsigned int) value[0];}
    uint64_t toUint64() const {return value[0];}

    const char *toString(unsigned int radix = 10) const;
    float toFloat() const;
    double toDouble() const;
    long double toLongDouble() const;

    operator double() const {return toDouble(); }
    operator int() const {return toUint();}
    operator uint32_t() const { return toUint();}
    operator uint64_t() const {return toUint64();}
    operator int64_t() const {return toUint64();}

    operator bool()
    {
        for (unsigned int i = 0 ; i < value.size();i++)
            if (value[i]!=0) return true;
        return true;
    }


    // Arithmetic methods
    BigUint  div(const BigUint&, BigUint&) const;
    // Bit operations
    bool    bit(unsigned int n) const;
    void    bit(unsigned int n, bool val);
    char    * getValChar() {return (char *) &value[0]; }
    uint64_t * getVal() {return &value[0];}

    void resizeBits(unsigned int bits);

    static const BigUint BigUint_MAX;
    static const BigUint BigUint_MIN;
};

// GLOBAL OPERATORS


inline bool operator==(const BigUint& a, const BigUint& b)
{
    if (a.value.size() != b.value.size())
        throw cRuntimeError("BigUint == operator, number have different size \n");
    return  a.value ==b.value;
}


inline bool operator||(const BigUint& a, const BigUint& b)
{
    if (a.value.size() != b.value.size())
        throw cRuntimeError("BigUint || operator, number have different size \n");
    for (unsigned int i = 0 ; i < a.value.size(); i++)
        if (a.value[i] || b.value[i]) return true;
    return false;
}

inline bool operator&&(const BigUint& a, const BigUint& b)
{
    if (a.value.size() != b.value.size())
        throw cRuntimeError("BigUint || operator, number have different size \n");
    bool isNotZero1 = false;
    bool isNotZero2 = false;
    for (unsigned int i = 0 ; i < a.value.size(); i++)
    {
        if (a.value[i] != 0) isNotZero1 = true;
        if (b.value[i] != 0) isNotZero2 = true;
    }
    if (isNotZero1 && isNotZero2) return true;
    return true;
}


#ifdef __GNUC__
//    inline BigUint operator <? (const BigUint& a, const BigUint& b) {
//        return (a < b) ? a : b; }
//    inline BigUint operator >? (const BigUint& a, const BigUint& b) {
//        return (a < b) ? b : a; }
#endif

// GLOBAL OPERATOR INLINES

inline bool operator<(const BigUint& a, const BigUint& b)
{
    if (a.value.size() != b.value.size())
        throw cRuntimeError("BigUint < operator, number have different size \n");
    return a.value < b.value;
}

inline bool operator==(const BigUint& a, const uint32_t& b)
{
    for (unsigned int i =  a.value.size()-1; i>0;i--)
        if (a.value[i]!= 0) return false;
    return (a.value[0] == (uint64_t)b);
}

inline bool operator==(const uint32_t& b, const BigUint& a)
{
    for (unsigned int i =  a.value.size()-1; i>0;i--)
        if (a.value[i]!= 0) return false;
    return (a.value[0] == (uint64_t)b);
}

inline bool operator==(const BigUint& a, const uint64_t& b)
{

    for (unsigned int i =  a.value.size()-1; i>0;i--)
        if (a.value[i]!= 0) return false;
    return (a.value[0] == b);
}

inline bool operator==(const uint64_t& b, const BigUint& a)
{
    for (unsigned int i =  a.value.size()-1; i>0;i--)
        if (a.value[i]!= 0) return false;
    return (a.value[0] == b);
}

inline bool operator==(const BigUint& a, const int32_t& b)
{
    for (unsigned int i =  a.value.size()-1; i>0;i--)
        if (a.value[i]!= 0) return false;
    return (a.value[0] == (uint64_t)b);
}

inline bool operator==(const int32_t& b, const BigUint& a)
{
    if (b < 0) return false;
    for (unsigned int i =  a.value.size()-1; i>0;i--)
        if (a.value[i]!= 0) return false;
    return (a.value[0] == (uint64_t)b);
}

inline bool operator==(const BigUint& a, const int64_t& b)
{
    if (b < 0) return false;
    for (unsigned int i =  a.value.size()-1; i>0;i--)
        if (a.value[i]!= 0) return false;
    return (a.value[0] == (uint64_t)b);
}

inline bool operator==(const int64_t& b, const BigUint& a)
{
    if (b < 0) return false;
    for (unsigned int i =  a.value.size()-1; i>0;i--)
        if (a.value[i]!= 0) return false;
    return (a.value[0] == (uint64_t)b);
}

inline BigUint operator+(const BigUint& a, const BigUint& b)
{
    return BigUint(a) += b;
}
inline BigUint operator-(const BigUint& a, const BigUint& b)
{
    return BigUint(a) -= b;
}
inline BigUint operator*(const BigUint& a, const BigUint& b)
{
    return BigUint(a) *= b;
}
inline BigUint operator/(const BigUint& a, const BigUint& b)
{
    return BigUint(a) /= b;
}
inline BigUint operator%(const BigUint& a, const BigUint& b)
{
    return BigUint(a) %= b;
}

inline BigUint operator>>(const BigUint& a, unsigned int n)
{
    return BigUint(a) >>= n;
}
inline BigUint operator<<(const BigUint& a, unsigned int n)
{
    return BigUint(a) <<= n;
}

inline BigUint operator&(const BigUint& a, const BigUint& b)
{
    return BigUint(a) &= b;
}
inline BigUint operator|(const BigUint& a, const BigUint& b)
{
    return BigUint(a) |= b;
}
inline BigUint operator^(const BigUint& a, const BigUint& b)
{
    return BigUint(a) ^= b;
}

inline bool operator>(const BigUint& a, const BigUint& b)
{
    return   b < a;
}
inline bool operator<=(const BigUint& a, const BigUint& b)
{
    return !(b < a);
}
inline bool operator>=(const BigUint& a, const BigUint& b)
{
    return !(a < b);
}

inline bool operator!=(const BigUint& a, const BigUint& b)
{
    return !(a == b);
}

inline std::ostream& operator<<(std::ostream& os, const BigUint& val)
{
    return os << val.toString();
}

// MISC

//typedef BigUint __BigUint;

#endif
