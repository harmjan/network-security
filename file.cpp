#include <string>
#include <sstream>
#include <fstream>

#include "file.hpp"

namespace {
	/**
	 * Create the filename that keeps the routes for
	 * an ip.
	 */
	std::string build_filename( const ipv4& ip ) {
		std::ostringstream ss("");
		ss << ip;
		std::string filename = ss.str();
		filename.replace( filename.find('/'), 1, "-" );
		return "ip/" + filename + ".ssv";
	}
}

namespace File {
	/**
	 * Write a set of Routes to a file as a csv
	 * file. Overwrite the file if it already
	 * exists.
	 */
	void write( const ipv4& ip, const Routes& routes ) {
		std::ofstream file( build_filename(ip) );
		for( const auto route : routes ) {
			// Write a comma separated version 
			file << route.time << " " << route.from << " " << route.sensor << " " << (route.type==Route::ADVERTISED?"Advertised":"Withdrawn") << " " << (route.path_ordered?"Sequence":"Set") << " " << route.path.size();
			for( const auto as : route.path ) file << " " << as;
			file << std::endl;
		}
	}

	/**
	 * Read a set of Routes from a file.
	 */
	Routes read( const ipv4& ip ) {
		std::ifstream file( build_filename(ip) );
		Routes res;
		Route r;
		// While the file still has routes in it
		while( file >> r.time ) {
			std::string type, ordered;
			std::size_t path_size;
			// Read out all the route properties and
			// fill the object r.
			file >> r.from >> r.sensor >> type >> ordered >> path_size;
			r.type         = (type=="Advertised") ? Route::ADVERTISED : Route::WITHDRAWN;
			r.path_ordered = (ordered=="Sequence");
			r.path.resize(path_size);
			for( std::size_t i=0; i<path_size; ++i ) file >> r.path[i];

			// Add the route to the routeset
			res.push_back( r );
		}
		return res;
	}
}
