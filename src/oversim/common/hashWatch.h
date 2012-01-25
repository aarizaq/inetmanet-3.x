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
 * @file hashWatch.h
 * @author Helge Backhaus
 */

#ifndef __WATCHABLECONTAINERS_H__
#define __WATCHABLECONTAINERS_H__

#include <omnetpp.h>

#include <map>
#include <deque>
#include <oversim_mapset.h>

template<class T>
class SIM_API cHashSetWatcher : public cStdVectorWatcherBase
{
    protected:
        UNORDERED_SET<T>& v;
        std::string classname;
        mutable typename UNORDERED_SET<T>::iterator it;
        mutable int itPos;
    public:
        cHashSetWatcher(const char *name, UNORDERED_SET<T>& var) : cStdVectorWatcherBase(name), v(var) {
            itPos=-1;
            classname = std::string("unordered_set<")+opp_typename(typeid(T))+">";
        }
        const char *getClassName() const {return classname.c_str();}
        virtual const char *getElemTypeName() const {return opp_typename(typeid(T));}
        virtual int size() const {return v.size();}
        virtual std::string at(int i) const {
            if (i==0) {
                it=v.begin(); itPos=0;
            } else if (i==itPos+1 && it!=v.end()) {
                ++it; ++itPos;
            } else {
                it=v.begin();
                for (int k=0; k<i && it!=v.end(); k++) ++it;
                itPos=i;
            }
            if (it==v.end()) {
                return std::string("out of bounds");
            }
            return atIt();
        }
        virtual std::string atIt() const {
            std::stringstream out;
            out << (*it);
            return out.str();
        }
};

template <class T>
void createHashSetWatcher(const char *varname, UNORDERED_SET<T>& v)
{
    new cHashSetWatcher<T>(varname, v);
};

template<class T>
class SIM_API cDequeWatcher : public cStdVectorWatcherBase
{
    protected:
        std::deque<T>& v;
        std::string classname;
        mutable typename std::deque<T>::iterator it;
        mutable int itPos;
    public:
        cDequeWatcher(const char *name, std::deque<T>& var) : cStdVectorWatcherBase(name), v(var) {
            itPos=-1;
            classname = std::string("deque<")+opp_typename(typeid(T))+">";
        }
        const char *className() const {return classname.c_str();}
        virtual const char *getElemTypeName() const {return opp_typename(typeid(T));}
        virtual int size() const {return v.size();}
        virtual std::string at(int i) const {
            if (i==0) {
                it=v.begin(); itPos=0;
            } else if (i==itPos+1 && it!=v.end()) {
                ++it; ++itPos;
            } else {
                it=v.begin();
                for (int k=0; k<i && it!=v.end(); k++) ++it;
                itPos=i;
            }
            if (it==v.end()) {
                return std::string("out of bounds");
            }
            return atIt();
        }
        virtual std::string atIt() const {
            std::stringstream out;
            out << (*it);
            return out.str();
        }
};

template <class T>
void createDequeWatcher(const char *varname, std::deque<T>& v)
{
    new cDequeWatcher<T>(varname, v);
};

template<class KeyT, class ValueT, class CmpT>
class SIM_API cHashMapWatcher : public cStdVectorWatcherBase
{
    protected:
        UNORDERED_MAP<KeyT,ValueT,CmpT>& m;
        mutable typename UNORDERED_MAP<KeyT,ValueT,CmpT>::iterator it;
        mutable int itPos;
        std::string classname;
    public:
        cHashMapWatcher(const char *name, UNORDERED_MAP<KeyT,ValueT,CmpT>& var) : cStdVectorWatcherBase(name), m(var) {
            itPos=-1;
            classname = std::string("unordered_map<")+opp_typename(typeid(KeyT))+","+opp_typename(typeid(ValueT))+">";
        }
        const char *getClassName() const {return classname.c_str();}
        virtual const char *getElemTypeName() const {return "struct pair<*,*>";}
        virtual int size() const {return m.size();}
        virtual std::string at(int i) const {
            if (i==0) {
                it=m.begin(); itPos=0;
            } else if (i==itPos+1 && it!=m.end()) {
                ++it; ++itPos;
            } else {
                it=m.begin();
                for (int k=0; k<i && it!=m.end(); k++) ++it;
                itPos=i;
            }
            if (it==m.end()) {
                return std::string("out of bounds");
            }
            return atIt();
        }
        virtual std::string atIt() const {
            std::stringstream out;
            out << it->first << " ==> " << it->second;
            return out.str();
        }
};

template <class KeyT, class ValueT, class CmpT>
void createHashMapWatcher(const char *varname, UNORDERED_MAP<KeyT,ValueT,CmpT>& m)
{
    new cHashMapWatcher<KeyT,ValueT,CmpT>(varname, m);
};

template<class KeyT, class ValueT, class CmpT>
class SIM_API cConstHashMapWatcher : public cStdVectorWatcherBase
{
    protected:
        const UNORDERED_MAP<KeyT,ValueT,CmpT>& m;
        mutable typename UNORDERED_MAP<KeyT,ValueT,CmpT>::const_iterator it;
        mutable int itPos;
        std::string classname;
    public:
        cConstHashMapWatcher(const char *name, const UNORDERED_MAP<KeyT,ValueT,CmpT>& var) : cStdVectorWatcherBase(name), m(var) {
            itPos=-1;
            classname = std::string("unordered_map<")+opp_typename(typeid(KeyT))+","+opp_typename(typeid(ValueT))+">";
        }
        const char *getClassName() const {return classname.c_str();}
        virtual const char *getElemTypeName() const {return "struct pair<*,*>";}
        virtual int size() const {return m.size();}
        virtual std::string at(int i) const {
            if (i==0) {
                it=m.begin(); itPos=0;
            } else if (i==itPos+1 && it!=m.end()) {
                ++it; ++itPos;
            } else {
                it=m.begin();
                for (int k=0; k<i && it!=m.end(); k++) ++it;
                itPos=i;
            }
            if (it==m.end()) {
                return std::string("out of bounds");
            }
            return atIt();
        }
        virtual std::string atIt() const {
            std::stringstream out;
            out << it->first << " ==> " << it->second;
            return out.str();
        }
};

template <class KeyT, class ValueT, class CmpT>
void createHashMapWatcher(const char *varname, const UNORDERED_MAP<KeyT,ValueT,CmpT>& m)
{
    new cConstHashMapWatcher<KeyT,ValueT,CmpT>(varname, m);
};

template<class KeyT, class ValueT, class CmpT>
class SIM_API cPointerMapWatcher : public cStdVectorWatcherBase
{
    protected:
        std::map<KeyT,ValueT,CmpT>& m;
        mutable typename std::map<KeyT,ValueT,CmpT>::iterator it;
        mutable int itPos;
        std::string classname;
    public:
        cPointerMapWatcher(const char *name, std::map<KeyT,ValueT,CmpT>& var) : cStdVectorWatcherBase(name), m(var) {
            itPos=-1;
            classname = std::string("pointer_map<")+opp_typename(typeid(KeyT))+","+opp_typename(typeid(ValueT))+">";
        }
        const char *getClassName() const {return classname.c_str();}
        virtual const char *getElemTypeName() const {return "struct pair<*,*>";}
        virtual int size() const {return m.size();}
        virtual std::string at(int i) const {
            if (i==0) {
                it=m.begin(); itPos=0;
            } else if (i==itPos+1 && it!=m.end()) {
                ++it; ++itPos;
            } else {
                it=m.begin();
                for (int k=0; k<i && it!=m.end(); k++) ++it;
                itPos=i;
            }
            if (it==m.end()) {
                return std::string("out of bounds");
            }
            return atIt();
        }
        virtual std::string atIt() const {
            std::stringstream out;
            out << it->first << " ==> " << *(it->second);
            return out.str();
        }
};

template <class KeyT, class ValueT, class CmpT>
void createPointerMapWatcher(const char *varname, std::map<KeyT,ValueT,CmpT>& m)
{
    new cPointerMapWatcher<KeyT,ValueT,CmpT>(varname, m);
};

template<class KeyT, class ValueT, class CmpT>
class cStdMultiMapWatcher : public cStdVectorWatcherBase
{
  protected:
    std::multimap<KeyT,ValueT,CmpT>& m;
    mutable typename std::multimap<KeyT,ValueT,CmpT>::iterator it;
    mutable int itPos;
    std::string classname;
  public:
    cStdMultiMapWatcher(const char *name, std::multimap<KeyT,ValueT,CmpT>& var) : cStdVectorWatcherBase(name), m(var) {
        itPos=-1;
        classname = std::string("std::multimap<")+opp_typename(typeid(KeyT))+","+opp_typename(typeid(ValueT))+">";
    }
    const char *getClassName() const {return classname.c_str();}
    virtual const char *getElemTypeName() const {return "struct pair<*,*>";}
    virtual int size() const {return m.size();}
    virtual std::string at(int i) const {
        // std::map doesn't support random access iterator and iteration is slow,
        // so we have to use a trick, knowing that Tkenv will call this function with
        // i=0, i=1, etc...
        if (i==0) {
            it=m.begin(); itPos=0;
        } else if (i==itPos+1 && it!=m.end()) {
            ++it; ++itPos;
        } else {
            it=m.begin();
            for (int k=0; k<i && it!=m.end(); k++) ++it;
            itPos=i;
        }
        if (it==m.end()) {
            return std::string("out of bounds");
        }
        return atIt();
    }
    virtual std::string atIt() const {
        std::stringstream out;
        out << it->first << " ==> " << it->second;
        return out.str();
    }
};

template <class KeyT, class ValueT, class CmpT>
void createStdMultiMapWatcher(const char *varname, std::multimap<KeyT,ValueT,CmpT>& m)
{
    new cStdMultiMapWatcher<KeyT,ValueT,CmpT>(varname, m);
};


/**
 * Makes unordered_sets inspectable in Tkenv.
 *
 * @hideinitializer
 */
#define WATCH_UNORDERED_SET(variable)    createHashSetWatcher(#variable,(variable))

/**
  * Makes unordered_sets inspectable in Tkenv.
  *
  * @hideinitializer
  */
#define WATCH_DEQUE(variable)    createDequeWatcher(#variable,(variable))

/**
 * Makes unordered_maps inspectable in Tkenv.
 *
 * @hideinitializer
 */
#define WATCH_UNORDERED_MAP(m)           createHashMapWatcher(#m,(m))

/**
 * Makes pointer_maps inspectable in Tkenv.
 *
 * @hideinitializer
 */
#define WATCH_POINTER_MAP(m)        createPointerMapWatcher(#m,(m))

/**
 * Makes std::multimaps inspectable in Tkenv. See also WATCH_MULTIPTRMAP().
 *
 * @hideinitializer
 */
#define WATCH_MULTIMAP(m)         createStdMultiMapWatcher(#m,(m))

#endif
