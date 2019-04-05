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
import inet.linklayer.ethernet.IEtherEncap;
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
        @display("i=block/switch;bgb=1278,839");

        // Optional: Number of Buffers with Incoming Module for TT and RC traffic 
        // (When not manually declared in own deriving .ned file)
        int numBuffer = default(0);
        // Forwardingdelay of the device hardware
        double hardware_delay @unit(s) = default(8us);

        *.interfaceTableModule = default(absPath(".interfaceTable"));
        beswitch.hasStp = default(false);
        encap.fcsMode = default("declared");

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
            @display("p=791,415");
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

        up: MessageDispatcher {
            parameters:
                @display("p=890,195;b=600,5");
        }

        down: MessageDispatcher {
            parameters:
                @display("p=890,497;b=600,5");
        }
        // Physical ports of the switch
        phy[sizeof(ethg)]: TTEAVBPHYPort {
            parameters:
                @display("p=711,754,c,130");
                //For the switch it is necessary to have promiscuous mode enabled
                promiscuous = true;
        }

		// AVB critical traffic control
        avbCTC: AVBIncoming {
            parameters:
                @display("p=677,548");
                hardware_delay = hardware_delay;
            gates:
                AVBAout[sizeof(ethg)];
                AVBBout[sizeof(ethg)];
        }

		// AVB class A buffer
        avbABuffer[sizeof(ethg)]: AVBQueueBuffer {
            parameters:
                srClass = "A";
                @display("p=711,642,c,130");
        }

		// AVB class B buffer
        avbBBuffer[sizeof(ethg)]: AVBQueueBuffer {
            parameters:
                srClass = "B";
                @display("p=632,640,c,130");
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
            @display("p=791,80");
        }

        encap: <default("EtherEncap")> like IEtherEncap {
            @display("p=789,272");
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

        srpProtocol.out --> up.in++;
        srpProtocol.in <-- up.out++;

        encap.upperLayerOut --> up.in++;
        encap.upperLayerIn <-- up.out++;

        encap.lowerLayerIn <-- beswitch.upperLayerOut;
        encap.lowerLayerOut --> beswitch.upperLayerIn;
}

