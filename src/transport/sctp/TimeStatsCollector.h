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

#ifndef TIMESTATSCOLLECTOR_H
#define TIMESTATSCOLLECTOR_H

#include <omnetpp.h>


class TimeStatsCollector : public cWeightedStdDev,
                           public cOutVector
{
   public:
   TimeStatsCollector(cModule* module = NULL, const char* name = NULL, const char* unit = NULL);
   ~TimeStatsCollector();

   virtual void initialize(cModule* module = NULL, const char* name = NULL, const char* unit = NULL);
   virtual void dumpScalars();
   void setUtilizationMaximum(const double maximum);

   inline void update() {
      if(HasBeenInitialized) {
         const simtime_t timeDifference = simTime() - LastUpdate;
         if (timeDifference > 0)
         	collect2(Value, timeDifference);
      }
      LastUpdate = simTime();
   }

   virtual bool record(const double value);
   bool recordWithLimit(const double value, const double limit);

   // ====== Private data ===================================================
   private:
   simtime_t   LastUpdate;
   double      Value;
   bool        HasBeenInitialized;
   bool        HasBeenDumped;
   bool        DumpUtilization;
   std::string Name;
   std::string Unit;
   cModule*    Parent;
   double      UtilizationMaximum;
};

#endif
