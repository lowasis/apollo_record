Q_ = $(if $(V),,@)
APOLLO_MONITOR_DIR = ./monitor/
APOLLO_CONTROLLER_DIR = ./controller/

.PHONY : all clean install uninstall

all:
	$(Q_)make -C $(APOLLO_MONITOR_DIR) all
	$(Q_)make -C $(APOLLO_CONTROLLER_DIR) all

clean:
	$(Q_)make -C $(APOLLO_MONITOR_DIR) clean
	$(Q_)make -C $(APOLLO_CONTROLLER_DIR) clean

install: all
	$(Q_)make -C $(APOLLO_MONITOR_DIR) install
	$(Q_)make -C $(APOLLO_CONTROLLER_DIR) install

uninstall:
	$(Q_)make -C $(APOLLO_MONITOR_DIR) uninstall
	$(Q_)make -C $(APOLLO_CONTROLLER_DIR) uninstall
