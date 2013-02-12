//
//
// Copyright (C) 2007 Roland Siedlaczek
// Copyright (C) 2007 Universität Dortmund, Germany
//
// MAC-Paket Data Unit Header
// Defenition des Generic MAC Header und des Bandwidth Request Header
//
//


cplusplus {{
#include "MACAddress.h"
}}

class noncobject MACAddress;

enum ip_traffic_types; // von VITA hinzugef�gt

enum Ieee80216ManagementMessageType
{
}

struct SubType
{
    bool Mesh_Subheader;
    bool ARQ;
    bool Extended;
    bool Frag_Subheader;
    bool Pack_Subheader;
    bool Subheader;
}

struct myEKS
{
    bool A;
    bool B;
}

// Header Informationen die sowohl im Generic Header als auch Bandwidth Request Header 
//
//
packet Ieee80216MacHeader
{
    byteLength = 36;    // Länge des MAC-Header
    bool HT;            // Header Type "0 = Generic MAC Header" & "1 = Bandwitdh Request Header"
    bool EC;            // Encryption Control "0 = payload unverschlisselt" & "1 = payload verschlüsselt"
    int CID;            // Connection Identifier
    int HCS;            // Header Check Sequence. Fehlererkennung im MAC-Header

    double SNR;         //Achtung nicht im Standard, nur eine Hilfsvariable
    double rcvdPower;
    double thermNoise;
    double bitrate;     //Achtung nicht im Standard, nur eine Hilfsvariable
    double abstand;     //Achtung nicht im Standard, nur eine Hilfsvariable
    double xPos;        //Achtung nicht im Standard, nur eine Hilfsvariable
    double yPos;        //Achtung nicht im Standard, nur eine Hilfsvariable
}

//
// Defenition des Generic MAC Header
//
packet Ieee80216GenericMacHeader extends Ieee80216MacHeader
{
    HT = 0;             // 0 = Generic MAC Header
    EC = 0;             // 0 = payload unverschlüsselt
    SubType TYPE;       // Identifizieren vorhandener Subheader und ARQ Payload
    bool ESF;           // Erweiterter Subheader Feld " 1 = Subheaderist aktive"
    bool CI;            // CRC Indikator CRC Summe
    myEKS EKS;          // Encryption Key Sequence nur wenn Payload verschlüsselt
    short LEN;          // Gesamtlänge der MAC PDU
    CID = 0;            //
    HCS = 0;            //
}

//
// Defenition des Bandwidth Request Header
//
packet Ieee80216BandwidthMacHeader extends Ieee80216MacHeader
{
    HT = 1;             // Type 1 = Bandwitdh Request Header
    EC = 0;             // Bandwidth Request immer unverschlüsselt senden "EC = 0"
    short TYPE_BD;      // 3 bit
    int BR;             // 19 bit
}

// Small packing subheader for connections without ARQ 
packet Ieee80216PackingSubHeader
{
    short bit_length = 16; // non-standard information
    Ieee80216GenericMacHeader included_datagrams[];

    short FC;           // 2 bit
    short FSN;          // 3 bit
    short LENGTH;       // 11 bit
}

//
// Base class for 802.16 management frames (subclasses will add frame body contents)
//
packet Ieee80216ManagementMessage extends Ieee80216GenericMacHeader
{
}
