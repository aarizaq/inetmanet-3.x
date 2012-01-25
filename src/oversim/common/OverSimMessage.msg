cplusplus {{
#if OMNETPP_VERSION<0x0401
#define getEncapsulatedPacket getEncapsulatedMsg
#endif
}}

packet OverSimMessage
{
    @customize(true);
}

