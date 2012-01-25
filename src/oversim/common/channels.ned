package inet.oversim.common;

// Ethernet channel for SimpleUnderlay
channel simple_ethernetline extends ned.DatarateChannel
{
    parameters:
        delay = 0ms;
        datarate = 10Mbps;
}

// Ethernet channel with packet loss for SimpleUnderlay
channel simple_ethernetline_lossy extends ned.DatarateChannel
{
    parameters:
        delay = 0ms;
        datarate = 10Mbps;
        ber = 1e-5;
}

// DSL channel for SimpleUnderlay
channel simple_dsl extends ned.DatarateChannel
{
    parameters:
        delay = 20ms;
        datarate = 1Mbps;
}

// DSL channel with packet loss for SimpleUnderlay
channel simple_dsl_lossy extends ned.DatarateChannel
{
    parameters:
        delay = 20ms;
        datarate = 1Mbps;
        ber = 1e-5;
}

// Fiberline channel for InetUnderlay
channel inet_fiberline extends ned.DatarateChannel
{
    parameters:
        delay = 1ms;
        datarate = 10Gbps;
}

// Ethernet channel for InetUnderlay
channel inet_ethernetline extends ned.DatarateChannel
{
    parameters:
        delay = 10ms;
        datarate = 10Mbps;
}

// Ethernet channel with packet loss for InetUnderlay
channel inet_ethernetline_lossy extends ned.DatarateChannel
{
    parameters:
        delay = 10ms;
        datarate = 10Mbps;
        ber = 1e-5;
}

// DSL channel for InetUnderlay
channel inet_dsl extends ned.DatarateChannel
{
    parameters:
        delay = 30ms;
        datarate = 1Mbps;
}

// DSL channel with packet loss for InetUnderlay
channel inet_dsl_lossy extends ned.DatarateChannel
{
    parameters:
        delay = 30ms;
        datarate = 1Mbps;
        ber = 1e-5;
}

// 56k modem channel for InetUnderlay
channel inet_dialup_56k extends ned.DatarateChannel
{
    parameters:
        delay = 220ms;
        datarate = 56kbps;
}
