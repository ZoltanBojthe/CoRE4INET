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

package core4inet.nodes.inet;

import inet.networklayer.ipv4.Ipv4NetworkLayer;


//
// Network layer of an Ipv4 node.
//
// Interfaces to transport layer: Tcp, Udp, echo/ping, Rsvp
//
// Supports Realtime Ethernet by using IPv4oRE Module
//
// This Module is mainly cloned from the inet.nodes.inet.NetworkLayer module
//
module RTNetworkLayer extends Ipv4NetworkLayer
{
    parameters:
        ip.typename = "IPv4oRE";
    gates:
        input RCin;
        input TTin;

    connections allowunconnected:
        RCin --> ip.RCIn;
        TTin --> ip.TTIn;
}

