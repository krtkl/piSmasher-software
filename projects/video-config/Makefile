CC = arm-linux-gnueabihf-gcc
APPLICATION = video-config
OBJS	+= i2c.o
OBJS	+= tda998x.o
OBJS	+= tda1997x.o
OBJS	+= hdmi_tx.o
OBJS	+= hdmi_rx.o
OBJS	+= vtc.o
OBJS	+= vid_tpg.o
OBJS	+= video-config.o

all: $(OBJS)
	$(CC) -o $(APPLICATION) $(OBJS)

%.o: %.c
	$(CC) -c $< -o $@

.PHONY: clean
clean:
	-rm $(APPLICATION) $(OBJS)
