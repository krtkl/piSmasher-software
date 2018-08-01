CC = arm-linux-gnueabihf-gcc
APPLICATION = audio-config
OBJS	+= i2c.o
OBJS	+= tlv320aic3104.o
OBJS	+= audio-config.o

all: $(OBJS)
	$(CC) -o $(APPLICATION) $(OBJS)

%.o: %.c
	$(CC) -c $< -o $@

.PHONY: clean
clean:
	-rm $(APPLICATION) $(OBJS)
