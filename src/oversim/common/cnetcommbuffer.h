//=========================================================================
//  CMEMCOMMBUFFER.H - part of
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
#ifndef __CNETCOMMBUFFER_H__
#define __CNETCOMMBUFFER_H__

#include <ccommbuffer.h>


/**
 * Communication buffer that packs data into a memory buffer without any
 * transformation.
 *
 * @ingroup Parsim
 */
class cNetCommBuffer : public cCommBuffer
{
  protected:
    char *mBuffer;    // the buffer
    int mBufferSize;  // size of buffer allocated
    int mMsgSize;     // current msg size (incremented by pack() functions)
    int mPosition;    // current position in buffer for unpacking

  protected:
    void extendBufferFor(int dataSize);

  public:
    /**
     * Constructor.
     */
    cNetCommBuffer();

    /**
     * Destructor.
     */
    virtual ~cNetCommBuffer();

    /** @name Buffer management */
    //@{
    /**
     * Returns the buffer after packing.
     */
    char *getBuffer() const;

    /**
     * Returns the size of the buffer.
     */
    int getBufferLength() const;

    /**
     * Extend buffer to the given size is needed. Existing buffer contents
     * may be lost.
     */
    void allocateAtLeast(int size);

    /**
     * Set message length in the buffer. Used after receiving a message
     * and copying it to the buffer.
     */
    void setMessageSize(int size);

    /**
     * Returns message length in the buffer.
     */
    int getMessageSize() const;

    /**
     * Reset buffer to an empty state.
     */
    void reset();

    /**
     * Returns true if all data in buffer was used up during unpacking.
     * Returns false if there was underflow (too much data unpacked)
     * or still there are unpacked data in the buffer.
     */
    virtual bool isBufferEmpty() const;

    /**
     * Utility function. To be called after unpacking a communication buffer,
     * it checks whether the buffer is empty. If it is not (i.e. an underflow
     * or overflow occurred), an exception is thrown.
     */
    virtual void assertBufferEmpty();
    //@}

    /** @name Pack basic types */
    //@{
    virtual void pack(char d);
    virtual void pack(unsigned char d);
    virtual void pack(bool d);
    virtual void pack(short d);
    virtual void pack(unsigned short d);
    virtual void pack(int d);
    virtual void pack(unsigned int d);
    virtual void pack(long d);
    virtual void pack(unsigned long d);
    virtual void pack(opp_long_long d);
    virtual void pack(opp_unsigned_long_long d);
    virtual void pack(float d);
    virtual void pack(double d);
    virtual void pack(long double d);
    virtual void pack(const char *d);
    virtual void pack(const opp_string& d);
    virtual void pack(SimTime d);
    //@}

    /** @name Pack arrays of basic types */
    //@{
    virtual void pack(const char *d, int size);
    virtual void pack(const unsigned char *d, int size);
    virtual void pack(const bool *d, int size);
    virtual void pack(const short *d, int size);
    virtual void pack(const unsigned short *d, int size);
    virtual void pack(const int *d, int size);
    virtual void pack(const unsigned int *d, int size);
    virtual void pack(const long *d, int size);
    virtual void pack(const unsigned long *d, int size);
    virtual void pack(const opp_long_long *d, int size);
    virtual void pack(const opp_unsigned_long_long *d, int size);
    virtual void pack(const float *d, int size);
    virtual void pack(const double *d, int size);
    virtual void pack(const long double *d, int size);
    virtual void pack(const char **d, int size);
    virtual void pack(const opp_string *d, int size);
    virtual void pack(const SimTime *d, int size);
    //@}

    /** @name Unpack basic types */
    //@{
    virtual void unpack(char& d);
    virtual void unpack(unsigned char& d);
    virtual void unpack(bool& d);
    virtual void unpack(short& d);
    virtual void unpack(unsigned short& d);
    virtual void unpack(int& d);
    virtual void unpack(unsigned int& d);
    virtual void unpack(long& d);
    virtual void unpack(unsigned long& d);
    virtual void unpack(opp_long_long& d);
    virtual void unpack(opp_unsigned_long_long& d);
    virtual void unpack(float& d);
    virtual void unpack(double& d);
    virtual void unpack(long double& d);
    virtual void unpack(const char *&d);
    /**
     * Unpacks a string -- storage will be allocated for it via new char[].
     */
    void unpack(char *&d)  {unpack((const char *&)d);}
    virtual void unpack(opp_string& d);
    virtual void unpack(SimTime&);

    //@}

    /** @name Unpack arrays of basic types */
    //@{
    virtual void unpack(char *d, int size);
    virtual void unpack(unsigned char *d, int size);
    virtual void unpack(bool *d, int size);
    virtual void unpack(short *d, int size);
    virtual void unpack(unsigned short *d, int size);
    virtual void unpack(int *d, int size);
    virtual void unpack(unsigned int *d, int size);
    virtual void unpack(long *d, int size);
    virtual void unpack(unsigned long *d, int size);
    virtual void unpack(opp_long_long *d, int size);
    virtual void unpack(opp_unsigned_long_long *d, int size);
    virtual void unpack(float *d, int size);
    virtual void unpack(double *d, int size);
    virtual void unpack(long double *d, int size);
    virtual void unpack(const char **d, int size);
    virtual void unpack(opp_string *d, int size);
    virtual void unpack(SimTime *d, int size);
    //@}

    /**
     * return the length of the remaining buffer in bytes
     */
    size_t getRemainingMessageSize();

    /**
     * Packs an object.
     */
    virtual void packObject(cObject *obj);

    /**
     * Unpacks and returns an object.
     */
    virtual cObject *unpackObject();
};

#endif
