OBJS = zcfg_fe_rdm_string.o
CURRENT_DIR = $(shell pwd)
LINK_LIB = -ljson -lzcfg_msg -lzcfg_fe_rdm_access -lzos
INC_PATH = -I$(TOOLCHAIN)/include -I.

LDFLAGS += $(LINK_LIB)
CFLAGS += $(INC_PATH) -DZCFG_TR98_SUPPORT -std=gnu99


ifeq ($(CONFIG_ZCFG_LOG_USE_DEBUGCFG),y)
CFLAGS += -DZCFG_LOG_USE_DEBUGCFG
endif

ifeq ($(CONFIG_ZCFG_RPC_REQUEST_ADDITIONS),y)
CFLAGS += -DZCFG_RPC_REQUEST_ADDITIONS
endif

ifeq ($(CONFIG_ZYXEL_MEMORY_LEAK_DETECT),y)
CFLAGS += -DZCFG_MEMORY_LEAK_DETECT
endif

.PHONY : libzcfg_fe_rdm_string

all: clean libzcfg_fe_rdm_string

.c.o:
	$(CC) $(CFLAGS) -Wall -fPIC -c $< -o $@

libzcfg_fe_rdm_string : $(OBJS)
	$(CC) $(LDFLAGS) -shared -o libzcfg_fe_rdm_string.so $(OBJS)

clean:
	rm -rf *.o
	rm -rf *.so
