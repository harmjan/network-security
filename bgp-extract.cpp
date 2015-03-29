#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>

#include "import.hpp"
#include "file.hpp"

int main(int argc, char** argv) {
	if( argc < 2 ) {
		std::cerr << "Need at least 1 file name" << std::endl;
		std::cerr << "Usage: " << argv[0] << " file [file, ...]" << std::endl;
		return EXIT_FAILURE;
	}

	RouteHistory v4_routes;

	// Loop over the file names and import them
	for( int i=1; i<argc; ++i ) {
		// Print some progress info
		std::cout << "\rReading file " << i << " of " << (argc-1) << " : " << argv[i] << "                                            " << std::flush;

		// The first 2 characters of the filename are currently
		// the number of the collector. In the MRT file you could
		// try to extract the collector IP and map those to the
		// physical locations but I decided that this was easier.
		unsigned int collector;
		std::sscanf( argv[i], "data/%u.", &collector );

		// Open the file
		std::ifstream file_stream( argv[i] );
		if( !file_stream ) {
			std::cerr << "Couldn't open file " << argv[i] << std::endl;
			return EXIT_FAILURE;
		}

		// Import the file into the maps
		Import::import( file_stream, v4_routes, collector );

		// Every 100 files merge and clear the map
		if( i%100==0 || i==argc-1 ) {
			// Sort the events that happened per ip range
			// based on time.
			std::size_t count=1;
			for( auto& route : v4_routes ) {
				if( count%1000 == 0 )
					std::cout << "\rReading file " << i << " of " << (argc-1) << " : " << argv[i] << " | Sorting " << count << "/" << v4_routes.size() << std::flush;
				++count;
				std::sort( route.second.begin(), route.second.end() );
			}

			// Read, merge and write the new files
			count=1;
			for( const auto route : v4_routes ) {
				if( count%1000 == 0 )
					std::cout << "\rReading file " << i << " of " << (argc-1) << " : " << argv[i] << " | Writing " << count << "/" << v4_routes.size() << " " << std::flush;
				++count;
				const Routes  old_file   = File::read(route.first);
				const Routes& new_routes = route.second;
				Routes new_file(old_file.size()+new_routes.size());
				std::merge(
					old_file.begin(),   old_file.end(),
					new_routes.begin(), new_routes.end(),
					new_file.begin()
				);
				File::write(route.first,new_file);
			}
			v4_routes.clear();
		}
	}

	std::cout << std::endl;

	return 0;
}
