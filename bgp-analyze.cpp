#include <iostream>
#include <vector>
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

	std::cout << "Doing analysis" << std::endl;
	size_t count = 1;
	for( const ipv4 prefix : prefixes ) {
		std::cout << "\r" << count << "/" << prefixes.size() << std::flush;
		++count;
		Routes routes = File::read( prefix );
		std::cerr << prefix << std::endl;
	}
}
