#pragma once

#include <iosfwd>
#include <regex>
#include <vector>
#include <stdint.h>

const int DBG_LEVEL   = 0;
const int DBG_LEVEL_0 = 1;
const int DBG_LEVEL_1 = 2;
const int DBG_LEVEL_2 = 3;

struct options_t
{
	typedef std::vector<std::regex> regexes_t;

	int verbose = 1;
	int64_t threshold = 0;
	regexes_t excludes;
	bool want_threshold = false;
	bool want_exclude = false;
};

extern std::ostream& dbg;
