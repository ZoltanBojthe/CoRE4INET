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

cplusplus {{
//Auto-generated Messages

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Winconsistent-missing-override"
#elif defined(__GNUC__) && __GNUC__ >= 5
#  pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
}}

import inet.common.packet.chunk.Chunk;

namespace CoRE4INET;

enum AVBPriority {
    AVB_VIDEO_PRIORITY = 4;
    AVB_VOICE_PRIORITY = 5;
}
//
// Represents a AVB Frame
//
// @see ~EthernetIIFrameWithQTag
//
// @author Philipp Meyer 
//
class AVBFrame extends inet::FieldsChunk
{    
    // ID of the AVB Stream
    //
    // This field is not stored in a real AVB frame. In a real network the streamid
    // is identified just through the multicast destination mac address.
    // It is just contained for debuging.
    //
    // 64 Bit Field
    unsigned long streamID;
    
	//string displayString="b=15,15,rect,black,blue,5";
}

