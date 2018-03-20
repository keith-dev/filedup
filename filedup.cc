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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

//---------------------------------------------------------------------------

void scan_dir (files_t& files, options_t& opts, const std::string& dir);
void scan_file(files_t& files, options_t& opts, file_info_t& file);

void scan(files_t& files, options_t& opts, const std::string& name)
{
	struct stat info;
	if (stat(name.c_str(), &info) == -1) throw std::runtime_error("cannot open: " + name);
	if (S_ISLNK(info.st_mode))           throw std::runtime_error("ignore link: " + name);

	if (S_ISDIR(info.st_mode)) {
		scan_dir(files, opts, name);
	}
	else if (S_ISREG(info.st_mode)) {
		file_info_t rec = std::make_tuple(info.st_ino, info.st_nlink, info.st_size, name);
		scan_file(files, opts, rec);
	}
}

void scan_dir(files_t& files, options_t& opts, const std::string& dirname)
{
	if (DIR* d = opendir(dirname.c_str())) {
		while (struct dirent* entry = readdir(d)) {
			if ((entry->d_name[0] == '.' && entry->d_name[1] == '\0') ||
				(entry->d_name[0] == '.' && entry->d_name[1] == '.' && entry->d_name[2] == '\0'))
				continue;

			std::string name = dirname + std::string((dirname.back() != '/' ? 1 : 0), '/') + entry->d_name;

			struct stat info;
			if (stat(name.c_str(), &info) != -1) {
				if (S_ISLNK(info.st_mode)) {
					continue;
				}
				else if (S_ISDIR(info.st_mode)) {
					scan_dir(files, opts, name);
				}
				else if (S_ISREG(info.st_mode)) {
					file_info_t rec = std::make_tuple(info.st_ino, info.st_nlink, info.st_size, name);
					scan_file(files, opts, rec);
				}
			}
		}

		closedir(d);
	}
}

void scan_file(files_t& files, options_t& opts, file_info_t& info)
try
{
	std::string filename = file_name(info);
	off_t       filesize = file_size(info);

	const off_t K = 1024;
	const off_t M = 1024 * K;
	const off_t G = 1024 * M;
	if (filesize > 4*G)            throw std::runtime_error("skip large file: " + filename);
	if (filesize < opts.threshold) throw std::runtime_error("threshold breach: " + filename);
	if (opts.verbose > DBG_LEVEL)  dbg << "filename: " << filename << "\n";

	std::ifstream is(filename, std::ios::binary);
	if (!is)                       throw std::runtime_error("cannot open: " + filename);

	std::unique_ptr<char[]> buf(new char[filesize]);
	is.read(buf.get(), file_size(info));

	md5_t stamp;
	MD5((unsigned char*)buf.get(), filesize, stamp.value);
	files.emplace(std::make_pair(stamp, std::move(info)));
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
