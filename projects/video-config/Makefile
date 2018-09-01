target = video-config

DEPS	+= i2c.o
DEPS	+= tda998x.o
DEPS	+= tda1997x.o
DEPS	+= vtc.o
DEPS	+= vid_tpg.o

OBJS	+= hdmi_tx.o
OBJS	+= hdmi_rx.o
OBJS	+= video-config.o
OBJS	+= $(addprefix ../$(LIBDIR)/, $(DEPS))

CFLAGS	+= $(addprefix -I ,../$(LIBDIR))

all: $(OBJS)
	@printf "APP $(target)\n"
	@$(CC) -o $(addprefix ../$(BUILDDIR)/, $(target)) $(OBJS)

%.o: %.c
	@printf "CC $(notdir $@)\n"
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	@rm -f $(addprefix ../$(BUILDDIR)/, $(target)) $(OBJS)
