//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2012 Alfonso Ariza
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


//
// based on the video streaming app of the similar name by Johnny Lai
//


#include <iostream>
#include <fstream>
#include "UDPVideoStreamSvr2.h"

#include "UDPControlInfo_m.h"
#include "UDPVideoData_m.h"
#include "VideoPacket_m.h"



Define_Module(UDPVideoStreamSvr2);

simsignal_t UDPVideoStreamSvr2::reqStreamBytesSignal = SIMSIGNAL_NULL;
simsignal_t UDPVideoStreamSvr2::sentPkSignal = SIMSIGNAL_NULL;

inline std::ostream& operator<<(std::ostream& out, const UDPVideoStreamSvr2::VideoStreamData& d)
{
    out << "client=" << d.clientAddr << ":" << d.clientPort
        << "  size=" << d.videoSize << "  pksent=" << d.numPkSent << "  bytesleft=" << d.bytesLeft;
    return out;
}


void UDPVideoStreamSvr2::fileParser(const char *fileName)
{
    std::string fi(fileName);
    fi = "./"+fi;
    std::ifstream inFile(fi.c_str());

    if (!inFile)
    {
        error("Error while opening input file (File not found or incorrect type)\n");
    }

    trace.clear();
    simtime_t timedata;
    while (!inFile.eof())
    {
        int seqNum = 0;
        float time = 0;
        float YPSNR = 0;
        float UPSNR = 0;
        float VPSNR = 0;
        int len = 0;
        char frameType = 0;
        inFile >> seqNum >>  time >> frameType >> len >> YPSNR >> UPSNR >> VPSNR;
        VideoInfo info;
        info.seqNum = seqNum;
        info.type = frameType;
        info.size = len;
        info.timeFrame = time;
        // now insert in time order
        if (trace.empty() || trace.back().timeFrame < info.timeFrame)
            trace.push_back(info);
        else
        {
            // search the place
            for (int i = trace.size()-1 ; i >= 0; i--)
            {
                if (trace[i].timeFrame < info.timeFrame)
                {
                    trace.insert(trace.begin()+i+1,info);
                    break;
                }

            }
        }
    }
    inFile.close();
}


UDPVideoStreamSvr2::UDPVideoStreamSvr2()
{
}

UDPVideoStreamSvr2::~UDPVideoStreamSvr2()
{
    for (unsigned int i=0; i < streamVector.size(); i++)
        delete streamVector[i];
    trace.clear();
}

void UDPVideoStreamSvr2::initialize()
{
    sendInterval = &par("sendInterval");
    packetLen = &par("packetLen");
    videoSize = &par("videoSize");
    stopTime = &par("stopTime");
    localPort = par("localPort");

    macroPackets = par("macroPackets");
    maxSizeMacro = par("maxSizeMacro").longValue();

    // statistics
    numStreams = 0;
    numPkSent = 0;
    reqStreamBytesSignal = registerSignal("reqStreamBytes");
    sentPkSignal = registerSignal("sentPk");

    trace.clear();
    std::string fileName(par("traceFileName").stringValue());
    if (!fileName.empty())
        fileParser(fileName.c_str());

    WATCH_PTRVECTOR(streamVector);

    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
}

void UDPVideoStreamSvr2::finish()
{
}

void UDPVideoStreamSvr2::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // timer for a particular video stream expired, send packet
        sendStreamData(msg);
    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        // start streaming
        processStreamRequest(msg);
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

void UDPVideoStreamSvr2::processStreamRequest(cMessage *msg)
{
    // register video stream...
    UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(msg->getControlInfo());

    for (unsigned int i = 0 ; i < streamVector.size(); i++)
    {
        if (streamVector[i]->clientAddr == ctrl->getSrcAddr() && streamVector[i]->clientPort == ctrl->getSrcPort())
        {
            if (streamVector[i]->bytesLeft > 0)
            {
                delete msg;
                return;
            }
        }
    }

    VideoStreamData *d = new VideoStreamData;
    d->clientAddr = ctrl->getSrcAddr();
    d->clientPort = ctrl->getSrcPort();
    d->videoSize = (*videoSize);
    d->bytesLeft = d->videoSize;
    d->traceIndex = 0;
    d->timeInit = simTime();
    d->fileTrace = false;
    double stop = (*stopTime);
    if (stop > 0)
        d->stopTime = simTime() + stop;
    else
        d->stopTime = 0;
    d->numPkSent = 0;

    if (!trace.empty())
        d->fileTrace = true;

    streamVector.push_back(d);
    delete msg;

    cMessage *timer = new cMessage("VideoStreamTmr");
    timer->setContextPointer(d);

    // ... then transmit first packet right away
    sendStreamData(timer);

    numStreams++;
    emit(reqStreamBytesSignal, d->videoSize);
}

void UDPVideoStreamSvr2::sendStreamData(cMessage *timer)
{
    bool deleteTimer = false;
    VideoStreamData *d = (VideoStreamData *) timer->getContextPointer();

    // generate and send a packet

    if (d->stopTime > 0 && d->stopTime < simTime())
    {
        for (unsigned int i = 0 ; i < streamVector.size(); i++)
        {
            if (d == streamVector[i])
            {
                streamVector.erase(streamVector.begin()+i);
                delete d;
                break;
            }
        }
        cancelAndDelete(timer);
        return;
    }

    UDPVideoDataPacket *pkt = new UDPVideoDataPacket("VideoStrmPk");
    if (!d->fileTrace)
    {
        long pktLen = packetLen->longValue();

        if (pktLen > d->bytesLeft)
            pktLen = d->bytesLeft;

        pkt->setVideoSize(d->videoSize);
        pkt->setBytesLeft(d->bytesLeft);
        pkt->setNumPkSent(d->numPkSent);

        pkt->setByteLength(pktLen);

        d->bytesLeft -= pktLen;
        d->numPkSent++;
        numPkSent++;
        // reschedule timer if there's bytes left to send
        if (d->bytesLeft != 0)
        {
            simtime_t interval = (*sendInterval);
            scheduleAt(simTime()+interval, timer);
        }
        else
        {
            deleteTimer = true;
        }
    }
    else
    {
        if (macroPackets)
        {
            simtime_t tm;
            uint64_t size = 0;
            VideoPacket *videopk = NULL;
            std::vector<VideoPacket *> macroPkt;
            do{
                VideoPacket *videopk = new VideoPacket();
                videopk->setBitLength(trace[d->traceIndex].size);
                videopk->setType(trace[d->traceIndex].type);
                videopk->setSeqNum(trace[d->traceIndex].seqNum);
                size += videopk->getByteLength();
                macroPkt.push_back(videopk);
                d->traceIndex++;
            } while(size + trace[d->traceIndex].size/8 < maxSizeMacro);
            videopk = NULL;

            while(!macroPkt.empty())
            {
                VideoPacket *videopkaux = macroPkt.back();
                macroPkt.pop_back();
                if (videopk)
                    videopkaux->encapsulate(videopk);
                videopk = videopkaux;
            }

            pkt->setVideoSize(videopk->getByteLength());
            pkt->encapsulate(videopk);
        }
        else
        {
            VideoPacket *videopk = new VideoPacket();

            videopk->setBitLength(trace[d->traceIndex].size);
            videopk->setType(trace[d->traceIndex].type);
            videopk->setSeqNum(trace[d->traceIndex].seqNum);

            pkt->setVideoSize(trace[d->traceIndex].size/8);
            pkt->encapsulate(videopk);
            d->traceIndex++;
        }
        if (d->traceIndex >= trace.size())
            deleteTimer = true;
        else
            scheduleAt(d->timeInit + trace[d->traceIndex].timeFrame, timer);

    }
    emit(sentPkSignal, pkt);
    socket.sendTo(pkt, d->clientAddr, d->clientPort);

    if (deleteTimer)
    {
        for (unsigned int i = 0 ; i < streamVector.size(); i++)
        {
            if (d == streamVector[i])
            {
                streamVector.erase(streamVector.begin()+i);
                delete d;
                break;
            }
        }
        cancelAndDelete(timer);
    }
}

