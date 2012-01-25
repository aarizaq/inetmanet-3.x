package inet.oversim.underlay.reaseunderlay.topologies.topology_example;

import inet.nodes.inet.Router;

channel host2edge extends ned.DatarateChannel
{
	parameters:
		delay = 5ms;
		datarate = 0.128Mbps;
}
channel edge2host extends ned.DatarateChannel
{
	parameters:
		delay = 5ms;
		datarate = 0.768Mbps;
}
channel edge2gateway extends ned.DatarateChannel
{
	parameters:
		delay = 1ms;
		datarate = 155.0Mbps;
}
channel gateway2core extends ned.DatarateChannel
{
	parameters:
		delay = 1ms;
		datarate = 1000.0Mbps;
}
channel core2core extends ned.DatarateChannel
{
	parameters:
		delay = 1ms;
		datarate = 2500.0Mbps;
}
channel stub2stub extends ned.DatarateChannel
{
	parameters:
		delay = 20ms;
		datarate = 5000.0Mbps;
}
channel stub2transit extends ned.DatarateChannel
{
	parameters:
		delay = 20ms;
		datarate = 5000.0Mbps;
}
channel transit2transit extends ned.DatarateChannel
{
	parameters:
		delay = 50ms;
		datarate = 10000.0Mbps;
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

	connections:
		pppg++ <--> core0.pppg++;
		pppg++ <--> core0.pppg++;

		core0.pppg++ <--> gateway2core <--> gw1.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge2.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge3.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge4.pppg++;
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

	connections:
		pppg++ <--> core0.pppg++;

		core0.pppg++ <--> gateway2core <--> gw1.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge2.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge3.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge4.pppg++;
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

	connections:
		pppg++ <--> core0.pppg++;

		core0.pppg++ <--> gateway2core <--> gw1.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge2.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge3.pppg++;
		gw1.pppg++ <--> edge2gateway <--> edge4.pppg++;
}

