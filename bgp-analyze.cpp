#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

#include "file.hpp"
#include "data.hpp"

int main() {
	std::vector<ipv4> prefixes;

	// The filenames are passed to this program via
	// stdin, read them in
	std::cout << "Reading in filenames" << std::endl;
	std::string filename;
	while( std::cin >> filename ) {
		ipv4 prefix;
		std::sscanf( filename.c_str(), "%hhu.%hhu.%hhu.%hhu-%hhu.ssv",
		        &prefix.val[0],
		        &prefix.val[1],
		        &prefix.val[2],
		        &prefix.val[3],
		        &prefix.suffix
		);
		prefixes.push_back( prefix );
	}

	// And sort them again, the ordering that the
	// filesystem gives them is not a 100% correct.
	std::cout << "Sorting filenames" << std::endl;
	std::sort( prefixes.begin(), prefixes.end() );

	// Store the chain of prefixes that contain each
	// other with the set of AS numbers that advertise
	// that prefix.
	std::vector<std::pair<ipv4,std::set<AsNumber>>> chain;
	std::cout << "Doing analysis" << std::endl;
	size_t count = 1;
	for( const ipv4 prefix : prefixes ) {
		// Print a progress message
		std::cout << "\r" << count << "/" << prefixes.size() << std::flush;
		++count;
		// Erase all prefixes in the chain that this prefix is
		// not a descendent of.
		for( auto it=chain.begin(); it!=chain.end(); ++it ) {
			if( !(prefix << it->first) ) {
				// Erase the rest of the chain
				chain.erase(it, chain.end());
				break;
			}
		}
		// Collect all AS numbers that are advertising
		// this prefix
		std::set<AsNumber> as_numbers;
		for( Route route : File::read(prefix) ) {
			if( route.path.size() != 0 ) {
				as_numbers.insert( route.path.back() );
			}
		}
		// Check if this prefix is advertised by a new AS
		for( const auto i : chain ) {
			for( AsNumber as_number : as_numbers ) {
				// If this is a new as number print an
				// error message
				if( i.second.find(as_number) == i.second.end() ) {
					std::cerr << prefix << " advertised by " << as_number << " is more specific than " << i.first << " advertised by";
					for( AsNumber j : i.second ) std::cerr << " " << j;
					std::cerr << std::endl;
				}
			}
		}
		// Add this prefix to the chain
		chain.push_back( std::make_pair(prefix,as_numbers) );
	}
	std::cout << std::endl << "All done!" << std::endl;
}
