#pragma once

#include "filename.hpp"
#include "md5.hpp"
#include <sys/types.h>
#include <tuple>
#include <map>

struct options_t;

//---------------------------------------------------------------------------

typedef std::tuple<ino_t, nlink_t, off_t, filename_t> file_info_t;

inline ino_t&       file_inode(file_info_t& info)      { return std::get<0>(info); }
inline nlink_t&     file_links(file_info_t& info)      { return std::get<1>(info); }
inline off_t&       file_size(file_info_t& info)       { return std::get<2>(info); }
inline filename_t&  file_name(file_info_t& info)       { return std::get<3>(info); }

inline ino_t        file_inode(const file_info_t& info){ return std::get<0>(info); }
inline nlink_t      file_links(const file_info_t& info){ return std::get<1>(info); }
inline off_t        file_size(const file_info_t& info) { return std::get<2>(info); }
inline filename_t   file_name(const file_info_t& info) { return std::get<3>(info); }

//---------------------------------------------------------------------------

typedef std::multimap<md5_t, file_info_t> files_t;

void scan(files_t& files, options_t& opts, const std::string& name);
void show(const files_t& files, options_t& opts);
