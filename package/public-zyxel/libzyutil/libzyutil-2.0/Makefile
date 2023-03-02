CURRENT_DIR = $(shell pwd)
#COMMON_ZCFG_INCLUDE_DIR = $(CURRENT_DIR)/../../../private/apps/zcfg/include

#PLATFORM=broadcom


LIBS_PATH = -L.
LINK_LIBS = -ljson -lcrypto -lzlog -lzos -lzcfg_msg

#INC_PATH = -I$(TOOLCHAIN)/include -I$(COMMON_ZCFG_INCLUDE_DIR)
INC_PATH +=  -I. -I./$(ZCFG_PLATFORM)
INC_PATH += -I./libzyutil_encrypt

ifneq ($(CONFIG_EXTERNAL_TOOLCHAIN),)
INC_PATH += -idirafter$(STAGING_DIR)/usr/include
endif

ifeq ($(ZCFG_PLATFORM),BROADCOM)
## for broadcom-sdk-502L07/shared/opensource/include/bcm963xx/bcmTag.h
ZCFLAGS += -DBCMTAG_EXE_USE

ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(BCM_BUILD_DIR)/shared/broadcom/include/$(BRCM_BOARD) \
                         -I$(BCM_BUILD_DIR)/shared/opensource/include/$(BRCM_BOARD) \
                         -I$(STAGING_DIR)/usr/include

INC_PATH += $(ALLOWED_INCLUDE_PATHS)
endif

ifeq ($(ZCFG_PLATFORM),ECONET)
INC_PATH += -I$(STAGING_DIR)/usr/include/trx
ifneq ($(strip $(TCSUPPORT_SECURE_BOOT)),)
INC_PATH += -I$(STAGING_DIR)/usr/include/global_inc/
ZCFLAGS+=-DTCSUPPORT_SECURE_BOOT
endif
ifneq ($(strip $(TCSUPPORT_SECURE_BOOT_V1)),)
ZCFLAGS+=-DTCSUPPORT_SECURE_BOOT_V1
endif
ifneq ($(strip $(TCSUPPORT_SECURE_BOOT_V2)),)
ZCFLAGS+=-DTCSUPPORT_SECURE_BOOT_V2
endif
ifneq ($(strip $(TCSUPPORT_SECURE_BOOT_FLASH_OTP)),)
ZCFLAGS+=-DTCSUPPORT_SECURE_BOOT_FLASH_OTP
endif

ifeq ($(CONFIG_TC_SDK_VERSION),"7.3.267.750")
LINK_LIBS+= -lubootenv
endif

LINK_LIBS+= -lzcfg_msg

ifeq ($(CONFIG_TC_SDK_VERSION),"7.3.267.750")
LINK_LIBS+= -lubootenv
endif

# to include zyxel_province.h
ifneq ($(ZYXEL_MULTI_PROVINCE_VENDOR),)
ZCFLAGS += -DZYXEL_MULTI_PROVINCE_VENDOR
endif

ifeq ($(ZYXEL_15_BYTE_SERIAL_NUMBER),y)
ZCFLAGS += -DZYXEL_15_BYTE_SERIAL_NUMBER
endif

endif

ifeq ($(ZYXEL_SYSTEMCMD_REPLACEMENT),y)
ZCFLAGS += -DZYXEL_IFCONFIG_BY_IOCTL
ZCFLAGS += -DZYXEL_ECHO_REPLACEMENT
endif

ifeq ($(CONFIG_ZYXEL_TUTK_CLOUD_AGENT),y)
ZCFLAGS += -DZYXEL_TUTK_CLOUD_AGENT
endif

ifeq ($(CONFIG_ZYXEL_AT_SET_TUTK_UID),y)
ZCFLAGS += -DZYXEL_AT_SET_TUTK_UID
endif

LIB_DEBUG_OBJS = debugcfg.o

ZCFLAGS += -DZYXEL=1 -std=gnu99
LIB_OBJS = common.o flash.o skconn.o zyutil_cofw.o
LIB_OBJS += zyutil_sfp.o

# ZYXEL_FEATURE_BITS
ifneq ($(findstring ZYXEL_FEATURE_BITS,$(ZYXEL_LIB_CFLAGS)),)
LIB_OBJS += libzyutil_zyxelfeaturebits.o
endif

PKG_NAME = libzyutil
LIB_OBJS += libzyutil_encrypt/libzyutil_encrypt.o
LIB_OBJS += libzyutil_encrypt/libzyutil_encrypt_SHA256.o
LIB_OBJS += libzyutil_encrypt/libzyutil_encrypt_MD5.o

ifneq ($(BROADCOM_UBOOT_PLATFORM),)
LIB_OBJS += $(ZCFG_PLATFORM)/$(PKG_NAME)_uboot_wrapper.o
else
LIB_OBJS += $(ZCFG_PLATFORM)/$(PKG_NAME)_wrapper.o
endif

#LDFLAGS += -L$(STAGING_DIR)/usr/lib
.PHONY : libzyutil

all: clean libzyutil libdebug.a

### Implicit rules ###
.c.o:
	$(CC) -c -fPIC $(CFLAGS) $(LIBS_PATH) $(INC_PATH) $< -o $@

libzyutil : debug $(LIB_OBJS)
	$(CC) -shared -o libzyutil.so $(LIB_OBJS) $(LINK_LIBS) $(LDFLAGS)
	$(STRIP) libzyutil.so

debug:
	@echo CFLAGS=$(CFLAGS)

libdebug.a : $(LIB_DEBUG_OBJS)
	$(AR) rs $@ $(LIB_DEBUG_OBJS)

install:
	install -m 666 zyutil.h $(COMMON_ZCFG_INCLUDE_DIR)
	install -m 666 zyutil_cofw.h $(COMMON_ZCFG_INCLUDE_DIR)
	install -m 666 zyutil_sfp.h $(COMMON_ZCFG_INCLUDE_DIR)

clean:
	rm -rf *.o
	rm -rf *.so
	rm -rf $(COMMON_ZCFG_INCLUDE_DIR)/zyutil.h
	rm -rf $(COMMON_ZCFG_INCLUDE_DIR)/zyutil_cofw.h
	rm -rf $(COMMON_ZCFG_INCLUDE_DIR)/zyutil_sfp.h

#CURR_DIR:=$(shell pwd)
#BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
#BUILD_DIR:=$(word 1, $(BUILD_DIR))

ifeq ($(ZCFG_PLATFORM),BROADCOM)
include $(BCM_BUILD_DIR)/make.common
endif

CFLAGS+= $(ZCFLAGS) $(ZYXEL_LIB_CFLAGS)
CFLAGS +=  -DZYXEL_PRODUCT_NAME='"$(PRODUCT_NAME)"'
