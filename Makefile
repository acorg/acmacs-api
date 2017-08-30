# -*- Makefile -*-
# Eugene Skepner 2017

# to generate compile_commands.json use
# intercept-build --override-compiler --use-c++ /usr/local/opt/llvm/bin/clang++ make -j6

# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

ACMACS_API_SERVER = $(DIST)/acmacs-api-server
ACMACS_API_SERVER_SOURCES = acmacs-api-server.cc mongo-access.cc session.cc acmacs-c2.cc client-connection.cc $(COMMANDS_SOURCES)

API_DIRECT = $(DIST)/api-direct
API_DIRECT_SOURCES = api-direct.cc mongo-access.cc session.cc acmacs-c2.cc client-connection.cc $(COMMANDS_SOURCES)

ACMACS_C2 = $(DIST)/acmacs-c2
ACMACS_C2_SOURCES = acmacs-c2.cc acmacs-c2-main.cc

# ----------------------------------------------------------------------

COMMANDS_SOURCES = command.cc command-factory.cc command-info.cc command-session.cc command-admin.cc command-chart.cc command-chain.cc

# ----------------------------------------------------------------------

MONGO_LDLIBS = -L$(AD_LIB) -lmongocxx -lbsoncxx -L/usr/local/opt/openssl/lib $$(pkg-config --libs libssl)
ACMACS_API_SERVER_LIBS = $(MONGO_LDLIBS) -lacmacswebserver

# ----------------------------------------------------------------------

TARGET_ROOT=$(shell if [ -f /Volumes/rdisk/ramdisk-id ]; then echo /Volumes/rdisk/AD; else echo $(ACMACSD_ROOT); fi)
include $(TARGET_ROOT)/share/Makefile.g++
include $(TARGET_ROOT)/share/Makefile.dist-build.vars

OPTIMIZATION = -O3 #-fvisibility=hidden -flto
PROFILE = # -pg
CXXFLAGS = -g -MMD $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -Icc -I$(AD_INCLUDE) $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LDLIBS = -L$(AD_LIB) -lboost_filesystem -lboost_system -lpthread $$(pkg-config --libs liblzma) -lcurl

PKG_INCLUDES = -I$(AD_INCLUDE)/mongocxx/v_noabi -I$(AD_INCLUDE)/bsoncxx/v_noabi $$(pkg-config --cflags liblzma) $$(pkg-config --cflags libcrypto)

ifeq ($(shell uname -s),Darwin)
PKG_INCLUDES += -I/usr/local/opt/openssl/include
# $$(pkg-config --cflags libuv)
endif

PROGS = $(API_DIRECT) $(ACMACS_API_SERVER) $(ACMACS_C2)

# ----------------------------------------------------------------------

CC = cc

all: checks kill-server $(PROGS) client

install: checks $(PROGS) client
	@#ln -sf $(ACMACS_) $(AD_BIN)

.PHONY: client
client:
ifeq ($(shell uname -s),Darwin)
	$(MAKE) -C client -f Makefile.sub BUILD=$(abspath $(BUILD))/client DIST=$(abspath $(DIST))
endif

test: install
	test/test

checks: check-acmacsd-root check-libcurl

RTAGS_TARGET = $(PROGS)
include $(AD_SHARE)/Makefile.rtags

# ----------------------------------------------------------------------

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

$(API_DIRECT): $(patsubst %.cc,$(BUILD)/%.o,$(API_DIRECT_SOURCES)) | $(DIST)
	@echo "LINK       " $@ # '<--' $^
	@$(CXX) $(LDFLAGS) -o $@ $^ $(MONGO_LDLIBS) $(LDLIBS)

$(ACMACS_API_SERVER): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_API_SERVER_SOURCES)) | $(DIST)
	@echo "LINK       " $@ # '<--' $^
	@$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_API_SERVER_LIBS) $(LDLIBS)

$(ACMACS_C2): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_C2_SOURCES)) | $(DIST)
	@echo "LINK       " $@ # '<--' $^
	@$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# ----------------------------------------------------------------------

$(BUILD)/%.o: $(CC)/%.cc | $(BUILD)
	@echo $(CXX_NAME) $<
	@$(CXX) $(CXXFLAGS) -c -o $@ $(abspath $<)

# ----------------------------------------------------------------------

check-libcurl:
	ls /usr/lib/libcurl.* >/dev/null 2>&1 || ls /usr/lib/*/libcurl.so.4 >/dev/null 2>&1 || ( echo "ERROR: Please install libcurl (apt-get install libcurl4-openssl-dev)" >&2 && false )

ifeq ($(shell uname -s),Darwin)
 SASSC_INSTALL = "brew install sassc (https://github.com/sass/sassc)"
else
 SASSC_INSTALL = "https://github.com/sass/sassc"
endif

kill-server:
ifneq ($(KILL_SERVER),NO)
	if [ "`uname`" = "Darwin" ]; then killall acmacs-api-server 2>/dev/null || true; fi
endif

include $(AD_SHARE)/Makefile.dist-build.rules

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
