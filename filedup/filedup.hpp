#pragma once

#include "filename.hpp"
#include "md5.hpp"
#include <sys/types.h>

#ifdef TUPLE_FILEINFO
  #include <tuple>
#endif

#include <map>
#include <vector>

struct options_t;

//---------------------------------------------------------------------------

#ifdef TUPLE_FILEINFO

typedef std::tuple<ino_t, nlink_t, off_t, filename_t> file_info_t;

inline file_info_t make_fileinfo(ino_t ino, nlink_t nlink, off_t size, std::string name) {
	return std::make_tuple(ino, nlink, size, name);
}

inline ino_t&       file_inode(file_info_t& info)		{ return std::get<0>(info); }
inline nlink_t&     file_links(file_info_t& info)		{ return std::get<1>(info); }
inline off_t&       file_size(file_info_t& info)		{ return std::get<2>(info); }
inline filename_t&  file_name(file_info_t& info)		{ return std::get<3>(info); }

inline ino_t        file_inode(const file_info_t& info)	{ return std::get<0>(info); }
inline nlink_t      file_links(const file_info_t& info)	{ return std::get<1>(info); }
inline off_t        file_size(const file_info_t& info)	{ return std::get<2>(info); }
inline const filename_t& file_name(const file_info_t& info)	{ return std::get<3>(info); }

#else	//-------------------------------------------------------------------

struct file_info_t
{
	inline file_info_t(ino_t ino, nlink_t nlink, off_t size, const std::string &name);

	ino_t	inode;
	nlink_t	nlink;
	off_t	size;
	filename_t name;
};

inline file_info_t::file_info_t(ino_t ino, nlink_t nlink, off_t size, const std::string &name) :
	inode(ino), nlink(nlink), size(size), name(name)
{
}

inline file_info_t make_fileinfo(ino_t ino, nlink_t nlink, off_t size, std::string name) {
	return file_info_t(ino, nlink, size, name);
}

inline ino_t&		file_inode(file_info_t& info)		{ return info.inode; }
inline nlink_t&		file_links(file_info_t& info)		{ return info.nlink; }
inline off_t&		file_size(file_info_t& info)		{ return info.size; }
inline filename_t&	file_name(file_info_t& info)		{ return info.name; }

inline ino_t		file_inode(const file_info_t& info)	{ return info.inode; }
inline nlink_t		file_links(const file_info_t& info)	{ return info.nlink; }
inline off_t		file_size(const file_info_t& info)	{ return info.size; }
inline const filename_t& file_name(const file_info_t& info)	{ return info.name; }

#endif	// TUPLE_FILEINFO

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

//---------------------------------------------------------------------------

void scan(file_stamps_t& file_stamps, const options_t& opts, std::string name);
void show(const file_stamps_t& file_stamps, const options_t& opts);
