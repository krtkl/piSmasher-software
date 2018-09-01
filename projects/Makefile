export CC = arm-linux-gnueabihf-gcc

export OK_COLOR    = \033[0;32m
export ERROR_COLOR = \033[0;31m
export WARN_COLOR  = \033[0;33m
export NO_COLOR    = \033[m

targets		+= video-config
targets		+= hdmi-config
targets		+= hdmi-edid
targets		+= audio-config
targets		+= vid-tpg-config
targets		+= uio-vdma
targets		+= uio-vtc
targets		+= uio-clk-wiz

export BUILDDIR	= build
export LIBDIR = lib

MAKEFLAGS	+= --no-print-directory

all: $(targets)

clean: $(targets)

$(targets):
	@$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all clean $(targets)
