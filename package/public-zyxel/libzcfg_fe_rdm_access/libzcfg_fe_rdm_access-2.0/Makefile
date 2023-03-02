OBJS = zcfg_fe_rdm_access.o
CURRENT_DIR = $(shell pwd)
LINK_LIBS = -ljson -lzcmd_tool -lzcfg_msg -lzos
LIBS_PATH = -L$(USRLIBDIR)
INC_PATH = -I$(TOOLCHAIN)/include -I.

CFLAGS += -DZCFG_TR98_SUPPORT $(INC_PATH) -std=gnu99
LDFLAGS += $(LIBS_PATH) $(LINK_LIBS)

ifeq ($(CONFIG_ZCFG_RPC_REQUEST_ADDITIONS),y)
CFLAGS += -DZCFG_RPC_REQUEST_ADDITIONS
endif

ifeq ($(CONFIG_TAAAB_CUSTOMIZATION_TR069),y)
CFLAGS += -DCONFIG_TAAAB_CUSTOMIZATION_TR069
endif

ifeq ($(CONFIG_ABUU_CUSTOMIZATION_MULTIPLE_CONFIG),y)
CFLAGS += -DABUU_CUSTOMIZATION_MULTIPLE_CONFIG
endif

ifeq ($(CONFIG_ZYXEL_MEMORY_LEAK_DETECT),y)
CFLAGS += -DZCFG_MEMORY_LEAK_DETECT
endif

ifeq ($(CONFIG_ZYXEL_WIFI_KEY_REMINDER),y)
CFLAGS += -DZYXEL_WIFI_KEY_REMINDER
endif

ifeq ($(CONFIG_ZYXEL_WEB_GUI_SHOW_ZYEE),y)
CFLAGS += -DZYXEL_WEB_GUI_SHOW_ZYEE
endif

ifeq ($(CONFIG_ZYXEL_PARTIAL_RESET),y)
CFLAGS += -DZYXEL_PARTIAL_RESET
endif

.PHONY : libzcfg_fe_rdm_access

all: clean libzcfg_fe_rdm_access

.c.o:
	$(CC) $(CFLAGS) -Wall -fPIC -c $< -o $@

libzcfg_fe_rdm_access : $(OBJS)
	$(CC) $(LDFLAGS) -shared -o libzcfg_fe_rdm_access.so $(OBJS)
	
clean:
	rm -rf *.o
	rm -rf *.so
