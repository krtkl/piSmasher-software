target = hdmi-edid

DEPS	+= i2c.o
DEPS	+= tda998x.o
DEPS	+= tda1997x.o

OBJS	+= hdmi_tx.o
OBJS	+= hdmi_rx.o
OBJS	+= hdmi-edid.o
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
