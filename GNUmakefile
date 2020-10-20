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

SUBDIR = filedup src load

.PHONY: $(SUBDIR)

all: $(SUBDIR)

clean:
	for P in $(SUBDIR); do $(MAKE) --directory=$$P clean; done

$(SUBDIR):
	$(MAKE) RELEASE=$(RELEASE) -C $@
