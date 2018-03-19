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
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <openssl/md5.h>

#include <algorithm>
#include <map>
#include <memory>
#include <fstream>
#include <iostream>
#include <iomanip>

struct md5_t
{
	unsigned char value[MD5_DIGEST_LENGTH];
};
inline bool operator< (const md5_t& a, const md5_t& b) { return memcmp(a.value, b.value, MD5_DIGEST_LENGTH) <  0; }
inline bool operator==(const md5_t& a, const md5_t& b) { return memcmp(a.value, b.value, MD5_DIGEST_LENGTH) == 0; }
std::ostream& md5(std::ostream& os, const md5_t& n);

typedef std::tuple<ino_t, off_t, std::string> file_info_t;
inline ino_t&       file_inode(file_info_t& info)      { return std::get<0>(info); }
inline off_t&       file_size(file_info_t& info)       { return std::get<1>(info); }
inline std::string& file_name(file_info_t& info)       { return std::get<2>(info); }
inline ino_t        file_inode(const file_info_t& info){ return std::get<0>(info); }
inline off_t        file_size(const file_info_t& info) { return std::get<1>(info); }
inline std::string  file_name(const file_info_t& info) { return std::get<2>(info); }

typedef std::multimap<md5_t, file_info_t> files_t;
void scan_dir (files_t& files, std::string dir, int64_t threshold);
void scan_file(files_t& files, file_info_t& file, int64_t threshold);
void show_duplicates(const files_t& files);

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
try
{
	int threshold = 0;
	bool want_threshold = false;

	files_t files;
	std::for_each(argv + 1, argv + argc, [&](const char* arg) {
		if (strcmp(arg, "-s") == 0 || strcmp(arg, "--size") == 0) {
			want_threshold = true;
			return;
		}
		else if (want_threshold) {
			threshold = atoi(arg);
			want_threshold = false;
			return;
		}
		else if (arg[0] == '-') {
			throw std::runtime_error(std::string("bad arg: ") + arg);
		}

		scan_dir(files, arg, threshold);
	});
	std::cout << "nfiles=" << files.size() << std::endl;
	show_duplicates(files);
}
catch (const std::exception &e)
{
	std::clog << e.what() << "\n";
}

//---------------------------------------------------------------------------

void show_duplicates(const files_t& files)
{
	if (files.empty()) return;

	auto p = files.begin();
	auto* key = &p->first;
	auto* val = &p->second;

	for (;;) {
		if ((++p) == files.end()) return;

		if (p->first == *key) {
			std::cout << "\n";
			std::cout << file_name(*val) << "\n";

			do {
				key = &p->first;
				val = &p->second;
				std::cout << file_name(*val) << "\n";

				if ((++p) == files.end()) return;
			}
			while (p->first == *key);
		}

		key = &p->first;
		val = &p->second;
	}
}

//---------------------------------------------------------------------------

void scan_dir(files_t& files, std::string dirname, int64_t threshold)
{
	if (DIR* d = opendir(dirname.c_str())) {
		while (struct dirent* entry = readdir(d)) {
			if ((entry->d_name[0] == '.' && entry->d_name[1] == '\0') || (entry->d_name[0] == '.' && entry->d_name[1] == '.' && entry->d_name[2] == '\0'))
				continue;

			std::string name = dirname + (dirname.back() != '/' ? std::string("/") : std::string()) + entry->d_name;

			struct stat info;
			if (stat(name.c_str(), &info) != -1) {
				if (S_ISLNK(info.st_mode)) {
					continue;
				}
				if (S_ISDIR(info.st_mode)) {
					scan_dir(files, name, threshold);
				}
				if (S_ISREG(info.st_mode)) {
					file_info_t rec = std::make_tuple(info.st_ino, info.st_size, std::move(name));
					scan_file(files, rec, threshold);
				}
			}
		}

		closedir(d);
	}
}

void scan_file(files_t& files, file_info_t& info, int64_t threshold)
try
{
	if (file_size(info) < threshold) return;
	std::unique_ptr<char[]> buf(new char[file_size(info)]);

	std::ifstream is(file_name(info), std::ios::binary);
	if (!is)
		throw std::runtime_error("cannot open: " + file_name(info));
	is.read(buf.get(), file_size(info));

	md5_t stamp;
	MD5((unsigned char*)buf.get(), file_size(info), stamp.value);
	files.emplace(std::make_pair(stamp, std::move(info)));
}
catch (const std::bad_alloc &)
{
	std::clog << "file too large: " << file_name(info) << "\n";
}
catch (const std::exception &e)
{
//	std::clog << e.what() << "\n";
}

//---------------------------------------------------------------------------
/*
int test()
{
	md5_t stamp;
	char str[] = "hello world";
	MD5((unsigned char*)str, strlen(str), stamp);
	md5(std::cout, stamp) << std::endl;

	return 0;
}
 */

std::ostream& md5(std::ostream& os, const md5_t& n)
{
	os << std::hex;
	for (int i = 0; i != MD5_DIGEST_LENGTH; ++i)
		os << std::setw(2) << std::setfill('0') << (unsigned)n.value[i];

	return os << std::dec;
}
