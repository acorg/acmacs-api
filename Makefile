# -*- Makefile -*-
# Eugene Skepner 2017

# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

MONGO_TEST = $(DIST)/mongo-test
MONGO_FIND = $(DIST)/mongo-find
MONGO_RAW = $(DIST)/mongo-raw
MONGO_DIRECT = $(DIST)/mongo-direct
ACMACS_API_SERVER = $(DIST)/acmacs-api-server
ACMACS_API_CLIENT = $(DIST)/acmacs-api-client.js.gz

MONGO_TEST_SOURCES = mongo-test.cc
MONGO_FIND_SOURCES = mongo-find.cc
MONGO_RAW_SOURCES = mongo-raw.cc session.cc
MONGO_DIRECT_SOURCES = mongo-direct.cc session.cc
ACMACS_API_SERVER_SOURCES = acmacs-api-server.cc session.cc
ACMACS_API_CLIENT_SOURCES = acmacs-api-client.cc

MONGO_LDLIBS = -L$(LIB_DIR) -lmongocxx -lbsoncxx -L/usr/local/opt/openssl/lib $$(pkg-config --libs libssl)
ACMACS_API_SERVER_LIBS = $(MONGO_LDLIBS) -lacmacswebserver

# ----------------------------------------------------------------------

CHEERP = /opt/cheerp/bin/clang++ -target cheerp  -Wno-unknown-pragmas
CHEERP_FLAGS = -std=c++1z -I. -Iinclude -g $(OPTIMIZATION) $(WEVERYTHING)

# ----------------------------------------------------------------------

CLANG = $(shell if g++ --version 2>&1 | grep -i llvm >/dev/null; then echo Y; else echo N; fi)
ifeq ($(CLANG),Y)
  WEVERYTHING = -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded
  WARNINGS = -Wno-weak-vtables # -Wno-padded
  STD = c++1z
else
  WEVERYTHING = -Wall -Wextra
  WARNINGS =
  STD = c++1z
endif

LIB_DIR = $(ACMACSD_ROOT)/lib

OPTIMIZATION = -O3 #-fvisibility=hidden -flto
PROFILE = # -pg
CXXFLAGS = -g -MMD $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -Icc -I$(BUILD)/include -I$(ACMACSD_ROOT)/include $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LDLIBS = -L$(LIB_DIR) -lboost_system -lpthread $$(pkg-config --libs liblzma)

PKG_INCLUDES = -I$(ACMACSD_ROOT)/include/mongocxx/v_noabi -I$(ACMACSD_ROOT)/include/bsoncxx/v_noabi $$(pkg-config --cflags liblzma) $$(pkg-config --cflags libcrypto)

ifeq ($(shell uname -s),Darwin)
PKG_INCLUDES += -I/usr/local/opt/openssl/include
# $$(pkg-config --cflags libuv)
endif

PROGS = $(MONGO_TEST) $(MONGO_FIND) $(MONGO_RAW) $(MONGO_DIRECT) $(ACMACS_API_SERVER) $(ACMACS_API_CLIENT)

# ----------------------------------------------------------------------

BUILD = build
DIST = $(abspath dist)
CC = cc

all: check-acmacsd-root $(PROGS)

install: check-acmacsd-root $(PROGS)
	@#ln -sf $(ACMACS_) $(ACMACSD_ROOT)/bin

test: install
	test/test

# ----------------------------------------------------------------------

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

$(MONGO_TEST): $(patsubst %.cc,$(BUILD)/%.o,$(MONGO_TEST_SOURCES)) | $(DIST)
	g++ $(LDFLAGS) -o $@ $^ $(MONGO_LDLIBS) $(LDLIBS)

$(MONGO_FIND): $(patsubst %.cc,$(BUILD)/%.o,$(MONGO_FIND_SOURCES)) | $(DIST)
	g++ $(LDFLAGS) -o $@ $^ $(MONGO_LDLIBS) $(LDLIBS)

$(MONGO_RAW): $(patsubst %.cc,$(BUILD)/%.o,$(MONGO_RAW_SOURCES)) | $(DIST)
	g++ $(LDFLAGS) -o $@ $^ $(MONGO_LDLIBS) $(LDLIBS)

$(MONGO_DIRECT): $(patsubst %.cc,$(BUILD)/%.o,$(MONGO_DIRECT_SOURCES)) | $(DIST)
	g++ $(LDFLAGS) -o $@ $^ $(MONGO_LDLIBS) $(LDLIBS)

$(ACMACS_API_SERVER): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_API_SERVER_SOURCES)) | $(DIST)
	g++ $(LDFLAGS) -o $@ $^ $(ACMACS_API_SERVER_LIBS) $(LDLIBS)

$(ACMACS_API_CLIENT): $(patsubst %,client/%,$(ACMACS_API_CLIENT_SOURCES)) | $(DIST)
	$(CHEERP) $(CHEERP_FLAGS) -cheerp-sourcemap=$@.map -o - $^ | /usr/bin/gzip -9 >$@

# ----------------------------------------------------------------------

clean:
	rm -rf $(DIST) $(BUILD)/*.o $(BUILD)/*.d

distclean: clean
	rm -rf $(BUILD)

# ----------------------------------------------------------------------

$(BUILD)/%.o: $(CC)/%.cc | $(BUILD)
	@echo $<
	@g++ $(CXXFLAGS) -c -o $@ $<

# ----------------------------------------------------------------------

check-acmacsd-root:
ifndef ACMACSD_ROOT
	$(error ACMACSD_ROOT is not set)
endif

$(DIST):
	mkdir -p $(DIST)

$(BUILD):
	mkdir -p $(BUILD)

.PHONY: check-acmacsd-root

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
