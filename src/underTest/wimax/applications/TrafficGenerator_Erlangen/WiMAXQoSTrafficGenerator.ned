package inet.underTest.wimax.applications.TrafficGenerator_Erlangen;

simple WiMAXQoSTrafficGenerator
{
    parameters:
        double defaultTrafConfigId;
        xml trafConfig;
		//ugs_station: string;

    gates:
        input lowerLayerIn;
        output lowerLayerOut;
}
