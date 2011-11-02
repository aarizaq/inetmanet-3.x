/**
 * @short This class provides facilities to obtain snapshots of the energy
 *  available in the network
 * @author Isabel Dietrich
*/

#ifndef ANALYSIS_ENERGY_H
#define ANALYSIS_ENERGY_H

// SYSTEM INCLUDES
#include <omnetpp.h>
#include <vector>
#include <string>
#include <fstream>

#include "Coord.h"          // provides: struct for position coordinates
#include "Energy.h"         // provides: class for maintaining energy
#include "BasicBattery.h"   // provides: access to the node batteries
#include "ChannelControl.h" // provides: global position knowledge


class AnalysisEnergy : public cSimpleModule, protected cListener
{
  public:
    // LIFECYCLE

    virtual void    initialize(int);
    virtual void    finish();
    virtual int numInitStages(){return 4;};
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

    // OPERATIONS
    void            handleMessage(cMessage*);
    void            Snapshot();


  private:
    static simsignal_t mobilityStateChangedSignal;

    Coord   myCord;
    // OPERATIONS
    void            SnapshotEnergies();
    void            SnapshotLifetimes();

    // MEMBER VARIABLES
    /** Parameters */
    bool        mCoreDebug;
    int         mNumHosts;
    std::string mpHostModuleName;

    int         mNumHostsDepleted;

    /** Snapshot creation variables */
    cMessage*               mCreateSnapshot;


};

#endif
