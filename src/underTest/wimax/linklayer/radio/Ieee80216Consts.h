
#ifndef CONSTS_H
#define CONSTS_H

// XXX these are taken over from Consts80211.h
// the phy has recognized a bit error in the packet
#define BITERROR -1
// packet lost due to collision
#define COLLISION 9

// frame lengths in bits
// XXX this is duplicate, it's already in Ieee80211Frame.msg
const unsigned int LENGTH_RTS = 160;
const unsigned int LENGTH_CTS = 112;
const unsigned int LENGTH_ACK = 112;

// time slot ST, short interframe space SIFS, distributed interframe
// space DIFS, and extended interframe space EIFS
const_simtime_t ST = 20E-6;
const_simtime_t SIFS = 10E-6;
const_simtime_t DIFS = 2 * ST + SIFS;
const_simtime_t MAX_PROPAGATION_DELAY = 2E-6; // 300 meters at the speed of light

const int RETRY_LIMIT = 7;

/** Minimum size (initial size) of contention window */
const int CW_MIN = 31;

/** Maximum size of contention window */
const int CW_MAX = 255;

const int PHY_HEADER_LENGTH = 192;
const int HEADER_WITHOUT_PREAMBLE = 48;
const double BITRATE_HEADER = 1E+6;
const double BANDWIDTH = 2E+6;

#endif
