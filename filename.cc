#include "filename.hpp"
#include <string.h>

filename_t::strings_t filename_t::sm_strings;

filename_t::filename_t(const std::string& str, const char* delimiter) : m_delimiter(sm_strings.emplace(delimiter).first) {
	const char* p = str.c_str();
	const char* q;
	while ((q = strstr(p, m_delimiter->c_str()))) {
		size_t qlen = m_delimiter->size();
		m_path.emplace_back(sm_strings.emplace(q, qlen).first);

		p = q + m_delimiter->size();
		if (*p && memcmp(q, m_delimiter->c_str(), m_delimiter->size()) == 0) {
			q += m_delimiter->size();

			const char* r = strstr(q, m_delimiter->c_str());
			qlen = r ? r - q : strlen(q);
			m_path.emplace_back(sm_strings.emplace(q, qlen).first);
		}
	}
}

size_t filename_t::size() const {
	size_t len = {};
	for (strings_t::const_iterator p : m_path) len += p->size();
	return len;
}

filename_t::operator std::string () const {
	std::string str;
	for (strings_t::const_iterator p : m_path) str += *p;
	return str;
}
