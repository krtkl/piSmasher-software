CC = arm-linux-gnueabihf-gcc
APPLICATION = hdmi-edid
OBJS	+= i2c.o
OBJS	+= tda998x.o
OBJS	+= tda1997x.o
OBJS	+= hdmi_tx.o
OBJS	+= hdmi_rx.o
OBJS	+= hdmi-edid.o

all: $(OBJS)
	$(CC) -o $(APPLICATION) $(OBJS)

%.o: %.c
	$(CC) -c $< -o $@

.PHONY: clean
clean:
	-rm $(APPLICATION) $(OBJS)
