#pragma once
#ifndef __INET_PHASEDARRAY_H
#define __INET_PHASEDARRAY_H

#include <iostream>
#include "inet/physicallayer/base/packetlevel/AntennaBase.h"
#include "inet/physicallayer/antenna/Mempos.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/power/contract/IEnergySource.h"
#include "inet/power/contract/IEnergyStorage.h"
#include "inet/power/storage/SimpleEnergyStorage.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"

#include <vector>
namespace inet {

namespace physicallayer {

using std::cout;
extern int counter;
using namespace inet::power;

class INET_API PhasedArray : public AntennaBase, IEnergyConsumer, protected cListener
{



    public:
         static simsignal_t phaseArrayConfigureChange;
    protected:
    m length;
    double freq;
    double distance;
   mutable double  phiz;
    int sectorWidth;
    Mempos *mp;
    IRadio *radio;
    IEnergySource *energySource;
    // internal state
    int energyConsumerId;

    bool pendingConfiguration = false;
    double newConfigurtion = 0;

    W actualConsumption = W(0);


  protected:
    virtual void initialize(int stage) override;

  public:
    PhasedArray();


virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual m getLength() const { return length; }
    virtual double getMaxGain() const override {
        int numel = getNumAntennas();
        double maxGain = 10 * log10(numel);
        return maxGain;
     }

   virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) override;
   virtual void receiveSignal(cComponent *source, simsignal_t signalID, long d, cObject *details) override;

   virtual double getAngolo(Coord p1, Coord p2)const;
   virtual double computeGain(EulerAngles direction)const override;
   virtual double computePar(EulerAngles direction);
   virtual double getPhizero() {return phiz; }
   virtual Mempos *getMempos()const{return mp;}
   virtual int getSectorWidth()const {return sectorWidth;}
   virtual int getCurrentActiveSector()const;
   virtual bool isWithinSector (EulerAngles direction)const;
   virtual std::vector<int> getSectorVector()const;
   std::vector<int> getSectorVectorProva()const;
   virtual int getNumSectors()const;
   virtual int switchElements ();
   // Consumption methods
   virtual W getPowerConsumption() const override {return actualConsumption;}
   virtual void setConsumption()
   {
       if (energySource)
           energySource->setPowerConsumption(energyConsumerId,getPowerConsumption());
   }


};



} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_DIPOLEANTENNA_H
