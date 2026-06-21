NAME = SCReceive
FILES_DSP = SCReceive.cpp

# Link the POSIX RT library for Shared Memory
LDFLAGS += -lrt

include ../../Makefile.plugins.mk

all: lv2
	@echo "---manually forcing TTL generation---"
	@../../utils/lv2_ttl_generator/lv2_ttl_generator ../../bin/SCReceive.lv2/SCReceive.so
	@cp *.ttl ../../bin/SCReceive.lv2/
	@echo "ttls copied to plugin directory"