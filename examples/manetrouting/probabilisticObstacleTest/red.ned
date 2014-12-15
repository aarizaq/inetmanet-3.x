package inet.examples.manetrouting.probabilisticObstacleTest;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.AdhocHost;
import inet.world.radio.ChannelControl;
import inet.world.obstacles.ObstacleControl;
import inet.world.annotations.AnnotationManager;

network red
{
    parameters:
        int numHosts;

    submodules:
        obstacles: ObstacleControl {
            @display("p=139,24");
        }
        annotations: AnnotationManager {
            @display("p=235,24");
        }
        host[numHosts]: AdhocHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=473,51");
        }
        channelControl: ChannelControl {
            parameters:
                @display("p=60,50;i=misc/sun");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=140,50;i=block/cogwheel_s");
        }
    connections allowunconnected:
}



