CC = gcc
CFLAGS = -lpthread -g
OBJECTS = pc.c

producer_consumer.exe : $(OBJECTS)
	$(CC) $(OBJECTS) -o producer_consumer.exe $(CFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<
