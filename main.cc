#include "filedup.hpp"
#include "options.hpp"
#include <algorithm>
#include <iostream>

void usage();

int main(int argc, char* argv[])
try
{
	typedef std::vector<std::string> strings_t;

	options_t opts;
	strings_t args;
	std::for_each(argv + 1, argv + argc, [&opts, &args](const char* arg) {
		if (strcmp(arg, "-q") == 0 || strcmp(arg, "--quiet") == 0) {
			if (opts.verbose != 1)
				throw std::runtime_error(std::string("bad arg: -q and -v are mutually exclusive"));
			opts.verbose = 0;
			return;
		}
		else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
			if (opts.verbose == 0)
				throw std::runtime_error(std::string("bad arg: -q and -v are mutually exclusive"));
			++opts.verbose;
			return;
		}
		else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--size") == 0) {
			opts.want_threshold = true;
			return;
		}
		else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
			usage();
			return;
		}
		else if (opts.want_threshold) {
			opts.threshold = atoll(arg);
			opts.want_threshold = false;
			return;
		}
		else if (strcmp(arg, "-x") == 0 || strcmp(arg, "--exclude") == 0) {
			opts.want_exclude = true;
			return;
		}
		else if (opts.want_exclude) {
			opts.excludes.emplace_back(arg, std::regex_constants::ECMAScript);
			opts.want_exclude = false;
			return;
		}
		else if (arg[0] == '-') {
			throw std::runtime_error(std::string("bad arg: ") + arg);
		}

		args.emplace_back(arg);
	});

	file_stamps_t file_stamps;
	for (const std::string& arg : args)
		scan(file_stamps, opts, arg);

	if (opts.verbose > DBG_LEVEL_0) dbg << "nfiles=" << file_stamps.files.size() << "\n";
	show(file_stamps, opts);
}
catch (const std::exception &e)
{
	std::clog << "fatal: " << e.what() << "\n";
}

void usage()
{
	const char *text[] = {
		"filedup -- Find duplicate files",
		"\tfiledup [options] arg1 arg2 ... argn",
		"\toptions:",
		"\t-h, --help",
		"\t\tshow this text",
		"\t-q, --quiet",
		"\t\tquiet mode, ignore warnings",
		"\t-s, --size",
		"\t\tminimum file size to process",
		"\t-v, --verbose",
		"\t\tincrease reported information",
		"\t-x, --exclude",
		"\t\texclude matching string",
		nullptr
	};

	for (size_t i = 0; text[i]; ++i)
		std::cout << text[i] << '\n';
	std::cout.flush();
}
