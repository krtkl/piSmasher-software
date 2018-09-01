target = uio-vtc

DEPS	+= vtc.o

OBJS	+= uio-vtc.o
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
