#
#   source code makefile
#
LIB_NAME    = libzos
OBJS        = zos_api.o
CURRENT_DIR = $(shell pwd)

ifeq ($(CONFIG_ZYXEL_MEMORY_LEAK_DETECT),y)
CFLAGS += -DZCFG_MEMORY_LEAK_DETECT
LINK_LIB +=  -ljson
endif

ifeq ($(CONFIG_ABUU_CUSTOMIZATION),y)
CFLAGS += -DABUU_CUSTOMIZATION
endif

CFLAGS += -Wno-format -Wno-format-security

LINK_LIB += -lrt -lzlog
#LIBS_PATH =
LDFLAGS += $(LIBS_PATH) $(LINK_LIB)

#CFLAGS += -I$(TOOLCHAIN)/include -I. -std=gnu99
CFLAGS += -I. -std=gnu99

.PHONY : $(LIB_NAME)

all: clean $(LIB_NAME)

.c.o:
	$(CC) $(CFLAGS) -Wall -Werror -fPIC -c $< -o $@

$(LIB_NAME) : $(OBJS)
	$(CC) $(LDFLAGS) -shared -o $(LIB_NAME).so $(OBJS)

clean:
	rm -rf *.o
	rm -rf *.so
