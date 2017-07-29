# -*- Makefile -*-
# Eugene Skepner 2017

# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

ACMACS_API_SERVER = $(DIST)/acmacs-api-server
ACMACS_API_SERVER_SOURCES = acmacs-api-server.cc mongo-access.cc session.cc $(COMMANDS_SOURCES)

ACMACS_API_CLIENT_JS = $(DIST)/acmacs-api-client.js.gz
ACMACS_API_CLIENT_CSS = $(DIST)/acmacs-api-client.css.gz
ACMACS_API_CLIENT_SOURCES = acmacs-api-client.cc asm.cc session.cc

API_DIRECT = $(DIST)/api-direct
API_DIRECT_SOURCES = api-direct.cc mongo-access.cc session.cc $(COMMANDS_SOURCES)

ACMACS_C2 = $(DIST)/acmacs-c2
ACMACS_C2_SOURCES = acmacs-c2.cc acmacs-c2-main.cc

# ----------------------------------------------------------------------

COMMANDS_SOURCES = command.cc command-factory.cc command-info.cc command-session.cc command-admin.cc command-chart.cc command-chain.cc

# ----------------------------------------------------------------------

MONGO_LDLIBS = -L$(LIB_DIR) -lmongocxx -lbsoncxx -L/usr/local/opt/openssl/lib $$(pkg-config --libs libssl)
ACMACS_API_SERVER_LIBS = $(MONGO_LDLIBS) -lacmacswebserver

ifeq ($(shell uname -s),Darwin)
  MAKE_CLIENT = 1
else
  MAKE_CLIENT = 0
endif

# ----------------------------------------------------------------------

CHEERP = /opt/cheerp/bin/clang++ -target cheerp
CHEERP_COMPILE_FLAGS = -std=c++1z -MMD -I. -Iinclude -g $(OPTIMIZATION) $(WEVERYTHING) -Wno-unknown-pragmas
# --cheerp-preexecute
CHEERP_LINK_FLAGS = $(OPTIMIZATION)

SASSC = sassc

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/Makefile.g++

LIB_DIR = $(ACMACSD_ROOT)/lib

OPTIMIZATION = -O3 #-fvisibility=hidden -flto
PROFILE = # -pg
CXXFLAGS = -g -MMD $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -Icc -I$(BUILD)/include -I$(ACMACSD_ROOT)/include $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LDLIBS = -L$(LIB_DIR) -lboost_filesystem -lboost_system -lpthread $$(pkg-config --libs liblzma)

PKG_INCLUDES = -I$(ACMACSD_ROOT)/include/mongocxx/v_noabi -I$(ACMACSD_ROOT)/include/bsoncxx/v_noabi $$(pkg-config --cflags liblzma) $$(pkg-config --cflags libcrypto)

ifeq ($(shell uname -s),Darwin)
PKG_INCLUDES += -I/usr/local/opt/openssl/include
# $$(pkg-config --cflags libuv)
endif

PROGS = $(API_DIRECT) $(ACMACS_API_SERVER) $(ACMACS_C2)
ifeq ($(MAKE_CLIENT),1)
  PROGS += acmacs-api-client
endif

# ----------------------------------------------------------------------

BUILD = build
DIST = $(abspath dist)
CC = cc
CLIENT = client

all: checks kill-server $(PROGS)

install: checks $(PROGS)
	@#ln -sf $(ACMACS_) $(ACMACSD_ROOT)/bin

test: install
	test/test

checks: check-acmacsd-root check-cheerp check-sassc check-libcurl

# ----------------------------------------------------------------------

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

$(API_DIRECT): $(patsubst %.cc,$(BUILD)/%.o,$(API_DIRECT_SOURCES)) | $(DIST)
	@echo $@ '<--' $^
	@$(GXX) $(LDFLAGS) -o $@ $^ $(MONGO_LDLIBS) $(LDLIBS)

$(ACMACS_API_SERVER): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_API_SERVER_SOURCES)) | $(DIST)
	@echo $@ '<--' $^
	@$(GXX) $(LDFLAGS) -o $@ $^ $(ACMACS_API_SERVER_LIBS) $(LDLIBS)

$(ACMACS_C2): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_C2_SOURCES)) | $(DIST)
	@echo $@ '<--' $^
	@$(GXX) $(LDFLAGS) -o $@ $^ $(LDLIBS) -lcurl

acmacs-api-client: $(ACMACS_API_CLIENT_JS) $(ACMACS_API_CLIENT_CSS)

$(ACMACS_API_CLIENT_JS): $(patsubst %.cc,$(BUILD)/%.bc,$(ACMACS_API_CLIENT_SOURCES)) | $(DIST)
	@echo cheerp-link $(notdir $@)
	@$(CHEERP) $(CHEERP_LINK_FLAGS) -cheerp-sourcemap=$(basename $@).map -o $(basename $@) $^
	@gzip -9f $(basename $@) $(basename $@).map


# ----------------------------------------------------------------------

clean:
	rm -rf $(DIST) $(BUILD)/*.o $(BUILD)/*.d $(BUILD)/*.bc

distclean: clean
	rm -rf $(BUILD) $(DIST)

# ----------------------------------------------------------------------

$(BUILD)/%.o: $(CC)/%.cc | $(BUILD)
	@echo $<
	@$(GXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/%.bc: $(CLIENT)/%.cc | $(BUILD)
	@echo cheerp $<
	@$(CHEERP) $(CHEERP_COMPILE_FLAGS) -c -o $@ $<

$(DIST)/%.css.gz: $(CLIENT)/%.sass $(wildcard $(CLIENT)/*.sass) | $(DIST)
	@echo $(notdir $@)
	@$(SASSC) --style compressed -I $(CLIENT) $< | gzip -9 >$@

# ----------------------------------------------------------------------

check-acmacsd-root:
ifndef ACMACSD_ROOT
	$(error ACMACSD_ROOT is not set)
endif

check-cheerp:
ifeq ($(MAKE_CLIENT),1)
	@$(CHEERP) -v >/dev/null 2>&1 || ( echo "ERROR: Please install cheerp (http://leaningtech.com/cheerp/download/)" >&2 && false )
endif

check-libcurl:
	ls /usr/lib/libcurl.* >/dev/null 2>&1 || ls /usr/lib/*/libcurl.so.4 >/dev/null 2>&1 || ( echo "ERROR: Please install libcurl (apt-get install libcurl4-openssl-dev)" >&2 && false )

ifeq ($(shell uname -s),Darwin)
 SASSC_INSTALL = "brew install sassc (https://github.com/sass/sassc)"
else
 SASSC_INSTALL = "https://github.com/sass/sassc"
endif
check-sassc:
ifeq ($(MAKE_CLIENT),1)
	@$(SASSC) -v >/dev/null 2>&1 || ( echo "ERROR: Please install SASSC:" $(SASSC_INSTALL) >&2 && false )
endif

kill-server:
	if [ "`uname`" = "Darwin" ]; then killall acmacs-api-server 2>/dev/null || true; fi

$(DIST):
	mkdir -p $(DIST)

$(BUILD):
	mkdir -p $(BUILD)

.PHONY: check-acmacsd-root

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
