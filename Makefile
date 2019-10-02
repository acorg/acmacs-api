# -*- Makefile -*-
# ----------------------------------------------------------------------

# to generate compile_commands.json use
# intercept-build --override-compiler --use-c++ /usr/local/opt/llvm/bin/clang++ make -j6

# ----------------------------------------------------------------------

ACMACS_API_SERVER = $(DIST)/acmacs-api-server
ACMACS_API_SERVER_SOURCES = acmacs-api-server.cc mongo-access.cc session.cc acmacs-c2.cc curl.cc client-connection.cc $(COMMANDS_SOURCES)

API_DIRECT = $(DIST)/api-direct
API_DIRECT_SOURCES = api-direct.cc mongo-access.cc session.cc acmacs-c2.cc curl.cc client-connection.cc $(COMMANDS_SOURCES)

ACMACS_C2 = $(DIST)/acmacs-c2
ACMACS_C2_SOURCES = acmacs-c2.cc curl.cc acmacs-c2-main.cc

# ----------------------------------------------------------------------

COMMANDS_SOURCES = \
    command.cc command-factory.cc command-info.cc command-session.cc \
    command-admin.cc command-chart.cc command-chain.cc command-hidb.cc

# ----------------------------------------------------------------------

all: install

CONFIGURE_OPENSSL = 1
CONFIGURE_MONGO = 1
CONFIGURE_LIBCURL = 1
include $(ACMACSD_ROOT)/share/Makefile.config

MONGO_LDLIBS = -L$(AD_LIB) -lmongocxx -lbsoncxx $(OPENSSL_LIBS)
ACMACS_API_SERVER_LIBS = $(MONGO_LIBS) $(OPENSSL_LIBS) $(AD_LIB)/$(call shared_lib_name,libacmacswebserver,1,0)

# ----------------------------------------------------------------------

LDLIBS = \
  $(AD_LIB)/$(call shared_lib_name,libacmacsbase,1,0) \
  $(AD_LIB)/$(call shared_lib_name,liblocationdb,1,0) \
  $(AD_LIB)/$(call shared_lib_name,libacmacschart,2,0) \
  $(AD_LIB)/$(call shared_lib_name,libhidb,5,0) \
  $(AD_LIB)/$(call shared_lib_name,libseqdb,2,0) \
  $(AD_LIB)/$(call shared_lib_name,libacmacsdraw,1,0) \
  $(AD_LIB)/$(call shared_lib_name,libacmacsmapdraw,2,0) \
  -lpthread $(XZ_LIBS) $(LIBCURL_LIBS) $(CXX_LIBS)

PROGS = $(API_DIRECT) $(ACMACS_API_SERVER) $(ACMACS_C2)
RTAGS_TARGET = $(PROGS)

# ----------------------------------------------------------------------

CC = cc

install: $(PROGS)
	$(call symbolic_link_wildcard,$(abspath bin)/*,$(AD_BIN))
	$(call symbolic_link,$(DIST)/acmacs-api-server,$(AD_BIN))
	mkdir -p $(AD_SHARE)/js/acmacs-api
	$(call symbolic_link_wildcard,$(abspath js)/*,$(AD_SHARE)/js/acmacs-api)

test: install
	test/test
.PHONY: test

# ----------------------------------------------------------------------

$(API_DIRECT): $(patsubst %.cc,$(BUILD)/%.o,$(API_DIRECT_SOURCES)) | $(DIST)
	$(call echo_link_exe,$@)
	$(CXX) $(LDFLAGS) -o $@ $^ $(MONGO_LIBS) $(OPENSSL_LIBS) $(LDLIBS) $(AD_RPATH)

$(ACMACS_API_SERVER): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_API_SERVER_SOURCES)) | $(DIST)
	$(call echo_link_exe,$@)
	$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_API_SERVER_LIBS) $(LDLIBS) $(AD_RPATH)

$(ACMACS_C2): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_C2_SOURCES)) | $(DIST)
	$(call echo_link_exe,$@)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS) $(AD_RPATH)

# ----------------------------------------------------------------------

kill-server:
ifneq ($(KILL_SERVER),NO)
	if [ "`uname`" = "Darwin" ]; then killall acmacs-api-server 2>/dev/null || true; fi
endif

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
