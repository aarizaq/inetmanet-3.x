/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "ModulationType.h"
#include "Ieee80211DataRate.h"
#include "WifiMode.h"


ModulationType
WifiModulationType::GetDsssRate1Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_DSSS);
   mode.setBandwidth(22000000);
   mode.setDataRate(1000000);
   mode.setCodeRate(CODE_RATE_UNDEFINED);
   mode.setConstellationSize(2);
   mode.setIsMandatory(true);
   return mode;
}

ModulationType
WifiModulationType::GetDsssRate2Mbps()
{

   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_DSSS);
   mode.setBandwidth(22000000);
   mode.setDataRate(2000000);
   mode.setCodeRate(CODE_RATE_UNDEFINED);
   mode.setConstellationSize(2);
   mode.setIsMandatory(true);
   return mode;
}

/**
 * Clause 18 rates (HR/DSSS)
 */
ModulationType
WifiModulationType::GetDsssRate5_5Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_DSSS);
   mode.setBandwidth(22000000);
   mode.setDataRate(5500000);
   mode.setCodeRate(CODE_RATE_UNDEFINED);
   mode.setConstellationSize(4);
   mode.setIsMandatory(true);
   return mode;
}

ModulationType
WifiModulationType::GetDsssRate11Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_DSSS);
   mode.setBandwidth(22000000);
   mode.setDataRate(11000000);
   mode.setCodeRate(CODE_RATE_UNDEFINED);
   mode.setConstellationSize(4);
   mode.setIsMandatory(true);
   return mode;
}


/**
 * Clause 19.5 rates (ERP-OFDM)
 */
ModulationType
WifiModulationType::GetErpOfdmRate6Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_ERP_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(6000000);
   mode.setCodeRate(CODE_RATE_1_2);
   mode.setConstellationSize(2);
   mode.setIsMandatory(true);
   return mode;
}

ModulationType
WifiModulationType::GetErpOfdmRate9Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_ERP_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(9000000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(2);
   mode.setIsMandatory(false);
   return mode;
}

ModulationType
WifiModulationType::GetErpOfdmRate12Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_ERP_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(12000000);
   mode.setCodeRate(CODE_RATE_1_2);
   mode.setConstellationSize(4);
   mode.setIsMandatory(true);
   return mode;
}

ModulationType
WifiModulationType::GetErpOfdmRate18Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_ERP_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(18000000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(4);
   mode.setIsMandatory(false);
   return mode;
}

ModulationType
WifiModulationType::GetErpOfdmRate24Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_ERP_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(24000000);
   mode.setCodeRate(CODE_RATE_1_2);
   mode.setConstellationSize(16);
   mode.setIsMandatory(true);
   return mode;
}

ModulationType
WifiModulationType::GetErpOfdmRate36Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_ERP_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(36000000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(16);
   mode.setIsMandatory(false);
   return mode;
}

ModulationType
WifiModulationType::GetErpOfdmRate48Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_ERP_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(48000000);
   mode.setCodeRate(CODE_RATE_2_3);
   mode.setConstellationSize(64);
   mode.setIsMandatory(false);
   return mode;
}

ModulationType
WifiModulationType::GetErpOfdmRate54Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_ERP_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(54000000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(64);
   mode.setIsMandatory(false);
   return mode;
}


/**
 * Clause 17 rates (OFDM)
 */
ModulationType
WifiModulationType::GetOfdmRate6Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(6000000);
   mode.setCodeRate(CODE_RATE_1_2);
   mode.setConstellationSize(2);
   mode.setIsMandatory(true);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate9Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(9000000);
   mode.setCodeRate(CODE_RATE_UNDEFINED);
   mode.setConstellationSize(2);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate12Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(12000000);
   mode.setCodeRate(CODE_RATE_UNDEFINED);
   mode.setConstellationSize(4);
   mode.setIsMandatory(true);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate18Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(18000000);
   mode.setCodeRate(CODE_RATE_UNDEFINED);
   mode.setConstellationSize(4);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate24Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(24000000);
   mode.setCodeRate(CODE_RATE_1_2);
   mode.setConstellationSize(16);
   mode.setIsMandatory(true);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate36Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(36000000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(16);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate48Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(48000000);
   mode.setCodeRate(CODE_RATE_2_3);
   mode.setConstellationSize(64);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate54Mbps()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(20000000);
   mode.setDataRate(54000000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(64);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

/* 10 MHz channel rates */
ModulationType
WifiModulationType::GetOfdmRate3MbpsBW10MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(10000000);
   mode.setDataRate(3000000);
   mode.setCodeRate(CODE_RATE_1_2);
   mode.setConstellationSize(2);
   mode.setIsMandatory(true);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate4_5MbpsBW10MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(10000000);
   mode.setDataRate(4500000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(2);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate6MbpsBW10MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(10000000);
   mode.setDataRate(6000000);
   mode.setCodeRate(CODE_RATE_1_2);
   mode.setConstellationSize(4);
   mode.setIsMandatory(true);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate9MbpsBW10MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(10000000);
   mode.setDataRate(9000000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(4);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate12MbpsBW10MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(10000000);
   mode.setDataRate(12000000);
   mode.setCodeRate(CODE_RATE_1_2);
   mode.setConstellationSize(16);
   mode.setIsMandatory(true);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate18MbpsBW10MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(10000000);
   mode.setDataRate(18000000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(16);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate24MbpsBW10MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(10000000);
   mode.setDataRate(24000000);
   mode.setCodeRate(CODE_RATE_2_3);
   mode.setConstellationSize(64);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate27MbpsBW10MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(10000000);
   mode.setDataRate(27000000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(64);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

/* 5 MHz channel rates */
ModulationType
WifiModulationType::GetOfdmRate1_5MbpsBW5MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(5000000);
   mode.setDataRate(1500000);
   mode.setCodeRate(CODE_RATE_1_2);
   mode.setConstellationSize(2);
   mode.setIsMandatory(true);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate2_25MbpsBW5MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(5000000);
   mode.setDataRate(2250000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(2);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate3MbpsBW5MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(5000000);
   mode.setDataRate(3000000);
   mode.setCodeRate(CODE_RATE_1_2);
   mode.setConstellationSize(4);
   mode.setIsMandatory(true);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate4_5MbpsBW5MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(5000000);
   mode.setDataRate(4500000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(4);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate6MbpsBW5MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(5000000);
   mode.setDataRate(6000000);
   mode.setCodeRate(CODE_RATE_1_2);
   mode.setConstellationSize(16);
   mode.setIsMandatory(true);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate9MbpsBW5MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(5000000);
   mode.setDataRate(9000000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(16);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate12MbpsBW5MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(5000000);
   mode.setDataRate(12000000);
   mode.setCodeRate(CODE_RATE_2_3);
   mode.setConstellationSize(64);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

ModulationType
WifiModulationType::GetOfdmRate13_5MbpsBW5MHz()
{
   ModulationType mode;
   mode.setModulationClass(MOD_CLASS_OFDM);
   mode.setBandwidth(5000000);
   mode.setDataRate(13500000);
   mode.setCodeRate(CODE_RATE_3_4);
   mode.setConstellationSize(64);
   mode.setIsMandatory(false);
   mode.setFrequency(5000);
   return mode;
}

/*Clause 20*/

ModulationType
WifiModulationType::GetOfdmRate6_5MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(6500000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(2);
    mode.setIsMandatory(true);
    return mode;
 }

ModulationType
WifiModulationType::GetOfdmRate7_2MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(7200000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(2);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate13MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(13000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(4);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate14_4MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(14400000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(4);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate19_5MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(19500000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(4);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate21_7MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(21700000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(4);
    mode.setIsMandatory(false);
    return mode;
}


ModulationType
WifiModulationType::GetOfdmRate26MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(26000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(16);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate28_9MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(28900000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(16);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate39MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(39000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(16);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate43_3MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(43300000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(16);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate52MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(52000000);
    mode.setCodeRate(CODE_RATE_2_3);
    mode.setConstellationSize(64);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate57_8MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(57800000);
    mode.setCodeRate(CODE_RATE_2_3);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}


ModulationType
WifiModulationType::GetOfdmRate58_5MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(58500000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(64);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate65MbpsBW20MHzShGi ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(65000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate65MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(65000000);
    mode.setCodeRate(CODE_RATE_5_6);
    mode.setConstellationSize(64);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate72_2MbpsBW20MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(20000000);
    mode.setDataRate(72200000);
    mode.setCodeRate(CODE_RATE_5_6);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate13_5MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(13500000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(2);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate15MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(15000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(2);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate27MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(27000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(4);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate30MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(30000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(4);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate40_5MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(40500000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(4);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate45MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(45000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(4);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate54MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(54000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(16);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate60MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(60000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(16);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate81MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(81000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(16);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate90MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(90000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(16);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate108MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(108000000);
    mode.setCodeRate(CODE_RATE_2_3);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate120MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(120000000);
    mode.setCodeRate(CODE_RATE_2_3);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate121_5MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(121500000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate135MbpsBW40MHzShGi ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(135000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate135MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(135000000);
    mode.setCodeRate(CODE_RATE_5_6);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType
WifiModulationType::GetOfdmRate150MbpsBW40MHz ()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_HT);
    mode.setBandwidth(40000000);
    mode.setDataRate(150000000);
    mode.setCodeRate(CODE_RATE_5_6);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}



//Added by Ghada to support 11n
//return the L-SIG

ModulationType WifiModulationType::getMFPlcpHeaderMode(ModulationType payloadMode, WifiPreamble preamble)
{
    switch (payloadMode.getBandwidth())
    {
        case 20000000:
            return WifiModulationType::GetOfdmRate6_5MbpsBW20MHz();
        case 40000000:
            return WifiModulationType::GetOfdmRate13_5MbpsBW40MHz();
        default:
            return WifiModulationType::GetOfdmRate6_5MbpsBW20MHz();
    }

}

simtime_t WifiModulationType::getPlcpHtTrainingSymbolDuration(ModulationType payloadMode, WifiPreamble preamble, const uint32_t & nss,const uint32_t & ness)
{
    switch (preamble)
    {
        case WIFI_PREAMBLE_HT_MF:
            return (4 + (4 * nss))/1000000.0;
        case WIFI_PREAMBLE_HT_GF:
            return (4 * nss) + (4 * ness);
        default:
            // no training for non HT
            return 0;
    }
}

//return L-SIG
simtime_t WifiModulationType::getPlcpHtSigHeaderDuration(ModulationType payloadMode, WifiPreamble preamble)
{
    switch (preamble)
    {
        case WIFI_PREAMBLE_HT_MF:
            // HT-SIG
            return 8/1000000.0;
        case WIFI_PREAMBLE_HT_GF:
            //HT-SIG
            return 8/1000000.0;
        default:
            // no HT-SIG for non HT
            return 0;
    }
}

//end added by Ghada



ModulationType WifiModulationType::getModulationType(char mode, double bitrate)
{
    int i = Ieee80211Descriptor::getIdx(mode, bitrate);
    return Ieee80211Descriptor::getDescriptor(i).modulationType;
}


simtime_t
WifiModulationType::getPlcpHeaderDuration(ModulationType payloadMode, WifiPreamble preamble)
{
  switch (payloadMode.getModulationClass())
  {
      case MOD_CLASS_OFDM:
      {
          switch (payloadMode.getBandwidth()) {
              case 20000000:
              default:
                  // IEEE Std 802.11-2007, section 17.3.3 and figure 17-4
                  // also section 17.3.2.3, table 17-4
                  // We return the duration of the SIGNAL field only, since the
                  // SERVICE field (which strictly speaking belongs to the PLCP
                  // header, see section 17.3.2 and figure 17-1) is sent using the
                  // payload mode.
                  return 4.0/1000000.0;
              case 10000000:
                  // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
                  return 8/1000000.0;
              case 5000000:
                  // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
                  return 16.0/1000000.0;
          }
          break;
      }
      //Added by Ghada to support 11n
      case MOD_CLASS_HT: { //IEEE 802.11n Figure 20.1
          switch (preamble)
          {
              case WIFI_PREAMBLE_HT_MF:
                  // L-SIG
                  return 4/1000000.0;
              case WIFI_PREAMBLE_HT_GF:
                  //L-SIG
                  return 0;
              default:
                  // L-SIG
                  return 4/1000000.0;
            }
      }
      case MOD_CLASS_ERP_OFDM:
          return 16.0/1000000.0;
      case MOD_CLASS_DSSS:
          if (preamble == WIFI_PREAMBLE_SHORT)
          {
              // IEEE Std 802.11-2007, section 18.2.2.2 and figure 18-2
              return 24.0/1000000.0;
          }
          else // WIFI_PREAMBLE_LONG
          {
              // IEEE Std 802.11-2007, sections 18.2.2.1 and figure 18-1
              return 48.0/1000000.0;
          }
      default:
          opp_error("unsupported modulation class");
      return 0;
    }
}

simtime_t
WifiModulationType::getPlcpPreambleDuration(ModulationType payloadMode, WifiPreamble preamble)
{
    switch (payloadMode.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        {
            switch (payloadMode.getBandwidth())
            {
                case 20000000:
                default:
                // IEEE Std 802.11-2007, section 17.3.3,  figure 17-4
                // also section 17.3.2.3, table 17-4
                    return 16.0/1000000.0;
                case 10000000:
                // IEEE Std 802.11-2007, section 17.3.3, table 17-4
                // also section 17.3.2.3, table 17-4
                    return 32.0/1000000.0;
                case 5000000:
                // IEEE Std 802.11-2007, section 17.3.3
                // also section 17.3.2.3, table 17-4
                    return 64.0/1000000.0;
            }
            break;
        }
        case MOD_CLASS_HT:
        { //IEEE 802.11n Figure 20.1 the training symbols before L_SIG or HT_SIG
               return 16/1000000.0;
        }
        case MOD_CLASS_ERP_OFDM:
            return 4.0/1000000.0;
        case MOD_CLASS_DSSS:
            if (preamble == WIFI_PREAMBLE_SHORT)
            {
                // IEEE Std 802.11-2007, section 18.2.2.2 and figure 18-2
                return 72.0/1000000.0;
            }
            else // WIFI_PREAMBLE_LONG
            {
                // IEEE Std 802.11-2007, sections 18.2.2.1 and figure 18-1
                return 144.0/1000000.0;
            }
        default:
            opp_error("unsupported modulation class");
        return 0;
    }
}
//
// Compute the Payload duration in function of the modulation type
//
simtime_t
WifiModulationType::getPayloadDuration(uint64_t size, ModulationType payloadMode, const uint32_t & nss, bool isStbc)
{
    simtime_t val;
    switch (payloadMode.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        case MOD_CLASS_ERP_OFDM:
        {
            // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
            // corresponds to T_{SYM} in the table
            uint32_t symbolDurationUs;
            switch (payloadMode.getBandwidth())
            {
                case 20000000:
                default:
                    symbolDurationUs = 4;
                    break;
                case 10000000:
                    symbolDurationUs = 8;
                    break;
                case 5000000:
                    symbolDurationUs = 16;
                    break;
            }
            // IEEE Std 802.11-2007, section 17.3.2.2, table 17-3
            // corresponds to N_{DBPS} in the table
            double numDataBitsPerSymbol = payloadMode.getDataRate()  * symbolDurationUs / 1e6;
            // IEEE Std 802.11-2007, section 17.3.5.3, equation (17-11)
            uint32_t numSymbols = lrint(ceil((16 + size + 6.0)/numDataBitsPerSymbol));

            // Add signal extension for ERP PHY
            double aux;
            if (payloadMode.getModulationClass() == MOD_CLASS_ERP_OFDM)
                aux = numSymbols*symbolDurationUs + 6;
            else
                aux = numSymbols*symbolDurationUs;
            val = (aux/1000000.0);
            return val;
        }
        case MOD_CLASS_DSSS:
            // IEEE Std 802.11-2007, section 18.2.3.5
            double aux;
            aux = lrint(ceil((size) / (payloadMode.getDataRate() / 1.0e6)));
            val = (aux/1000000.0);
            return val;
            break;
        case MOD_CLASS_HT:
        {
            double symbolDurationUs;
            double m_Stbc;
            //if short GI data rate is used then symbol duration is 3.6us else symbol duration is 4us
            //In the future has to create a stationmanager that only uses these data rates if sender and reciever support GI
            if (payloadMode == WifiModulationType::GetOfdmRate135MbpsBW40MHzShGi() || payloadMode == WifiModulationType::GetOfdmRate65MbpsBW20MHzShGi() )
            {
                symbolDurationUs=3.6;
            }
            else
            {
                switch (payloadMode.getDataRate ()/nss)
                { //shortGi
                    case 7200000:
                    case 14400000:
                    case 21700000:
                    case 28900000:
                    case 43300000:
                    case 57800000:
                    case 72200000:
                    case 15000000:
                    case 30000000:
                    case 45000000:
                    case 60000000:
                    case 90000000:
                    case 120000000:
                    case 150000000:
                       symbolDurationUs=3.6;
                       break;
                     default:
                       symbolDurationUs=4;
                }
            }
            if  (isStbc)
                m_Stbc=2;
            else
                m_Stbc=1;
            double numDataBitsPerSymbol = payloadMode.getDataRate () * nss * symbolDurationUs / 1e6;
            //check tables 20-35 and 20-36 in the standard to get cases when nes =2
            double Nes=1;
            // IEEE Std 802.11n, section 20.3.11, equation (20-32)
            uint32_t numSymbols = lrint (m_Stbc*ceil ((16 + size * 8.0 + 6.0*Nes) / (m_Stbc* numDataBitsPerSymbol)));
            return (numSymbols * symbolDurationUs)/1e6;
        }
        default:
            opp_error("unsupported modulation class");
            return 0;
    }
}

//
// Return the physical header duration, useful for the mac
//
simtime_t
WifiModulationType::getPreambleAndHeader(ModulationType payloadMode, WifiPreamble preamble)
{
    return (getPlcpPreambleDuration(payloadMode, preamble)+ getPlcpHeaderDuration(payloadMode, preamble));
}

simtime_t
WifiModulationType::calculateTxDuration(uint64_t size, ModulationType payloadMode, WifiPreamble preamble,const uint32_t & nss, const uint32_t & ness, bool isStbc)
{
    simtime_t duration = getPlcpPreambleDuration(payloadMode, preamble)
                       + getPlcpHeaderDuration(payloadMode, preamble)
                       + getPayloadDuration(size, payloadMode,nss,isStbc)
                       + getPlcpHtSigHeaderDuration(payloadMode, preamble)
                       + getPlcpHtTrainingSymbolDuration(payloadMode, preamble,nss,ness);
    return duration;
}

ModulationType
WifiModulationType::getPlcpHeaderMode(ModulationType payloadMode, WifiPreamble preamble)
{
    switch (payloadMode.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        {
            switch (payloadMode.getBandwidth())
            {
                case 5000000:
                    return WifiModulationType::GetOfdmRate1_5MbpsBW5MHz();
                case 10000000:
                    return WifiModulationType::GetOfdmRate3MbpsBW10MHz();
                default:
                // IEEE Std 802.11-2007, 17.3.2
                // actually this is only the first part of the PlcpHeader,
                // because the last 16 bits of the PlcpHeader are using the
                // same mode of the payload
                    return WifiModulationType::GetOfdmRate6Mbps();
            }
            break;
        }
        case MOD_CLASS_ERP_OFDM:
            return WifiModulationType::GetErpOfdmRate6Mbps();
            //Added by Ghada to support 11n
        case MOD_CLASS_HT:
        {  //return the HT-SIG
            // IEEE Std 802.11n, 20.3.23
            switch (preamble)
            {
                case WIFI_PREAMBLE_HT_MF:
                    switch (payloadMode.getBandwidth())
                    {
                        case 20000000:
                            return WifiModulationType::GetOfdmRate13MbpsBW20MHz ();
                        case 40000000:
                            return WifiModulationType::GetOfdmRate27MbpsBW40MHz ();
                        default:
                            return WifiModulationType::GetOfdmRate13MbpsBW20MHz();
                    }
                case WIFI_PREAMBLE_HT_GF:
                    switch (payloadMode.getBandwidth())
                    {
                        case 20000000:
                            return WifiModulationType::GetOfdmRate13MbpsBW20MHz();
                        case 40000000:
                            return WifiModulationType::GetOfdmRate27MbpsBW40MHz();
                        default:
                            return WifiModulationType::GetOfdmRate13MbpsBW20MHz();
                    }
                default:
                    return WifiModulationType::GetOfdmRate6Mbps();
            }
        }
        case MOD_CLASS_DSSS:
            if (preamble == WIFI_PREAMBLE_LONG)
            {
                // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
                return WifiModulationType::GetDsssRate1Mbps();
            }
            else //  WIFI_PREAMBLE_SHORT
            {
                // IEEE Std 802.11-2007, section 18.2.2.2
                return WifiModulationType::GetDsssRate2Mbps();
            }
       default:
           opp_error("unsupported modulation class");
           return ModulationType();
     }
}


simtime_t
WifiModulationType::getSlotDuration(ModulationType modType, WifiPreamble preamble)
{
    switch (modType.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        {
            switch (modType.getBandwidth())
            {
                case 5000000:
                    return (21.0/1000000.0);
                case 10000000:
                    return (13.0/1000000.0);
                default:
                    // IEEE Std 802.11-2007, 17.3.2
                    // actually this is only the first part of the PlcpHeader,
                    // because the last 16 bits of the PlcpHeader are using the
                    // same mode of the payload
                    return (9.0/1000000.0);
            }
            break;
        }
        case MOD_CLASS_ERP_OFDM:
            if (preamble == WIFI_PREAMBLE_LONG)
            {
                // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
                return (20.0/1000000.0);
            }
            else //  WIFI_PREAMBLE_SHORT
            {
                // IEEE Std 802.11-2007, section 18.2.2.2
                return (9.0/1000000.0);
            }
        case MOD_CLASS_HT:
        {  //return the HT-SIG
            // IEEE Std 802.11.2012, 20.4.4
            switch (modType.getFrequency())
            {
                case 2400:
                    if (preamble == WIFI_PREAMBLE_LONG)
                        return (20.0/1000000.0);
                    else //  WIFI_PREAMBLE_SHORT
                        return (9.0/1000000.0);
                case 5000:
                    return (9.0/1000000.0);
                default:
                    return (9.0/1000000.0);
            }
        }
        case MOD_CLASS_DSSS:
            return (20.0/1000000.0);
        default:
            opp_error("unsupported modulation class");
            return SIMTIME_ZERO;
    }
}

simtime_t
WifiModulationType::getSifsTime(ModulationType modType, WifiPreamble preamble)
{
    switch (modType.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        {
            switch (modType.getBandwidth())
            {
                case 5000000:
                    return (64.0/1000000.0);
                case 10000000:
                    return (32.0/1000000.0);
                default:
                    // IEEE Std 802.11-2007, 17.3.2
                    // actually this is only the first part of the PlcpHeader,
                    // because the last 16 bits of the PlcpHeader are using the
                    // same mode of the payload
                    return (16.0/1000000);
            }
            break;
        }
        case MOD_CLASS_HT:
        {  //return the HT-SIG
            // IEEE Std 802.11.2012, 20.4.4
            switch (modType.getFrequency())
            {
                case 2400:
                    return (10.0/1000000.0);
                case 5000:
                    return (16.0/1000000.0);
                default:
                    return (10.0/1000000.0);
            }
        }
        case MOD_CLASS_ERP_OFDM:
            // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
            return (10.0/1000000.0);
        case MOD_CLASS_DSSS:
            return (10.0/1000000.0);
        default:
            opp_error("unsupported modulation class");
        return SIMTIME_ZERO;
    }
}

simtime_t
WifiModulationType::get_aPHY_RX_START_Delay(ModulationType modType, WifiPreamble preamble)
{
    switch (modType.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        {
            switch (modType.getBandwidth())
            {
                case 5000000:
                    return (97.0/1000000.0);
                case 10000000:
                    return (49.0/1000000.0);
                default:
                    // IEEE Std 802.11-2007, 17.3.2
                    // actually this is only the first part of the PlcpHeader,
                    // because the last 16 bits of the PlcpHeader are using the
                    // same mode of the payload
                    return (25.0/1000000.0);
            }
        }
        case MOD_CLASS_HT:
            return 33.0/1000000.0;
        case MOD_CLASS_ERP_OFDM:
            // IEEE Std 802.11-2007, section 18.2.2.2
            return (24.0/1000000.0);
        case MOD_CLASS_DSSS:
            if (preamble == WIFI_PREAMBLE_LONG)
            {
                // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
                return (192.0/1000000.0);
            }
            else //  WIFI_PREAMBLE_SHORT
            {
                // IEEE Std 802.11-2007, section 18.2.2.2
                return (96.0/1000000.0);
            }
         default:
             opp_error("unsupported modulation class");
         return SIMTIME_ZERO;
    }
}

