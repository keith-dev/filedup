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

#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <memory>

#include <string.h>

//---------------------------------------------------------------------------

void scan_file(file_stamps_t& file_stamps, const options_t& opts, file_info_t file);

#ifdef USE_FTS_CMP_CONST_PTR
int mastercmp(const FTSENT * const *a, const FTSENT * const *b) {
	return strcmp((*a)->fts_name, (*b)->fts_name);
}
#else
int mastercmp(const FTSENT **a, const FTSENT **b) {
	return strcmp((*a)->fts_name, (*b)->fts_name);
}
#endif

void scan(file_stamps_t& file_stamps, const options_t& opts, std::string name)
try {
	if (opts.excludes)
		for (const std::regex& regex : *opts.excludes)
			if (std::regex_search(name, regex))
				 throw std::runtime_error("excluding: " + name);

	std::unique_ptr<char, decltype(&free)> namech(strdup(name.c_str()), free);
	char* paths[] = { namech.get(), nullptr };

	std::unique_ptr<FTS, decltype(&fts_close)> ftsp(fts_open(paths, FTS_PHYSICAL|FTS_XDEV, mastercmp), fts_close);
	if (!ftsp.get()) {
		// 'name' is a one-off filename
		struct stat info;
		if (stat(name.c_str(), &info) != -1) {
			if (opts.threshold.value_or(0) <= info.st_size) {
				file_info_t rec = make_fileinfo(
					info.st_ino,
					info.st_nlink,
					info.st_size,
					std::move(name));
				scan_file(file_stamps, opts, std::move(rec));
			}
			return;
		}

		throw std::runtime_error("cannot access: " + name);
	}

	FTSENT *ftsentp = fts_read(ftsp.get());
	if (!ftsentp)
		throw std::runtime_error("cannot read: " + name);

	switch (ftsentp->fts_info) {
	case FTS_D:		// directory
		if (opts.verbose.value_or(0) > DBG_LEVEL_0) dbg << "type directory: " << name << "\n";
		for (FTSENT* ftschild = fts_children(ftsp.get(), 0); ftschild; ftschild = ftschild->fts_link) {
			std::string name;
			name.reserve(ftsentp->fts_pathlen + 1 + ftschild->fts_namelen);
			name  = ftsentp->fts_path;
			if (name.back() != '/')
			    name += "/";
			name += ftschild->fts_name;

			if (S_ISDIR(ftschild->fts_statp->st_mode)) {
				scan(file_stamps, opts, std::move(name));
			}
			else if (S_ISREG(ftschild->fts_statp->st_mode)) {
				file_info_t rec = make_fileinfo(
					ftschild->fts_statp->st_ino,
					ftschild->fts_statp->st_nlink,
					ftschild->fts_statp->st_size,
					std::move(name));
				scan_file(file_stamps, opts, std::move(rec));
			}
		}
		break;
	case FTS_F:	{	// file
		if (opts.verbose.value_or(0) > DBG_LEVEL_0) dbg << "type file: " << name << "\n";
		if (opts.threshold.value_or(0) <= ftsentp->fts_statp->st_size) {
			file_info_t rec = make_fileinfo(
				ftsentp->fts_statp->st_ino,
				ftsentp->fts_statp->st_nlink,
				ftsentp->fts_statp->st_size,
				std::move(name));
			scan_file(file_stamps, opts, std::move(rec));
		}
		break;
	}
	case FTS_SL:	// symbolic link
		if (opts.verbose.value_or(0) > DBG_LEVEL_0) dbg << "type symlink: " << name << "\n";
		break;
	case FTS_SLNONE:// symbolic link to nothing
		if (opts.verbose.value_or(0) > DBG_LEVEL_0) dbg << "type symlink to nothing: " << name << "\n";
		break;
	case FTS_DC:	// cycle
		if (opts.verbose.value_or(0) > DBG_LEVEL_0) dbg << "type cyclical reference: " << name << "\n";
		break;
	case FTS_DNR:	// directory cannot be read
		if (opts.verbose.value_or(0) > DBG_LEVEL_0) dbg << "type cannot be read: " << name << "\n";
		break;
	case FTS_ERR:	// error
		if (opts.verbose.value_or(0) > DBG_LEVEL_0) dbg << "type error: " << name << "\n";
		break;
	default:
		if (opts.verbose.value_or(0) > DBG_LEVEL_0) dbg << "type unknow: " << name << "\n";
	}
}
catch (const std::exception &e) {
	if (opts.verbose > DBG_LEVEL) dbg << e.what() << "\n";
}

namespace {
	struct context_t {
		static constexpr size_t datasz{64 * 1024};
		unsigned char data[datasz];
	};
	context_t ctx;
}

void scan_file(file_stamps_t& file_stamps, const options_t& opts, file_info_t info)
try {
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

	file_stamps.emplace(std::move(sig), std::move(info));
}
catch (const std::bad_alloc &) {
	const std::string filename = file_name(info);
	if (opts.verbose > DBG_LEVEL) dbg << "file too large: " << filename << "\n";
}
catch (const std::exception &e) {
	if (opts.verbose > DBG_LEVEL) dbg << e.what() << "\n";
}

//---------------------------------------------------------------------------

std::string file_stamps_t::to_json() const {
	std::ostringstream os;
	to_json(os);;
	return os.str();
}

file_stamps_t file_stamps_t::from_json(std::string json) {
	return file_stamps_t();
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

namespace {
/*
	std::ostream& to_json(std::ostream& os, const file_stamps_t& file_stamps) {
		return file_stamps.to_json(os);
	}
 */
}

void show(const file_stamps_t& file_stamps, const options_t& opts) {
//	to_json(file_stamps, opts, std::cout);
	file_stamps.to_json(std::cout);
}
