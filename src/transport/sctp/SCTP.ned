//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.transport.sctp;
import inet.transport.ISCTP;

simple SCTP like ISCTP
{
      parameters:
      // ====== SCTP Association Parameters =================================
      int    numGapReports                = default(3);
      double rtoInitial @unit(s)          = default(3s);
      double rtoMin @unit(s)              = default(1s);
      double rtoMax @unit(s)              = default(60s);
      double rtoAlpha                     = default(0.125);
      double rtoBeta                      = default(0.250);
      int    maxBurst                     = default(4);
      int    assocMaxRetrans              = default(10);
      int    pathMaxRetrans               = default(5);
      int    maxInitRetrans               = default(8);
      double sackPeriod @unit(s)          = default(200ms);
      int    sackFrequency                = default(2);
      bool   reactivatePrimaryPath        = default(false);
      int    sendQueueLimit               = default(0);
      double validCookieLifetime @unit(s) = default(10s);

      // ====== Testing =====================================================
      bool   checkQueues                  = default(true);     // T.D. 20.11.2009

      // ====== Heartbeats ==================================================
      bool   enableHeartbeats             = default(true);
      double hbInterval @unit(s)          = default(30s);
      bool   sendHeartbeatsOnActivePaths  = default(false);

      // ====== Nagle Algorithm =============================================
      bool   nagleEnabled                 = default(true);
      int    naglePoint                   = default(1468);

      // ====== Congestion Control ==========================================
      int    RTXMethod                    = default(0);        // 0=Once after 3 Sacks; 1=once per RTT; 2=Switch off Fast Retransmit; 3=Always after 3 Sacks
      bool   fastRecoverySupported        = default(true);
      string sctpAlgorithmClass           = default("SCTPAlg");
      int    ccModule                     = default(0);        // RFC4960=0
      bool   osbWithHeader                = default(false);

      // ====== Other Parameters ============================================
      bool   natFriendly                  = default(false);
      int    bytesToAddPerRcvdChunk       = default(0);        // for flowcontrol
      int    bytesToAddPerPeerChunk       = default(0);        // for flowcontrol
      bool   tellArwnd                    = default(false);    // for flowcontrol
      int    messageAcceptLimit           = default(0);        // for flowcontrol
      double fairStart @unit(s)           = default(0s);
      double fairStop @unit(s)            = default(0s);
      int    ssModule                     = default(0);        // ROUND_ROBIN=0
      string streamsToPaths               = default("");
      int    arwnd                        = default(65535);
      int    swsLimit                     = default(3000);     // Limit for SWS
      int    startEndToEndDelay           = default(0);
      int    stopEndToEndDelay            = default(10);
      bool   udpEncapsEnabled             = default(false);
      double throughputInterval           = default(1);

      // ====== Chunk Authentication ========================================
      bool   auth                         = default(false);
      string chunks                       = default("");
      bool   padding                      = default(false);

      // ====== Add-IP ======================================================
      bool   addIP                        = default(false);
      double addTime @unit(s)             = default(0s);
      string addAddress                   = default("");
      string addIpType                    = default("0");      // SET_PRIMARY_ADDRESS=49156,ADD_IP_ADDRESS=49153,DELETE_IP_ADDRESS=49154

      // ====== Packet Drop =================================================
      bool   packetDrop                   = default(false);

      // ====== SACK Now ====================================================
      bool   sackNow                      = default(false);


      // ====== SACK Sequence Numbers =======================================
      bool   checkSackSeqNumber           = default(false);    // T.D. 23.02.2010

      // ====== NR-SACK =====================================================
      bool   nrSack                       = default(false);    // T.D. 03.03.2010
      bool   disableReneging              = default(false);    // T.D. 29.10.2010




   gates:
      input from_appl[];
      input from_ip;
      input from_ipv6;
      output to_appl[];
      output to_ip;
      output to_ipv6;
}
