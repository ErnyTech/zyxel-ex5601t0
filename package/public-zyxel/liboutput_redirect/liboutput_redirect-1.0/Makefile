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

BUILD_APPS = liboutput_redirect.so 



CFLAGS += $(INC_PATH) -std=gnu99 -Wall -fno-omit-frame-pointer -O2
CFGAGS += -Werror
SRC_OBJS:= output_redirect.c

LDFLAGS += 
all: $(BUILD_APPS)

$(BUILD_APPS): $(SRC_OBJS)
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -fPIC -shared -o $@

debug: debug.c 
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -L. -loutput_redirect -o $@

clean:
	@rm *.o *.so
	@echo "==> clean"

