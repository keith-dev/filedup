#pragma once

#include <iosfwd>
#include <optional>
#include <regex>
#include <tuple>
#include <vector>

#include <stdint.h>

constexpr int DBG_LEVEL   = 0;
constexpr int DBG_LEVEL_0 = 1;
constexpr int DBG_LEVEL_1 = 2;
constexpr int DBG_LEVEL_2 = 3;

typedef std::vector<std::string> args_t;

struct options_t {
	typedef void usage_func_t();
	typedef std::vector<std::regex> regexes_t;

	std::optional<int> verbose;
	std::optional<int64_t> threshold;
	std::optional<regexes_t> excludes;

	static std::tuple<options_t, args_t>
		create(int argc, char* argv[], usage_func_t usage = nullptr);
};

extern std::ostream& dbg;
