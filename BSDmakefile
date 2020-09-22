PROG_CXX = filedup
SRCS     = filename.cc  options.cc  md5.cc  filedup.cc  main.cc
INCS     = filename.hpp options.hpp md5.hpp filedup.hpp
MAN      = filedup.1

CXXFLAGS  = -O3 -g -std=c++17 -Wall -Wextra -Wno-unused-const-variable -Wno-unused-parameter -DUSE_FTS_CMP_CONST_PTR
LDFLAGS  += -lcrypto

.include <bsd.prog.mk>
