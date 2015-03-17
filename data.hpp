#pragma once

#include <cstdint>
#include <ostream>
#include <algorithm>
#include <map>
#include <vector>

/**
 * Represent an IP address range.
 *
 * \tparam bytes The number of bytes in the address
 */
template<size_t bytes>
struct ip {
	std::uint8_t val[bytes];
	std::uint8_t suffix;

	/**
	 * Check if this IP is less than another ip.
	 *
	 * This implementation first orders them on
	 * the ip address numbers, if a range falls
	 * within another range will the more specific
	 * range be ordered first.
	 */
	bool operator < (const ip<bytes>& other_ip ) const {
		std::uint8_t min_suffix = std::min(suffix, other_ip.suffix);
		for( size_t i=0; i<bytes; ++i ) {
			if( (i+1)*8 > min_suffix ) {
				std::uint8_t mask = 0;
				for( size_t j=0; j<min_suffix%8; ++j ) {
					mask = (mask<<1) | 1;
				}
				if( (val[i]&mask) < (other_ip.val[i]&mask) ) return true;
				if( (val[i]&mask) > (other_ip.val[i]&mask) ) return false;
				break;
			}
			if( val[i] < other_ip.val[i] ) return true;
			if( val[i] > other_ip.val[i] ) return false;
		}
		return suffix > other_ip.suffix;
	}
};

/**
 * Typedef the templates for the two ip versions.
 */
typedef ip<4>  ipv4;
typedef ip<16> ipv6;

/**
 * Specialized functions to display the IP addresses.
 * These functions are included in data.cpp to prevent
 * every file from compiling their own version of them.
 */
std::ostream& operator<<(std::ostream& os, const ipv4& obj);
std::ostream& operator<<(std::ostream& os, const ipv6& obj);

// Some literal typedef's
typedef uint32_t AsNumber;
typedef uint32_t Timestamp;

/**
 * An event that happened route in BGP.
 */
struct Route {
	// If this is and advertise or withdraw
	// event. If this is a withdraw event will
	// the path vector be unused.
	enum {
		ADVERTISED,
		WITHDRAWN
	} type;
	// The timestamp when the route was advertised
	Timestamp time;
	// The AS that advertised this route
	AsNumber from;
	// The AS that received this route
	AsNumber sensor;
	// If the path is ordered, if the sequence
	// of AS numbers is advertised in the correct
	// order.
	bool path_ordered;
	// The advertised path
	std::vector<AsNumber> path;

	/**
	 * Order routes based on timestamp.
	 */
	bool operator < (const Route& other_route ) const {
		return time < other_route.time;
	}
};

// This object stores the mapping from ip
// to the sequence of events.
typedef std::map<ipv4,std::vector<Route>> RouteHistory;
