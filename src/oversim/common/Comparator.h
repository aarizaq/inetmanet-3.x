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
//

/**
 * @file Comparator.h
 * @author Sebastian Mies, Bernhard Heep
 */

#ifndef __COMPARATOR_H
#define __COMPARATOR_H

#include <OverlayKey.h>
#include <ProxNodeHandle.h>

/**
 * Default Comparator
 */
template<class T>
class Comparator
{
public:
    /**
     * virtual destructor
     */
    virtual ~Comparator()
    {}

    /**
     * compares two variables of type T and indicates
     * which one is smaller or if they are equal
     *
     * @param lhs first variable to compare
     * @param rhs second variable to compare
     * @return -1 if rhs is smaller, 0 if lhs and rhs are
     *         equal and 1 if rhs is greater
     */
    virtual int compare( const T& lhs, const T& rhs ) const
    {
        return lhs.compareTo(rhs);
    }
};

/**
 * OverlayKey comparator
 */
//class KeyComparator : public Comparator<OverlayKey>
//{
//    //...
//};
typedef Comparator<OverlayKey> KeyComparator; //TODO needed?

/**
 * OverlayKey standard metric
 */
class KeyStdMetric
{
public:
    /**
     * calculates the distance from x to y with the standard metric
     *
     * @param x origination key
     * @param y destination key
     * @return |y-x|
     */
    inline OverlayKey distance(const OverlayKey& x,
                               const OverlayKey& y) const
    {
        return x > y ? (x-y) : (y-x);
    }
};

/**
 * OverlayKey XOR Metric
 */
class KeyXorMetric
{
public:
    /**
     * calculates the distance from x to y with the XOR metric
     *
     * @param x origination key
     * @param y destination key
     * @return x XOR y
     */
    inline OverlayKey distance(const OverlayKey& x,
                               const OverlayKey& y) const
    {
        return x^y;
    }
};


/**
 * OverlayKey Ring Metric
 */
class KeyRingMetric
{
public:
    /**
     * calculates the distance from x to y on a bidirectional ring
     *
     * @param x origination key
     * @param y destination key
     * @return |y-x| on a bidirectional ring
     */
    static inline OverlayKey distance(const OverlayKey& x,
                               const OverlayKey& y)
    {
        OverlayKey dist1(x - y);
        OverlayKey dist2(y - x);

	if (dist1 > dist2)
	    return dist2;
	else
	    return dist1;
    }
};


/**
 * OverlayKey Unidirectional Ring Metric
 */
class KeyUniRingMetric
{
public:
    /**
     * calculates the distance from x to y on a unidirectional ring
     *
     * @param x origination key
     * @param y destination key
     * @return distance from x to y on a unidirectional ring
     */
    inline OverlayKey distance(const OverlayKey& x,
                               const OverlayKey& y) const
    {
        return y-x;
    }
};


/**
 * OverlayKey prefix metric
 */
class KeyPrefixMetric
{
public:
    KeyPrefixMetric()
    {
        bitsPerDigit = 1;
    }

    /**
     * calculates the distance from x to y with the prefix metric
     *
     * @param x origination key
     * @param y destination key
     * @return |differing suffix|
     */
    inline OverlayKey distance(const OverlayKey& x,
                               const OverlayKey& y) const
    {
        return OverlayKey::getLength() / bitsPerDigit
               - x.sharedPrefixLength(y, bitsPerDigit);
    }

    inline void setBitsPerDigit(uint8_t bitsPerDigit)
    {
        this->bitsPerDigit = bitsPerDigit;
    }

private:
    uint8_t bitsPerDigit;
};

/**
 * OverlayKey distance comparator
 */
template<class Metric = KeyStdMetric>
class KeyDistanceComparator : public Comparator<OverlayKey>
{
private:
    Metric m; /**< indicates which metric to use for the comparison */
    OverlayKey key; /**< the relative key to which distances are compared */
public:

    /**
     * constructor
     */
    KeyDistanceComparator( const OverlayKey& relativeKey )
    {
        this->key = relativeKey;
    }

    /**
     * indicates which of the two given keys is closer to
     * the relative key
     *
     * @param lhs first key
     * @param rhs second key
     * @return -1 if lhs is closer, 0 if lhs and rhs are equal and 1
     *         if rhs closer to the relative key
     */
    int compare( const OverlayKey& lhs, const OverlayKey& rhs ) const
    {
        return m.distance(lhs, key).compareTo(m.distance(rhs, key));
    }
};

template<>
class KeyDistanceComparator<KeyPrefixMetric> : public Comparator<OverlayKey>
{
private:
    KeyPrefixMetric m; /**< indicates which metric to use for the comparison */
    OverlayKey key; /**< the relative key to which distances are compared */
public:

    /**
     * constructor
     */
    KeyDistanceComparator(const OverlayKey& relativeKey, uint32_t bitsPerDigit = 1)
    {
        key = relativeKey;
        m.setBitsPerDigit(bitsPerDigit);
    }
    /**
     * indicates which of the two given keys has a longer distance to
     * the relative key
     *
     * @param lhs first key
     * @param rhs second key
     * @return -1 if lhs is closer, 0 if lhs and rhs are equal and 1
     *         if rhs closer to the relative key
     */
    int compare( const OverlayKey& lhs, const OverlayKey& rhs ) const
    {
        return m.distance(lhs, key).compareTo(m.distance(rhs, key));
    }
};

class AbstractProxComparator
{
  public:
    virtual ~AbstractProxComparator() {};

    /**
     * indicates which of the two given proximities is "better"
     *
     * @param lhs first proximity value
     * @param rhs second proximity value
     * @return -1 if lhs is closer, 0 if lhs and rhs are equal and 1
     *         if rhs closer
     */
    virtual int compare(const Prox& lhs, const Prox& rhs) const = 0;
};

class StdProxComparator : public AbstractProxComparator
{
  public:
    int compare(const Prox& lhs, const Prox& rhs) const
    {
        // return 0 if accuracy is too low
        if (lhs.accuracy < 0.5 || rhs.accuracy < 0.5) return 0;

        if (lhs.proximity < rhs.proximity) return -1;
        if (lhs.proximity > rhs.proximity) return 1;
        return 0;
    }
};

class AbstractProxKeyComparator
{
  public:
      virtual ~AbstractProxKeyComparator() {};

    /**
     * indicates which of the two given prox/key-pairs is closer to
     * the relative key
     *
     * @param lhs first prox/key-pair
     * @param rhs second prox/key-pair
     * @return -1 if lhs is closer, 0 if lhs and rhs are equal and 1
     *         if rhs closer to the relative key
     */
    virtual int compare(const ProxKey& lhs, const ProxKey& rhs) const = 0;
};

template<class Metric, class ProxComp = StdProxComparator>
class ProxKeyComparator : public AbstractProxKeyComparator
{
  protected:
    Metric m; /**< indicates which metric to use for the key comparison */
    ProxComp pc;
    OverlayKey key; /**< the relative key to which distances are compared */

  public:
    /**
     * constructor
     */
    ProxKeyComparator(const OverlayKey& relativeKey)
    {
        this->key = relativeKey;
    }
};


template<>
class ProxKeyComparator<KeyPrefixMetric> : public AbstractProxKeyComparator
{
  protected:
    KeyPrefixMetric m; /**< indicates which metric to use for the key comparison */
    StdProxComparator pc;
    OverlayKey key; /**< the relative key to which distances are compared */

  public:
    /**
     * constructor
     */
    ProxKeyComparator(const OverlayKey& relativeKey, uint32_t bitsPerDigit = 1)
    {
        this->key = relativeKey;
        m.setBitsPerDigit(bitsPerDigit);
    }
};

class KademliaPRComparator : public ProxKeyComparator<KeyPrefixMetric>
{
  public:
    KademliaPRComparator(const OverlayKey& relativeKey, uint32_t bitsPerDigit = 1)
    : ProxKeyComparator<KeyPrefixMetric, StdProxComparator>(relativeKey, bitsPerDigit) { }

    int compare(const ProxKey& lhs, const ProxKey& rhs) const
    {
        int temp = m.distance(lhs.key, key).compareTo(m.distance(rhs.key, key));
        if (temp != 0) {
            return temp;
        }
        return pc.compare(lhs.prox, rhs.prox);
    }
};

#endif

