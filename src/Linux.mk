ifeq ("$(PLATFORM)", "86")
	ARCH=-m32
endif

CC = gcc
CFLAGS += $(ARCH) -ffunction-sections -fPIC -Werror -Wno-format-truncation -Wall -Wextra -O2 -I.
LDFLAGS = -shared
RM = rm -f

TARGET_LIB = libtlv.so
SRCS_LIB = util.c libtlv.c llist.c
OBJS_LIB = $(SRCS_LIB:.c=.o)

TARGET_BIN = apptesthash

.PHONY: all
all: $(TARGET_LIB) $(TARGET_BIN) 

$(TARGET_LIB): $(OBJS_LIB)
	$(CC) $(ARCH) $(LDFLAGS) -o $@ $^ 

$(TARGET_BIN):
	$(CC) $(CFLAGS) -c -o apptesthash.o apptesthash.c
	$(CC) $(ARCH) -o $@ apptesthash.o -L. -ltlv

.o:%.c
	$(CC) $(CFLAGS) $< >$@

.PHONY: clean
clean:
	@rm -f *.so $(TARGET_BIN) *.o
