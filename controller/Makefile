Q_ = $(if $(V),,@)
CC = $(CROSS_COMPILE)gcc
CFLAGS = -MMD `xml2-config --cflags`
LDFLAGS =
INCS =
LIBS = -lpthread -llirc_client -lxml2 -lsqlite3 -lpython2.7
OBJS = main.o ipc.o irremote.o database.o messenger.o epg.o
TARGET = apollo_controller
ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

EPG2XML = ../external/epg2xml/epg2xml.py
EPG2XML_DUMP = epg2xml.h

.PHONY : all clean install uninstall

all: $(TARGET)

clean:
	$(Q_)rm -f *.d $(EPG2XML_DUMP) $(OBJS) $(TARGET)
	@echo "---- cleaned"

install: $(TARGET)
	@echo "---- install $(notdir $(TARGET))"
	$(Q_)install -d $(DESTDIR)$(PREFIX)/bin/
	$(Q_)install $(TARGET) $(DESTDIR)$(PREFIX)/bin/

uninstall:
	$(Q_)rm -f $(DESTDIR)$(PREFIX)/bin/$(notdir $(TARGET))
	@echo "---- uninstalled"

-include *.d

$(EPG2XML_DUMP):
	@echo "---- dump $(notdir $@)"
	$(Q_)xxd -i -a $(EPG2XML) > $@

%.o: %.c
	@echo "---- compile $(notdir $@)"
	$(Q_)$(CC) -o $@ $< -c $(CFLAGS) $(INCS)

$(TARGET): $(EPG2XML_DUMP) $(OBJS)
	@echo "---- build $(notdir $@)"
	$(Q_)$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
