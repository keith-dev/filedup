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
#ifdef __GNUC__
  #if __x86_64__ || __ppc64__
    #define BITS64 1
  #else
    #define BITS32 1
  #endif
#endif

#include "filedup.hpp"
#include "options.hpp"
#include "md5.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <fts.h>

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <memory>

#include <string.h>
#include <stdio.h>

//---------------------------------------------------------------------------

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
	for (std::regex& regex : opts.excludes)
		if (std::regex_search(name, regex))
			 throw std::runtime_error("excluding: " + name);

	std::unique_ptr<char, decltype(&free)> namech(strdup(name.c_str()), free);
	char* paths[] = { namech.get(), nullptr };

	std::unique_ptr<FTS, decltype(&fts_close)> ftsp(fts_open(paths, FTS_PHYSICAL|FTS_XDEV, mastercmp), fts_close);
	if (!ftsp.get()) {
		// 'name' is a one-off filename
		struct stat info;
		if (stat(name.c_str(), &info) != -1) {
			file_info_t rec = make_fileinfo(
				info.st_ino,
				info.st_nlink,
				info.st_size,
				name);
			scan_file(files, opts, rec);
			return;
		}

		throw std::runtime_error("cannot access: " + name);
	}

	FTSENT *ftsentp = fts_read(ftsp.get());
	if (!ftsentp)
		throw std::runtime_error("cannot read: " + name);

	switch (ftsentp->fts_info) {
	case FTS_D:		// directory
		if (opts.verbose > DBG_LEVEL_1) dbg << "type directory: " << name << "\n";
		if (FTSENT* ftschild = fts_children(ftsp.get(), 0)) {
			std::string name;

			do {
				name.reserve(std::max(name.capacity(), ftsentp->fts_pathlen + 1 + ftschild->fts_namelen));
				name  = ftsentp->fts_path;
				name += "/";
				name += ftschild->fts_name;

				if (S_ISDIR(ftschild->fts_statp->st_mode)) {
					scan(files, opts, name);
				}
				else {
					file_info_t rec = make_fileinfo(
						ftschild->fts_statp->st_ino,
						ftschild->fts_statp->st_nlink,
						ftschild->fts_statp->st_size,
						std::move(name));
					scan_file(files, opts, rec);
				}

				ftschild = ftschild->fts_link;
			}
			while (ftschild);
		}
		break;
	case FTS_F:	{	// file
		if (opts.verbose > DBG_LEVEL_1) dbg << "type file: " << name << "\n";
		file_info_t rec = make_fileinfo(
			ftsentp->fts_statp->st_ino,
			ftsentp->fts_statp->st_nlink,
			ftsentp->fts_statp->st_size,
			name);
		scan_file(files, opts, rec);
		break;
	}
	case FTS_SL:	// symbolic link
		if (opts.verbose > DBG_LEVEL_1) dbg << "type symlink: " << name << "\n";
		break;
	case FTS_SLNONE:// symbolic link to nothing
		if (opts.verbose > DBG_LEVEL_1) dbg << "type symlink to nothing: " << name << "\n";
		break;
	case FTS_DC:	// cycle
		if (opts.verbose > DBG_LEVEL_1) dbg << "type cyclical reference: " << name << "\n";
		break;
	case FTS_DNR:	// directory cannot be read
		if (opts.verbose > DBG_LEVEL_1) dbg << "type cannot be read: " << name << "\n";
		break;
	case FTS_ERR:	// error
		if (opts.verbose > DBG_LEVEL_1) dbg << "type error: " << name << "\n";
		break;
	default:
		if (opts.verbose > DBG_LEVEL_1) dbg << "type unknow: " << name << "\n";
	}
}
catch (const std::exception &e)
{
	if (opts.verbose > DBG_LEVEL) dbg << e.what() << "\n";
}

namespace {
	struct context_t {
		static constexpr size_t datasz{64 * 1024};
		unsigned char data[datasz];
	};
	context_t ctx;
}

void scan_file(files_t& files, options_t& opts, file_info_t& info)
try
{
	const std::string filename = file_name(info);

	MD5_CTX md_context;
	MD5_Init(&md_context);

	int fd;
	if ((fd = open(filename.c_str(), O_RDONLY)) != -1) {
		int nbytes;
		while ((nbytes = read(fd, ctx.data, sizeof(ctx.data))) != 0)
			MD5_Update(&md_context, ctx.data, nbytes);
		close(fd);
	}
	else
		throw std::runtime_error{"cannot open: " + filename};

	md5_t sig;
	MD5_Final(sig.value, &md_context);

	files.emplace(std::make_pair(sig, std::move(info)));
}
catch (const std::bad_alloc &)
{
	const std::string filename = file_name(info);
	if (opts.verbose > DBG_LEVEL) dbg << "file too large: " << filename << "\n";
}
catch (const std::exception &e)
{
	if (opts.verbose > DBG_LEVEL) dbg << e.what() << "\n";
}

//---------------------------------------------------------------------------

namespace {
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
}

void show(const files_t& files, options_t& opts)
{
	show_duplicates(files, opts, std::cout);
}
