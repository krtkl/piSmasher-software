CC = arm-linux-gnueabihf-gcc
APPLICATION = uio-vtc
OBJS	+= vtc.o
OBJS	+= uio-vtc.o

all: $(OBJS)
	$(CC) -o $(APPLICATION) $(OBJS)

%.o: %.c
	$(CC) -c $< -o $@

.PHONY: clean
clean:
	-rm $(APPLICATION) $(OBJS)
