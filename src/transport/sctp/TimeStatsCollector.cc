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

#include "TimeStatsCollector.h"


// ###### Constructor #######################################################
TimeStatsCollector::TimeStatsCollector(cModule* module, const char* name, const char* unit)
{
   initialize(module, name, unit);
   HasBeenDumped      = false;
   DumpUtilization    = false;
   UtilizationMaximum = 0.0;
}


// ###### Destructor ########################################################
TimeStatsCollector::~TimeStatsCollector()
{
   if(!HasBeenDumped) {
      dumpScalars();
   }
}


// ###### Initialize ########################################################
void TimeStatsCollector::initialize(cModule* module, const char* name, const char* unit)
{
   Parent             = module;
   Value              = 0.0;
   HasBeenInitialized = false;
   Name               = std::string((name != NULL) ? name : "");
   Unit               = std::string((unit != NULL) ? unit : "");
   cOutVector::setName(name);
   cOutVector::setUnit(unit);
}


// ###### Set maximum value for computing utilization scalars ###############
void TimeStatsCollector::setUtilizationMaximum(const double maximum)
{
   DumpUtilization    = true;
   UtilizationMaximum = maximum;
}


// ###### Record new value ##################################################
bool TimeStatsCollector::record(const double value)
{
   update();
   Value              = value;
   HasBeenInitialized = true;
   return(cOutVector::record(Value));
}


// ###### Record new value with limitation ##################################
bool TimeStatsCollector::recordWithLimit(const double value, const double limit)
{
   if(value < limit) {
      // Call TimeStatsCollector::record to record vector and statistics entries
      return(record(value));
   }
   else {
      LastUpdate = simTime();
      Value      = value;
      return(cOutVector::record(Value));   // Just record vector entry
   }
}


// ###### Dump statistics scalars ###########################################
void TimeStatsCollector::dumpScalars()
{
   update();
   if(getCount() > 0) {
      Parent->recordScalar(("Minimum " + Name).c_str(),
                           (getCount() > 0) ? getMin() : 0.0,
                           Unit.c_str());
      Parent->recordScalar(("Average " + Name).c_str(),
                           (getCount() > 0) ? getMean() : 0.0,
                           Unit.c_str());
      Parent->recordScalar(("Maximum " + Name).c_str(),
                           (getCount() > 0) ? getMax() : 0.0,
                           Unit.c_str());
      Parent->recordScalar(("StdDev of " + Name).c_str(),
                           (getCount() > 0) ? getStddev() : 0.0,
                           Unit.c_str());

      if(DumpUtilization) {
         Parent->recordScalar(("Utilization Minimum " + Name).c_str(),
                              (getCount() > 0) ? (getMin() / UtilizationMaximum) : 0.0,
                              Unit.c_str());
         Parent->recordScalar(("Utilization Average " + Name).c_str(),
                              (getCount() > 0) ? (getMean() / UtilizationMaximum) : 0.0,
                              Unit.c_str());
         Parent->recordScalar(("Utilization Maximum " + Name).c_str(),
                              (getCount() > 0) ? (getMax() / UtilizationMaximum) : 0.0,
                              Unit.c_str());
      }
   }
   HasBeenDumped = true;
}
