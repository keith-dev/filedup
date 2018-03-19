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
#include <set>
#include <list>
#include <memory>
#include <fstream>
#include <iostream>
#include <iomanip>

//---------------------------------------------------------------------------

class filename_t
{
	typedef std::set<std::string> strings_t;
	typedef std::list<strings_t::const_iterator> path_t;

	static strings_t sm_strings;
	const strings_t::const_iterator m_delimiter;
	path_t m_path;

public:
	filename_t(const std::string& str, const char* delimiter = "/");
	size_t size() const;
	operator std::string () const;

	size_t strings_size() const { return sm_strings.size(); }
};

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

struct md5_t
{
	unsigned char value[MD5_DIGEST_LENGTH];
};
inline bool operator< (const md5_t& a, const md5_t& b) { return memcmp(a.value, b.value, MD5_DIGEST_LENGTH) <  0; }
inline bool operator==(const md5_t& a, const md5_t& b) { return memcmp(a.value, b.value, MD5_DIGEST_LENGTH) == 0; }
std::ostream& md5(std::ostream& os, const md5_t& n);

//---------------------------------------------------------------------------

typedef std::multimap<md5_t, file_info_t> files_t;
void scan(files_t& files, const std::string& name, int64_t threshold);
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

		scan(files, arg, threshold);
	});
	std::cout << "nfiles=" << files.size() << std::endl;
	show_duplicates(files);
}
catch (const std::exception &e)
{
	std::clog << e.what() << "\n";
}

//---------------------------------------------------------------------------

void scan_dir (files_t& files, const std::string& dir, int64_t threshold);
void scan_file(files_t& files, file_info_t& file, int64_t threshold);

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
			std::cout << static_cast<std::string>(file_name(*val)) << "\n";

			do {
				key = &p->first;
				val = &p->second;
				std::cout << static_cast<std::string>(file_name(*val)) << "\n";

				if ((++p) == files.end()) return;
			}
			while (p->first == *key);
		}

		key = &p->first;
		val = &p->second;
	}
}

//---------------------------------------------------------------------------

void scan(files_t& files, const std::string& name, int64_t threshold)
{
	struct stat info;
	if (stat(name.c_str(), &info) == -1) throw std::runtime_error("cannot open: " + name);

	if (S_ISDIR(info.st_mode)) {
		scan_dir(files, name, threshold);
	}
	if (S_ISREG(info.st_mode)) {
		file_info_t rec = std::make_tuple(info.st_ino, info.st_nlink, info.st_size, name);
		scan_file(files, rec, threshold);
	}
}

void scan_dir(files_t& files, const std::string& dirname, int64_t threshold)
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
					file_info_t rec = std::make_tuple(info.st_ino, info.st_nlink, info.st_size, name);
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
	std::string filename = file_name(info);
	off_t       filesize = file_size(info);

	static const off_t gig = 1024 * 1024 * 1024;
	if (filesize > 4*gig)       throw std::runtime_error("skip large file: " + filename);
	if (filesize < threshold)   throw std::runtime_error("threshold breach: " + filename);

	std::ifstream is(filename, std::ios::binary);
	if (!is)                    throw std::runtime_error("cannot open: " + filename);

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

//---------------------------------------------------------------------------

filename_t::strings_t filename_t::sm_strings;

filename_t::filename_t(const std::string& str, const char* delimiter) : m_delimiter(sm_strings.emplace(delimiter).first) {
	const char* p = str.c_str();
	const char* q;
	while ((q = strstr(p, m_delimiter->c_str()))) {
		size_t qlen = m_delimiter->size();
		m_path.emplace_back(sm_strings.emplace(q, qlen).first);

		p = q + m_delimiter->size();
		if (*p && memcmp(q, m_delimiter->c_str(), m_delimiter->size()) == 0) {
			q += m_delimiter->size();
			qlen = strstr(q, m_delimiter->c_str()) ? strstr(q, m_delimiter->c_str()) - q : strlen(q);
			m_path.emplace_back(sm_strings.emplace(q, qlen).first);
		}
	}
}

size_t filename_t::size() const {
	size_t len = {};
	for (strings_t::const_iterator p : m_path) len += p->size();
	return len;
}

filename_t::operator std::string () const {
	std::string str;
	for (strings_t::const_iterator p : m_path) str += *p;
	return str;
}
