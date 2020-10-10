# ---------------------------------------------------------------------------
#	Copyright (c) 2013-20 Keith Williams.
#	All rights reserved.
#
#	Redistribution and use in source and binary forms are permitted
#	provided that the above copyright notice and this paragraph are
#	duplicated in all such forms and that any documentation,
#	advertising materials, and other materials related to such
#	distribution and use acknowledge that the software was developed
#	by Webbusy Ltd.
# ---------------------------------------------------------------------------

PROG_CXX = filedup
SRCS     = main.cc
INCS     = 
MAN      = filedup.1

PRRFIX   ?= $(HOME)
CXXFLAGS += -g -std=c++17 -pedantic -Wall -Wextra -Wno-unused-parameter
CXXFLAGS += -I..
LDADD    += -L$(PRRFIX)/lib -lfiledup -lcrypto

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
