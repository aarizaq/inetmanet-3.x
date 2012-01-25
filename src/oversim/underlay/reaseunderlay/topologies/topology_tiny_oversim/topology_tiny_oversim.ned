package inet.oversim.underlay.reaseunderlay.topologies.topology_tiny_oversim;

import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;

channel host2edge extends ned.DatarateChannel
{
	parameters:
		delay = 1us;
		datarate = 1Mbps;
}
channel edge2host extends ned.DatarateChannel
{
	parameters:
		delay = 1us;
		datarate = 1Mbps;
}
channel edge2gateway extends ned.DatarateChannel
{
	parameters:
		delay = 1us;
		datarate = 155Mbps;
}
channel gateway2core extends ned.DatarateChannel
{
	parameters:
		delay = 1us;
		datarate = 622Mbps;
}
channel core2core extends ned.DatarateChannel
{
	parameters:
		delay = 1us;
		datarate = 1000Mbps;
}
channel stub2stub extends ned.DatarateChannel
{
	parameters:
		delay = 1us;
		datarate = 1000Mbps;
}
channel stub2transit extends ned.DatarateChannel
{
	parameters:
		delay = 1us;
		datarate = 5000Mbps;
}
channel transit2transit extends ned.DatarateChannel
{
	parameters:
		delay = 1us;
		datarate = 10000Mbps;
}

module Internet extends inet.oversim.underlay.reaseunderlay.ReaSEUnderlayNetworkBase
{
	parameters:
		@Internet();
	submodules:
		tas0: TAS0 {
			parameters:
				@AS();
				@display("i=misc/globe_s");
		}
		sas1: SAS1 {
			parameters:
				@AS();
				@display("i=misc/cloud_s");
		}
		sas2: SAS2 {
			parameters:
				@AS();
				@display("i=misc/cloud_s");
		}

	connections:
		tas0.pppg++ <--> stub2transit <--> sas1.pppg++;
		tas0.pppg++ <--> stub2transit <--> sas2.pppg++;
		sas1.pppg++ <--> stub2stub <--> sas2.pppg++;

}

network ReaSEUnderlayNetwork extends Internet
{
	parameters:
}

module TAS0
{
	gates:
		inout pppg[];
	submodules:
		core0: Router {
			parameters:
				@RL();
				@CoreRouter();
				@display("i=abstract/switch");
		}
		gw1: Router {
			parameters:
				@RL();
				@GatewayRouter();
				@display("i=abstract/router2");
		}
		edge2: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		edge3: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		edge4: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		edge5: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		edge6: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		host7: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host8: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host9: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host10: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host11: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host12: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host13: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host14: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host15: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host16: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host17: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host18: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host19: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host20: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host21: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host22: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}

	connections:
		pppg++ <--> core0.pppg++;
		pppg++ <--> core0.pppg++;

		core0.pppg++ <--> gateway2core <--> gw1.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge2.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge3.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge4.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge5.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge6.pppg++;
		host7.pppg$o++ --> host2edge --> edge2.pppg$i++;
		edge2.pppg$o++ --> edge2host --> host7.pppg$i++;
		host8.pppg$o++ --> host2edge --> edge2.pppg$i++;
		edge2.pppg$o++ --> edge2host --> host8.pppg$i++;
		host9.pppg$o++ --> host2edge --> edge2.pppg$i++;
		edge2.pppg$o++ --> edge2host --> host9.pppg$i++;
		host10.pppg$o++ --> host2edge --> edge2.pppg$i++;
		edge2.pppg$o++ --> edge2host --> host10.pppg$i++;
		host11.pppg$o++ --> host2edge --> edge3.pppg$i++;
		edge3.pppg$o++ --> edge2host --> host11.pppg$i++;
		host12.pppg$o++ --> host2edge --> edge3.pppg$i++;
		edge3.pppg$o++ --> edge2host --> host12.pppg$i++;
		host13.pppg$o++ --> host2edge --> edge4.pppg$i++;
		edge4.pppg$o++ --> edge2host --> host13.pppg$i++;
		host14.pppg$o++ --> host2edge --> edge4.pppg$i++;
		edge4.pppg$o++ --> edge2host --> host14.pppg$i++;
		host15.pppg$o++ --> host2edge --> edge4.pppg$i++;
		edge4.pppg$o++ --> edge2host --> host15.pppg$i++;
		host16.pppg$o++ --> host2edge --> edge4.pppg$i++;
		edge4.pppg$o++ --> edge2host --> host16.pppg$i++;
		host17.pppg$o++ --> host2edge --> edge5.pppg$i++;
		edge5.pppg$o++ --> edge2host --> host17.pppg$i++;
		host18.pppg$o++ --> host2edge --> edge5.pppg$i++;
		edge5.pppg$o++ --> edge2host --> host18.pppg$i++;
		host19.pppg$o++ --> host2edge --> edge5.pppg$i++;
		edge5.pppg$o++ --> edge2host --> host19.pppg$i++;
		host20.pppg$o++ --> host2edge --> edge5.pppg$i++;
		edge5.pppg$o++ --> edge2host --> host20.pppg$i++;
		host21.pppg$o++ --> host2edge --> edge6.pppg$i++;
		edge6.pppg$o++ --> edge2host --> host21.pppg$i++;
		host22.pppg$o++ --> host2edge --> edge6.pppg$i++;
		edge6.pppg$o++ --> edge2host --> host22.pppg$i++;
}

module SAS1
{
	gates:
		inout pppg[];
	submodules:
		core0: Router {
			parameters:
				@RL();
				@CoreRouter();
				@display("i=abstract/switch");
		}
		gw1: Router {
			parameters:
				@RL();
				@GatewayRouter();
				@display("i=abstract/router2");
		}
		edge2: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		edge3: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		edge4: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		edge5: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		host6: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host7: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host8: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host9: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host10: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host11: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host12: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host13: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host14: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host15: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host16: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host17: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}

	connections:
		pppg++ <--> core0.pppg++;
		pppg++ <--> core0.pppg++;

		core0.pppg++ <--> gateway2core <--> gw1.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge2.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge3.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge4.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge5.pppg++;
		host6.pppg$o++ --> host2edge --> edge2.pppg$i++;
		edge2.pppg$o++ --> edge2host --> host6.pppg$i++;
		host7.pppg$o++ --> host2edge --> edge2.pppg$i++;
		edge2.pppg$o++ --> edge2host --> host7.pppg$i++;
		host8.pppg$o++ --> host2edge --> edge3.pppg$i++;
		edge3.pppg$o++ --> edge2host --> host8.pppg$i++;
		host9.pppg$o++ --> host2edge --> edge3.pppg$i++;
		edge3.pppg$o++ --> edge2host --> host9.pppg$i++;
		host10.pppg$o++ --> host2edge --> edge3.pppg$i++;
		edge3.pppg$o++ --> edge2host --> host10.pppg$i++;
		host11.pppg$o++ --> host2edge --> edge4.pppg$i++;
		edge4.pppg$o++ --> edge2host --> host11.pppg$i++;
		host12.pppg$o++ --> host2edge --> edge4.pppg$i++;
		edge4.pppg$o++ --> edge2host --> host12.pppg$i++;
		host13.pppg$o++ --> host2edge --> edge4.pppg$i++;
		edge4.pppg$o++ --> edge2host --> host13.pppg$i++;
		host14.pppg$o++ --> host2edge --> edge4.pppg$i++;
		edge4.pppg$o++ --> edge2host --> host14.pppg$i++;
		host15.pppg$o++ --> host2edge --> edge5.pppg$i++;
		edge5.pppg$o++ --> edge2host --> host15.pppg$i++;
		host16.pppg$o++ --> host2edge --> edge5.pppg$i++;
		edge5.pppg$o++ --> edge2host --> host16.pppg$i++;
		host17.pppg$o++ --> host2edge --> edge5.pppg$i++;
		edge5.pppg$o++ --> edge2host --> host17.pppg$i++;
}

module SAS2
{
	gates:
		inout pppg[];
	submodules:
		core0: Router {
			parameters:
				@RL();
				@CoreRouter();
				@display("i=abstract/switch");
		}
		gw1: Router {
			parameters:
				@RL();
				@GatewayRouter();
				@display("i=abstract/router2");
		}
		edge2: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		edge3: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		edge4: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		edge5: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		edge6: Router {
			parameters:
				@RL();
				@EdgeRouter();
				@display("i=abstract/router");
		}
		host7: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host8: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host9: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host10: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host11: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host12: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host13: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host14: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host15: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host16: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host17: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host18: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host19: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host20: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host21: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host22: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}
		host23: StandardHost {
			parameters:
				@RL();
				@Host();
				@display("i=device/laptop");
		}

	connections:
		pppg++ <--> core0.pppg++;
		pppg++ <--> core0.pppg++;

		core0.pppg++ <--> gateway2core <--> gw1.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge2.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge3.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge4.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge5.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge6.pppg++;
		host7.pppg$o++ --> host2edge --> edge2.pppg$i++;
		edge2.pppg$o++ --> edge2host --> host7.pppg$i++;
		host8.pppg$o++ --> host2edge --> edge2.pppg$i++;
		edge2.pppg$o++ --> edge2host --> host8.pppg$i++;
		host9.pppg$o++ --> host2edge --> edge2.pppg$i++;
		edge2.pppg$o++ --> edge2host --> host9.pppg$i++;
		host10.pppg$o++ --> host2edge --> edge2.pppg$i++;
		edge2.pppg$o++ --> edge2host --> host10.pppg$i++;
		host11.pppg$o++ --> host2edge --> edge3.pppg$i++;
		edge3.pppg$o++ --> edge2host --> host11.pppg$i++;
		host12.pppg$o++ --> host2edge --> edge3.pppg$i++;
		edge3.pppg$o++ --> edge2host --> host12.pppg$i++;
		host13.pppg$o++ --> host2edge --> edge3.pppg$i++;
		edge3.pppg$o++ --> edge2host --> host13.pppg$i++;
		host14.pppg$o++ --> host2edge --> edge3.pppg$i++;
		edge3.pppg$o++ --> edge2host --> host14.pppg$i++;
		host15.pppg$o++ --> host2edge --> edge4.pppg$i++;
		edge4.pppg$o++ --> edge2host --> host15.pppg$i++;
		host16.pppg$o++ --> host2edge --> edge4.pppg$i++;
		edge4.pppg$o++ --> edge2host --> host16.pppg$i++;
		host17.pppg$o++ --> host2edge --> edge4.pppg$i++;
		edge4.pppg$o++ --> edge2host --> host17.pppg$i++;
		host18.pppg$o++ --> host2edge --> edge5.pppg$i++;
		edge5.pppg$o++ --> edge2host --> host18.pppg$i++;
		host19.pppg$o++ --> host2edge --> edge5.pppg$i++;
		edge5.pppg$o++ --> edge2host --> host19.pppg$i++;
		host20.pppg$o++ --> host2edge --> edge6.pppg$i++;
		edge6.pppg$o++ --> edge2host --> host20.pppg$i++;
		host21.pppg$o++ --> host2edge --> edge6.pppg$i++;
		edge6.pppg$o++ --> edge2host --> host21.pppg$i++;
		host22.pppg$o++ --> host2edge --> edge6.pppg$i++;
		edge6.pppg$o++ --> edge2host --> host22.pppg$i++;
		host23.pppg$o++ --> host2edge --> edge6.pppg$i++;
		edge6.pppg$o++ --> edge2host --> host23.pppg$i++;
}

