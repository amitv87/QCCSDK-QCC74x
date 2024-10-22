.PHONY: all clean

all: qcctl qcctld

CC ?= gcc
CFLAGS := -Wall -Wextra -O2 -g -pthread
QCCTL_INCLUDE_DIRS := -I./ -I../

QCCTL_OBJS := qcctl.o qcctl_ota.o utils.o qcctl_log.o msg_handlers.o

qcctl: $(QCCTL_OBJS) qcctl_main.o
	@$(info Linking $@)
	@$(CC) $(CFLAGS) $(QCCTL_INCLUDE_DIRS) -o $@ $^

qcctld: $(QCCTL_OBJS) qcctld_main.o
	@$(info Linking $@)
	@$(CC) $(CFLAGS) $(QCCTL_INCLUDE_DIRS) -o $@ $^

%.o: %.c
	@$(info CC $@)
	@$(CC) $(CFLAGS) $(QCCTL_INCLUDE_DIRS) -c -o $@ $<

clean:
	rm -f qcctl qcctld *.o
