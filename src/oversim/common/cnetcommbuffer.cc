//=========================================================================
//  CMEMCOMMBUFFER.CC - part of
//
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//   Written by:  Andras Varga, 2003
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2003-2005 Andras Varga
  Monash University, Dept. of Electrical and Computer Systems Eng.
  Melbourne, Australia

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/
#include <string.h>
#include <platdep/sockets.h>
#include <stdexcept>
#include <omnetpp.h>
#include "cnetcommbuffer.h"

/*
#define STORE(type,d)    {*(type *)(mBuffer+mMsgSize)=d; mMsgSize+=sizeof(type);}
#define EXTRACT(type,d)  {d=*(type *)(mBuffer+mPosition); mPosition+=sizeof(type);}
*/

#define STOREARRAY(type,d,size)   {memcpy(mBuffer+mMsgSize,d,size*sizeof(type)); mMsgSize+=size*sizeof(type);}
#define EXTRACTARRAY(type,d,size) {\
    if ((mPosition + size*sizeof(type)) <= (uint32_t)mBufferSize) {\
        memcpy(d,mBuffer+mPosition,size*sizeof(type)); mPosition+=size*sizeof(type);\
    } else {\
        throw cRuntimeError("OverSim cnetcommbuffer.cc: EXTRACTARRAY buffer overflow!");\
    }\
}

#define STORE(type,d)   {memcpy(mBuffer+mMsgSize,(void*)&d,sizeof(type)); mMsgSize+=sizeof(type);}
#define EXTRACT(type,d) {\
    if ((mPosition + sizeof(type)) <= (uint32_t)mBufferSize) {\
        memcpy((void*)&d,mBuffer+mPosition,sizeof(type)); mPosition+=sizeof(type);\
    } else {\
        throw cRuntimeError("OverSim cnetcommbuffer.cc: EXTRACT buffer overflow!");\
    }\
}

cNetCommBuffer::cNetCommBuffer()
{
    mBuffer = 0;
    mBufferSize = 0;
    mPosition = 0;
    mMsgSize = 0;
}

cNetCommBuffer::~cNetCommBuffer ()
{
    delete [] mBuffer;
}

char *cNetCommBuffer::getBuffer() const
{
    return mBuffer;
}

int cNetCommBuffer::getBufferLength() const
{
    return mBufferSize;
}

void cNetCommBuffer::allocateAtLeast(int size)
{
    size += 4; // allocate a bit more room, for sentry (used in cFileCommBuffer, etc.)
    if (mBufferSize < size)
    {
        delete [] mBuffer;
        mBuffer = new char[size];
        mBufferSize = size;
    }
}

void cNetCommBuffer::setMessageSize(int size)
{
    mMsgSize = size;
    mPosition = 0;
}

int cNetCommBuffer::getMessageSize() const
{
    return mMsgSize;
}

void cNetCommBuffer::reset()
{
    mMsgSize = 0;
    mPosition = 0;
}

void cNetCommBuffer::extendBufferFor(int dataSize)
{
    // TBD move reallocate+copy out of loop (more efficient)
    while (mMsgSize+dataSize >= mBufferSize)
    {
        // increase the size of the buffer while
        // retaining its own existing contents
        char *tempBuffer;
        int i, oldBufferSize = 0;

        oldBufferSize = mBufferSize;
        if (mBufferSize == 0)
            mBufferSize = 1000;
        else
            mBufferSize += mBufferSize;

        tempBuffer = new char[mBufferSize];
        for(i = 0; i < oldBufferSize; i++)
            tempBuffer[i] = mBuffer[i];

        delete [] mBuffer;
        mBuffer = tempBuffer;
    }
}

bool cNetCommBuffer::isBufferEmpty() const
{
    return mPosition == mMsgSize;
}

void cNetCommBuffer::assertBufferEmpty()
{
    if (mPosition == mMsgSize)
        return;

    if (mPosition > mMsgSize)
    {
        throw cRuntimeError("internal error: cCommBuffer pack/unpack mismatch: "
                             "read %d bytes past end of buffer while unpacking %d bytes",
                             mPosition-mMsgSize, mPosition);
    }
    else
    {
        throw cRuntimeError("internal error: cCommBuffer pack/unpack mismatch: "
                            "%d extra bytes remained in buffer after unpacking %d bytes",
                            mMsgSize-mPosition, mPosition);
    }
}


void cNetCommBuffer::pack(char d)
{
    extendBufferFor(sizeof(char));
    STORE(char,d);
}


void cNetCommBuffer::pack(unsigned char d)
{
    extendBufferFor(sizeof(unsigned char));
    STORE(unsigned char,d);
}


void cNetCommBuffer::pack(bool d)
{
    extendBufferFor(sizeof(bool));
    STORE(bool,d);
}


void cNetCommBuffer::pack(short d)
{
    extendBufferFor(sizeof(short));
    short d_buf = htons(d);
    STORE(short,d_buf);
}


void cNetCommBuffer::pack(unsigned short d)
{
    extendBufferFor(sizeof(unsigned short));
    unsigned short d_buf = htons(d);
    STORE(unsigned short,d_buf);
}


void cNetCommBuffer::pack(int d)
{
    extendBufferFor(sizeof(int));
    int d_buf = htonl(d);
    STORE(int,d_buf);
}


void cNetCommBuffer::pack(unsigned int d)
{
    extendBufferFor(sizeof(unsigned int));
    unsigned int d_buf = htonl(d);
    STORE(unsigned int,d_buf);
}


void cNetCommBuffer::pack(long d)
{
    extendBufferFor(sizeof(long));
    long d_buf = htonl(d);
    STORE(long,d_buf);
}


void cNetCommBuffer::pack(unsigned long d)
{
    extendBufferFor(sizeof(unsigned long));
    unsigned long d_buf = htonl(d);
    STORE(unsigned long,d_buf);
}


void cNetCommBuffer::pack(opp_long_long d)
{
    extendBufferFor(sizeof(opp_long_long));
    STORE(opp_long_long,d);
}


void cNetCommBuffer::pack(opp_unsigned_long_long d)
{
    extendBufferFor(sizeof(opp_unsigned_long_long));
    STORE(opp_unsigned_long_long,d);
}


void cNetCommBuffer::pack(float d)
{
    extendBufferFor(sizeof(float));
    STORE(float,d);
}


void cNetCommBuffer::pack(double d)
{
    extendBufferFor(sizeof(double));
    STORE(double,d);
}


void cNetCommBuffer::pack(long double d)
{
    extendBufferFor(sizeof(long double));
    STORE(long double,d);
}



// pack a string
void cNetCommBuffer::pack(const char *d)
{
    int len = d ? strlen(d) : 0;

    pack(len);
    extendBufferFor(len*sizeof(char));
    STOREARRAY(char,d,len);
}

void cNetCommBuffer::pack(const opp_string& d)
{
    pack(d.c_str());
}

void cNetCommBuffer::pack(const SimTime *d, int size)
{
    for (int i = 0; i < size; i++)
        pack(d[i]);
}

void cNetCommBuffer::pack(const char *d, int size)
{
    extendBufferFor(size*sizeof(char));
    STOREARRAY(char,d,size);
}


void cNetCommBuffer::pack(const unsigned char *d, int size)
{
    extendBufferFor(size*sizeof(unsigned char));
    STOREARRAY(unsigned char,d,size);
}


void cNetCommBuffer::pack(const bool *d, int size)
{
    extendBufferFor(size*sizeof(bool));
    STOREARRAY(bool,d,size);
}

//FIXME: not portable!
void cNetCommBuffer::pack(const short *d, int size)
{
    extendBufferFor(size*sizeof(short));
    STOREARRAY(short,d,size);
}


//FIXME: not portable!
void cNetCommBuffer::pack(const unsigned short *d, int size)
{
    extendBufferFor(size*sizeof(unsigned short));
    STOREARRAY(unsigned short,d,size);
}


//FIXME: not portable!
void cNetCommBuffer::pack(const int *d, int size)
{
    extendBufferFor(size*sizeof(int));
    STOREARRAY(int,d,size);
}


//FIXME: not portable!
void cNetCommBuffer::pack(const unsigned int *d, int size)
{
    extendBufferFor(size*sizeof(unsigned int));
    STOREARRAY(unsigned int,d,size);
}


//FIXME: not portable!
void cNetCommBuffer::pack(const long *d, int size)
{
    extendBufferFor(size*sizeof(long));
    STOREARRAY(long,d,size);
}


//FIXME: not portable!
void cNetCommBuffer::pack(const unsigned long *d, int size)
{
    extendBufferFor(size*sizeof(unsigned long));
    STOREARRAY(unsigned long,d,size);
}


void cNetCommBuffer::pack(const opp_long_long *d, int size)
{
    extendBufferFor(size*sizeof(opp_long_long));
    STOREARRAY(opp_long_long,d,size);
}


void cNetCommBuffer::pack(const opp_unsigned_long_long *d, int size)
{
    extendBufferFor(size*sizeof(opp_unsigned_long_long));
    STOREARRAY(opp_unsigned_long_long,d,size);
}


void cNetCommBuffer::pack(const float *d, int size)
{
    extendBufferFor(size*sizeof(float));
    STOREARRAY(float,d,size);
}


void cNetCommBuffer::pack(const double *d, int size)
{
    extendBufferFor(size*sizeof(double));
    STOREARRAY(double,d,size);
}


void cNetCommBuffer::pack(const long double *d, int size)
{
    extendBufferFor(size*sizeof(long double));
    STOREARRAY(long double,d,size);
}


// pack string array
void cNetCommBuffer::pack(const char **d, int size)
{
    for (int i = 0; i < size; i++)
        pack(d[i]);
}

void cNetCommBuffer::pack(const opp_string *d, int size)
{
    for (int i = 0; i < size; i++)
        pack(d[i]);
}

void cNetCommBuffer::pack(SimTime d)
{
    pack((opp_long_long)d.raw());
}

//--------------------------------

void cNetCommBuffer::unpack(char& d)
{
    EXTRACT(char,d);
}


void cNetCommBuffer::unpack(unsigned char& d)
{
    EXTRACT(unsigned char,d);
}

void cNetCommBuffer::unpack(bool& d)
{
    EXTRACT(bool,d);
}

void cNetCommBuffer::unpack(short& d)
{
    EXTRACT(short,d);
    d = ntohs(d);
}


void cNetCommBuffer::unpack(unsigned short& d)
{
    EXTRACT(unsigned short,d);
    d = ntohs(d);
}


void cNetCommBuffer::unpack(int& d)
{
    EXTRACT(int,d);
    d = ntohl(d);
}


void cNetCommBuffer::unpack(unsigned int& d)
{
    EXTRACT(unsigned int,d);
    d = ntohl(d);
}


void cNetCommBuffer::unpack(long& d)
{
    EXTRACT(long,d);
    d = ntohl(d);
}


void cNetCommBuffer::unpack(unsigned long& d)
{
    EXTRACT(unsigned long,d);
    d = ntohl(d);
}


void cNetCommBuffer::unpack(opp_long_long& d)
{
    EXTRACT(opp_long_long,d);
}


void cNetCommBuffer::unpack(opp_unsigned_long_long& d)
{
    EXTRACT(opp_unsigned_long_long,d);
}


void cNetCommBuffer::unpack(float& d)
{
    EXTRACT(float,d);
}


void cNetCommBuffer::unpack(double& d)
{
    EXTRACT(double,d);
}


void cNetCommBuffer::unpack(long double& d)
{
    EXTRACT(long double,d);
}


// unpack a string
void cNetCommBuffer::unpack(const char *&d)
{
    int len;
    unpack(len);

    char *tmp = new char[len+1];
    EXTRACTARRAY(char,tmp,len);
    tmp[len] = '\0';
    d = tmp;
}


void cNetCommBuffer::unpack(opp_string& d)
{
    int len;
    unpack(len);

    d.reserve(len+1);
    EXTRACTARRAY(char,d.buffer(),len);
    d.buffer()[len] = '\0';
}


void cNetCommBuffer::unpack(SimTime& d)
{
    opp_long_long raw;
    unpack(raw);
    d.setRaw(raw);
}


void cNetCommBuffer::unpack(char *d, int size)
{
    EXTRACTARRAY(char,d,size);
}

void cNetCommBuffer::unpack(unsigned char *d, int size)
{
    EXTRACTARRAY(unsigned char,d,size);
}


void cNetCommBuffer::unpack(bool *d, int size)
{
    EXTRACTARRAY(bool,d,size);
}

void cNetCommBuffer::unpack(short *d, int size)
{
    EXTRACTARRAY(short,d,size);
}


void cNetCommBuffer::unpack(unsigned short *d, int size)
{
    EXTRACTARRAY(unsigned short,d,size);
}


void cNetCommBuffer::unpack(int *d, int size)
{
    EXTRACTARRAY(int,d,size);
}


void cNetCommBuffer::unpack(unsigned int *d, int size)
{
    EXTRACTARRAY(unsigned,d,size);
}


void cNetCommBuffer::unpack(long *d, int size)
{
    EXTRACTARRAY(long,d,size);
}


void cNetCommBuffer::unpack(unsigned long *d, int size)
{
    EXTRACTARRAY(unsigned long,d,size);
}


void cNetCommBuffer::unpack(opp_long_long *d, int size)
{
    EXTRACTARRAY(opp_long_long,d,size);
}


void cNetCommBuffer::unpack(opp_unsigned_long_long *d, int size)
{
    EXTRACTARRAY(opp_unsigned_long_long,d,size);
}


void cNetCommBuffer::unpack(float *d, int size)
{
    EXTRACTARRAY(float,d,size);
}


void cNetCommBuffer::unpack(double *d, int size)
{
    EXTRACTARRAY(double,d,size);
}


void cNetCommBuffer::unpack(long double *d, int size)
{
    EXTRACTARRAY(long double,d,size);
}

void cNetCommBuffer::unpack(const char **d, int size)
{
    for (int i = 0; i < size; i++)
        unpack(d[i]);
}

void cNetCommBuffer::unpack(opp_string *d, int size)
{
    for (int i = 0; i < size; i++)
        unpack(d[i]);
}


void cNetCommBuffer::unpack(SimTime *d, int size)
{
    for (int i = 0; i < size; i++)
        unpack(d[i]);
}


size_t cNetCommBuffer::getRemainingMessageSize()
{
    return (mMsgSize - mPosition);
}

void cNetCommBuffer::packObject(cObject *obj)
{
    pack(obj->getClassName());
    obj->parsimPack(this);
}

cObject *cNetCommBuffer::unpackObject()
{
    char *classname = NULL;
    cObject *obj = NULL;
    try {
        unpack(classname);
        obj = createOne(classname);
        obj->parsimUnpack(this);
    } catch (...) {
        delete [] classname;
        delete obj;
        throw std::invalid_argument("Failed to parse received packet");
    }

    delete [] classname;
    return obj;
}

