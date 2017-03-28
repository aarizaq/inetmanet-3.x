#include "inet/physicallayer/antenna/PhasedArray.h"
#include "inet/physicallayer/base/packetlevel/AntennaBase.h"

#include "inet/common/ModuleAccess.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <istream>
#include <vector>
#include <complex>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <math.h>
namespace inet {

namespace physicallayer {
using std::cout;

int counter = 0;
//double PhasedArray::phiz=0;
simsignal_t PhasedArray::phaseArrayConfigureChange = registerSignal(
        "phaseArrayConfigureChange");

simsignal_t PhasedArray::triggerChange = registerSignal("triggerChange");;

Define_Module(PhasedArray);

PhasedArray::PhasedArray() :
        AntennaBase(), length(NaN),
        freq(1000000),
        distance (0.5),
        phiz(120),
        sectorWidth(45)

{
    cout<<" nodo  "<< counter << " creato " << endl;
    if (counter == 29) {

        std::list<double> lista;
        cout<<"Gains Vector[Gain(dB)/Angle(deg)]: "<<endl;

        for(double d=0.0; d<=6.28;d=d+0.01) {
            double m = computePar(EulerAngles(0,d,0));
            lista.push_back(m);
        }
        double index = 0.0;
        for (auto i = lista.begin(); i != lista.end(); ++i,index=index+0.01) {
            std::cout <<"["<<*i <<";"<<index *180/3.14<<"°"<<"]"<<" ";
        }
        cout<<"\n"<< endl;
        auto biggest = std::max_element(std::begin(lista), std::end(lista));
        cout<<"Main Lobe (deg): "<<phiz<<"  Max Gain (dB): " <<*biggest<<endl;
    }
    counter ++;
}

void PhasedArray::initialize(int stage) {
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        length = m(par("length"));
        freq = (par("freq"));
        distance = (par("distance"));
        phiz = (par("phiz"));
        sectorWidth = (par("sectorWidth"));
#ifdef REGISTER_IN_NODE
		// this code register in the node module
		cModule *mod = getContainingNode(this);
		mod->subscribe(phaseArrayConfigureChange, this);
#else
		// register in the interface module
        getParentModule()->getParentModule()->subscribe(phaseArrayConfigureChange, this);
		
#endif
        // cout << "Posizione: " << getMobility()->getCurrentPosition() << endl;

        cModule *radioModule = getParentModule();
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radioModule->subscribe(IRadio::receivedSignalPartChangedSignal, this);

        getSimulation()->getSystemModule()->subscribe(triggerChange, this);

        radio = check_and_cast<IRadio *>(radioModule);

        const char *energySourceModule = par("energySourceModule");

        energySource = dynamic_cast<IEnergySource *>(getModuleByPath(energySourceModule));
        if (energySource)
            energyConsumerId = energySource->addEnergyConsumer(this);
    }
}

double PhasedArray::computeGain(EulerAngles direction) const {
    getCurrentActiveSector();
       IRadio *radio= check_and_cast<IRadio *>(getParentModule());
       IRadioMedium *ra =  const_cast< IRadioMedium *> (radio->getMedium());
       RadioMedium *rm = dynamic_cast< RadioMedium *>(ra);

       phiz = -1;
      // phiz = rm->getMainAngle();



    if (phiz == -1) {
        // omni
        return 1;
    }

    //int numel = const_cast<PhasedArray*>(this)->switchElements();  //with energy control
    int numel = getNumAntennas(); //without energy control
    cout<<"ACTIVE ARRAY ELEMENTS:"<<numel<<endl;

    double c = 300000 * 10 * 10 * 10;
    double lambda = c / freq;
    double d = distance * lambda;

    double k = (2 * M_PI) / lambda;
    double phizero = phiz * (M_PI / 180);
    cout<<"PHI:"<<phiz<<endl;
    double psi = k * d * (cos(direction.beta) - cos(phizero));
    double num = sin((numel * psi) / 2);
    double den = sin(psi / 2);
    double rap = num / den;
    double mod = fabs(rap);
    double ef = cos(direction.beta);
    double efa = fabs(ef);
    double gain = 10 * log10(mod * efa);
    //if (phiz==0)gain=1;
    if (gain<0)gain=1; // if high losses , switch to omni
    cout<<"GAIN:"<<gain<<endl;
    return gain;

}



int PhasedArray::switchElements () {
    int numel;
    cModule *host = getContainingNode(this);
    IEnergyStorage *mod = check_and_cast<IEnergyStorage *>(host->getSubmodule("energyStorage"));
    if((mod->getResidualCapacity())< (mod->getNominalCapacity()*0.2) ) // if currentResidual< 20% nominalCapacity
    {numel=this->numAntennas -1;} //switch off one element
    else if((mod->getResidualCapacity())< (mod->getNominalCapacity()*0.3) ) // if currentResidual< 30% nominalCapacity
        {numel=this->numAntennas -2;} //switch off two elements
    else if((mod->getResidualCapacity())< (mod->getNominalCapacity()*0.4) ) // if currentResidual< 40% nominalCapacity
            {numel=this->numAntennas -3;} //switch off three elements
    else numel=getNumAntennas();
    cout<<"ACTIVE ARRAY ELEMENTS:"<<numel<<endl;
    return numel;

}

double PhasedArray::computePar(EulerAngles direction) {

    int numel = getNumAntennas();
    double c = 300000 * 10 * 10 * 10;
    double lambda = c / freq;
    double d = distance * lambda;

    double k = (2 * M_PI) / lambda;
    double phizero = phiz * (M_PI / 180);
    double psi = k * d * (cos(direction.beta) - cos(phizero));
    double num = sin((numel * psi) / 2);
    double den = sin(psi / 2);
    double rap = num / den;
    double mod = fabs(rap);
    double ef = cos(direction.beta);
    double efa = fabs(ef);
    double gain = 10 * log10(mod * efa);

    return gain;

}

double PhasedArray::getAngolo(Coord p1, Coord p2) const {
    double angolo;
    double cangl, intercept;
    double x1, y1, x2, y2;
    double dx, dy;

    x1 = p1.x;
    y1 = p1.y;
    x2 = p2.x;
    y2 = p2.y;
    dx = x2 - x1;
    dy = y2 - y1;
    cangl = dy / dx;
    angolo = atan(cangl) * (180 / 3.14);
    return angolo;

}

int PhasedArray::getNumSectors()const { // returns the plane sectorization size based on the width sector value
    switch (getSectorWidth()){
    case 45 : return 8;break;
    case 60 : return 6;break;
    case 90 : return 4;break;
    case 120 : return 3;break;
    case 180 : return 2;break;
    default : return -1;
    }

}
std::vector<int> PhasedArray::getSectorVector()const { // return the sectorizated vector based on the width secto value
    std::vector<int>sector;
    switch (getNumSectors()){
    case 8 :
            sector.resize(8);
            for (int pos=0; pos<sector.size();pos++){sector[pos]=pos;}
            return sector;
            break;
    case 6 :
            sector.resize(6);
            for (int pos=0; pos<sector.size();pos++){sector[pos]=pos;}
            return sector;
            break;
    case 4 :
            sector.resize(4);
            for (int pos=0; pos<sector.size();pos++){sector[pos]=pos;}
            return sector;
            break;
    case 3 :
            sector.resize(3);
            for (int pos=0; pos<sector.size();pos++){sector[pos]=pos;}
            return sector;
            break;
    case 2 :
            sector.resize(2);
            for (int pos=0; pos<sector.size();pos++){sector[pos]=pos;}
            return sector;
            break;
    default : return sector;

    }
}
std::vector<int> PhasedArray::getSectorVectorProva()const{
    std::vector<int>sect=getSectorVector();
    return sect;
}

int PhasedArray::getCurrentActiveSector()const{
    std::vector<int>sect=getSectorVector();
    int N = sect.size();
    double tslot=5; // duration of the sector (can be as output ned parameter to set in the ini)
    double T;
    double tsim=simTime().dbl();
    T=N*tslot;
    float rem= fmod(tsim,T);
    int sector = ceil(rem/tslot); // current active sector
    return sector;

}
bool PhasedArray::isWithinSector (EulerAngles direction)const {
    switch (getNumSectors()){
    case 8 :
        switch (getCurrentActiveSector()){
        case 1: if ((direction.alpha*(180/3.14) >= 0)&& (direction.alpha*(180/3.14) <= 45)){return true; break;} // first sector width 45°
        case 2: if ((direction.alpha*(180/3.14) > 45)&& (direction.alpha*(180/3.14) <= 90)){return true; break;} //second sector width 45°
        case 3: if ((direction.alpha*(180/3.14) > 90)&& (direction.alpha*(180/3.14) <= 135)){return true; break;} // third sector width 45°..
        case 4: if ((direction.alpha*(180/3.14) > 135)&& (direction.alpha*(180/3.14) <= 180)){return true; break;}
        case 5: if ((direction.alpha*(180/3.14) > -180)&& (direction.alpha*(180/3.14) <= -135)){return true; break;}
        case 6: if ((direction.alpha*(180/3.14) > -135)&& (direction.alpha*(180/3.14) <= -90)){return true; break;}
        case 7: if ((direction.alpha*(180/3.14) > -90)&& (direction.alpha*(180/3.14) <= -45)){return true; break;}
        case 8: if ((direction.alpha*(180/3.14) > -45)&& (direction.alpha*(180/3.14) < 0)){return true; break;}
        default: return false;
        }break;
    case  6:
        switch (getCurrentActiveSector()){
        case 1: if ((direction.alpha*(180/3.14) >= 0)&& (direction.alpha*(180/3.14) <= 60)){return true; break;}
        case 2: if ((direction.alpha*(180/3.14) > 60)&& (direction.alpha*(180/3.14) <= 120)){return true; break;}
        case 3: if ((direction.alpha*(180/3.14) > 120)&& (direction.alpha*(180/3.14) <= 180)){return true; break;}
        case 4: if ((direction.alpha*(180/3.14) > -180)&& (direction.alpha*(180/3.14) <= 120)){return true; break;}
        case 5: if ((direction.alpha*(180/3.14) > -120)&& (direction.alpha*(180/3.14) <= -60)){return true; break;}
        case 6: if ((direction.alpha*(180/3.14) > -60)&& (direction.alpha*(180/3.14) <= 0)){return true; break;}
        default: return false;
        }break;
    case  4:
        switch (getCurrentActiveSector()){
        case 1: if ((direction.alpha*(180/3.14) >= 0)&& (direction.alpha*(180/3.14) <= 90)){return true; break;}
        case 2: if ((direction.alpha*(180/3.14) > 90)&& (direction.alpha*(180/3.14) <= 180)){return true; break;}
        case 3: if ((direction.alpha*(180/3.14) > -180)&& (direction.alpha*(180/3.14) <= -90)){return true; break;}
        case 4: if ((direction.alpha*(180/3.14) > -90)&& (direction.alpha*(180/3.14) <= 0)){return true; break;}
        default: return false;
        }break;
    case  3:
        switch (getCurrentActiveSector()){
        case 1: if ((direction.alpha*(180/3.14) >= 0)&& (direction.alpha*(180/3.14) <= 120)){return true; break;}
        case 2: if ((direction.alpha*(180/3.14) > 120)&& (direction.alpha*(180/3.14) <= -120)){return true; break;}
        case 3: if ((direction.alpha*(180/3.14) > -120)&& (direction.alpha*(180/3.14) <= 0)){return true; break;}
        default: return false;
        }break;
    case  2:
        switch (getCurrentActiveSector()){
        case 1: if ((direction.alpha*(180/3.14) >= 0)&& (direction.alpha*(180/3.14) <= 180)){return true; break;}
        case 2: if ((direction.alpha*(180/3.14) > -180)&& (direction.alpha*(180/3.14) <= 0)){return true; break;}
        default: return false;
        }break;
        default: return false;
    }
}

std::ostream& PhasedArray::printToStream(std::ostream& stream, int level) const {
    stream << "PhasedArray";
    if (level >= PRINT_LEVEL_DETAIL) {

        stream << ", length = " << length;

        cout << getFullPath().substr(13, 8) << " Posizione: "
                << getMobility()->getCurrentPosition() << endl;

        //  std::vector<Coord>positions = this->getMempos()->getListaPos();
        //  cout<<getAngolo(Coord(2,0,0),Coord(1,2,0))<<endl;
        //  cout<<getAngolo(positions[0],positions[1])<<endl;
    }
    return AntennaBase::printToStream(stream, level);
}

void PhasedArray::receiveSignal(cComponent *source, simsignal_t signalID, double val, cObject *details)
{
    if (signalID == phaseArrayConfigureChange)
    {
        if (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER &&
                radio->getTransmissionState() == IRadio::TRANSMISSION_STATE_TRANSMITTING)
        {
            // pending
            pendingConfiguration = true;
            newConfigurtion = val;
        }
        if (val >= -180 && val <= 180)
            phiz = val;
        else if (val == 360)
            phiz = 360;
    }
}

void PhasedArray::receiveSignal(cComponent *source, simsignal_t signalID, long val, cObject *details)
{
    if (signalID == triggerChange) {

    }
    else if (signalID != phaseArrayConfigureChange)
        // Radio signals
        if (pendingConfiguration)
        {
            if (radio->getRadioMode() != IRadio::RADIO_MODE_TRANSMITTER ||
                    (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER && radio->getTransmissionState() != IRadio::TRANSMISSION_STATE_TRANSMITTING))
            {
                if (val >= -180 && val <= 180)
                    phiz = val;
                else if (val == 360)
                    phiz = 360;
            }
        }
}


} // namespace physicallayer

} // namespace inet



