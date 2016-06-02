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
simsignal_t PhasedArray::phaseArrayConfigureChange = registerSignal(
        "phaseArrayConfigureChange");

Define_Module(PhasedArray);

PhasedArray::PhasedArray() :
        AntennaBase(), length(NaN),
        freq(1000000),
        distance (0.5),
        phiz(120)
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
        radio = check_and_cast<IRadio *>(radioModule);

        const char *energySourceModule = par("energySourceModule");
        energySource = dynamic_cast<IEnergySource *>(getModuleByPath(energySourceModule));
        if (energySource)
            energyConsumerId = energySource->addEnergyConsumer(this);
    }
}

double PhasedArray::computeGain(EulerAngles direction) const {

    if (phiz == -1) {
        // omni
        return 1;
    }

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
    else
    {
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
}

} // namespace physicallayer

} // namespace inet

