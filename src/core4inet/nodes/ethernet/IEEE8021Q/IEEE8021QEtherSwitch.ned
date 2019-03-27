//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
package core4inet.nodes.ethernet.IEEE8021Q;

import core4inet.linklayer.port.IEEE8021QPHYPort;
import core4inet.scheduler.ITTEScheduler;
import core4inet.synchronisation.base.ISync;
import core4inet.utilities.modules.CheckedBuffer;
import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IMacAddressTable;
import inet.linklayer.contract.IMacRelayUnit;
import inet.networklayer.common.InterfaceTable;


//
// Module for a IEEE 802.1Q Ethernet Switch with variable number of ports.
//
// @see ~NodeStatus, ~IMacRelayUnit, ~IMacAddressTable, ~InterfaceTable, ~IEEE8021QPHYPort, ~ISync, ~ITTEScheduler, ~CheckedBuffer
//
// @author Till Steinbach, Philipp Meyerr 
//
module IEEE8021QEtherSwitch
{
    parameters:
        @display("i=block/switch;bgb=514,312");
        @networkNode;
        @labels(node,ethernet-node);

        // Optional: Number of Buffers with Incoming Module for TT and RC traffic 
        // (When not manually declared in own deriving .ned file)
        int numBuffer = default(0);
        // Forwardingdelay of the device hardware
        double hardware_delay @unit(s) = default(8us);

        *.interfaceTableModule = default(absPath(".interfaceTable"));

    gates:
        // Physical ports of the switch
        inout ethg[] @loose @labels(EtherFrame-conn);

    types:
        channel ProcessingDelay extends ned.DelayChannel
        {
        }

    submodules:
        // Keeps track of the status of network node (up, down, etc.)
        status: NodeStatus {
            @display("p=47,50;is=n");
        }

        // Implementation of the forwarding part
        beswitch: <default("MacRelayUnit")> like IMacRelayUnit {
            @display("p=326,123");
        }

        //The mac table associated with the beswitch
        macTable: <default("MacAddressTable")> like IMacAddressTable {
            @display("p=124,50");
        }

        // Holds interface names
        interfaceTable: InterfaceTable {
            @display("p=47,202");
        }

        // Physical ports of the switch
        phy[sizeof(ethg)]: IEEE8021QPHYPort {
            parameters:
                @display("p=171,260,column,50");
                //For the switch it is necessary to have promiscuous mode enabled
                promiscuous = true;
        }

        // Sync module for synchronisation of the scheduler
        sync: <default("DummySync")> like ISync {
            @display("p=47,123");
        }

        // Scheduler for the device
        scheduler: <default("Scheduler")> like ITTEScheduler {
            @display("p=124,123");
        }

        // Buffers with Incoming Module for TT and RC traffic (optional)
        checkedBuffer[numBuffer]: CheckedBuffer {
            @display("p=240,38");
        }

        down: MessageDispatcher {
            @display("p=326,201;b=350,5");
        }

    connections:
        down.in++ <-- ProcessingDelay {  delay = (hardware_delay/2); } <-- beswitch.ifOut;
        down.out++ --> ProcessingDelay {  delay = (hardware_delay/2); } --> beswitch.ifIn;
        for i=0..sizeof(ethg)-1 {
            phy[i].phys <--> { @display("m=s"); } <--> ethg[i];
            phy[i].upperLayerIn <-- down.out++;
            phy[i].upperLayerOut --> down.in++;
        }
}

