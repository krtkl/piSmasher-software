target = vid-tpg-config

DEPS	+= vid_tpg.o

OBJS	+= vid-tpg-config.o
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
