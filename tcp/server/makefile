CC := gcc
CFLAGS := -Wall -g -Os

OBJS = server.o

server : $(OBJS)
	$(CC) -o server $(CFLAGS) $(OBJS)

$(OBJS) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	@rm -rf *.o server
	@echo Done cleaning	
