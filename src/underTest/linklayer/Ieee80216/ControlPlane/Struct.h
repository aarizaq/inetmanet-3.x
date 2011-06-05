#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "MACAddress.h"

//TLV coding Type/Length/Vakue

struct TLV_LEN8
{
    TLV_LEN8()
    {
        Length = 0x18;
    }

    int Type;                 //8 Bit
    int Length;                 //8 Bit
    int Value;                  //8 Bit
};

struct TLV_LEN9
{
    TLV_LEN9()
    {
        Length = 0x19;
    }

    uint8_t Type;             //8 Bit
    uint8_t Length;             //8 Bit
    uint16_t Value;             //9 Bit
};

struct TLV_LEN13
{
    TLV_LEN13()
    {
        Length = 0x1D;
    }

    int Type;                 //8 Bit
    int Length;                 //8 Bit
    int Value;                  //13Bit
};

struct TLV_LEN16
{
    TLV_LEN16()
    {
        Length = 0x20;
    }

    int Type;                 //8 Bit
    int Length;                 //8 Bit
    int Value;                  //16 Bit
};

struct TLV_LEN24
{
    TLV_LEN24()
    {
        Length = 0x28;
    }

    int Type;                 //8 Bit
    int Length;                 //8 Bit
    int Value;                  //24 Bit
};

struct TLV_LEN32
{
    TLV_LEN32()
    {
        Length = 0x30;
    }

    int Type;                 //8 Bit
    int Length;                 //8 Bit
    int Value;                  //32 Bit
};

struct TLV_LEN40
{
    TLV_LEN40()
    {
        Length = 0x38;
    }

    int Type;                 //8 Bit
    int Length;                 //8 Bit
    int Value;                  //40 Bit
};

struct TLV_LEN48
{
    TLV_LEN48()
    {
        Length = 0x40;
    }

    int Type;                 //8 Bit
    int Length;                 //8 Bit
    int Value;                  //48 Bit
};
