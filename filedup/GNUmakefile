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

SHLIB = filedup
SHLIB_MAJOR = 0.0
SHNAME = lib$(SHLIB).so
SHLIBNAME = $(SHNAME).$(SHLIB_MAJOR)

SRCS = filename.cc filestamps.cc md5.cc options.cc scan.cc

CXXFLAGS += -g -fpic -std=c++17 -pedantic -Wall -Wextra
all: $(SHLIBNAME)

clean:
	- rm $(SHLIBNAME) $(SHNAME) $(SRCS:.cc=.o)

$(SHLIBNAME): $(SRCS:.cc=.o)
	$(LINK.c) -shared -o $@ $^
	- rm $(SHNAME)
	ln -s $(SHLIBNAME) $(SHNAME)
