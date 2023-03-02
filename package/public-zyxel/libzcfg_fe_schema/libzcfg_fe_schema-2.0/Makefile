OBJS = zcfg_fe_schema.o
CURRENT_DIR = $(shell pwd)
LINK_LIB = -ljson -lzcfg_fe_rdm_access
LIBS_PATH = -L$(USRLIBDIR)

LDFLAGS += $(LIBS_PATH) $(LINK_LIB)
CFLAGS += -I$(TOOLCHAIN)/include -I. -std=gnu99

.PHONY : libzcfg_fe_schema

all: clean libzcfg_fe_schema

.c.o:
	$(CC) $(CFLAGS) -Wall -fPIC -c $< -o $@

libzcfg_fe_schema : $(OBJS)
	$(CC) $(LDFLAGS) -shared -o libzcfg_fe_schema.so $(OBJS)

clean:
	rm -rf *.o
	rm -rf *.so
