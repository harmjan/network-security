#pragma once

namespace MRT {
	namespace Type {
		const uint16_t START     = 1;
		const uint16_t I_AM_DEAD = 3;

		const uint16_t OSPF          = 11;
		const uint16_t TABLE_DUMP    = 12;
		const uint16_t TABLE_DUMP_V2 = 13;
		const uint16_t BGP4MP        = 16;
		const uint16_t BGP4MP_ET     = 17;
		const uint16_t ISIS          = 32;
		const uint16_t ISIS_ET       = 33;
		const uint16_t OSPFv3        = 48;
		const uint16_t OSPFv3_ET     = 49;
	}

	namespace BGP4MP {
		const uint16_t STATE_CHANGE      = 0;
		const uint16_t MESSAGE           = 1;
		const uint16_t MESSAGE_AS4       = 4;
		const uint16_t STATE_CHANGE_AS4  = 5;
		const uint16_t MESSAGE_LOCAL     = 6;
		const uint16_t MESSAGE_AS4_LOCAL = 7;
	}
}

namespace BGP {
	namespace Type {
		const uint8_t OPEN         = 1;
		const uint8_t UPDATE       = 2;
		const uint8_t NOTIFICATION = 3;
		const uint8_t KEEPALIVE    = 4;
	}

	namespace Attribute {
		namespace Flags {
			const uint8_t OPTIONAL   = (1<<7);
			const uint8_t TRANSITIVE = (1<<6);
			const uint8_t PARTIAL    = (1<<5);
			const uint8_t EXTENDED   = (1<<4);
		}

		namespace Type {
			const uint8_t ORIGIN           = 1;
			const uint8_t AS_PATH          = 2;
			const uint8_t NEXT_HOP         = 3;
			const uint8_t MULTI_EXIT_DISC  = 4;
			const uint8_t LOCAL_PREF       = 5;
			const uint8_t ATOMIC_AGGREGATE = 6;
			const uint8_t AGGREGATOR       = 7;
		}
	}
}
