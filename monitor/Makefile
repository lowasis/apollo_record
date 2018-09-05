Q_ = $(if $(V),,@)
CC = $(CROSS_COMPILE)gcc
CFLAGS = -MMD
LDFLAGS =
INCS =
LIBS = -lebur128 -lasound
OBJS = main.o video.o audio.o analyzer.o
TARGET = apollo_monitor

.PHONY : all clean

all: $(TARGET)

clean:
	$(Q_)rm -f *.d $(OBJS) $(TARGET)
	@echo "---- cleaned"

-include *.d

%.o: %.c
	@echo "---- compile $(notdir $@)"
	$(Q_)$(CC) -o $@ $< -c $(CFLAGS) $(INCS)

$(TARGET): $(OBJS)
	@echo "---- build $(notdir $@)"
	$(Q_)$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
