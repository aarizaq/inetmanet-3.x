//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "SCTPAssociation.h"


// #define sctpEV3 std::cout


static inline double GET_SRTT(const double srtt)
{
   return(floor(1000.0 * srtt * 8.0));
}




void SCTPAssociation::cwndUpdateBeforeSack()
{
}




void SCTPAssociation::recordCwndUpdate(SCTPPathVariables* path)
{
   if(path == NULL) {
      uint32 totalSsthresh  = 0.0;
      uint32 totalCwnd      = 0.0;
      double totalBandwidth = 0.0;
      for (SCTPPathMap::iterator pathIterator = sctpPathMap.begin();
         pathIterator != sctpPathMap.end(); pathIterator++) {
         SCTPPathVariables* path = pathIterator->second;
         totalSsthresh  += path->ssthresh;
         totalCwnd      += path->cwnd;
         totalBandwidth += path->cwnd / GET_SRTT(path->srtt.dbl());
      }
      statisticsTotalSSthresh->recordWithLimit(totalSsthresh, 1000000000);
      statisticsTotalCwnd->recordWithLimit(totalCwnd, 1000000000);
      statisticsTotalBandwidth->record(totalBandwidth);
   }
   else {
      path->statisticsPathSSthresh->recordWithLimit(path->ssthresh, 1000000000);
      path->statisticsPathCwnd->recordWithLimit(path->cwnd, 1000000000);
      path->statisticsPathBandwidth->record(path->cwnd / GET_SRTT(path->srtt.dbl()));
   }
}


uint32 SCTPAssociation::getInitialCwnd(const SCTPPathVariables* path) const
{
   uint32 newCwnd;
   newCwnd = max(2 * path->pmtu, 4380);
   return(newCwnd);
}


void SCTPAssociation::initCCParameters(SCTPPathVariables* path)
{
   path->cwnd     = getInitialCwnd(path);
   path->ssthresh = state->peerRwnd;
   recordCwndUpdate(path);

   sctpEV3 << assocId << ": " << simTime() << ":\tCC [initCCParameters]\t" << path->remoteAddress
           << "\tsst="  << path->ssthresh
           << "\tcwnd=" << path->cwnd << endl;
}


int32 SCTPAssociation::rpPathBlockingControl(SCTPPathVariables* path, const double reduction)
{
   // ====== Compute new cwnd ===============================================
   const int32 newCwnd = (int32)ceil(path->cwnd - reduction);
   // NOTE: newCwnd may be negative!
   return(newCwnd);
}


void SCTPAssociation::cwndUpdateAfterSack()
{
   recordCwndUpdate(NULL);

   for (SCTPPathMap::iterator iter = sctpPathMap.begin(); iter != sctpPathMap.end(); iter++) {
      SCTPPathVariables* path = iter->second;
      if(path->fastRecoveryActive == false) {

         // ====== Retransmission required -> reduce congestion window ======
         if(path->requiresRtx) {
            double decreaseFactor = 0.5;
               path->ssthresh = max((int32)path->cwnd - (int32)rint(decreaseFactor * (double)path->cwnd),
                                    4 * (int32)path->pmtu);
               path->cwnd = path->ssthresh;

            recordCwndUpdate(path);
            path->partialBytesAcked = 0;

            path->vectorPathPbAcked->record(path->partialBytesAcked);
            sctpEV3 << "\t=>\tsst=" << path->ssthresh
                    << "\tcwnd=" << path->cwnd << endl;

            // ====== Fast Recovery =========================================
            if(state->fastRecoverySupported) {
               uint32 highestAckOnPath   = state->lastTsnAck;
               uint32 highestOutstanding = state->lastTsnAck;
               for(SCTPQueue::PayloadQueue::const_iterator chunkIterator = retransmissionQ->payloadQueue.begin();
                   chunkIterator != retransmissionQ->payloadQueue.end(); chunkIterator++) {
                   const SCTPDataVariables* chunk = chunkIterator->second;
                   if(chunk->getLastDestinationPath() == path) {
                      if(chunkHasBeenAcked(chunk)) {
                         if(tsnGt(chunk->tsn, highestAckOnPath)) {
                            highestAckOnPath = chunk->tsn;
                         }
                      }
                      else {
                         if(tsnGt(chunk->tsn, highestOutstanding)) {
                            highestOutstanding = chunk->tsn;
                         }
                      }
                   }
               }
               // This can ONLY become TRUE, when Fast Recovery IS supported.
               path->fastRecoveryActive       = true;
               path->fastRecoveryExitPoint    = highestOutstanding;
               path->fastRecoveryEnteringTime = simTime();
               path->vectorPathFastRecoveryState->record(path->cwnd);

               sctpEV3 << assocId << ": " << simTime() << ":\tCC [cwndUpdateAfterSack] Entering Fast Recovery on path "
                       << path->remoteAddress
                       << ", exit point is " << path->fastRecoveryExitPoint
                       << endl;
            }
         }
      }
      else {
         for (SCTPPathMap::iterator iter = sctpPathMap.begin(); iter != sctpPathMap.end(); iter++) {
            SCTPPathVariables* path = iter->second;
            if(path->fastRecoveryActive) {
               sctpEV3 << assocId << ": " << simTime() << ":\tCC [cwndUpdateAfterSack] Still in Fast Recovery on path "
                       << path->remoteAddress
                       << ", exit point is " << path->fastRecoveryExitPoint << endl;
            }
         }
      }
   }
}


void SCTPAssociation::cwndUpdateAfterRtxTimeout(SCTPPathVariables* path)
{
   cwndUpdateBeforeSack();

   double decreaseFactor = 0.5;
      path->ssthresh = max((int32)path->cwnd - (int32)rint(decreaseFactor * (double)path->cwnd),
                           4 * (int32)path->pmtu);
      path->cwnd     = path->pmtu;
   path->partialBytesAcked       = 0;
   path->vectorPathPbAcked->record(path->partialBytesAcked);
   sctpEV3 << "\t=>\tsst=" << path->ssthresh
           << "\tcwnd="    << path->cwnd << endl;
   recordCwndUpdate(path);

   // Leave Fast Recovery mode
   if (path->fastRecoveryActive == true) {
      path->fastRecoveryActive    = false;
      path->fastRecoveryExitPoint = 0;
      path->vectorPathFastRecoveryState->record(0);
   }
}


void SCTPAssociation::cwndUpdateBytesAcked(SCTPPathVariables* path,
                                           const uint32       ackedBytes,
                                           const bool         ctsnaAdvanced)
{
   if (path->fastRecoveryActive == false) {
      // T.D. 21.11.09: Increasing cwnd is only allowed when not being in
      //                Fast Recovery mode!

      // ====== Slow Start ==================================================
      if (path->cwnd <= path->ssthresh)  {
         // ------ Clear PartialBytesAcked counter --------------------------
         // uint32 oldPartialBytesAcked = path->partialBytesAcked;
         path->partialBytesAcked = 0;

         // ------ Increase Congestion Window -------------------------------
         if ((ctsnaAdvanced == true) &&
             (path->outstandingBytesBeforeUpdate >= path->cwnd)) {
               path->cwnd += (int32)min(path->pmtu, ackedBytes);

            recordCwndUpdate(path);
         }
         // ------ No need to increase Congestion Window --------------------
         else {
            sctpEV3 << assocId << ": " << simTime() << ":\tCC "
                    << "Not increasing cwnd of path " << path->remoteAddress << " in slow start:\t"
                    << "ctsnaAdvanced="       << ((ctsnaAdvanced == true) ? "yes" : "no") << "\t"
                    << "cwnd="                << path->cwnd                         << "\t"
                    << "sst="                 << path->ssthresh                     << "\t"
                    << "ackedBytes="          << ackedBytes                         << "\t"
                    << "pathOsbBeforeUpdate=" << path->outstandingBytesBeforeUpdate << "\t"
                    << "pathOsb="             << path->outstandingBytes             << "\t"
                    << "(pathOsbBeforeUpdate >= path->cwnd)="
                    << (path->outstandingBytesBeforeUpdate >= path->cwnd) << endl;
         }
      }

      // ====== Congestion Avoidance ========================================
      else
      {
         // ------ Increase PartialBytesAcked counter -----------------------
         path->partialBytesAcked += ackedBytes;

         // ------ Increase Congestion Window -------------------------------
         double increaseFactor = 1.0;
         if ( (path->partialBytesAcked >= path->cwnd) &&
              (ctsnaAdvanced == true) &&
              (path->outstandingBytesBeforeUpdate >= path->cwnd) ) {
               path->cwnd += (int32)rint(increaseFactor * path->pmtu);
            recordCwndUpdate(path);
            path->partialBytesAcked =
               ((path->cwnd < path->partialBytesAcked) ?
                  (path->partialBytesAcked - path->cwnd) : 0);
         }
         // ------ No need to increase Congestion Window -------------------
         else {
            sctpEV3 << assocId << ": " << simTime() << ":\tCC "
                    << "Not increasing cwnd of path " << path->remoteAddress << " in congestion avoidance: "
                    << "ctsnaAdvanced="       << ((ctsnaAdvanced == true) ? "yes" : "no") << "\t"
                    << "cwnd="                << path->cwnd                         << "\t"
                    << "sst="                 << path->ssthresh                     << "\t"
                    << "ackedBytes="          << ackedBytes                         << "\t"
                    << "pathOsbBeforeUpdate=" << path->outstandingBytesBeforeUpdate << "\t"
                    << "pathOsb="             << path->outstandingBytes             << "\t"
                    << "(pathOsbBeforeUpdate >= path->cwnd)="
                    << (path->outstandingBytesBeforeUpdate >= path->cwnd)           << "\t"
                    << "partialBytesAcked="   << path->partialBytesAcked            << "\t"
                    << "(path->partialBytesAcked >= path->cwnd)="
                    << (path->partialBytesAcked >= path->cwnd) << endl;
         }
      }

      // ====== Reset PartialBytesAcked counter if no more outstanding bytes
      if(path->outstandingBytes == 0) {
         path->partialBytesAcked = 0;
      }
      path->vectorPathPbAcked->record(path->partialBytesAcked);
   }
   else {
      sctpEV3 << assocId << ": " << simTime() << ":\tCC "
              << "Not increasing cwnd of path " << path->remoteAddress
              << " during Fast Recovery" << endl;
   }
}


void SCTPAssociation::updateFastRecoveryStatus(const uint32 lastTsnAck)
{
   for (SCTPPathMap::iterator iter = sctpPathMap.begin(); iter != sctpPathMap.end(); iter++) {
      SCTPPathVariables* path = iter->second;

      if (path->fastRecoveryActive) {
         if ( (tsnGt(lastTsnAck, path->fastRecoveryExitPoint)) ||
              (lastTsnAck == path->fastRecoveryExitPoint)
         ) {
            path->fastRecoveryActive    = false;
            path->fastRecoveryExitPoint = 0;
            path->vectorPathFastRecoveryState->record(0);

            sctpEV3 << assocId << ": " << simTime() << ":\tCC [cwndUpdateAfterSack] Leaving Fast Recovery on path "
                    << path->remoteAddress
                    << ", lastTsnAck=" << lastTsnAck
                    << endl;
         }
      }
   }
}


void SCTPAssociation::cwndUpdateMaxBurst(SCTPPathVariables* path)
{

      // ====== cwnd allows to send more than the maximum burst size ========
      if(path->cwnd > ((path->outstandingBytes + state->maxBurst * path->pmtu))) {
         sctpEV3 << assocId << ": " << simTime()   << ":\tCC [cwndUpdateMaxBurst]\t"
                 << path->remoteAddress
                 << "\tsst="      << path->ssthresh
                 << "\tcwnd="     << path->cwnd
                 << "\tosb="      << path->outstandingBytes
                 << "\tmaxBurst=" << state->maxBurst * path->pmtu;

         // ====== Update cwnd ==============================================
         path->cwnd = path->outstandingBytes + (state->maxBurst * path->pmtu);
         recordCwndUpdate(path);

         sctpEV3 << "\t=>\tsst="  << path->ssthresh
                 << "\tcwnd="     << path->cwnd
                 << endl;
      }
}


void SCTPAssociation::cwndUpdateAfterCwndTimeout(SCTPPathVariables* path)
{
   // When the association does not transmit data on a given transport address
   // within an RTO, the cwnd of the transport address SHOULD be adjusted to 2*MTU.
   sctpEV3 << assocId << ": " << simTime() << ":\tCC [cwndUpdateAfterCwndTimeout]\t" << path->remoteAddress
           << "\tsst="  << path->ssthresh
           << "\tcwnd=" << path->cwnd;
   path->cwnd = getInitialCwnd(path);
   sctpEV3 << "\t=>\tsst=" << path->ssthresh
           << "\tcwnd=" << path->cwnd << endl;
   recordCwndUpdate(path);
   recordCwndUpdate(NULL);
}
