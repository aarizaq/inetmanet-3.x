
#include "WiMAXPathLossReceptionModel.h"
#include "FWMath.h"

Register_Class(WiMAXPathLossReceptionModel);

void WiMAXPathLossReceptionModel::initializeFrom(cModule *radioModule)
{
    standardabweichung = radioModule->par("standardabweichung");

    cModule *cc = simulation.getModuleByPath("channelcontrol");
    if (!cc)
        opp_error("PathLossReceptionModel: module (ChannelControl)channelcontrol not found");
    //if (pathLossAlpha < (double) (cc->par("alpha")))
    //    opp_error("PathLossReceptionModel: pathLossAlpha can't be smaller than in ChannelControl -- please adjust the parameters");
    normalVector.setName("normal");
    logNormalVector.setName("logNormal");
}

double WiMAXPathLossReceptionModel::calculateReceivedPower(double pSend, double carrierFrequency,
                                                           double distance)
{
    /** @brief "Kapitel 7.6.3 " Kanalmodelle */
    double L_m;
    double L_a;
    double L_d;
    L_m = 0;
    L_a = 0;
    L_d = 0;

    L_m = (58.77 + 37.6 * log10(distance / 1000) + 21 * log10(carrierFrequency / 1000000)); //mitlere Übertragungsdaempfung
    L_d = lognormal(0, standardabweichung, 0); //Abschautungsdaempfung
    L_a = normal(0, standardabweichung, 0);
    normalVector.record(L_a);
    logNormalVector.record(L_d);
    double pEmpfang;
    pEmpfang = pSend * pow(10, -1 * (L_m + L_a) / 10);
    ev << "RadioPath pEmpfang:" << pEmpfang << " pSend:" << pSend << " L_m:" << L_m << " L_a:" <<
        L_a << endl;
    //return (pSend * pow(10,(-1*(58.77+37.6*log10(distance/1000)+21*log10(carrierFrequency/1000000)))/10));
    return (pEmpfang);
}
