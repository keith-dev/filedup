#include "filedup.hpp"
#include "options.hpp"
#include <algorithm>
#include <iostream>

int main(int argc, char* argv[])
try
{
	options_t opts;
	files_t files;
	std::for_each(argv + 1, argv + argc, [&](const char* arg) {
		if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
			++opts.verbose;
			return;
		}
		else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--size") == 0) {
			opts.want_threshold = true;
			return;
		}
		else if (opts.want_threshold) {
			opts.threshold = atoll(arg);
			opts.want_threshold = false;
			return;
		}
		else if (arg[0] == '-') {
			throw std::runtime_error(std::string("bad arg: ") + arg);
		}

		scan(files, opts, arg);
	});

	if (opts.verbose > DBG_LEVEL) dbg << "nfiles=" << files.size() << std::endl;

	show(files, opts);
}
catch (const std::exception &e)
{
	std::clog << e.what() << "\n";
}
