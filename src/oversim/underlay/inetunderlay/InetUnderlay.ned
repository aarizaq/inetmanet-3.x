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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


package inet.oversim.underlay.inetunderlay;

import inet.oversim.common.inet_fiberline;
import inet.oversim.common.GlobalObserver;
import inet.oversim.common.ChurnGenerator;


//
// The InetUnderlay ned-file @see InetUnderlayConfigurator
//
network InetUnderlayNetwork
{
    parameters:
        string overlayType;    // the overlay used in the simulation (for overlayBackboneRouters)
        string tier1Type;    // the application on top of the overlay used in the simulation (for overlayBackboneRouters)
        string tier2Type;    // the module type on top of tier 1 (for overlayBackboneRouters)
        string tier3Type;    // the module type on top of tier 2 (for overlayBackboneRouters)
        int backboneRouterNum;    // number of backbone routers in the network
        int accessRouterNum;    // number of access routers in the network
        int overlayBackboneRouterNum;    // number of backbone routers in the network that participate the overlay
        int overlayAccessRouterNum;    // number of access routers in the network that participate the overlay
        int outRouterNum;    // set to 1 if you want to use a TunOutRouter as connection to a real network
        double connectivity;    // degree of connectivity of backbone routers (1 = every backbone router is connected to all the others)
    submodules:
        overlayTerminal[0]: InetOverlayHost {
            @dynamic;
        }
        underlayConfigurator: InetUnderlayConfigurator {
            @display("p=70,160");
        }
        backboneRouter[backboneRouterNum]: Router {
            parameters:
                @display("i=device/server2");
        }
        outRouter[min(1, outRouterNum)]: TunOutRouter;
        accessRouter[accessRouterNum]: AccessRouter {
            parameters:
                @display("i=device/server2,gray,30");
        }
        overlayAccessRouter[overlayAccessRouterNum]: OverlayAccessRouter {
            parameters:
                overlayType = overlayType;    // the overlay used in the simulation
                tier1Type = tier1Type;    // the application on top off the overly used in the simulation
                tier2Type = tier2Type;    // tier 2 module to use
                tier3Type = tier3Type;    // tier 3 module to use
                @display("i=device/server2,gray,30;i2=block/circle_s");
        }
        overlayBackboneRouter[overlayBackboneRouterNum]: OverlayRouter {
            parameters:
                overlayType = overlayType;    // the overlay used in the simulation
                tier1Type = tier1Type;    // the application on top off the overly used in the simulation
                tier2Type = tier2Type;    // tier 2 module to use
                tier3Type = tier3Type;    // tier 3 module to use
                @display("i=device/server2;i2=block/circle_s");
        }
        churnGenerator[0]: ChurnGenerator {
            @display("p=70,240,column");
        }
        globalObserver: GlobalObserver {
            parameters:
                @display("i=block/control;p=70,50");
        }

    connections allowunconnected:

        //
        // BACKBONE ROUTER
        //
        // connect outRouter to a backboneRouter
        backboneRouter[0].pppg$o++ --> inet_fiberline --> outRouter[0].pppg$i++ if outRouterNum>0;
        backboneRouter[0].pppg$i++ <-- inet_fiberline <-- outRouter[0].pppg$o++ if outRouterNum>0;

        // connect backboneRouters with other backboneRouters
        for i=0..backboneRouterNum-1, for j=0..backboneRouterNum-1 {
            backboneRouter[i].pppg$o++ --> inet_fiberline --> backboneRouter[j].pppg$i++ if i<j&&uniform2(0, 1, i, 1)<=connectivity;
            backboneRouter[i].pppg$i++ <-- inet_fiberline <-- backboneRouter[j].pppg$o++ if i<j&&uniform2(0, 1, i, 0)<=connectivity;
        }

        // connect backboneRouters with overlayBackboneRouters
        for i=0..backboneRouterNum-1, for j=0..overlayBackboneRouterNum-1 {
            backboneRouter[i].pppg$o++ --> inet_fiberline --> overlayBackboneRouter[j].pppg$i++ if i<=j&&uniform2(0, 1, i, 0)<=connectivity;
            backboneRouter[i].pppg$i++ <-- inet_fiberline <-- overlayBackboneRouter[j].pppg$o++ if i<=j&&uniform2(0, 1, i, 0)<=connectivity;
        }

        // connect overlayBackboneRouters with backboneRouters
        for i=0..overlayBackboneRouterNum-1, for j=0..backboneRouterNum-1 {
            overlayBackboneRouter[i].pppg$o++ --> inet_fiberline --> backboneRouter[j].pppg$i++ if i<j&&uniform2(0, 1, i, 1)<=connectivity;
            overlayBackboneRouter[i].pppg$i++ <-- inet_fiberline <-- backboneRouter[j].pppg$o++ if i<j&&uniform2(0, 1, i, 0)<=connectivity;
        }

        // connect overlayBackboneRouters with other overlayBackboneRouters
        for i=0..overlayBackboneRouterNum-1, for j=0..overlayBackboneRouterNum-1 {
            overlayBackboneRouter[i].pppg$o++ --> inet_fiberline --> overlayBackboneRouter[j].pppg$i++ if i<j&&uniform2(0, 1, i, 0)<=connectivity;
            overlayBackboneRouter[i].pppg$i++ <-- inet_fiberline <-- overlayBackboneRouter[j].pppg$o++ if i<j&&uniform2(0, 1, i, 0)<=connectivity;
        }

        //
        // ACCESS ROUTER
        //
        // connect accessRouters with backboneRouters
        for i=0..accessRouterNum-1 {
            accessRouter[i].pppg$o++ --> inet_fiberline --> backboneRouter[intuniform2(0, backboneRouterNum-1, 0, 1)].pppg$i++ if uniform2(0, 1, i, 1)>overlayBackboneRouterNum/(overlayBackboneRouterNum+backboneRouterNum);
            accessRouter[i].pppg$i++ <-- inet_fiberline <-- backboneRouter[intuniform2(0, backboneRouterNum-1, 0, 0)].pppg$o++ if uniform2(0, 1, i, 0)>overlayBackboneRouterNum/(overlayBackboneRouterNum+backboneRouterNum);
        }

        // connect accessRouters with overlayBackboneRouters
        for i=0..accessRouterNum-1 {
            accessRouter[i].pppg$o++ --> inet_fiberline --> overlayBackboneRouter[intuniform2(0, overlayBackboneRouterNum-1, 0, 1)].pppg$i++ if uniform2(0, 1, i, 0)<=overlayBackboneRouterNum/(overlayBackboneRouterNum+backboneRouterNum);
            accessRouter[i].pppg$i++ <-- inet_fiberline <-- overlayBackboneRouter[intuniform2(0, overlayBackboneRouterNum-1, 0, 0)].pppg$o++ if uniform2(0, 1, i, 0)<=overlayBackboneRouterNum/(overlayBackboneRouterNum+backboneRouterNum);
        }

        // connect overlayAccessRouters with backboneRouters
        for i=0..overlayAccessRouterNum-1 {
            overlayAccessRouter[i].pppg$o++ --> inet_fiberline --> backboneRouter[intuniform2(0, backboneRouterNum-1, 0, 1)].pppg$i++ if uniform2(0, 1, i, 1)>overlayBackboneRouterNum/(overlayBackboneRouterNum+backboneRouterNum);
            overlayAccessRouter[i].pppg$i++ <-- inet_fiberline <-- backboneRouter[intuniform2(0, backboneRouterNum-1, 0, 0)].pppg$o++ if uniform2(0, 1, i, 0)>overlayBackboneRouterNum/(overlayBackboneRouterNum+backboneRouterNum);
        }

        // connect overlayAccessRouters with overlayBackboneRouters
        for i=0..overlayAccessRouterNum-1 {
            overlayAccessRouter[i].pppg$o++ --> inet_fiberline --> overlayBackboneRouter[intuniform2(0, overlayBackboneRouterNum-1, 0, 1)].pppg$i++ if uniform2(0, 1, i, 0)<=overlayBackboneRouterNum/(overlayBackboneRouterNum+backboneRouterNum);
            overlayAccessRouter[i].pppg$i++ <-- inet_fiberline <-- overlayBackboneRouter[intuniform2(0, overlayBackboneRouterNum-1, 0, 0)].pppg$o++ if uniform2(0, 1, i, 0)<=overlayBackboneRouterNum/(overlayBackboneRouterNum+backboneRouterNum);
        }

}
