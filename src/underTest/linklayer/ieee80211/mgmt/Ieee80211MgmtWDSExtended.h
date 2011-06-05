//
// Copyright (C) 2009 Juan-Carlos Maureira
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef IEEE80211_MGMT_WDS_EXTENDED_H
#define IEEE80211_MGMT_WDS_EXTENDED_H

#include <omnetpp.h>
#include <map>
#include "Ieee80211MgmtBaseExtended.h"
#include "NotificationBoard.h"


/**
 * Used in 802.11 WDS mode. 4 address frames used to communicate
 * with different APs in WDS mode.
 *
 * @author Juan-Carlos Maureira
 */
class INET_API Ieee80211MgmtWDSExtended : public Ieee80211MgmtBaseExtended
{
  public:
    /** State of a WDS Client */
    enum WDSClientStatus {NOT_AUTHENTICATED, AUTHENTICATED, CONNECTED};

    /** Describes a WDS Client */
    struct WDSClientInfo
    {
        MACAddress address;
        WDSClientStatus status;
        int authSeqExpected;  // when NOT_AUTHENTICATED: transaction sequence number of next expected auth frame
    };

    struct MAC_compare
    {
        bool operator()(const MACAddress& u1, const MACAddress& u2) const {return u1.compareTo(u2) < 0;}
    };
    typedef std::map<MACAddress,WDSClientInfo, MAC_compare> WDSClientList;

  protected:
    // configuration
    std::string ssid;
    int channelNumber;
    Ieee80211SupportedRatesElement supportedRates;

    // wds clients
    WDSClientList wdsList; // list of WDS clients

  protected:
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleTimer(cMessage *msg);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleUpperMessage(cPacket *msg);

    /** Implements Utility method to decapsulate a data frame */
    virtual cPacket *decapsulate(Ieee80211DataFrame *frame);

    /** Implements Utility method to encapsulate a packet. */
    virtual Ieee80211DataFrame* encapsulate(cPacket* pkt);

    /** Called by the NotificationBoard whenever a change occurs we're interested in */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

    /** Utility function: return sender STA's entry from our STA list, or NULL if not in there */
    virtual WDSClientInfo *lookupSenderWDSClient(MACAddress mac);

    /** @name Processing of different frame types */
    //@{
    virtual void handleDataFrame(Ieee80211DataFrame *frame);
    //@}
};

#endif

