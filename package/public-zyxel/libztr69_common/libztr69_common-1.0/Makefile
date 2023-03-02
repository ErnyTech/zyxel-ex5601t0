#/******************************************************************************/
#/*
#*  Copyright (C) 2012 ZyXEL Communications, Corp.
#*  All Rights Reserved.
#*
#* ZyXEL Confidential; Need to Know only.
#* Protected as an unpublished work.
#*
#* The computer program listings, specifications and documentation
#* herein are the property of ZyXEL Communications, Corp. and
#* shall not be reproduced, copied, disclosed, or used in whole or
#* in part for any reason without the prior express written permission of
#* ZyXEL Communications, Corp.
#*/
#/******************************************************************************/
#include $(OPAL_PRODUCT_ROOT)/include/opal_inc.mk

LIBRARY = libztr69_common
LIBRARY_SRC_PATH = ./lib/src
LIBRARY_INC_PATH = ./lib/include
LIBRARY_SOURCES += $(wildcard $(LIBRARY_SRC_PATH)/*.c)
LIBRARY_OBJS = $(patsubst %.c, %.o, $(LIBRARY_SOURCES))
CLI_EXE = ztr69cli
CLI_SRC_PATH = ./cli/src
CLI_INC_PATH = ./cli/include
CLI_SOURCES += $(wildcard $(CLI_SRC_PATH)/*.c)
CLI_OBJS = $(patsubst %.c, %.o, $(CLI_SOURCES))
INC_PATH += -I$(CLI_INC_PATH) -I$(LIBRARY_INC_PATH)

LIBS_PATH += -L$(TOOLCHAIN_TOP)/lib -L$(INSTALL_DIR)/lib/public -L$(INSTALL_DIR)/lib/private -L$(INSTALL_DIR)/usr/lib -L$(STAGING_DIR)/usr/lib -L./
LIBS_ZCFG = -lzcfg_fe_rdm_access -lzcmd_tool -ljson -lzcfg_msg -lzcfg_fe_schema -lzcfg_fe_rdm_string -lzcfg_fe_rdm_struct -lzcfg_fe_tr98 -lzcfg_fe_dal -lzos -lzlog -lsqlite3
LIBS_ZCFG += -lzyutil -lzywwan
LIBS += $(LIBS_ZCFG)
CLI_LIBS = -lclinkc -lexpat -lssl -luuid $(LIBS) -lztr69 -lztr69_common

CFLAGS += $(INC_PATH) $(LIBS_PATH) -std=gnu99 -Wall -fno-omit-frame-pointer -O2
CFLAGS += -Werror
all: $(LIBRARY) $(CLI_EXE)
clone:
	@echo Building directories...
	@mkdir -pv $(PKG_BUILD_DIR)
	@cp -rf . $(PKG_BUILD_DIR)

$(LIBRARY): $(LIBRARY_OBJS)
	$(CC) $(CFLAGS) -shared -o $@.so $(LIBRARY_OBJS)

$(LIBRARY_OBJS):
	$(CC) $(CFLAGS) -Wall -fPIC -c $(patsubst %.o, %.c, $@) -o $@

$(CLI_EXE): $(CLI_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLI_OBJS) -Wl,-rpath, $(CLI_LIBS)

clean:
	@rm -rf $(CLI_EXE)
	@rm $(CLI_OBJS)
	@rm -rf $(LIBRARY).so
	@rm $(LIBRARY_OBJS)
	@echo "==> clean"

