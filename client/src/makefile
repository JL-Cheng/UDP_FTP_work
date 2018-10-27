CC := gcc
CFLAGS := -Wall -g -Os

OBJS = client.o

client : $(OBJS)
	$(CC) -o client $(CFLAGS) $(OBJS)

$(OBJS) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	@rm -rf *.o client	
