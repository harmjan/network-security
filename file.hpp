#pragma once

#include "data.hpp"

namespace File {
	void write( const ipv4& ip, const Routes& routes );
	Routes read( const ipv4& ip );
}
