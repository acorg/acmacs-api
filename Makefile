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

MONGO_LDLIBS = -L$(LIB_DIR) -lmongocxx -lbsoncxx -L/usr/local/opt/openssl/lib $$(pkg-config --libs libssl)
ACMACS_API_SERVER_LIBS = $(MONGO_LDLIBS) -lacmacswebserver

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/Makefile.g++

LIB_DIR = $(ACMACSD_ROOT)/lib

OPTIMIZATION = -O3 #-fvisibility=hidden -flto
PROFILE = # -pg
CXXFLAGS = -g -MMD $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -Icc -I$(BUILD)/include -I$(ACMACSD_ROOT)/include $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LDLIBS = -L$(LIB_DIR) -lboost_filesystem -lboost_system -lpthread $$(pkg-config --libs liblzma) -lcurl

PKG_INCLUDES = -I$(ACMACSD_ROOT)/include/mongocxx/v_noabi -I$(ACMACSD_ROOT)/include/bsoncxx/v_noabi $$(pkg-config --cflags liblzma) $$(pkg-config --cflags libcrypto)

ifeq ($(shell uname -s),Darwin)
PKG_INCLUDES += -I/usr/local/opt/openssl/include
# $$(pkg-config --cflags libuv)
endif

PROGS = $(API_DIRECT) $(ACMACS_API_SERVER) $(ACMACS_C2)

# ----------------------------------------------------------------------

BUILD = build
DIST = $(abspath dist)
CC = cc

all: checks kill-server $(PROGS) client

install: checks $(PROGS) client
	@#ln -sf $(ACMACS_) $(ACMACSD_ROOT)/bin

.PHONY: client
client:
ifeq ($(shell uname -s),Darwin)
	$(MAKE) -C client -f Makefile.sub BUILD=$(abspath $(BUILD))/client DIST=$(abspath $(DIST))
endif

test: install
	test/test

checks: check-acmacsd-root check-libcurl

rtags:
	make -nk | /usr/local/bin/rc --compile - || true

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

clean:
	rm -rf $(DIST) $(BUILD)

# ----------------------------------------------------------------------

$(BUILD)/%.o: $(CC)/%.cc | $(BUILD)
	@echo "C++        " $<
	@$(CXX) $(CXXFLAGS) -c -o $@ $(abspath $<)

# ----------------------------------------------------------------------

check-acmacsd-root:
ifndef ACMACSD_ROOT
	$(error ACMACSD_ROOT is not set)
endif

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

$(DIST):
	mkdir -p $(DIST)

$(BUILD):
	mkdir -p $(BUILD)

.PHONY: check-acmacsd-root

# ----------------------------------------------------------------------

# ACMACS_API_CLIENT_JS = $(DIST)/acmacs-api-client.js.gz
# ACMACS_API_CLIENT_CSS = $(DIST)/acmacs-api-client.css.gz
# ACMACS_API_CLIENT_SOURCES = acmacs-api-client.cc asm.cc login.cc widget.cc login-widget.cc

# ifeq ($(shell uname -s),Darwin)
#   MAKE_CLIENT = 1
# else
#   MAKE_CLIENT = 0
# endif

# ifeq ($(MAKE_CLIENT),1)
#   PROGS += acmacs-api-client
# endif

# CHEERP = /opt/cheerp/bin/clang++ -target cheerp
# CHEERP_COMPILE_FLAGS = -std=c++1z -MMD -I. -Iinclude -g $(OPTIMIZATION) $(WEVERYTHING) -Wno-unknown-pragmas
# # --cheerp-preexecute
# CHEERP_LINK_FLAGS = $(OPTIMIZATION)

# SASSC = sassc

# acmacs-api-client: $(ACMACS_API_CLIENT_JS) $(ACMACS_API_CLIENT_CSS)

# $(ACMACS_API_CLIENT_JS): $(patsubst %.cc,$(BUILD)/%.bc,$(ACMACS_API_CLIENT_SOURCES)) | $(DIST)
#	@echo cheerp-link $(notdir $@)
#	@$(CHEERP) $(CHEERP_LINK_FLAGS) -cheerp-sourcemap=$(basename $@).map -o $(basename $@) $^
#	@gzip -9f $(basename $@) $(basename $@).map

# $(BUILD)/%.bc: $(CLIENT)/%.cc | $(BUILD)
#	@echo cheerp $<
#	@$(CHEERP) $(CHEERP_COMPILE_FLAGS) -c -o $@ $<

# $(DIST)/%.css.gz: $(CLIENT)/%.sass $(wildcard $(CLIENT)/*.sass) | $(DIST)
#	@echo $(notdir $@)
#	@$(SASSC) --style compressed -I $(CLIENT) $< | gzip -9 >$@

# check-cheerp:
# ifeq ($(MAKE_CLIENT),1)
#	@$(CHEERP) -v >/dev/null 2>&1 || ( echo "ERROR: Please install cheerp (http://leaningtech.com/cheerp/download/)" >&2 && false )
# endif

# check-sassc:
# ifeq ($(MAKE_CLIENT),1)
#	@$(SASSC) -v >/dev/null 2>&1 || ( echo "ERROR: Please install SASSC:" $(SASSC_INSTALL) >&2 && false )
# endif

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
