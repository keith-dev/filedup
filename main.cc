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

typedef std::multimap<md5_t, std::string> files_t;
void scan_dir (files_t& files, std::string dir,  int64_t threshold);
void scan_file(files_t& files, std::string file, int64_t size, int64_t threshold);
void show_duplicates(const files_t& files);

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	int threshold = 0;
	bool want_threshold = false;

	files_t files;
	std::for_each(argv + 1, argv + argc, [&](const char* arg) {
		if (want_threshold) {
			threshold = atoi(arg);
			want_threshold = false;
			return;
		}
		if (strcmp(arg, "-s") == 0 || strcmp(arg, "--size") == 0) {
			want_threshold = true;
			return;
		}

		scan_dir(files, arg, threshold);
	});
	std::cout << "nfiles=" << files.size() << std::endl;
	show_duplicates(files);
}

//---------------------------------------------------------------------------

void show_duplicates(const files_t& files)
{
	if (files.empty()) return;

	auto p = files.begin();
	auto key = p->first;
	auto val = p->second;

	for (;;) {
		if ((++p) == files.end()) return;

		if (p->first == key) {
			std::cout << std::endl;
			std::cout << val << std::endl;

			do {
				key = p->first;
				val = p->second;
				std::cout << val << std::endl;

				if ((++p) == files.end()) return;
			}
			while (p->first == key);
		}

		key = p->first;
		val = p->second;
	}
}

//---------------------------------------------------------------------------

void scan_dir(files_t& files, std::string dirname, int64_t threshold)
{
	if (DIR* d = opendir(dirname.c_str())) {
		while (struct dirent* entry = readdir(d)) {
			if ((entry->d_name[0] == '.' && entry->d_name[1] == '\0') || (entry->d_name[0] == '.' && entry->d_name[1] == '.' && entry->d_name[2] == '\0'))
				continue;

			const std::string name = dirname + (dirname.back() != '/' ? std::string("/") : std::string()) + entry->d_name;

			struct stat info;
			if (stat(name.c_str(), &info) != -1) {
				if (S_ISLNK(info.st_mode)) {
					continue;
				}
				if (S_ISDIR(info.st_mode)) {
					scan_dir(files, name, threshold);
				}
				if (S_ISREG(info.st_mode)) {
					scan_file(files, name, info.st_size, threshold);
				}
			}
		}

		closedir(d);
	}
}

void scan_file(files_t& files, std::string filename, int64_t size, int64_t threshold)
try
{
	if (size < threshold) return;
	std::unique_ptr<char[]> buf(new char[size]);

	std::ifstream is(filename, std::ios::binary);
	is.read(buf.get(), size);

	md5_t stamp;
	MD5((unsigned char*)buf.get(), size, stamp.value);
	files.emplace(std::make_pair(stamp, std::move(filename)));
}
catch (const std::bad_alloc &)
{
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
