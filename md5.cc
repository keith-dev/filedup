#include "md5.hpp"
#include <ostream>
#include <iomanip>

std::ostream& md5(std::ostream& os, const md5_t& n)
{
	os << std::hex;
	for (int i = 0; i != MD5_DIGEST_LENGTH; ++i)
		os << std::setw(2) << std::setfill('0') << (unsigned)n.value[i];

	return os << std::dec;
}
