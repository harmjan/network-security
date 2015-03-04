#include "data.hpp"

/**
 * Make an ipv4 address printable.
 */
std::ostream& operator<<(std::ostream& os, const ipv4& obj) {
	os << static_cast<int>(obj.val[0])
	   << "." << static_cast<int>(obj.val[1])
	   << "." << static_cast<int>(obj.val[2])
	   << "." << static_cast<int>(obj.val[3])
	   << "/" << static_cast<int>(obj.suffix);
	return os;
}

/**
 * Make an ipv6 address printable
 */
std::ostream& operator<<(std::ostream& os, const ipv6& obj) {
	os << std::hex;
	for( size_t i=0; i<16; i+=2 ) {
		std::uint16_t octet = (obj.val[i]<<8) + obj.val[i+1];
		os << octet;
		if( i<14 ) os << ":";
	}
	os << std::dec;
	os << "/" << obj.suffix;
	return os;
}
