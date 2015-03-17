#include "import.hpp"
#include "mrt.hpp"

#include <iostream>
#include <iomanip>
#include <ctime>
#include "arpa/inet.h"

namespace {
	/**
	 * Read some bytes from the stream and do nothing with
	 * them.
	 */
	void read_and_ditch( std::istream& stream, std::size_t length ) {
		char* temporary_buffer = new char[length];
		stream.read( temporary_buffer, length );
		delete[] temporary_buffer;
	}

	namespace bgp {
		/**
		 * Read in IP prefixes untill length amount of bytes
		 * are read. This is the same for reading the newly
		 * announced routes as it is reading in the withdrawn
		 * routes which is why this piece of code was refactored
		 * to a separate function.
		 */
		std::vector<ipv4> read_prefixes( std::istream& stream, std::uint16_t length ) {
			std::vector<ipv4> vec;
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

				vec.push_back( range );
			}
			return vec;
		}

		/**
		 * Read the attributes of a BGP message and put
		 * the relevant attributes in a Route object.
		 */
		template<typename AsNumberSize>
		void read_attributes( std::istream& stream, std::size_t length, Route& route ) {
			while( length > 0 ) {
				std::uint8_t attr_flags, attr_type;
				std::uint16_t attr_length;
				// Read out the attribute header
				stream.read( reinterpret_cast<char*>(&attr_flags), sizeof(attr_flags) );
				stream.read( reinterpret_cast<char*>(&attr_type),  sizeof(attr_type) );
				// The length is 2 octets if the extended flag bit is
				// 1, otherwise 1 octet.
				if( attr_flags & BGP::Attribute::Flags::EXTENDED ) {
					stream.read( reinterpret_cast<char*>(&attr_length), 2 );
					attr_length = ntohs(attr_length);
				}
				else {
					// The current code only works if the host uses little endian.
					attr_length = 0;
					stream.read( reinterpret_cast<char*>(&attr_length), 1 );
				}

				// We are only interested in the AS_PATH attribute
				if( attr_type == BGP::Attribute::Type::AS_PATH ) {
					route.path_ordered = true;
					std::uint16_t path_length = attr_length;
					while( path_length > 0 ) {
						std::uint8_t segment_type, segment_length;
						stream.read( reinterpret_cast<char*>(&segment_type), 1 );
						stream.read( reinterpret_cast<char*>(&segment_length), 1 );

						// If this segment is an unordered set store
						// that information in the route.
						if( segment_type == 1 ) route.path_ordered = false;

						// Read out the segment AS numbers
						//read_and_ditch( stream, segment_length*4 );
						for( int i=0; i<segment_length; ++i ) {
							AsNumberSize as_number;
							stream.read( reinterpret_cast<char*>(&as_number), sizeof(as_number) );
							if( sizeof(AsNumberSize) == 4 ) {
								as_number = ntohl(as_number);
							}
							else {
								as_number = ntohs(as_number);
							}
							route.path.push_back( as_number );
						}

						// Update the path_length variable
						path_length -= 2 + segment_length*sizeof(AsNumberSize);
					}
				}
				else {
					// If it's not a path just throw the data away
					read_and_ditch( stream, attr_length );
				}

				// Update the number of octets of attributes left
				length -= sizeof(attr_flags) + sizeof(attr_type) + ((attr_flags&BGP::Attribute::Flags::EXTENDED)?2:1) + attr_length;
			}
		}

		/**
		 * Read a BGP message from the stream and add the data to
		 * the routing table.
		 */
		template<typename AsNumberSize>
		void read_message( std::istream& stream, RouteHistory& v4_routes, Route route ) {
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
			// If this is a valid bgp message that has no value for
			// us just skip it.
			if( type == BGP::Type::OPEN ||
			    type == BGP::Type::NOTIFICATION ||
			    type == BGP::Type::KEEPALIVE ||
			    type == BGP::Type::ROUTEREFRESH ) {
				read_and_ditch( stream, length-19 );
				return;
			}
			// If it's still not an update message throw
			// an error tantrum and throw away the message.
			if( type != BGP::Type::UPDATE ) {
				std::cerr << "Non update BGP message, type " << static_cast<int>(type) << std::endl;
				read_and_ditch( stream, length-19 );
				return;
			}
			//std::cerr << "Read BGP message of type " << static_cast<int>(type) << " with length " << length << std::endl;

			// The first variable block in the BGP message contains the withdrawn
			// routes.
			std::uint16_t withdrawn_length;
			stream.read( reinterpret_cast<char*>(&withdrawn_length), sizeof(withdrawn_length) );
			withdrawn_length = ntohs( withdrawn_length );
			for( ipv4 range : read_prefixes( stream, withdrawn_length ) ) {
				Route r(route);
				r.type = Route::WITHDRAWN;
				v4_routes[range].push_back( r );
			}

			// Read the attributes from the stream and put the relevant
			// attributes into a Route object.
			std::uint16_t attribute_length;
			stream.read( reinterpret_cast<char*>(&attribute_length), sizeof(attribute_length) );
			attribute_length = ntohs( attribute_length );
			Route route_attributes(route);
			route_attributes.type = Route::ADVERTISED;
			read_attributes<AsNumberSize>( stream, attribute_length, route_attributes );

			// The next variable block in the BGP message is the Network
			// Layer Reachability Information. This block contains the
			// ipv4 addresses that are advertised in this message with
			// the path attributes in the previous block.
			std::uint16_t reachability_length = length - 23 - withdrawn_length - attribute_length;
			for( ipv4 range : read_prefixes( stream, reachability_length ) ) {
				v4_routes[range].push_back( route_attributes );
			}
		}
	}

	template<typename AsNumberSize>
	void read_mrt_bgp4mp_message( std::istream& stream, RouteHistory& v4_routes, Timestamp timestamp, uint32_t length ) {
		AsNumberSize peer, local;
		uint16_t interface, address_family;
		// Read the message info from the stream
		stream.read( reinterpret_cast<char*>(&peer),           sizeof(peer) );
		stream.read( reinterpret_cast<char*>(&local),          sizeof(local) );
		stream.read( reinterpret_cast<char*>(&interface),      sizeof(interface) );
		stream.read( reinterpret_cast<char*>(&address_family), sizeof(address_family) );
		length -= sizeof(peer) + sizeof(local) + sizeof(interface) + sizeof(address_family);

		// Correct for big endiannes
		if( sizeof(AsNumberSize) == 4 ) {
			peer       = ntohl( peer );
			local      = ntohl( local );
		}
		else {
			peer       = ntohs( peer );
			local      = ntohs( local );
		}
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

		// Create a route object and fill in information
		// that is already known, let the read_message function
		// fill in the rest if available.
		Route route;
		route.time   = timestamp;
		route.from   = peer;
		route.sensor = local;
		bgp::read_message<AsNumberSize>( stream, v4_routes, route );
	}

	/**
	 * Read a MRT message from the stream and add the data if applicable
	 * to the routing table.
	 */
	bool read_mrt_message( std::istream& stream, RouteHistory& v4_routes ) {
		Timestamp timestamp;
		uint32_t length;
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

		// We only want to extract BGP messages and nothing else, check if
		// the as number is 4 or 2 bytes and call the correct corresponding
		// template.
		if( type==MRT::Type::BGP4MP && subtype==MRT::BGP4MP::MESSAGE_AS4 ) {
			read_mrt_bgp4mp_message<std::uint32_t>( stream, v4_routes, timestamp, length );
		}
		else if( type==MRT::Type::BGP4MP && subtype==MRT::BGP4MP::MESSAGE ) {
			read_mrt_bgp4mp_message<std::uint16_t>( stream, v4_routes, timestamp, length );
		}
		else if( type==MRT::Type::BGP4MP && subtype==MRT::BGP4MP::STATE_CHANGE_AS4 ) {
			// These messages are in the data but have no useful information
			// for us, just throw it away.
			read_and_ditch( stream, length );
		}
		else {
			// If it's something else print an error and throw
			// it away.
			std::cerr << "Unsupported MRT type " << type << " with subtype " << subtype << " message ignored" << std::endl;
			read_and_ditch( stream, length );
		}

		return true;
	}
}

namespace Import {
	void import( std::istream& stream, RouteHistory& v4_routes ) {
		unsigned int messages = 0;
		while( read_mrt_message( stream, v4_routes ) ) ++messages;
	}
}
