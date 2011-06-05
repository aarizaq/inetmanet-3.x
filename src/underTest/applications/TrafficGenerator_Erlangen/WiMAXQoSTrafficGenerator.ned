package inet.underTest.applications.TrafficGenerator_Erlangen;

simple WiMAXQoSTrafficGenerator
{
    parameters:
        double defaultTrafConfigId;
        xml trafConfig;
		//ugs_station: string;

    gates:
        input lowergateIn;
        output lowergateOut;
}
