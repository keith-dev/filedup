#pragma once

#include "fileinfo.hpp"
#include "md5.hpp"

#include <map>
#include <vector>

//---------------------------------------------------------------------------

template <class T>
struct compare_ptr_t {
	bool operator()(const T* a, const T* b) const {
		if (!a && !b) return false;
		if (!a && b) return true;
		if (a && !b) return false;
		return *a < *b;
	}
};

using files_t = std::map<md5_t, std::vector<file_info_t>>;
using stamps_t = std::map<const std::string*, std::set<const md5_t*, compare_ptr_t<md5_t>>, compare_ptr_t<std::string>>;

struct file_stamps_t
{
	file_stamps_t() = default;
	file_stamps_t(const file_stamps_t&) = delete;
	file_stamps_t& operator=(const file_stamps_t&) = delete;

	inline void emplace(md5_t md5, file_info_t file_info);
	std::ostream& to_json(std::ostream& os) const;
	std::string to_json() const;

	static file_stamps_t from_json(std::string json);

	files_t files;
	stamps_t stamps;
};

inline void file_stamps_t::emplace(md5_t md5, file_info_t file_info) {
	auto p = files.find(md5);
	if (p == files.end()) {
		p = files.emplace(
					std::piecewise_construct,
					std::forward_as_tuple(std::move(md5)),
					std::forward_as_tuple(std::vector<file_info_t>{})).first;
	}
	p->second.emplace_back(std::move(file_info));

	stamps[&file_name(*p->second.rbegin()).last()].emplace(&p->first);
}
