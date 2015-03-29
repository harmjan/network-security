#pragma once

#include <istream>
#include <map>
#include "data.hpp"

namespace Import {
	void import( std::istream& stream, RouteHistory& v4_routes, unsigned int collector );
}
