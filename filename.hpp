#pragma once

#include <set>
#include <list>
#include <string>

//---------------------------------------------------------------------------

class filename_t
{
	typedef std::set<std::string> strings_t;
	typedef std::list<strings_t::const_iterator> path_t;

	static const std::string sm_empty;
	static strings_t sm_strings;
	const strings_t::const_iterator m_delimiter;
	path_t m_path;

public:
	filename_t(const std::string& str, const char* delimiter = "/");
	size_t size() const;
	operator std::string () const;
	std::string str() const;
	const std::string& last() const;

	static size_t strings_size() { return sm_strings.size(); }
};
