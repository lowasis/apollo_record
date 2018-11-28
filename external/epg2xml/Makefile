Q_ = $(if $(V),,@)
TARGET = epg2xml.py
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

.PHONY : install uninstall

install:
	@echo "---- install $(notdir $(TARGET))"
	$(Q_)install -d $(DESTDIR)$(PREFIX)/bin/
	$(Q_)install $(TARGET) $(DESTDIR)$(PREFIX)/bin/

uninstall:
	$(Q_)rm -f $(DESTDIR)$(PREFIX)/bin/$(notdir $(TARGET))
	@echo "---- uninstalled"
