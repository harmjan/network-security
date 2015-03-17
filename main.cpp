#include <iostream>
#include <fstream>
#include <algorithm>

#include <sstream>
#include <fstream>

#include "import.hpp"

int main(int argc, char** argv) {
	if( argc < 2 ) {
		std::cerr << "Need at least 1 file name" << std::endl;
		std::cerr << "Usage: " << argv[0] << " file [file, ...]" << std::endl;
		return EXIT_FAILURE;
	}

	RouteHistory v4_routes;

	// Loop over the file names and import them
	for( int i=1; i<argc; ++i ) {
		std::cout << "\rReading file " << i << " of " << (argc-1) << " : " << argv[i] << std::flush;
		// Open the file
		std::ifstream file_stream( argv[i] );
		if( !file_stream ) {
			std::cerr << "Couldn't open file " << argv[i] << std::endl;
			return EXIT_FAILURE;
		}

		// Import the file into the maps
		Import::import( file_stream, v4_routes );
	}

	std::cout << std::endl << "Sorting events on time" << std::endl;
	// Sort the events that happened per ip range
	// based on time.
	for( auto& i : v4_routes ) {
		std::sort( i.second.begin(), i.second.end() );
	}

	// Print these routes to file
	std::size_t count = 1;
	for( const auto i : v4_routes ) {
		std::cout << "\rWriting file " << count++ << " of " << v4_routes.size() << std::flush;
		std::ostringstream ss("");
		ss << i.first;
		std::string ip = ss.str();
		ip.replace( ip.find('/'), 1, "-" );
		std::ofstream file( "ip/" + ip + ".csv" );
		//std::cout << "\t" << i.first << std::endl;
		for( const auto j : i.second ) {
			//file << (j.type==Route::ADVERTISED?"Advertised":"Withdrawn") << " route on " << j.time << " from " << j.from << " to " << j.sensor << " via ";
			file << j.time << ";" << j.from << ";" << j.sensor << ";" << (j.type==Route::ADVERTISED?"Advertised":"Withdrawn") << ";" << j.path_ordered << ";" << j.path.size() << ";";
			for( const auto n : j.path ) file << n << " ";
			file << std::endl;
		}
	}
	std::cout << std::endl;

	return 0;
}
