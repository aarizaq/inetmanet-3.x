#include "RPSSocket.h"

RPSSocket::RPSSocket() {
#ifndef __unix__
    long rc;
    rc=WinsockStartup();
    socketState = offline;
#else
#endif
}

RPSSocket::~RPSSocket() {
    shutdownSocket();
}

int RPSSocket::sendStr(string command) {
    if (send(mySocket, command.c_str(), command.length() + 1, 0) == -1) {

        return -1;
    } else {

        return 0;
    }
}

string RPSSocket::getStr() {
    char buffer2[256];
    while (recv(mySocket, buffer2, 256, 0) == 0) {
#ifndef __unix__
        Sleep(1);
#else
        usleep( 1000 );
#endif
    }

    //printf(buffer2);
    return string(buffer2);
}

bool RPSSocket::createSocket() {
    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mySocket<0) {
        printf("Error: Socket coulnd'nt be created!\n");
        return false;
    } else {
        printf("Socket created!\n");
        return true;
    }
}

bool RPSSocket::connectSocket(string serverIP, int serverPort) {
    bool result = false;
    //Build Address of Server to connect to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(serverIP.c_str());
    server.sin_port = htons(serverPort);

    //Now Connect
    if (connect(mySocket, (struct sockaddr *) &server, sizeof server) == -1) {
        printf("Connect Error\n");
    } else {
        printf("Connected!!!\n");
        result = true;
    }
    return result;
}

void RPSSocket::shutdownSocket() {
    if (socketState == online) {
        shutdown(mySocket, 0x01); // 0x01 = SD_Send
        socketState = offline;
    }
}

bool RPSSocket::open(string Pfad) {
    sendStr("open(" + Pfad + ")");
    string ergebnis(getStr());
    if (ergebnis.compare("RPS object ready!") == 0) {
        //printf("File opened!\n");
        return true;
    } else {
        //printf("File couldn't be opened!\n");
        return false;
    }
}

bool RPSSocket::RemoveMS(long index) {
    stringstream befehl;
    befehl << "removems(" << index << ")";

    sendStr(befehl.str());

    return true;

}

bool RPSSocket::save(string Pfad) {
    sendStr("save(" + Pfad + ")");
    string ergebnis(getStr());
    if (ergebnis.compare("RPS object ready!") == 0) {
        return true;
    } else {
        return false;
    }
}
bool RPSSocket::quit() {
    sendStr("quit");
    string ergebnis(getStr());
    if (ergebnis.compare("Command finished.\n") == 0) {
        return true;
    } else {
        return false;
    }
}
bool RPSSocket::run() {
    sendStr("run");
    string ergebnis(getStr());
    if (ergebnis.compare("Simulation Finished.\n") == 0) {
        return true;
    } else {
        return false;
    }
}

bool RPSSocket::start() {
    sendStr("rpsstart");
    string ergebnis(getStr());
    if (ergebnis.compare("Command finished.\n") == 0) {
        return true;
    } else {
        return false;
    }
}

int RPSSocket::addMSgroup(string groupName) {
    sendStr("addmsgroup(" + groupName + ")");
    string ergebnis(getStr());
    return atoi(ergebnis.c_str());
}

long RPSSocket::AddMS(double PositionX, double PositionY, double PositionZ,
        double DimensionX, double DimensionY, long rxgroupid) {
    stringstream befehl;
    befehl << "addms(" << PositionX << ",";
    befehl << PositionY << ",";
    befehl << PositionZ << ",";
    befehl << DimensionX << ",";
    befehl << DimensionY << ",";
    befehl << rxgroupid << ")";

    sendStr(befehl.str());
    string ergebnis(getStr());
    return atol(ergebnis.c_str());
}

string RPSSocket::double2string(double zahl) {
    stringstream str;
    str << zahl;
    return str.str();
}

long RPSSocket::GetMSGroupIDFromName(string name) {
    sendStr("GetMSGroupIDFromName(" + name + ")");
    string ergebnis(getStr());
    return atol(ergebnis.c_str());
}

double RPSSocket::GetMSResult(long MSindex, long BSID) {
    stringstream befehl;
    befehl << "getmsresult(" << MSindex << ",MSPowerDB," << BSID << ")";
    sendStr(befehl.str());
    string ergebnis(getStr());
    return atof(ergebnis.c_str());
}

bool RPSSocket::SetMSParam(long MSindex, string Param, double value) {
    stringstream befehl;

    stringstream strs;
    strs << value;
    string str = strs.str();
    string key(".");
    size_t found;
    found = str.rfind(key);
    if (found != string::npos) {
        str.replace(found, key.length(), ",");
        befehl << "setMSParam(" << MSindex << "," << Param << "," << str << ")";
    } else {
        befehl << "setMSParam(" << MSindex << "," << Param << "," << value
                << ")";
    }

//	std::cout << befehl.str() << "\n";
    sendStr(befehl.str());
    string ergebnis(getStr());
    if (ergebnis.compare("Command finished.\n") == 0) {
        return true;
    } else {
        return false;
    }
}

double RPSSocket::GetMSParam(long MSindex, string Param) // DD CNI 2010-11-23
        {
    stringstream befehl;
    befehl << "GetMSParam(" << MSindex << "," << Param << ")";
    sendStr(befehl.str());
    string ergebnis(getStr());
    return atof(ergebnis.c_str());
}

bool RPSSocket::SetPreferenceParam(string Param, string Value) {
    sendStr("setpreferenceparam(" + Param + "," + Value + ")");
    string ergebnis(getStr());
    if (ergebnis.compare("Command finished.\n") == 0) {
        return true;
    } else {
        return false;
    }
}

bool RPSSocket::disablePredictions() {
    return SetPreferenceParam("EnableIncrementalPredictions", "false");
}

long RPSSocket::GetBScount() {
    sendStr("getbscount");
    string ergebnis(getStr());
    return atol(ergebnis.c_str());
}

bool RPSSocket::SetVisible(bool state) {
    if (state)
        sendStr("SetApplicationVisible(true)");
    else
        sendStr("SetApplicationVisible(false)");
    string ergebnis(getStr());
    if (ergebnis.compare("Command finished.\n") == 0) {
        return true;
    } else {
        return false;
    }
}

bool RPSSocket::SetSimulationMode(string Mode) {
    sendStr("setsimulationmode(" + Mode + ")");
    string ergebnis(getStr());
    if (ergebnis.compare("Command finished.\n") == 0) {
        return true;
    } else {
        return false;
    }
}

bool RPSSocket::prepareSimulation() {
    SetPreferenceParam("ThreadCount", "4");
    disablePredictions();
    return SetSimulationMode("RT25D");
    //SetVisible(false);
}

long RPSSocket::GetBSIDFromName(string Name) {
    sendStr("GetBSIDFromName(" + Name + ")");
    string ergebnis(getStr());
    return atol(ergebnis.c_str());
}

long RPSSocket::GetBSIDFromIndex(long index) {
    stringstream befehl;
    befehl << "GetBSIDFromIndex(" << index << ")";
    sendStr(befehl.str());
    string ergebnis(getStr());
    return atol(ergebnis.c_str());
}

bool RPSSocket::SetBSParam(long BSindex, string Param, string value) {
    stringstream befehl;
    befehl << "setBSParam(" << BSindex << "," << Param << "," << value << ")";
    sendStr(befehl.str());
    string ergebnis(getStr());
    if (ergebnis.compare("Command finished.\n") == 0) {
        return true;
    } else {
        return false;
    }
}

void RPSSocket::moveBS(long BSindex, double Xcoord, double Ycoord) {
    stringstream X;
    X << Xcoord;
    stringstream Y;
    Y << Ycoord;

//	string str = X.str();
//    string key (".");
//    size_t found;
//    found=str.rfind(key);
//    if (found!=string::npos){
//        str.replace (found,key.length(),",");
//        X<< str;
//    }
//
//    str = Y.str();
//    found=str.rfind(key);
//    if (found!=string::npos){
//        str.replace (found,key.length(),",");
//        Y<< str;
//    }

    SetBSParam(BSindex, "Position.X", X.str());
    SetBSParam(BSindex, "Position.Y", Y.str());
}

void RPSSocket::moveMS(long MSindex, double Xcoord, double Ycoord) {
    SetMSParam(MSindex, "Position.X", Xcoord);
    SetMSParam(MSindex, "Position.Y", Ycoord);
}

double RPSSocket::calculatePower(long MSindex, long groupID, long BSindex,
        double MSX, double MSY, double BSX, double BSY) {
    moveBS(BSindex, BSX, BSY);
    moveMS(MSindex, MSX, MSY);
    run();
    return GetMSResult(MSindex, BSindex);
}

double RPSSocket::calculateReceivedpower(long MSindex, long Senderindex) {
    return GetMSResult(MSindex, Senderindex);
}

bool RPSSocket::MoveObject(string object, int index, double x_coord,
        double y_coord) {
    stringstream command;
    command << "moveobject(" << object << "," << index << "," << x_coord << ","
            << y_coord << ")";
    sendStr(command.str());
    return true;
}

/* *************************************************
 * Start socket service under Windows
 */

long WinsockStartup() {
#ifndef __unix__
    long rc;

    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 1);

    rc = WSAStartup( wVersionRequested, &wsaData );
    return rc;
#else
    return 0;
#endif
}

