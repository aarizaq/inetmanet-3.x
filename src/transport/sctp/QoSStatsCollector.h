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

#ifndef QOSSTATSCOLLECTOR_H
#define QOSSTATSCOLLECTOR_H

#include <iostream>
#include <omnetpp.h>


class QoSStats : public cOutVector
{
   // ====== Public methods =================================================
   public:
   enum QoSStatsVariant {
      QSV_Rate = 1,
      QSV_Mean = 2
   };

   QoSStats(cModule*                   module,
            const std::string&         name,
            const enum QoSStatsVariant variant = QSV_Rate);
   virtual ~QoSStats();

   inline simtime_t getTrackingInterval() const {
      return(TrackingInterval);
   }
   inline void setTrackingInterval(const simtime_t& trackingInterval) {
      TrackingInterval = trackingInterval;
   }

   inline double getRate() const {
      const QoSStatsEntryType::iterator first = QoSStatsSet.begin();
      QoSStatsEntryType::iterator       last  = QoSStatsSet.end();
      if(first != last) {
         last--;   // There must be at least one element ...
         if(first != last) {   // There are at least two elements ...
            return(ValueSum / (double)((*last)->TimeStamp - (*first)->TimeStamp).dbl());
         }
      }
      return(0.0);
   }
   inline double getMean() const {
      const QoSStatsEntryType::iterator first = QoSStatsSet.begin();
      QoSStatsEntryType::iterator       last  = QoSStatsSet.end();
      if(first != last) {
         last--;   // There must be at least one element ...
         if(first != last) {   // There are at least two elements ...
            return(ValueSum / (double)QoSStatsSet.size());
         }
      }
      return(0.0);
   }

   void add(const simtime_t& now, const unsigned int value);
   void dump(std::ostream& os) const;

   // ====== Private data ===================================================
   private:
   struct QoSStatsEntry {
      simtime_t    TimeStamp;
      unsigned int Value;
   };
   struct QoSStatsEntryLess
   {
      inline bool operator()(const QoSStatsEntry* a, const QoSStatsEntry* b) const {
         return(a->TimeStamp <= b->TimeStamp);
      }
   };
   typedef std::set<QoSStatsEntry*, QoSStatsEntryLess> QoSStatsEntryType;

   void purge(const simtime_t& now);

   simtime_t            TrackingInterval;
   unsigned long long   ValueSum;
   QoSStatsEntryType    QoSStatsSet;
   std::string          Name;
   enum QoSStatsVariant Variant;
};


/*
   Actions to record:
   1. Chunk Enqueuing       (tsn, enqueuingTime, length)
   2. Chunk Dequeuing       (tsn, enqueuingTime, dequeuingTime, length)
   3. Chunk Transmission    (tsn, transmissionTime, length)
   4. Chunk Acknowledgement (tsn, transmissionTime, ackTime, length)
   5. Chunk Drop            (tsn, transmissionTime, dropTime, length)

   Stream QoS:
   Enqueuing Rate, Dequeuing Rate

   Path QoS:
   Enqueuing Rate ("scheduling rate"), Dequeuing Rate ("completion rate")
   Transmission Rate, Acknowledgement Rate, Drop Rate
*/

class QoSStatsCollector
{
   public:
   QoSStatsCollector();
   ~QoSStatsCollector();

   virtual void initialize(cModule* module = NULL, const char* name = NULL);
   virtual void clear();
   virtual void dumpScalars();

   inline bool isActive() const {
      return(IsActive);
   }
   void activate(const simtime_t& trackingInterval = -1.0);
   void deactivate();

   void recordEnqueuing(const uint32     length,
                        const simtime_t& enqueuingTime);
   void recordTransmission(const uint32     tsn,
                           const uint32     length,
                           const simtime_t& transmissionTime);
   void recordAcknowledgement(const uint32     tsn,
                              const uint32     length,
                              const simtime_t& transmissionTime,
                              const simtime_t& acknowledgementTime);
   void recordDequeuing(const uint32     length,
                        const simtime_t& enqueuingTime,
                        const simtime_t& dequeuingTime);
   void recordDrop(const uint32     tsn,
                   const uint32     length,
                   const simtime_t& transmissionTime,
                   const simtime_t& dropTime);

   // ====== Public data ====================================================
   public:
   QoSStats*   EnqueuingStats;           // Enqueuing (rate -> bytes/s)
   QoSStats*   DequeuingStats;           // Dequeuing (rate -> bytes/s)
   QoSStats*   TransmissionStats;        // Data Transmission (rate -> bytes/s)
   QoSStats*   AcknowledgementStats;     // Data Acknowledgement (rate -> bytes/s)
   QoSStats*   TxToAckStats;             // Delay from transmission to acknowledgement (mean -> microseconds)
   QoSStats*   DropStats;

   // ====== Private data ===================================================
   private:
   cModule*    Parent;
   std::string Name;
   simtime_t   TrackingInterval;
   bool        IsActive;
   bool        HasBeenInitialized;
   bool        HasBeenDumped;
};

#endif
