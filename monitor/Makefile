Q_ = $(if $(V),,@)
CC = $(CROSS_COMPILE)gcc
CFLAGS = -MMD
LDFLAGS =
INCS =
LIBS = -lavformat -lavcodec -lavutil -lswscale -lswresample -lebur128 -lasound \
       -lpthread
OBJS = main.o video.o audio.o analyzer.o logger.o recorder.o streamer.o fifo.o \
       ipc.o
TARGET = apollo_monitor
ifeq ($(PREFIX),)
	PREFIX = /usr/local
endif

.PHONY : all clean install uninstall

all: $(TARGET)

clean:
	$(Q_)rm -f *.d $(OBJS) $(TARGET)
	@echo "---- cleaned"

install: $(TARGET)
	@echo "---- install $(notdir $(TARGET))"
	$(Q_)install -d $(DESTDIR)$(PREFIX)/bin/
	$(Q_)install $(TARGET) $(DESTDIR)$(PREFIX)/bin/

uninstall:
	$(Q_)rm -f $(DESTDIR)$(PREFIX)/bin/$(notdir $(TARGET))
	@echo "---- uninstalled"

-include *.d

%.o: %.c
	@echo "---- compile $(notdir $@)"
	$(Q_)$(CC) -o $@ $< -c $(CFLAGS) $(INCS)

$(TARGET): $(OBJS)
	@echo "---- build $(notdir $@)"
	$(Q_)$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
