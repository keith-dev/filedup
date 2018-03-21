/*
 * foreach directory
 * 	foreach file
 * 		map MD5 -> fullpath
 * 	endfor
 * endfor
 *
 * foreach duplicate MD5
 * 	write all fullpaths
 * endfor
 */
#include "filedup.hpp"
#include "options.hpp"
#include "md5.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fts.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdexcept>
#include <memory>

#include <string.h>

//---------------------------------------------------------------------------

void scan_dir (files_t& files, options_t& opts, const std::string& dir);
void scan_file(files_t& files, options_t& opts, file_info_t& file);

#ifdef USE_FTS_CMP_CONST_PTR
int mastercmp(const FTSENT * const *a, const FTSENT * const *b)
{
	return strcmp((*a)->fts_name, (*b)->fts_name);
}
#else
int mastercmp(const FTSENT **a, const FTSENT **b)
{
	return strcmp((*a)->fts_name, (*b)->fts_name);
}
#endif

void scan(files_t& files, options_t& opts, const std::string& name)
try
{
	std::unique_ptr<char, void(*)(void*)> namech(strdup(name.c_str()), free);
	char* paths[] = { namech.get(), nullptr };

	std::unique_ptr<FTS, int(*)(FTS*)> ftsp(fts_open(paths, FTS_PHYSICAL|FTS_XDEV, mastercmp), fts_close);
	if (!ftsp.get()) {
		// 'name' is a one-off filename
		struct stat info;
		if (stat(name.c_str(), &info) != -1) {
			file_info_t rec = make_fileinfo(info.st_ino, info.st_nlink, info.st_size, name);
			scan_file(files, opts, rec);
			return;
		}

		throw std::runtime_error("cannot access: " + name);
	}

	FTSENT *ftsentp = fts_read(ftsp.get());
	switch (ftsentp->fts_info) {
	case FTS_D:		// directory
		if (opts.verbose > DBG_LEVEL_1) dbg << name << ": is a directory\n";
		scan_dir(files, opts, name);
		break;
	case FTS_F:	{	// file
		if (opts.verbose > DBG_LEVEL_1) dbg << name << ": is a file\n";
		file_info_t rec = make_fileinfo(
			ftsentp->fts_statp->st_ino,
			ftsentp->fts_statp->st_nlink,
			ftsentp->fts_statp->st_size,
			name);
		scan_file(files, opts, rec);
		break;
	}
	case FTS_SL:	// symbolic link
		if (opts.verbose > DBG_LEVEL_1) dbg << name << ": is a symlink\n";
		break;
	case FTS_SLNONE:// symbolic link to nothing
		if (opts.verbose > DBG_LEVEL_1) dbg << name << ": is symlink to nothing\n";
		break;
	case FTS_DC:	// cycle
		if (opts.verbose > DBG_LEVEL_1) dbg << name << ": is a cyclical reference\n";
		break;
	case FTS_DNR:	// directory cannot be read
		if (opts.verbose > DBG_LEVEL_1) dbg << name << ": cannot be read\n";
		break;
	case FTS_ERR:	// error
		if (opts.verbose > DBG_LEVEL_1) dbg << name << ": error\n";
		break;
	default:
		;
	}
}
catch (const std::exception &e)
{
	std::clog << e.what() << "\n";
}

// TODO: use fts stuff instead of dir stuff, integrate with scan(), and deprecate this function.
void scan_dir(files_t& files, options_t& opts, const std::string& dirname)
{
	if (DIR* d = opendir(dirname.c_str())) {
		while (struct dirent* entry = readdir(d)) {
			if ((entry->d_name[0] == '.' && entry->d_name[1] == '\0') ||
				(entry->d_name[0] == '.' && entry->d_name[1] == '.' && entry->d_name[2] == '\0'))
				continue;

			std::string name = dirname + std::string((dirname.back() != '/' ? 1 : 0), '/') + entry->d_name;
			scan(files, opts, name);
		}

		closedir(d);
	}
}

void scan_file(files_t& files, options_t& opts, file_info_t& info)
try
{
	std::string filename = file_name(info);
	off_t       filesize = file_size(info);

	const off_t K = 1024 * 1;
	const off_t M = 1024 * K;
	const off_t G = 1024 * M;
	if (filesize > 4*G)            throw std::runtime_error("skip large file: " + filename);
	if (filesize < opts.threshold) throw std::runtime_error("threshold breach: " + filename);
	if (opts.verbose > DBG_LEVEL)  dbg << "filename: " << filename << "\n";

	std::ifstream is(filename, std::ios::binary);
	if (!is)                       throw std::runtime_error("cannot open: " + filename);

	std::unique_ptr<char[]> buf(new char[filesize]);
	is.read(buf.get(), filesize);

	md5_t sig;
	MD5((unsigned char*)buf.get(), filesize, sig.value);
	files.emplace(std::make_pair(sig, std::move(info)));
}
catch (const std::bad_alloc &)
{
	std::string filename = file_name(info);
	std::clog << "file too large: " << filename << "\n";
}
catch (const std::exception &e)
{
	std::clog << e.what() << "\n";
}

//---------------------------------------------------------------------------

void show_duplicates(const files_t& files, options_t& opts, std::ostream& os);

void show(const files_t& files, options_t& opts)
{
	show_duplicates(files, opts, std::cout);
}

void show_duplicates(const files_t& files, options_t& opts, std::ostream& os)
{
	if (files.empty()) return;

	auto p = files.begin();
	auto* key = &p->first;
	auto* val = &p->second;

	for (;;) {
		if (++p == files.end()) return;

		if (p->first == *key) {
			os << "\n";
			os << static_cast<std::string>(file_name(*val)) << "\n";

			do {
				key = &p->first;
				val = &p->second;
				os << static_cast<std::string>(file_name(*val)) << "\n";

				if (++p == files.end()) return;
			}
			while (p->first == *key);
		}

		key = &p->first;
		val = &p->second;
	}
}
