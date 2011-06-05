
enum WiMAXPhyCommandCode
{
    WIMAXPHY_C_CONFIGURERADIO_DOWNLINK = 1;
    WIMAXPHY_C_CONFIGURERADIO_UPLINK = 2;
    WIMAXPHY_C_GET_UPLINK_CHANNEL = 3;
    WIMAXPHY_C_GET_DOWNLINK_CHANNEL = 4;
}

//
// Control info for controlling the physical layer (the radio).
//
class WiMAXPhyControlInfo
{
    int channelNumber = -1; // with PHY_C_CONFIGURERADIO: the channel to switch to
    double bitrate = -1; // with PHY_C_CONFIGURERADIO: the bitrate to switch to
}
