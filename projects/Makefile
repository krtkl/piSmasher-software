proj	+= hdmi-config/
proj	+= hdmi-edid/
proj	+= audio-config/
proj	+= vid-tpg-config/
proj	+= uio-vdma/
proj	+= uio-vtc/

all: $(proj)

clean: $(proj)

$(proj):
	+$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all clean $(proj)
