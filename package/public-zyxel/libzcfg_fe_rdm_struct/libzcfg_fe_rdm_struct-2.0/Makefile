OBJS = zcfg_fe_rdm_struct.o
CURRENT_DIR = $(shell pwd)
LIB_PATH = -L$(USRLIBDIR)
LINK_LIBS = -ljson -lzcfg_msg -lzcfg_fe_rdm_access -lzcmd_tool -lzos

ifeq ($(CONFIG_ZYXEL_MEMORY_LEAK_DETECT),y)
CFLAGS += -DZCFG_MEMORY_LEAK_DETECT
endif

LDFLAGS += $(LIB_PATH) $(LINK_LIBS)
CFLAGS += -I$(TOOLCHAIN)/include -I. -std=gnu99

.PHONY : libzcfg_fe_rdm_struct

all: clean libzcfg_fe_rdm_struct

.c.o:
	$(CC) $(CFLAGS) -Wall -fPIC -c $< -o $@

libzcfg_fe_rdm_struct: $(OBJS)
	$(CC) $(LDFLAGS) -shared -o libzcfg_fe_rdm_struct.so $(OBJS)

clean:
	rm -rf *.o
	rm -rf *.so
