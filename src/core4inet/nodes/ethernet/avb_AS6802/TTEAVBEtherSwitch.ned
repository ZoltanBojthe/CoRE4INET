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

package core4inet.nodes.ethernet.avb_AS6802;

import core4inet.buffer.avb.AVBQueueBuffer;
import core4inet.incoming.avb.AVBIncoming;
import core4inet.linklayer.beHandling.avb.relay.SRPRelay;
import core4inet.linklayer.port.TTEAVBPHYPort;
import core4inet.scheduler.ITTEScheduler;
import core4inet.services.avb.SRP.SRPTable;
import core4inet.services.avb.SRP.SRProtocol;
import core4inet.synchronisation.base.ISync;
import core4inet.utilities.modules.CheckedBuffer;

import inet.common.MessageDispatcher;
import inet.linklayer.contract.IMacAddressTable;
import inet.networklayer.common.InterfaceTable;

//
// Module for a TTEAVBEthernet Switch.
// Contains handler for TT, AVB, RC and BE Messages.
//
// @see ~SRPRelay, ~IMacAddressTable, ~InterfaceTable, ~SRPTable, ~TTEAVBPHYPort, ~AVBIncoming, ~AVBQueueBuffer, ~ISync, ~ITTEScheduler, ~CheckedBuffer, ~SRProtocol
//
// @author Philipp Meyer
//
module TTEAVBEtherSwitch
{
    parameters:
        @networkNode();
        @display("i=block/switch;bgb=1280,648");

        // Optional: Number of Buffers with Incoming Module for TT and RC traffic 
        // (When not manually declared in own deriving .ned file)
        int numBuffer = default(0);
        // Forwardingdelay of the device hardware
        double hardware_delay @unit(s) = default(8us);

        *.interfaceTableModule = default(absPath(".interfaceTable"));

    gates:
        // Physical ports of the switch
        inout ethg[] @loose;

    types:
        channel ProcessingDelay extends ned.DelayChannel
        {
        }

    submodules:
        // Implementation of the best-effort forwarding part
        beswitch: SRPRelay {
            @display("p=791.265,227.91");
        }

		//The mac table associated with the be switch
        macTable: <default("MacAddressTable")> like IMacAddressTable {
            @display("p=146,417");
        }

        // Holds interface names
        interfaceTable: InterfaceTable {
            @display("p=155,80");
        }

        // Stream Reservation Protocol table
        srpTable: SRPTable {
            @display("p=308,250");
        }

        down: MessageDispatcher {
            parameters:
                @display("p=991,311;b=800,5");
        }
        // Physical ports of the switch
        phy[sizeof(ethg)]: TTEAVBPHYPort {
            parameters:
                @display("p=712,568,c,130");
                //For the switch it is necessary to have promiscuous mode enabled
                promiscuous = true;
        }

		// AVB critical traffic control
        avbCTC: AVBIncoming {
            parameters:
                @display("p=677.31,362.73");
                hardware_delay = hardware_delay;
            gates:
                AVBAout[sizeof(ethg)];
                AVBBout[sizeof(ethg)];
        }

		// AVB class A buffer
        avbABuffer[sizeof(ethg)]: AVBQueueBuffer {
            parameters:
                srClass = "A";
                @display("p=712,455,c,130");
        }

		// AVB class B buffer
        avbBBuffer[sizeof(ethg)]: AVBQueueBuffer {
            parameters:
                srClass = "B";
                @display("p=635,454,c,130");
        }

        // Sync module for synchronisation of the scheduler
        sync: <default("DummySync")> like ISync {
            parameters:
                @display("p=146,250");
        }

        // Scheduler for the device
        scheduler: <default("Scheduler")> like ITTEScheduler {
            parameters:
                @display("p=308,80");
        }

        // Buffers with Incoming Module for TT and RC traffic (optional)
        checkedBuffer[numBuffer]: CheckedBuffer {
            @display("p=285,455");
        }

        // Stream Reservation Protocol implementation
        srpProtocol: SRProtocol {
            @display("p=791,110");
        }

    connections:
        for i=0..sizeof(ethg)-1 {
            avbCTC.AVBAout[i] --> avbABuffer[i].in;
            avbCTC.AVBBout[i] --> avbBBuffer[i].in;
            avbABuffer[i].out --> phy[i].AVBAin;
            avbBBuffer[i].out --> phy[i].AVBBin;

            down.out++ --> phy[i].upperLayerIn;
            down.in++ <-- phy[i].upperLayerOut;
            phy[i].phys <--> { @display("m=s"); } <--> ethg[i];
        }
        beswitch.ifOut --> ProcessingDelay {  delay = (hardware_delay/2); } --> down.in++;
        beswitch.ifIn <-- ProcessingDelay {  delay = (hardware_delay/2); } <-- down.out++;

        srpProtocol.out --> beswitch.srpIn;
        srpProtocol.in <-- beswitch.srpOut;
}
