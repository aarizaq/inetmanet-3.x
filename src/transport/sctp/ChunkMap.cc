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

#include "ChunkMap.h"

#include <iostream>


// ###### Constructor #######################################################
ChunkMap::ChunkMap(const unsigned int initialSize, const unsigned int maxSize)
{
   // ====== Set performance tuning parameters ==============================
   MaxEntries           = maxSize / MAP_ENTRY_BITSIZE;
   CumAckShiftThreshold = (initialSize  / MAP_ENTRY_BITSIZE) / 2;

   // ====== Create TSN map =================================================
   MapBaseTSN           = 0;
   MapEntries           = initialSize / MAP_ENTRY_BITSIZE;
   MapArray             = new MAP_ENTRY_TYPE[MapEntries];
   assert(MapArray != NULL);
   for(unsigned int i = 0;i < MapEntries;i++) {
      MapArray[i] = 0;
   }
   TSNEntries = MapEntries * MAP_ENTRY_BITSIZE;
   TSNArray   = new SCTPDataVariables*[TSNEntries];
   assert(TSNArray != NULL);
   for(unsigned int i = 0;i < TSNEntries;i++) {
      TSNArray[i] = NULL;
   }
}


// ###### Destructor ########################################################
ChunkMap::~ChunkMap()
{
   delete [] MapArray;
   MapArray = NULL;
   delete [] TSNArray;
   TSNArray = NULL;
}


// ###### Print map #########################################################
void ChunkMap::dump()
{
   std::cout << "ChunkMap[baseTSN=" << MapBaseTSN << "]: ";
   for(unsigned int i = 0; i < MapEntries * MAP_ENTRY_BYTESIZE; i++) {
      if(isAcked(MapBaseTSN + i)) {
         std::cout << "<" << MapBaseTSN + i << "> ";
      }
   }
   std::cout << std::endl;
}


// ###### Grow mapping and TSN arrays to include given TSN ##################
void ChunkMap::grow(const uint32 highestTSN)
{
   // ====== Calculate new array size =======================================
   const unsigned int newRequiredNumberOfEntries = ((highestTSN - MapBaseTSN) + (MAP_ENTRY_BITSIZE - 1)) /
                                                      MAP_ENTRY_BITSIZE;
   if(newRequiredNumberOfEntries > MaxEntries) {
      std::cerr << "ERROR: ChunkMap::grow() - Required " << newRequiredNumberOfEntries
                << " entries, but hard limit is " << MaxEntries << "!" << std::endl;
      abort();
   }
   unsigned int newMapEntries = 2 * MapEntries;
   while(newMapEntries < newRequiredNumberOfEntries) {
      newMapEntries *= 2;
   }
   newMapEntries = std::min(newMapEntries, MaxEntries);
   const unsigned int newTSNEntries = newMapEntries * MAP_ENTRY_BITSIZE;
   assert(newTSNEntries > TSNEntries);

   // ====== Copy map entries into new array ================================
   MAP_ENTRY_TYPE* newMapArray = new MAP_ENTRY_TYPE[newMapEntries];
   assert(newMapArray != NULL);
   unsigned int i;
   for(i = 0; i < MapEntries; i++) {
      newMapArray[i] = MapArray[i];
   }
   for(     ; i < newMapEntries; i++) {
      newMapArray[i] = 0;
   }
   delete [] MapArray;
   MapArray = newMapArray;

   // ====== Copy TSN entries into new array ================================
   SCTPDataVariables** newTSNArray = new SCTPDataVariables*[newTSNEntries];
   assert(newTSNArray != NULL);
   for(i = 0; i < TSNEntries; i++) {
      newTSNArray[i] = TSNArray[i];
   }
   for(     ; i < newTSNEntries; i++) {
      newTSNArray[i] = NULL;
   }
   delete [] TSNArray;
   TSNArray = newTSNArray;

   MapEntries = newMapEntries;
   TSNEntries = newTSNEntries;
}


// ###### Shift mapping and TSN arrays to handle CumAck #####################
void ChunkMap::shift(const uint32 tsnDifference)
{
   // ====== Shift mapping array ============================================
   const unsigned int mapDifference = tsnDifference / MAP_ENTRY_BITSIZE;
   memmove((void*)&MapArray[0], (void*)&MapArray[mapDifference],
            (MapEntries - mapDifference) * MAP_ENTRY_BYTESIZE);
   memset((void*)&MapArray[MapEntries - mapDifference], 0,
            mapDifference * MAP_ENTRY_BYTESIZE);

   // ====== Shift TSN array ================================================
   memmove((void*)&TSNArray[0], (void*)&TSNArray[tsnDifference],
            (TSNEntries - tsnDifference) * sizeof(SCTPDataVariables*));
   memset((void*)&TSNArray[TSNEntries - tsnDifference], 0,
            tsnDifference * sizeof(SCTPDataVariables*));
}


// ###### CumAck TSN ########################################################
void ChunkMap::cumAck(const uint32 cumAckTSN)
{
   const uint32 newMapBaseTSN = cumAckTSN - (uint32)GET_BIT(cumAckTSN);
   const unsigned int tsnDifference = newMapBaseTSN - MapBaseTSN;

   // ====== Move array, if appropriate =====================================
   if(tsnDifference >= CumAckShiftThreshold) {
      shift(tsnDifference);
      MapBaseTSN = newMapBaseTSN;
   }

   // ====== CumAck TSNs which are still in the array =======================
   for(uint32 tsn = MapBaseTSN; tsn <= cumAckTSN; tsn++) {
      ack(tsn);
      TSNArray[tsn - MapBaseTSN] = NULL;
   }
}
