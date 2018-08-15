CC = arm-linux-gnueabihf-gcc
APPLICATION = uio-clk-wiz
OBJS	+= clk_wiz.o
OBJS	+= uio-clk-wiz.o

all: $(OBJS)
	$(CC) -o $(APPLICATION) $(OBJS)

%.o: %.c
	$(CC) -c $< -o $@

.PHONY: clean
clean:
	-rm $(APPLICATION) $(OBJS)
