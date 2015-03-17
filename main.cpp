#include <iostream>
#include <fstream>

#include "import.hpp"

int main(int argc, char** argv) {
	if( argc < 2 ) {
		std::cerr << "Need at least 1 file name" << std::endl;
		return EXIT_FAILURE;
	}

	RouteHistory v4_routes;

	// Loop over the file names and import them
	for( int i=1; i<argc; ++i ) {
		// Open the file
		std::ifstream file_stream( argv[i] );
		if( !file_stream ) {
			std::cerr << "Couldn't open file " << argv[i] << std::endl;
			return EXIT_FAILURE;
		}

		// Import the file into the maps
		Import::import( file_stream, v4_routes );
	}

	for( auto i : v4_routes ) {
		std::cout << "\t" << i.first << std::endl;
		for( auto j : i.second ) {
			std::cout << "\t\t" << (j.type==Route::ADVERTISED?"Advertised":"Withdrawn") << " route on " << j.time << " from " << j.from << " to " << j.sensor << " via ";
			for( auto n : j.path ) std::cout << n << " ";
			std::cout << std::endl;
		}
	}

	return 0;
}
