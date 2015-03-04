#pragma once

#include <cstdint>
#include <ostream>
#include <algorithm>
#include <set>
#include <list>
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

typedef uint32_t as_number;
typedef uint32_t timestamp;

/**
 * An advertised route in BGP.
 */
struct Route {
	// The AS that advertised this route
	as_number from;
	// The AS that received this route
	as_number sensor;
	// The advertised path
	std::vector<as_number> path;
	// The timestamp when the route was advertised
	timestamp advertised;
	// The timestamp when the route was revoked
	timestamp revoked;
};

typedef std::list<Route> Routes;
typedef std::set<ipv4> RouteTableV4;
typedef std::set<ipv6> RouteTableV6;
