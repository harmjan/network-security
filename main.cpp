#include <iostream>
#include <fstream>

#include "import.hpp"

int main(int argc, char** argv) {
	if( argc < 2 ) {
		std::cerr << "Need at least 1 file name" << std::endl;
		return EXIT_FAILURE;
	}

	RouteTableV4 v4_routes;
	RouteTableV6 v6_routes;

	// Loop over the file names and import them
	for( int i=1; i<argc; ++i ) {
		// Open the file
		std::ifstream file_stream( argv[i] );
		if( !file_stream ) {
			std::cerr << "Couldn't open file " << argv[i] << std::endl;
			return EXIT_FAILURE;
		}

		// Import the file into the maps
		Import::import( file_stream, v4_routes, v6_routes );
	}

	for( auto i : v4_routes ) std::cout << "\t" << i << std::endl;

	return 0;
}
