OBJS = server.o
OBJS1 = client.o
CC ?= gcc  
all: $(OBJS) $(OBJS1)
	$(CC) $(OBJS) -o server
	$(CC) $(OBJS1) -o client
clean:
	rm *.o server client
