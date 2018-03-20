#pragma once

#include <iosfwd>
#include <stdint.h>

const int DBG_LEVEL   = 0;
const int DBG_LEVEL_1 = 1;
const int DBG_LEVEL_2 = 2;

struct options_t
{
	int verbose = 0;
	int64_t threshold = 0;
	bool want_threshold = false;
};

extern std::ostream& dbg;
