#include "import.hpp"
#include "mrt.hpp"

#include <iostream>
#include <iomanip>
#include <ctime>
#include "arpa/inet.h"

namespace {
	/**
	 * Read a BGP message from the stream and add the data to
	 * the routing table.
	 */
	void read_bgp_message( std::istream& stream, RouteTableV4& v4_routes, RouteTableV6& v6_routes ) {
		// Suppress warnings
		v6_routes.clear();

		std::uint8_t marker[16];
		std::uint16_t length;
		std::uint8_t type;
		// Read the BGP header from the stream
		stream.read( reinterpret_cast<char*>(&marker), sizeof(marker) );
		stream.read( reinterpret_cast<char*>(&length), sizeof(length) );
		stream.read( reinterpret_cast<char*>(&type),   sizeof(type) );

		// Correct for big Endiannes
		length = ntohs( length );

		// Check if the marker is still aligned
		for( size_t i=0; i<16; ++i ) {
			if( marker[i] != 0xFF ) std::cerr << "Marker invalid" << std::endl;
		}
		// If this is a keep-alive message we are done reading this,
		// nothing comes after this.
		if( type == BGP::Type::KEEPALIVE ) return;
		// If it's not a keep-alive and not an update message throw
		// an error tantrum.
		if( type != BGP::Type::UPDATE ) std::cerr << "Non update BGP message, type " << static_cast<int>(type) << std::endl;
		//std::cerr << "Read BGP message of type " << static_cast<int>(type) << " with length " << length << std::endl;

		std::uint16_t withdrawn_length;
		stream.read( reinterpret_cast<char*>(&withdrawn_length), sizeof(withdrawn_length) );
		withdrawn_length = ntohs( withdrawn_length );
		{
			char* tmp = new char[withdrawn_length];
			stream.read( tmp, withdrawn_length );
			delete[] tmp;
		}

		std::uint16_t path_length;
		stream.read( reinterpret_cast<char*>(&path_length), sizeof(path_length) );
		path_length = ntohs( path_length );
		{
//			std::uint8_t attr_flags, attr_type;
//			std::uint16_t attr_length;
//			// Read out the attribute header
//			stream.read( reinterpret_cast<char*>(&attr_flags), sizeof(attr_flags) );
//			stream.read( reinterpret_cast<char*>(&attr_type),  sizeof(attr_type) );
//			// The length is 2 octets if the extended flag bit is
//			// 1, otherwise 1 octet. The current code only works
//			// if the host uses little endian.
//			stream.read( reinterpret_cast<char*>(&attr_length), (attr_flags&BGP::Attribute::Flags::EXTENDED)?2:1 );
			char* tmp = new char[path_length];
			stream.read( tmp, path_length );
			delete[] tmp;
		}

		std::uint16_t reachability_length = length - 23 - withdrawn_length - path_length;
		{
			std::uint16_t length = reachability_length;
			while( length > 0 ) {
				ipv4 range;
				// Read out an IP range directly into the object.
				stream.read( reinterpret_cast<char*>(&range.suffix), sizeof(range.suffix) );
				std::uint8_t octets = (range.suffix+7)/8;
				stream.read( reinterpret_cast<char*>(range.val), octets );
				for( size_t i=octets; i<4; ++i ) range.val[i] = 0;

				// Update the length with the number of 
				// read octets.
				length -= sizeof(range.suffix) + octets;

				// Add this found route to the route table
				v4_routes.insert(range);
			}
		}
	}

	/**
	 * Read a MRT message from the stream and add the data if applicable
	 * to the routing table.
	 */
	bool read_mrt_message( std::istream& stream, RouteTableV4& v4_routes, RouteTableV6& v6_routes ) {
		uint32_t timestamp, length;
		uint16_t type, subtype;
		// Read a message from the stream
		stream.read( reinterpret_cast<char*>(&timestamp), sizeof(timestamp) );
		stream.read( reinterpret_cast<char*>(&type),      sizeof(type) );
		stream.read( reinterpret_cast<char*>(&subtype),   sizeof(subtype) );
		stream.read( reinterpret_cast<char*>(&length),    sizeof(length) );

		if( !stream.good() ) return false;

		// Correct for big endiannes
		timestamp = ntohl( timestamp );
		type      = ntohs( type );
		subtype   = ntohs( subtype );
		length    = ntohl( length );

		// Get the timestamp more logically
		//std::time_t tmp = timestamp;
		//std::cerr << std::ctime(&tmp) << " " << type << " " << subtype << " " << length << std::endl;

		// We only want to extract BGP messages and nothing else, the
		// data set used only has 4 byte AS numbers.
		if( type==MRT::Type::BGP4MP && subtype==MRT::BGP4MP::MESSAGE_AS4 ) {
			uint32_t peer, local;
			uint16_t interface, address_family;
			// Read the message info from the stream
			stream.read( reinterpret_cast<char*>(&peer),           sizeof(peer) );
			stream.read( reinterpret_cast<char*>(&local),          sizeof(local) );
			stream.read( reinterpret_cast<char*>(&interface),      sizeof(interface) );
			stream.read( reinterpret_cast<char*>(&address_family), sizeof(address_family) );
			length -= sizeof(peer) + sizeof(local) + sizeof(interface) + sizeof(address_family);

			// Correct for big endiannes
			peer           = ntohl( peer );
			local          = ntohl( local );
			interface      = ntohs( interface );
			address_family = ntohs( address_family );

			// Read out the correct ip addresses
			if( address_family == 1 ) {
				// If it's ipv4
				uint8_t peer_ip[4], local_ip[4];
				stream.read( reinterpret_cast<char*>(&peer_ip),  sizeof(peer_ip) );
				stream.read( reinterpret_cast<char*>(&local_ip), sizeof(local_ip) );
				length -= sizeof(peer_ip) + sizeof(local_ip);
			}
			else {
				// If it's ipv6
				uint8_t peer_ip[16], local_ip[16];
				stream.read( reinterpret_cast<char*>(&peer_ip),  sizeof(peer_ip) );
				stream.read( reinterpret_cast<char*>(&local_ip), sizeof(local_ip) );
				length -= sizeof(peer_ip) + sizeof(local_ip);
			}

			read_bgp_message( stream, v4_routes, v6_routes );
		}
		else if( type==MRT::Type::BGP4MP && subtype==MRT::BGP4MP::STATE_CHANGE_AS4 ) {
			// These messages are in the data but have no useful information
			// for us, just throw it away.
			{
				char* tmp = new char[length];
				stream.read( tmp, length );
				delete[] tmp;
			}
		}
		else {
			// If it's something else print an error and throw
			// it away.
			std::cerr << "Unsupported MRT type " << type << " with subtype " << subtype << " message ignored" << std::endl;
			{
				char* tmp = new char[length];
				stream.read( tmp, length );
				delete[] tmp;
			}
		}

		return true;
	}
}

namespace Import {
	void import( std::istream& stream, RouteTableV4& v4_routes, RouteTableV6& v6_routes ) {
		unsigned int messages = 0;
		while( read_mrt_message( stream, v4_routes, v6_routes ) ) ++messages;
		std::cerr << "Messages read: " << messages << std::endl;
	}
}
