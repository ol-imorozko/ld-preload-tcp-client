TARGET   = lib

CC       = gcc
CFLAGS   = -Wall -Wextra -fPIC

LINKER   = gcc
LFLAGS   = -shared -fPIC

SOURCES  := socket_cache.c syscalls.c ld_preload_tcp_client.o
INCLUDES := socket_cache.h syscalls.h
OBJECTS  := socket_cache.o syscalls.o ld_preload_tcp_client.o
rm       = rm -f

$(TARGET): $(OBJECTS)
	@$(LINKER) $(LFLAGS) -o ldtcp.so $(OBJECTS) -lpthread -ldl
	@echo "Created shared library"
	@echo "Linking library with test program"
	@gcc -o libsrv ./test/tcpsrv.c ./ldtcp.so
	@sudo setcap cap_sys_admin+ep ./libsrv

$(OBJECTS): %.o : %.c $(INCLUDES)
	@$(CC) $(CFLAGS) -c $< -o $@ #-DDEBUG_MODE
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!"
