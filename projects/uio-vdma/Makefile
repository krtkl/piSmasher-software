CC = arm-linux-gnueabihf-gcc
APPLICATION = uio-vdma
OBJS	+= vdma.o
OBJS	+= uio-vdma.o

all: $(OBJS)
	$(CC) -o $(APPLICATION) $(OBJS)

%.o: %.c
	$(CC) -c $< -o $@

.PHONY: clean
clean:
	-rm $(APPLICATION) $(OBJS)
