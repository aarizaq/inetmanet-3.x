//
// Copyright (C) 2007 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file NodeVector.h
 * @author Sebastian Mies
 * @author Felix M. Palmen
 */

#ifndef __NODE_VECTOR_H
#define __NODE_VECTOR_H

#include <vector>
#include <cassert>

#include <Comparator.h>
#include <NodeHandle.h>
#include <ProxNodeHandle.h>


template <class T> class KeyExtractor;
template <class T> class ProxExtractor;

template <class T,
          class T_key = KeyExtractor<T>,
          class T_prox = ProxExtractor<T> > class BaseKeySortedVector;

typedef BaseKeySortedVector< NodeHandle > NodeVector;
typedef BaseKeySortedVector< ProxNodeHandle > ProxNodeVector;

/**
 * Class for extracting the relevant OverlayKey from a type used as template
 * parameter T_key for NodeVector<T, T_key> - Generic Version for unknown
 * types, returns unspecified keys
 *
 * @author Felix M. Palmen
 */
template <class T>
struct KeyExtractor {
    static const OverlayKey& key(const T&)
    {
        return OverlayKey::UNSPECIFIED_KEY;
    };
};

/**
 * Class for extracting the relevant OverlayKey from a type used as template
 * parameter T_key for NodeVector<T, T_key> - Version for plain NodeHandle.
 *
 * @author Felix M. Palmen
 */
template <>
struct KeyExtractor<NodeHandle> {
    static const OverlayKey& key(const NodeHandle& node)
    {
        return node.getKey();
    };
};

template <>
struct KeyExtractor<ProxNodeHandle> {
    static const OverlayKey& key(const ProxNodeHandle& node)
    {
        return node.getKey();
    };
};

/**
 * Class for extracting the relevant OverlayKey from a type used as template
 * parameter T_key for NodeVector<T, T_key> - Version for a pair of
 * NodeHandles, first one is assumed to be relevant
 *
 * @author Felix M. Palmen
 */
template <>
struct KeyExtractor<std::pair<NodeHandle, simtime_t> > {
    static const OverlayKey& key(const std::pair<NodeHandle, simtime_t>& nodes)
    {
        return nodes.first.getKey();
    };
};

template <class T>
struct ProxExtractor {
    static Prox prox(const T&)
    {
        return Prox::PROX_UNKNOWN;
    };
};

template <>
struct ProxExtractor< ProxNodeHandle >{
    static Prox prox(const ProxNodeHandle& node)
    {
        return node.getProx();
    };
};

/**
 * A STL-vector that supports inserts sorted by an OverlayKey found somewhere
 * in the type
 *
 * @author Sebastian Mies
 */
//template <class T, class T_key, class T_rtt>
//class BaseKeySortedVector : public std::vector<T> {
//
//private://fields: comparator
//
//    const Comparator<OverlayKey>* comparator; /**< the OverlayKey Comparator for this vector */
//    uint16_t maxSize; /**< maximum nodes this vector holds */
//    bool useRtt; /**< sort by rtt after sorting using the comparator */
//
//public://construction
//
//    /**
//     * constructor
//     *
//     * @param maxSize maximum nodes this vector holds
//     * @param comparator OverlayKey Comparator for this vector
//     * @param useRtt sort by rtt after sorting using the comparator
//     */
//    BaseKeySortedVector(uint16_t maxSize = 0,
//                        const Comparator<OverlayKey>* comparator = NULL,
//                        bool useRtt = false) :
//    std::vector<T>(),
//    comparator(comparator),
//    maxSize(maxSize),
//    useRtt(useRtt) { };
//
//    /**
//     * destructor
//     */
//    virtual ~BaseKeySortedVector() {};
//
//    typedef typename std::vector<T>::iterator iterator; /**< iterator for this vector */
//    typedef typename std::vector<T>::const_iterator const_iterator; /**< read-only iterator for this vector */
//
//    static const T UNSPECIFIED_ELEMENT; /**< unspecified element of type T */
//
//
//public://methods: sorted add support
//
//    /**
//     * indicates if an object of type T can be added to the NodeVector
//     *
//     * @param element the element to add
//     * @return true if element can be added to the NodeVector, false otherwise
//     */
//    bool isAddable( const T& element ) const
//    {
//        if (maxSize == 0) {
//            return false;
//        }
//
//        return(std::vector<T>::size() != maxSize ||
//                (comparator && ( comparator->compare( T_key::key(element),
//                                                      T_key::key(std::vector<T>::back()) ) <= 0 )));
//    };
//
//    /**
//     * indicates if NodeVector holds maxSize nodes
//     *
//     * @return true if the actual size of NodeVector has reached its maxSize, false otherwise
//     */
//    bool isFull() const
//    {
//        return(std::vector<T>::size() == maxSize);
//    };
//
//    /**
//     * indicates if NodeVector holds at least one node
//     *
//     * @return true if NodeVector does not hold any node, false otherwise
//     */
//    bool isEmpty() const
//    {
//        return(std::vector<T>::size() == 0);
//    };
//
//    /**
//     * adds an element of type T in increasing order to the NodeVector and
//     * returns the position of the added element or -1 if the element was not added
//     *
//     * @param element the element to add
//     * @return position of the added element, -1 if the element was not added
//     */
//    int add( const T& element )
//    {
//        int pos = -1;
//
//        // check if handle is addable
//        if (isAddable(element)) { // yes ->
//
//            // add handle to the appropriate position
//            if ((std::vector<T>::size() != 0) && comparator) {
//                iterator i;
//                for (i = std::vector<T>::begin(), pos=0;
//                     i != std::vector<T>::end(); i++, pos++) {
//
//                    // don't add node with same key twice
//                    if (T_key::key(element) == T_key::key(*i)) {
//                        return -1;
//                    }
//
//                    int compResult = comparator->compare(T_key::key(element),
//                                                         T_key::key(*i));
//                    if ((compResult < 0) ||
//                            (useRtt && (compResult == 0) &&
//                                    T_rtt::rtt(element) < T_rtt::rtt(*i))) {
//
//                        std::vector<T>::insert(i, element);
//                        break;
//
//                        // if (useRtt &&
//                        //    (compResult == 0) &&
//                        //     T_rtt::rtt(element) < T_rtt::rtt(*i))
//                        //     std::cout << "!!!" << std::endl;
//                    }
//                }
//                if (i == std::vector<T>::end()) {
//                    pos = std::vector<T>::size();
//                    push_back(element);
//                }
//            } else {
//                for (iterator i = std::vector<T>::begin(); i != std::vector<T>::end();
//                     i++) {
//                    // don't add node with same key twice
//                    if (T_key::key(element) == T_key::key(*i)) {
//                        return -1;
//                    }
//                }
//                pos = std::vector<T>::size();
//                push_back(element);
//            }
//
//            // adjust size
//            if ((maxSize != 0) && (std::vector<T>::size() > maxSize)) {
//                std::vector<T>::resize(maxSize);
//            }
//        }
//
//        return pos;
//    };
//
//    /**
//     * Searches for an OverlayKey in NodeVector and returns true, if
//     * it is found.
//     *
//     * @param key the OverlayKey to find
//     * @return true, if the vector contains the key
//     */
//    const bool contains(const OverlayKey& key) const {
//        for (const_iterator i = std::vector<T>::begin();
//                i != std::vector<T>::end(); i++) {
//            if (T_key::key(*i) == key) return true;
//        }
//
//        return false;
//    }
//
//    /**
//     * searches for an OverlayKey in NodeVector
//     *
//     * @param key the OverlayKey to find
//     * @return the UNSPECIFIED_ELEMENT if there is no element with the defined key,
//     * the found element of type T otherwise
//     */
//    const T& find(const OverlayKey& key) const
//    {
//        for (const_iterator i = std::vector<T>::begin();
//                i != std::vector<T>::end(); i++) {
//            if (T_key::key(*i) == key) return *i;
//        }
//        return UNSPECIFIED_ELEMENT;
//    };
//
//    /**
//     * Searches for an OberlayKey in a NodeVector and returns an
//     * appropriate iterator.
//     *
//     * @param key The key to search
//     * @return iterator The iterator
//     */
//    iterator findIterator(const OverlayKey& key)
//    {
//        iterator i;
//        for (i = std::vector<T>::begin(); i != std::vector<T>::end(); i++)
//            if (T_key::key(*i) == key) break;
//        return i;
//    }
//
//    /**
//     * Downsize the vector to a maximum of maxElements.
//     *
//     * @param maxElements The maximum number of elements after downsizing
//     */
//    void downsizeTo(const uint32_t maxElements)
//    {
//        if (std::vector<T>::size() > maxElements) {
//            std::vector<T>::erase(std::vector<T>::begin()+maxElements, std::vector<T>::end());
//        }
//    }
//
//    void setComparator(const Comparator<OverlayKey>* comparator)
//    {
//        this->comparator = comparator;
//    }
//};

template <class T, class T_key, class T_prox>
class BaseKeySortedVector : public std::vector<T> {

private://fields: comparator
    const Comparator<OverlayKey>* comparator; /**< the OverlayKey Comparator for this vector */
    const AbstractProxComparator* proxComparator;
    const AbstractProxKeyComparator* proxKeyComparator;
    uint16_t maxSize; /**< maximum nodes this vector holds */
    uint16_t sizeProx;
    uint16_t sizeComb;

public://construction
    /**
     * constructor
     *
     * @param maxSize maximum nodes this vector holds
     * @param comparator OverlayKey comparator for this vector
     * @param proxComparator proximity comparator for this vector
     * @param proxKeyComparator proximity/key comparator for this vector
     * @param sizeProx number of nodes sorted by proximity
     * @param sizeComb number of nodes sorted by proximity/key
     */
    BaseKeySortedVector(uint16_t maxSize = 0,
                        const Comparator<OverlayKey>* comparator = NULL,
                        const AbstractProxComparator* proxComparator = NULL,
                        const AbstractProxKeyComparator* proxKeyComparator = NULL,
                        uint16_t sizeProx = 0,
                        uint16_t sizeComb = 0) :
    std::vector<T>(),
    comparator(comparator),
    proxComparator(proxComparator),
    proxKeyComparator(proxKeyComparator),
    maxSize(maxSize),
    sizeProx(sizeProx),
    sizeComb(sizeComb) { };

    /**
     * destructor
     */
    virtual ~BaseKeySortedVector() {};

    typedef typename std::vector<T>::iterator iterator; /**< iterator for this vector */
    typedef typename std::vector<T>::const_iterator const_iterator; /**< read-only iterator for this vector */

    static const T UNSPECIFIED_ELEMENT; /**< unspecified element of type T */


public://methods: sorted add support

    /**
     * indicates if an object of type T can be added to the NodeVector
     *
     * @param element the element to add
     * @return true if element can be added to the NodeVector, false otherwise
     */
    bool isAddable( const T& element ) const
    {
        if (maxSize == 0) {
            return true;
        }

        return (std::vector<T>::size() != maxSize ||
               (comparator &&
                (comparator->compare(T_key::key(element),
                                     T_key::key(std::vector<T>::back())) <= 0 )) ||
               (proxComparator &&
                (proxComparator->compare(T_prox::prox(element),
                                         T_prox::prox(std::vector<T>::back())) <= 0 )) ||
               (proxKeyComparator &&
                (proxKeyComparator->compare(ProxKey(T_prox::prox(element),
                                                    T_key::key(element)),
                                            ProxKey(T_prox::prox(std::vector<T>::back()),
                                                    T_key::key(std::vector<T>::back()))) <= 0 )));
    };

    /**
     * indicates if NodeVector holds maxSize nodes
     *
     * @return true if the actual size of NodeVector has reached its maxSize, false otherwise
     */
    bool isFull() const
    {
        return(std::vector<T>::size() == maxSize);
    };

    /**
     * indicates if NodeVector holds at least one node
     *
     * @return true if NodeVector does not hold any node, false otherwise
     */
    bool isEmpty() const
    {
        return(std::vector<T>::size() == 0);
    };

    /**
     * adds an element of type T in increasing order to the NodeVector and
     * returns the position of the added element or -1 if the element was not added
     *
     * @param element the element to add
     * @return position of the added element, -1 if the element was not added
     */
    int add( const T& element )
    {
        int pos = -1;

        // check if handle is addable
        if (isAddable(element)) { // yes ->

            // add handle to the appropriate position
            if ((std::vector<T>::size() != 0) &&
                (comparator || proxComparator || proxKeyComparator)) {
                iterator i;
                for (i = std::vector<T>::begin(), pos=0;
                     i != std::vector<T>::end(); i++, pos++) {

                    // don't add node with same key twice
                    if (T_key::key(element) == T_key::key(*i)) {
                        return -1;
                    }

                    if (pos < sizeProx) {
                        assert(proxComparator);
                        // only compare proximity
                        int compResult =
                            proxComparator->compare(T_prox::prox(element),
                                                    T_prox::prox(*i));
                        //if (T_prox::prox(element).compareTo(T_prox::prox(*i))) {
                        if (compResult < 0) {
                            iterator temp_it = std::vector<T>::insert(i, element);
                            T temp = *(temp_it++);
                            std::vector<T>::erase(temp_it);
                            //re-insert replaced entry into other 2 ranges
                            add(temp);
                            break;
                        }
                    } else if (pos < sizeProx + sizeComb) {
                        assert(proxKeyComparator);
                        // compare proximity and key distance
                        int compResult = proxKeyComparator->compare(ProxKey(T_prox::prox(element), T_key::key(element)),
                                                                    ProxKey(T_prox::prox(*i), T_key::key(*i)));
                        if (compResult < 0) {
                            iterator temp_it = std::vector<T>::insert(i, element);
                            T temp = *(temp_it++);
                            std::vector<T>::erase(temp_it);
                            //re-insert replaced entry into last range
                            add(temp);
                            break;
                        }
                    } else {
                        assert(comparator);
                        // only consider key distance
                        int compResult = comparator->compare(T_key::key(element),
                                                             T_key::key(*i));
                        if (compResult < 0) {
                            std::vector<T>::insert(i, element);
                            break;
                        }
                    }
                }
                if (i == std::vector<T>::end()) {
                    pos = std::vector<T>::size();
                    push_back(element);
                }
            } else {
                for (iterator i = std::vector<T>::begin(); i != std::vector<T>::end();
                     i++) {
                    // don't add node with same key twice
                    if (T_key::key(element) == T_key::key(*i)) {
                        return -1;
                    }
                }
                pos = std::vector<T>::size();
                push_back(element);
            }

            // adjust size
            if ((maxSize != 0) && (std::vector<T>::size() > maxSize)) {
                std::vector<T>::resize(maxSize);
            }
        }
        return pos;
    };

    /**
     * Searches for an OverlayKey in NodeVector and returns true, if
     * it is found.
     *
     * @param key the OverlayKey to find
     * @return true, if the vector contains the key
     */
    const bool contains(const OverlayKey& key) const {
        for (const_iterator i = std::vector<T>::begin();
                i != std::vector<T>::end(); i++) {
            if (T_key::key(*i) == key) return true;
        }

        return false;
    }

    /**
     * searches for an OverlayKey in NodeVector
     *
     * @param key the OverlayKey to find
     * @return the UNSPECIFIED_ELEMENT if there is no element with the defined key,
     * the found element of type T otherwise
     */
    const T& find(const OverlayKey& key) const
    {
        for (const_iterator i = std::vector<T>::begin();
                i != std::vector<T>::end(); i++) {
            if (T_key::key(*i) == key) return *i;
        }
        return UNSPECIFIED_ELEMENT;
    };

    /**
     * Searches for an OberlayKey in a NodeVector and returns an
     * appropriate iterator.
     *
     * @param key The key to search
     * @return iterator The iterator
     */
    iterator findIterator(const OverlayKey& key)
    {
        iterator i;
        for (i = std::vector<T>::begin(); i != std::vector<T>::end(); i++)
            if (T_key::key(*i) == key) break;
        return i;
    }

    /**
     * Downsize the vector to a maximum of maxElements.
     *
     * @param maxElements The maximum number of elements after downsizing
     */
    void downsizeTo(const uint32_t maxElements)
    {
        if (std::vector<T>::size() > maxElements) {
            std::vector<T>::erase(std::vector<T>::begin()+maxElements, std::vector<T>::end());
        }
    }

    void setComparator(const Comparator<OverlayKey>* comparator)
    {
        this->comparator = comparator;
    }
};
template <class T, class T_key, class T_rtt>
const T BaseKeySortedVector<T, T_key, T_rtt>::UNSPECIFIED_ELEMENT; /**< an unspecified element of the NodeVector */

#endif
