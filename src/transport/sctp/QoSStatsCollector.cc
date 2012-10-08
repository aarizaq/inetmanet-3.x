//
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "QoSStatsCollector.h"

#include <assert.h>
#include <iostream>


// ###### Constructor #######################################################
QoSStats::QoSStats(cModule*                             module,
                   const std::string&                   name,
                   const enum QoSStats::QoSStatsVariant variant)
{
   Name             = name;
   Variant          = variant;
   TrackingInterval = 1.0;
   ValueSum         = 0;
   cOutVector::setName(name.c_str());
   cOutVector::setUnit("s");
}


// ###### Destructor ########################################################
QoSStats::~QoSStats()
{
   QoSStatsEntryType::iterator first = QoSStatsSet.begin();
   while(first != QoSStatsSet.end()) {
      delete *first;
      QoSStatsSet.erase(first);
      first = QoSStatsSet.begin();
   }
}


// ###### Purge out-of-date values ##########################################
void QoSStats::purge(const simtime_t& now)
{
   QoSStatsEntryType::iterator first = QoSStatsSet.begin();
   while(first != QoSStatsSet.end()) {

      const QoSStatsEntry* statEntry = *first;
      if(now - statEntry->TimeStamp <= TrackingInterval) {
         break;
      }
      assert(ValueSum >=  statEntry->Value);
      ValueSum -= statEntry->Value;
      delete statEntry;
      QoSStatsSet.erase(first);

      first = QoSStatsSet.begin();
   }
}


// ###### Dump storage ######################################################
void QoSStats::dump(std::ostream& os) const
{
   unsigned long long valueSum = 0;
   os << Name << " (ValueSum=" << ValueSum << "):" << endl;
   for(QoSStatsEntryType::iterator iterator = QoSStatsSet.begin();
       iterator != QoSStatsSet.end(); iterator++) {
      const QoSStatsEntry* statEntry = *iterator;
      valueSum += statEntry->Value;
      os << " - " << statEntry->TimeStamp << "\t" << statEntry->Value << endl;
   }
   assert(ValueSum == valueSum);
}

// ###### Add value #########################################################
void QoSStats::add(const simtime_t& now, const unsigned int value)
{
   // ====== Get rid of out-of-date entries =================================
   purge(now);

   // ====== Add new entry ==================================================
   QoSStatsEntry* statEntry = new QoSStatsEntry;
   statEntry->TimeStamp = now;
   statEntry->Value     = value;
   QoSStatsSet.insert(statEntry);
   ValueSum += statEntry->Value;

   // ====== Record rate or mean ============================================
   double result;
   switch(Variant) {
      case QSV_Rate:
         result = getRate();
       break;
      case QSV_Mean:
         result = getMean();
       break;
      default:
         abort();
       break;
   }
   record(result);
}


// ###### Constructor #######################################################
QoSStatsCollector::QoSStatsCollector()
{
   HasBeenDumped        = false;
   IsActive             = false;
   TrackingInterval     = 1.0;

   EnqueuingStats       = NULL;
   DequeuingStats       = NULL;
   TransmissionStats    = NULL;
   AcknowledgementStats = NULL;
   DropStats            = NULL;
}


// ###### Destructor ########################################################
QoSStatsCollector::~QoSStatsCollector()
{
   if(!HasBeenDumped) {
      dumpScalars();
   }
   clear();
}


// ###### Activate QoS tracking #############################################
void QoSStatsCollector::activate(const simtime_t& trackingInterval)
{
   IsActive = true;
   if(trackingInterval > 0.0) {
      TrackingInterval = trackingInterval;
   }
}


// ###### Deactivate QoS tracking ###########################################
void QoSStatsCollector::deactivate()
{
   IsActive = false;
}


// ###### Record action #####################################################
void QoSStatsCollector::recordEnqueuing(const uint32     length,
                                        const simtime_t& enqueuingTime)
{
   if((IsActive) && (EnqueuingStats)) {
      EnqueuingStats->add(enqueuingTime, length);
   }
}


// ###### Record action ######################################################
void QoSStatsCollector::recordTransmission(const uint32     tsn,
                                           const uint32     length,
                                           const simtime_t& transmissionTime)
{
   if((IsActive) && (TransmissionStats)) {
      TransmissionStats->add(transmissionTime, length);
   }
}


// ###### Record action ######################################################
void QoSStatsCollector::recordAcknowledgement(const uint32     tsn,
                                              const uint32     length,
                                              const simtime_t& transmissionTime,
                                              const simtime_t& acknowledgementTime)
{
   if(IsActive) {
      if(AcknowledgementStats) {
         AcknowledgementStats->add(acknowledgementTime, length);
      }
      if(TxToAckStats) {
         const unsigned int txToAckTime =
            (unsigned int)rint(1000000.0 * (acknowledgementTime - transmissionTime).dbl());
         TxToAckStats->add(acknowledgementTime, txToAckTime);
      }
   }
}


// ###### Record action ######################################################
void QoSStatsCollector::recordDequeuing(const uint32     length,
                                        const simtime_t& enqueuingTime,
                                        const simtime_t& dequeuingTime)
{
   if((IsActive) && (DequeuingStats)) {
      DequeuingStats->add(dequeuingTime, length);
   }
}


// ###### Record action ######################################################
void QoSStatsCollector::recordDrop(const uint32     tsn,
                                   const uint32     length,
                                   const simtime_t& transmissionTime,
                                   const simtime_t& dropTime)
{
   if(IsActive) {
      // ...
   }
}


// ###### Initialize ########################################################
void QoSStatsCollector::initialize(cModule* module, const char* name)
{
   HasBeenInitialized   = false;
   Parent               = module;
   Name                 = std::string((name != NULL) ? name : "");

   EnqueuingStats       = new QoSStats(module, "Enqueuing Statistics " + Name);
   DequeuingStats       = new QoSStats(module, "Dequeuing Statistics " + Name);
   TransmissionStats    = new QoSStats(module, "Transmission Statistics " + Name);
   AcknowledgementStats = new QoSStats(module, "Acknowledgement Statistics " + Name);
   TxToAckStats         = new QoSStats(module, "TxToAck Statistics " + Name, QoSStats::QSV_Mean);
   DropStats            = new QoSStats(module, "Drop Statistics " + Name);
}


// ###### Clear all statistics ##############################################
void QoSStatsCollector::clear()
{
   if(EnqueuingStats) {
      delete EnqueuingStats;
   }
   if(DequeuingStats) {
      delete DequeuingStats;
   }
   if(TransmissionStats) {
      delete TransmissionStats;
   }
   if(AcknowledgementStats) {
      delete AcknowledgementStats;
   }
   if(TxToAckStats) {
      delete TxToAckStats;
   }
   if(DropStats) {
      delete DropStats;
   }
   EnqueuingStats       = NULL;
   DequeuingStats       = NULL;
   TransmissionStats    = NULL;
   AcknowledgementStats = NULL;
   DropStats            = NULL;
}


// ###### Dump statistics scalars ###########################################
void QoSStatsCollector::dumpScalars()
{
   HasBeenDumped = true;
}
