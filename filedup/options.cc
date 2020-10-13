#include "options.hpp"

#include <algorithm>
#include <iostream>

std::ostream& dbg = std::clog;

std::tuple<options_t, args_t> options_t::create(int argc, char* argv[], usage_func_t usage) {
	options_t opts;
	args_t args;
	bool stop_arg_processing{};
	bool want_threshold{};
	bool want_exclude{};
	std::for_each(argv + 1, argv + argc, [&](const char* arg) {
		auto quiet = [&opts]() {
			opts.verbose = std::nullopt;
		};
		auto verbose = [&opts]() {
			if (!opts.verbose)
				opts.verbose = 0;
			++*opts.verbose;
		};
		auto threshold = [&opts, &want_threshold]() {
			want_threshold = true;
			if (!opts.threshold)
				opts.threshold = {};
		};
		auto exclude = [&opts, &want_exclude]() {
			want_exclude = true;
			if (!opts.excludes)
				opts.excludes = {};
		};

		if (stop_arg_processing) {
			args.emplace_back(arg);
			return;
		}
		else if (strcmp(arg, "--") == 0) {
			stop_arg_processing = true;
			return;
		}

		if (strncmp(arg, "--", 2) == 0) {
			if (strcmp(arg + 2, "quiet") == 0) {
				if (opts.verbose != 1)
					throw std::runtime_error(std::string("bad arg: -q and -v are mutually exclusive"));
				quiet();
			}
			else if (strcmp(arg, "--verbose") == 0) {
				if (opts.verbose == 0)
					throw std::runtime_error(std::string("bad arg: -q and -v are mutually exclusive"));
				verbose();
			}
			else if (strcmp(arg, "--size") == 0) {
				threshold();
			}
			else if (strcmp(arg, "--exclude") == 0) {
				exclude();
			}
			else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
				if (usage)
					usage();
				exit(0);
			}
			else
				throw std::runtime_error(std::string("bad arg: ") + arg);

			return;
		}
		else if (*arg == '-') {
			if (arg[1] == '\0')
				throw std::runtime_error(std::string("bad arg: ") + arg);

			for (size_t i = 1, len = strlen(arg); i != len; ++i) {
				if (arg[i] == 'q') {
					if (opts.verbose != 1)
						throw std::runtime_error(std::string("bad arg: -q and -v are mutually exclusive"));
					quiet();
				}
				else if (arg[i] == 'v') {
					if (opts.verbose == 0)
						throw std::runtime_error(std::string("bad arg: -q and -v are mutually exclusive"));
					verbose();
				}
				else if (arg[i] == 's') {
					threshold();
				}
				else if (arg[i] == 'x') {
					exclude();
				}
				else if (arg[i] == 'h') {
					if (usage)
						usage();
					exit(0);
				}
				else
					throw std::runtime_error(std::string("bad arg: -") + arg[i]);
			}
			return;
		}
		else if (want_threshold) {
			*opts.threshold = atoll(arg);
			want_threshold = false;
			return;
		}
		else if (want_exclude) {
			(*opts.excludes).emplace_back(arg, std::regex_constants::ECMAScript);
			want_exclude = false;
			return;
		}

		args.emplace_back(arg);
	});

	return {opts, args};
}
