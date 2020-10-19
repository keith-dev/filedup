/*
 * foreach directory
 * 	foreach file
 *   map MD5 -> fullpath
 * 	endfor
 * endfor
 *
 * foreach duplicate MD5
 * 	write all fullpaths
 * endfor
 */

#include "filestamps.hpp"
#include "md5.hpp"

#include <sstream>

//---------------------------------------------------------------------------

std::string file_stamps_t::to_json() const {
	std::ostringstream os;
	to_json(os);;
	return os.str();
}

file_stamps_t file_stamps_t::from_json(std::string /*json*/) {
	return file_stamps_t{};
}

std::ostream& file_stamps_t::to_json(std::ostream& os) const {
	os << "{\n";

	os << "  \"md5s\": [";
	for (auto p = files.cbegin(); p != files.cend(); ++p) {
		if (p != files.cbegin())
			os << ",";
		os << "\n    {\"md5\": \"" << p->first << "\", \"paths\": [";
		for (auto q = p->second.cbegin(); q != p->second.cend(); ++q) {
			if (q != p->second.cbegin())
				os << ", ";
			os << "\"" << file_name(*q).str() << "\"";
		}
		os << "]}";
	}
	os << "\n  ],\n";

	os << "  \"files\": [";
	for (auto p = stamps.cbegin(); p != stamps.cend(); ++p) {
		if (p != stamps.cbegin())
			os << ",";
		os << "\n    {\"file\": \"" << *p->first << "\", \"md5s\": [";
		for (auto q = p->second.cbegin(); q != p->second.cend(); ++q) {
			if (q != p->second.cbegin())
				os << ", ";
			os << "\"" << **q << "\"";
		}
		os << "]}";
	}
	os << "\n  ]\n";

	os << "}\n";
	return os;
}
