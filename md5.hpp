#pragma once

#include <openssl/md5.h>
#include <iosfwd>
#include <string.h>

struct md5_t
{
	unsigned char value[MD5_DIGEST_LENGTH];
};
inline bool operator< (const md5_t& a, const md5_t& b) { return memcmp(a.value, b.value, MD5_DIGEST_LENGTH) <  0; }
inline bool operator==(const md5_t& a, const md5_t& b) { return memcmp(a.value, b.value, MD5_DIGEST_LENGTH) == 0; }

std::ostream& operator<<(std::ostream& os, const md5_t& n);
