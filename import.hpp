#pragma once

#include <istream>
#include <map>
#include "data.hpp"

namespace Import {
	void import( std::istream& stream, RouteTableV4& v4_routes, RouteTableV6& v6_routes );
}
