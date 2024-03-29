#include "TCPQueueTesterFunctions.h"

using namespace inet;
using namespace inet::tcp;

void enqueue(TCPMsgBasedSendQueue *sq, const char *msgname, ulong numBytes)
{
    EV << "SQ:" << "enqueue(\"" << msgname << "\", " << numBytes << "):";

    cPacket *msg = new cPacket(msgname);
    msg->setByteLength(numBytes);
    sq->enqueueAppData(msg);

    EV << " --> " << sq->str() <<"\n";
}

void tryenqueue(TCPMsgBasedSendQueue *sq, const char *msgname, ulong numBytes)
{
    EV << "SQ:" << "enqueue(\"" << msgname << "\", " << numBytes << "):";

    cPacket *msg = new cPacket(msgname);
    msg->setByteLength(numBytes);
    try {
    sq->enqueueAppData(msg);
    } catch (cRuntimeError& e) {
        delete msg;
        EV << " --> Error: " << e.what();
    }

    EV << " --> " << sq->str() <<"\n";
}

TCPSegment *createSegmentWithBytes(TCPMsgBasedSendQueue *sq, uint32 fromSeq, uint32 toSeq)
{
    EV << "SQ:" << "createSegmentWithBytes(" << fromSeq << ", " << toSeq << "):";

    ulong numBytes = toSeq-fromSeq;
    TCPSegment *tcpseg = sq->createSegmentWithBytes(fromSeq, numBytes);

    for (int i=0; i<tcpseg->getPayloadMsgArraySize(); i++)
    {
        const cPacket *payloadMsg = tcpseg->getPayloadMsg(i);
        uint32_t payloadSeq = tcpseg->getPayloadEndSequenceNo(i);
        uint32_t startSeq = payloadSeq - payloadMsg->getByteLength();
        EV << (i?", ":" ") << payloadMsg->getName() << '[' << startSeq << ".." << payloadSeq <<')';
    }
    EV << "\n";

    return tcpseg;
}

void discardUpTo(TCPMsgBasedSendQueue *sq, uint32 seqNum)
{
    EV << "SQ:" << "discardUpTo(" << seqNum << "): ";
    sq->discardUpTo(seqNum);

    EV << sq->str() <<"\n";
}

//////////////////////////////////////////////////////////////

void insertSegment(TCPMsgBasedRcvQueue *rq, TCPSegment *tcpseg)
{
    uint32_t beg = tcpseg->getSequenceNo();
    uint32_t end = beg + tcpseg->getPayloadLength();
    EV << "RQ:" << "insertSeg [" << beg << ".." << end << ")";
    uint32 rcv_nxt = rq->insertBytesFromSegment(tcpseg);
    delete tcpseg;
    EV << " --> " << rq->str() <<"\n";
}

void tryinsertSegment(TCPMsgBasedRcvQueue *rq, TCPSegment *tcpseg)
{
    uint32_t beg = tcpseg->getSequenceNo();
    uint32_t end = beg + tcpseg->getPayloadLength();
    EV << "RQ:" << "insertSeg [" << beg << ".." << end << ")";
    try {
        uint32 rcv_nxt = rq->insertBytesFromSegment(tcpseg);
    } catch (cRuntimeError& e) {
        EV << " --> Error: " << e.what();
    }
    delete tcpseg;

    EV << " --> " << rq->str() <<"\n";
}

void extractBytesUpTo(TCPMsgBasedRcvQueue *rq, uint32 seq)
{
    EV << "RQ:" << "extractUpTo(" << seq << "): <";
    cPacket *msg;
    while ((msg=rq->extractBytesUpTo(seq)) != NULL)
    {
        EV << " < " << msg->getName() << ": " << msg->getByteLength() << " bytes >";
        delete msg;
    }
    EV << " > --> " << rq->str() <<"\n";
}

/////////////////////////////////////////////////////////////////////////

void insertSegment(TCPVirtualDataRcvQueue *q, uint32 beg, uint32 end)
{
    EV << "RQ:" << "insertSeg [" << beg << ".." << end << ")";

    TCPSegment *tcpseg = new TCPSegment();
    tcpseg->setSequenceNo(beg);
    tcpseg->setPayloadLength(end-beg);
    uint32 rcv_nxt = q->insertBytesFromSegment(tcpseg);
    delete tcpseg;

    EV << " --> " << q->str() <<"\n";
}

void tryinsertSegment(TCPVirtualDataRcvQueue *q, uint32 beg, uint32 end)
{
    EV << "RQ:" << "insertSeg [" << beg << ".." << end << ")";
    TCPSegment *tcpseg = new TCPSegment();
    tcpseg->setSequenceNo(beg);
    tcpseg->setPayloadLength(end-beg);
    try {
        uint32 rcv_nxt = q->insertBytesFromSegment(tcpseg);
    } catch (cRuntimeError& e) {
        EV << " --> Error: " << e.what();
    }
    delete tcpseg;
    EV << " --> " << q->str() <<"\n";
}

void extractBytesUpTo(TCPVirtualDataRcvQueue *q, uint32 seq)
{
    EV << "RQ:" << "extractUpTo(" << seq << "):";
    cPacket *msg;
    while ((msg=q->extractBytesUpTo(seq))!=NULL)
    {
        EV << " msglen=" << msg->getByteLength();
        delete msg;
    }
    EV << " --> " << q->str() <<"\n";
}

