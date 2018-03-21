#pragma once

#include "filename.hpp"
#include "md5.hpp"
#include <sys/types.h>
#include <tuple>
#include <map>

struct options_t;

//---------------------------------------------------------------------------
/*
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
inline filename_t   file_name(const file_info_t& info)	{ return std::get<3>(info); }
 */
//---------------------------------------------------------------------------

struct file_info_t
{
	file_info_t(ino_t ino, nlink_t nlink, off_t size, const std::string &name);

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
inline std::string	file_name(const file_info_t& info)	{ return info.name; }

//---------------------------------------------------------------------------

typedef std::multimap<md5_t, file_info_t> files_t;

void scan(files_t& files, options_t& opts, const std::string& name);
void show(const files_t& files, options_t& opts);
