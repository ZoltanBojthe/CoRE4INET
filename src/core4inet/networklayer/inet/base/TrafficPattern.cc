/*
 * CoRE4INET_TrafficPattern.cc
 *
 *  Created on: 09.09.2014
 *      Author: Kai-Uwe
 */

//==============================================================================
#include "core4inet/networklayer/inet/base/TrafficPattern.h"

//==============================================================================

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/sctp/SctpHeader.h"

//==============================================================================

namespace CoRE4INET {

//==============================================================================

TrafficPattern::TrafficPattern() :
        srcAddr(), srcPrefixLength(-1), destAddr(), destPrefixLength(-1), protocol(-1), tos(-1), tosMask(0), srcPortMin(
                -1), srcPortMax(-1), destPortMin(-1), destPortMax(-1)
{
}

//==============================================================================

TrafficPattern::~TrafficPattern()
{
}

//==============================================================================

bool TrafficPattern::matches(const cPacket *cpacket)
{
    const inet::Packet *packet = dynamic_cast<const inet::Packet*>(cpacket);
    if (!packet)
        return false;

    auto protocolTag = packet->findTag<inet::PacketProtocolTag>();
    if (!protocolTag)
        return false;
    const inet::Protocol *curProtocol = protocolTag->getProtocol();
    if (curProtocol != &inet::Protocol::ipv4 && curProtocol != &inet::Protocol::ipv6)
        return false;

    const auto& datagram = inet::peekNetworkProtocolHeader(packet, *curProtocol);

    if (srcPrefixLength > 0
            && ((srcAddr.getType() == inet::L3Address::IPv6)        //TODO L3Address.matches(other, length) also works with IPv6 addresses
                    || !datagram->getSourceAddress().matches(srcAddr, srcPrefixLength)))
        return false;
    if (destPrefixLength > 0
            && ((destAddr.getType() == inet::L3Address::IPv6)        //TODO L3Address.matches(other, length) also works with IPv6 addresses
                    || !datagram->getDestinationAddress().matches(destAddr, destPrefixLength)))
        return false;
    if (protocol >= 0 && inet::ProtocolGroup::ipprotocol.getProtocolNumber(datagram->getProtocol()) != protocol)
        return false;
    if (tosMask != 0) {
        if (auto ipv4Header = inet::dynamicPtrCast<const inet::Ipv4Header>(datagram)) {
            if ((tos & tosMask) != (datagram->getTypeOfService() & tosMask))
                return false;
        }
     }
    if (srcPortMin >= 0 || destPortMin >= 0)
    {
        int srcPort = -1, destPort = -1;
        auto curProtocol = datagram->getProtocol();
        if (*curProtocol == inet::Protocol::udp)
        {
            auto udpPacket = packet->peekAt<inet::UdpHeader>(datagram->getChunkLength());
            srcPort = static_cast<int>(udpPacket->getSourcePort());
            destPort = static_cast<int>(udpPacket->getDestinationPort());
        }
        else if (*curProtocol == inet::Protocol::tcp)
        {
            auto tcpSegment = packet->peekAt<inet::tcp::TcpHeader>(datagram->getChunkLength());
            srcPort = tcpSegment->getSrcPort();
            destPort = tcpSegment->getDestPort();
        }
        else if (*curProtocol == inet::Protocol::sctp)
        {
            auto sctpPacket = packet->peekAt<inet::sctp::SctpHeader>(datagram->getChunkLength());
            srcPort = static_cast<int>(sctpPacket->getSrcPort());
            destPort = static_cast<int>(sctpPacket->getDestPort());
        }

        if (srcPortMin >= 0 && (srcPort < srcPortMin || srcPort > srcPortMax))
            return false;
        if (destPortMin >= 0 && (destPort < destPortMin || destPort > destPortMax))
            return false;
    }
    return true;
}

//==============================================================================

} /* namespace CoRE4INET */

//==============================================================================
// EOF
