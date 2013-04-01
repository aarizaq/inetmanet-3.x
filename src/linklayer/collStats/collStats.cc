/*
 * collStats.cc
 *
 *  Created on: 11.10.2011
 *      Author: Eugen Paul
 */

#include "collStats.h"
#include "Ieee80211Frame_m.h"
#include "IPv4Datagram.h"
#include "UDPPacket_m.h"

collStats* collStats::instance = 0;

collStats::collStats(){
    udpDataColl = 0l;
    protocolColl = 0l;
    macColl = 0l;
}

collStats::~collStats(){

}

void collStats::incUdpDataColl(){
    ev << "UDP collision found\n";
    udpDataColl++;
}

void collStats::incProtocolColl(){
    ev << "Protocol collision found\n";
    protocolColl++;
}

void collStats::incMacColl(){
    ev << "MAC collision found\n";
    macColl++;
}

long collStats::getUdpDataColl(){
    return udpDataColl;
}

long collStats::getProtocolColl(){
    return protocolColl;
}

long collStats::getMacColl(){
    return macColl;
}

void collStats::readPaket(Ieee80211DataFrame* paket){
    cPacket *tempPackIeee = paket->getEncapsulatedPacket();
    if(dynamic_cast<IPv4Datagram *>(tempPackIeee)){
        IPv4Datagram *tempIPData = dynamic_cast<IPv4Datagram *>(tempPackIeee);
        cPacket *tempPackIPData = tempIPData->getEncapsulatedPacket();
        ev << "IPDatagram:OK\n";
        if(dynamic_cast<UDPPacket *>(tempPackIPData)){
            UDPPacket *tempUDP = dynamic_cast<UDPPacket *>(tempPackIPData);
            ev << "UDP:OK\n";
            if(tempUDP->getDestinationPort() == 1234){
                //udpDataColl
                incUdpDataColl();
            }else{
                //protocolColl
                incProtocolColl();
            }
        }
        else{
            incMacColl();
        }
    }
    else{
        incMacColl();
    }
}

void collStats::addCollPacket(AirFrame * airframe){
    cPacket *packet = airframe->getEncapsulatedPacket();
    if(dynamic_cast<Ieee80211DataFrame *>(packet)){
        Ieee80211DataFrame *tempIeee = dynamic_cast<Ieee80211DataFrame *>(packet);
        readPaket(tempIeee);
    }
    else{
        incMacColl();
    }
}

void collStats::addCollPacketAirFrame(AirFrame* airframe){
    cPacket *packet = airframe->getEncapsulatedPacket();
    if(dynamic_cast<Ieee80211DataFrame *>(packet)){
        Ieee80211DataFrame *tempIeee = dynamic_cast<Ieee80211DataFrame *>(packet);
        readPaket(tempIeee);
    }
    else{
        incMacColl();
    }
//    if(dynamic_cast<Ieee80211DataFrame *>(airframe)){
//        Ieee80211DataFrame *tempIeee = dynamic_cast<Ieee80211DataFrame *>(airframe);
//        cPacket *tempPackIeee = tempIeee->getEncapsulatedPacket();
//        if(dynamic_cast<IPDatagram *>(tempPackIeee)){
//            IPDatagram *tempIPData = dynamic_cast<IPDatagram *>(tempPackIeee);
//            cPacket *tempPackIPData = tempIPData->getEncapsulatedPacket();
//            ev << "IPDatagram:OK\n";
//            if(dynamic_cast<UDPPacket *>(tempPackIPData)){
//                UDPPacket *tempUDP = dynamic_cast<UDPPacket *>(tempPackIPData);
//                ev << "UDP:OK\n";
//                if(tempUDP->getDestinationPort() == 1234){
//                    //udpDataColl
//                    collStats::getInstance()->incUdpDataColl();
//                }else{
//                    //protocolColl
//                    collStats::getInstance()->incProtocolColl();
//                }
//            }
//            else{
//                collStats::getInstance()->incMacColl();
//            }
//        }
//        else{
//            collStats::getInstance()->incMacColl();
//        }
//    }
//    else{
//        collStats::getInstance()->incMacColl();
//    }
}

collStats* collStats::getInstance(){
    if(instance == 0){
        instance = new collStats();
    }
    return instance;
}

void collStats::destroy()
{
   if ( instance )
      delete instance;
   instance = 0;
}

