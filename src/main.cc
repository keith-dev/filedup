#include "options.hpp"
#include "scan.hpp"

#include "filedup/filestamps.hpp"

#include <iostream>

void usage();

int main(int argc, char* argv[])
try {
	const auto ret{ options_t::create(argc, argv, usage) };
	const options_t& opts{ std::get<0>(ret) };
	const args_t& args{ std::get<1>(ret) };

	file_stamps_t file_stamps;
	for (const std::string& arg : args)
		scan(file_stamps, opts, arg);

	if (opts.verbose.value_or(0) > DBG_LEVEL_0) dbg << "nfiles=" << file_stamps.files.size() << "\n";
	show(file_stamps, opts);
}
catch (const std::exception &e) {
	std::clog << "fatal: " << e.what() << "\n";
}

void usage() {
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
