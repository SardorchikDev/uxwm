# uxwm Makefile
VERSION = 1.0

PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin
MANDIR  = $(PREFIX)/share/man/man1
SCRIPTS = scripts/ux-session scripts/uxmenu_run

CC      = cc
INCS    = -I/usr/include -I/usr/include/freetype2
LIBS    = -L/usr/lib -lX11 -lXinerama -lXft -lfontconfig

CFLAGS  = -std=c99 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -pedantic -Wall -Wextra -Wno-unused-parameter \
          -Os -DXINERAMA $(INCS)
LDFLAGS = $(LIBS)

SRC = wm/uxwm.c wm/x11.c wm/wm.c wm/input.c wm/layout.c wm/drw.c wm/util.c
OBJ = $(SRC:.c=.o)

all: uxwm

uxwm: $(OBJ)
	@echo "  LD   uxwm"
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	@echo "  CC   $<"
	@$(CC) -c $(CFLAGS) -o $@ $<

# ensure config.h is found when compiling wm.c
wm/wm.o: wm/wm.c wm/wm.h wm/config.h
	@echo "  CC   $<"
	@$(CC) -c $(CFLAGS) -Iwm -o $@ $<

wm/x11.o: wm/x11.c wm/wm.h
	@echo "  CC   $<"
	@$(CC) -c $(CFLAGS) -Iwm -o $@ $<

wm/input.o: wm/input.c wm/wm.h
	@echo "  CC   $<"
	@$(CC) -c $(CFLAGS) -Iwm -o $@ $<

wm/layout.o: wm/layout.c wm/wm.h
	@echo "  CC   $<"
	@$(CC) -c $(CFLAGS) -Iwm -o $@ $<

wm/uxwm.o: wm/uxwm.c wm/wm.h
	@echo "  CC   $<"
	@$(CC) -c $(CFLAGS) -Iwm -o $@ $<

wm/drw.o: wm/drw.c wm/drw.h wm/util.h
	@echo "  CC   $<"
	@$(CC) -c $(CFLAGS) -Iwm -o $@ $<

wm/util.o: wm/util.c wm/util.h
	@echo "  CC   $<"
	@$(CC) -c $(CFLAGS) -Iwm -o $@ $<

clean:
	@echo "  CLEAN"
	@rm -f uxwm wm/*.o

install: uxwm
	@echo "  INSTALL $(BINDIR)/uxwm"
	@mkdir -p $(DESTDIR)$(BINDIR)
	@cp -f uxwm $(SCRIPTS) $(DESTDIR)$(BINDIR)
	@chmod 755 $(DESTDIR)$(BINDIR)/uxwm $(DESTDIR)$(BINDIR)/ux-session $(DESTDIR)$(BINDIR)/uxmenu_run

uninstall:
	@rm -f $(DESTDIR)$(BINDIR)/uxwm $(DESTDIR)$(BINDIR)/ux-session $(DESTDIR)$(BINDIR)/uxmenu_run

.PHONY: all clean install uninstall
