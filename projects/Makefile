proj	+= hdmi-config/
proj	+= audio-config/
proj	+= vid-tpg-config/
proj	+= uio-vdma/


all: $(proj)

clean: $(proj)

$(proj):
	+$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all clean $(proj)
