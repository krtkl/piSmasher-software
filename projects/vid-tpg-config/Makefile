CC = arm-linux-gnueabihf-gcc
APPLICATION = vid-tpg-config
OBJS	+= vid_tpg.o
OBJS	+= vid-tpg-config.o

all: $(OBJS)
	$(CC) -o $(APPLICATION) $(OBJS)

%.o: %.c
	$(CC) -c $< -o $@

.PHONY: clean
clean:
	-rm $(APPLICATION) $(OBJS)
