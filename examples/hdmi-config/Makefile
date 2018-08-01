CC = arm-linux-gnueabihf-gcc
CFLAGS := -g -Wall
RM := rm -rf
TARGET := hdmi-config
OBJS := hdmi-config.o
LIBS :=

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM v7 Linux gcc linker'
	$(CC) -o '$(TARGET)' $(OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

clean:
	-$(RM) $(TARGET)$(OBJS)
	-@echo ' '

.PHONY: all clean
.SECONDARY:
