#ifndef RPSSOCKET_H_
#define RPSSOCKET_H_

#ifndef __unix__


#include <iostream>
#include <sstream>
#include <sys/types.h>
#include "winsock.h"
//#include <netinet/in.h>
#include <WinInet.h>
#include <windows.h>
#pragma comment (lib, "ws2_32.lib")

#else

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
//#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#endif

#include <omnetpp.h>

long WinsockStartup();


using namespace std;

class RPSSocket
{
public:
	enum socketState_t {
		online = 0,
		offline = 1
	};
private:
	int mySocket;

	socketState_t socketState;
	string double2string(double zahl);
public:
	RPSSocket();
	virtual ~RPSSocket();

	int sendStr(string command);
	string getStr();
	bool createSocket();
	bool connectSocket(string serverIP,int serverPort);
	void shutdownSocket();
	// RPS Commands
	bool start();
	bool open(string Pfad);
	bool save(string Pfad);
	bool quit();
	bool run();
	int addMSgroup(string groupName);
	long AddMS(double PositionX,double PositionY, double PositionZ, double DimensionX, double DimensionY, long rxgroupid);
	double GetMSResult(long MSindex,long groupIndex);
	bool SetMSParam(long MSindex,string Param,double value);
	double GetMSParam(long MSindex,string Param);	// DD CNI 2010-11-23
	bool SetPreferenceParam(string Param,string Value);
	bool disablePredictions();
	long GetBScount();
	long GetMSGroupIDFromName(string name);
	bool SetVisible(bool state);
	bool SetSimulationMode(string Mode);
	bool prepareSimulation();
	long GetBSIDFromName(string Name);
	long GetBSIDFromIndex(long index);
	bool SetBSParam(long BSindex,string Param,string value);
	void moveBS(long BSindex, double Xcoord, double Ycoord);
	void moveMS(long MSindex, double Xcoord, double Ycoord);
	double calculatePower(long MSindex,long groupID, long BSindex, double MSX,double MSY, double BSX, double BSY);
	double calculateReceivedpower(long MSindex, long Senderindex);
	bool RemoveMS(long index);
	bool MoveObject(string object,int index,double x_coord,double y_coord);
};

#endif /*RPSSOCKET_H_*/
