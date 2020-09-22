PROG_CXX = filedup
SRCS     = filename.cc  options.cc  md5.cc  filedup.cc  main.cc
INCS     = filename.hpp options.hpp md5.hpp filedup.hpp
MAN      = filedup.1

CXXFLAGS += -O3 -g -std=c++17 -pedantic -Wall -Wextra -Wno-unused-parameter
LDADD    += -lcrypto

DEPENDS  = $(PROG_CXX).depends
MANPAGE  = $(MAN).gz

all: $(PROG_CXX) $(MANPAGE)

clean:
	- rm $(PROG_CXX) $(MANPAGE) $(DEPENDS) $(SRCS:.cc=.o) $(SRCS:.cc=.depend)

$(PROG_CXX): $(SRCS:.cc=.o)
	$(LINK.cc) -o $@ $^ $(LDADD)

$(MANPAGE): $(MAN)
	gzip -cn $(MAN) > $(MANPAGE)

$(DEPENDS): $(SRCS:.cc=.depend)
	cat $(SRCS:.cc=.depend) > $(DEPENDS)

%.depend: %.cc
	$(COMPILE.cc) -MM $< > $@

-include $(DEPENDS)
