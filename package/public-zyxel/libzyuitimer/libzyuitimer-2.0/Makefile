LIB_NAME = libzyuitimer
CFLAGS = -fPIC
INC_PATH = -I$(CURDIR)
INC_PATH += -I$(STAGING_DIR)/usr/include
ZCFG_LIBS =
LIBS_PATH = -lpthread -lzos -lzlog
LIBS_PATH += -L$(STAGING_DIR)/usr/lib
EXTRA_LIBS =
OBJS = uitimer.o timer_list.o

.PHONY: clean install

default: all

all: clean $(LIB_NAME)

ifneq ($(LIBC_USE_MUSL),)
CFLAGS += -DLIBC_USE_MUSL
endif


CFLAGS += -Wall -Werror -Wno-error=unused-variable -Wno-array-bounds

.c.o:
	$(CC) $(CFLAGS) $(ZCFLAGS) $(INC_PATH) -c $< -o $@

$(LIB_NAME): $(OBJS)
	$(CC) -shared -o $@.so $(OBJS) $(LIBS_PATH) $(ZCFG_LIBS) $(EXTRA_LIBS)

clean:
	rm -rf *.o
	rm -rf *.so

install:
	mkdir -p $(INSTALL_DIR)/lib
	cp -fp $(LIB_NAME).so $(INSTALL_DIR)/lib

