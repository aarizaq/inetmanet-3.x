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

#ifndef CHUNKMAP_H
#define CHUNKMAP_H

#include <omnetpp.h>
#include <assert.h>


struct SCTPDataVariables;


// ====== Entry type definition  ============================================
// ~~~~~~ 64-bit entry type ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// #define MAP_ENTRY_TYPE       unsigned long long
// ~~~~~~ 32-bit entry type ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define MAP_ENTRY_TYPE       uint32
// ~~~~~~ 8-bit entry type ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// #define MAP_ENTRY_TYPE       unsigned char
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define MAP_ENTRY_BYTESIZE   (unsigned int)sizeof(MAP_ENTRY_TYPE)
#define MAP_ENTRY_BITSIZE    (8 * MAP_ENTRY_BYTESIZE)


// ====== Helper functions to map relative TSN to entry and bit numbers =====
#define GET_ENTRY(relTSN) ((unsigned int)((uint32)relTSN / (uint32)MAP_ENTRY_BITSIZE))
#define GET_BIT(relTSN)   ((unsigned int)((uint32)relTSN & ((uint32)MAP_ENTRY_BITSIZE - 1)))


class ChunkMap
{
   public:
   ChunkMap(const unsigned int initialSize, const unsigned int maxSize);
   ~ChunkMap();

   void dump();
   void cumAck(const uint32 cumAckTSN);

   inline void ack(const uint32 tsn)   { ackOrUnack(tsn, true);  }
   inline void unack(const uint32 tsn) { ackOrUnack(tsn, false); }

   inline SCTPDataVariables* getChunk(const uint32 tsn) const {
      const unsigned int relTSN = (tsn - MapBaseTSN);
      assert(tsn >= MapBaseTSN);
      assert(relTSN < TSNEntries);
      return(TSNArray[relTSN]);
   }

   inline void setChunk(const uint32 tsn, SCTPDataVariables* chunk) {
      const unsigned int relTSN = (tsn - MapBaseTSN);
      assert(tsn >= MapBaseTSN);
      if(relTSN >= TSNEntries) {
         grow(tsn);
      }
      assert(relTSN < TSNEntries);
      TSNArray[relTSN] = chunk;
   }

   inline void ackOrUnack(const uint32 tsn, const bool ack) {
      const unsigned int relTSN = (tsn - MapBaseTSN);
      const unsigned int entry  = GET_ENTRY(relTSN);
      const unsigned int bit    = GET_BIT(relTSN);
      assert(tsn >= MapBaseTSN);
      if(entry >= MapEntries) {
         grow(tsn - MapBaseTSN);
         assert(entry < MapEntries);
      }
      if(ack) {
         MapArray[entry] |= ((MAP_ENTRY_TYPE)1 << bit);
      }
      else {
         MapArray[entry] &= ~((MAP_ENTRY_TYPE)1 << bit);
      }
   }

   inline bool isAcked(const uint32 tsn) const {
      const unsigned int relTSN = (tsn - MapBaseTSN);
      const unsigned int entry  = GET_ENTRY(relTSN);
      const unsigned int bit    = GET_BIT(relTSN);
      assert(tsn >= MapBaseTSN);
      assert(entry < MapEntries);
      return(MapArray[entry] & ((MAP_ENTRY_TYPE)1 << bit));
   }


   // ====== Private data ===================================================
   private:
   void grow(const uint32 highestTSN);
   void shift(const uint32 tsnDifference);

   unsigned int        MapEntries;
   uint32              MapBaseTSN;
   MAP_ENTRY_TYPE*     MapArray;

   unsigned int        TSNEntries;
   SCTPDataVariables** TSNArray;

   unsigned int        MaxEntries;
   unsigned int        CumAckShiftThreshold;
};

#endif
