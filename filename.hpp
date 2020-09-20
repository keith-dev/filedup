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

	strings_t::const_iterator m_delimiter;
	path_t m_path;

public:
	filename_t(const std::string& str, const char* delimiter = "/");

	filename_t(const filename_t& n) = delete;
	filename_t& operator=(const filename_t& n) = delete;

	inline filename_t(filename_t&& n);
	inline filename_t& operator=(filename_t&& n);

	operator std::string () const;

	size_t size() const;
	std::string str() const;
	const std::string& last() const;

	static size_t strings_size() { return sm_strings.size(); }
};

inline filename_t::filename_t(filename_t&& n) :
	m_delimiter(std::move(n.m_delimiter)),
	m_path(std::move(n.m_path))
{
	n.m_delimiter = std::cend(sm_strings);
}

inline filename_t& filename_t::operator=(filename_t&& n)
{
	if (this != &n) {
		m_delimiter = std::move(n.m_delimiter);
		n.m_delimiter = std::cend(sm_strings);

		m_path = std::move(n.m_path);
	}
	return *this;
}
