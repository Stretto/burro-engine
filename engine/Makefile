# Makefile -- the makefile for the libx

# Copyright 2015, Michael L. Gran

# This file is part of the Project Burro game engine.

# Project Burro is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.

# Project Burro is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Project Burro.  If not, see
# <http://www.gnu.org/licenses/>. */

SHELL = /bin/sh

srcdir = .

CC       = gcc --std=c11 -fdiagnostics-color=auto -fmessage-length=78
CXX      = gcc --std=c++11 -fdiagnostics-color=auto -fmessage-length=78

################################################################
# FLAGS

# `pkg-config cairo gtk+-3.0 gdk-3.0 guile-2.0 libpulse --cflags`

PKG_CFLAGS_32 := -I/usr/lib/dbus-1.0/include -I/usr/lib/glib-2.0/include
PKG_CFLAGS_64 := -I/usr/lib64/dbus-1.0/include -I/usr/lib64/glib-2.0/include

PKG_CFLAGS := -pthread -D_REENTRANT \
	-I/usr/include/at-spi-2.0 \
	-I/usr/include/at-spi2-atk/2.0 \
	-I/usr/include/atk-1.0 \
	-I/usr/include/cairo \
	-I/usr/include/dbus-1.0 \
	-I/usr/include/freetype2 \
	-I/usr/include/gdk-pixbuf-2.0 \
	-I/usr/include/gio-unix-2.0/ \
	-I/usr/include/glib-2.0 \
	-I/usr/include/gtk-3.0 \
	-I/usr/include/guile/2.0 \
	-I/usr/include/harfbuzz \
	-I/usr/include/libdrm \
	-I/usr/include/libpng16 \
	-I/usr/include/pango-1.0 \
	-I/usr/include/pixman-1 \
    $(PKG_CFLAGS_$(strip $(shell getconf LONG_BIT)))

CPPFLAGS = \
 -Wall \
 -Wunused-macros \
 -Wendif-labels \
 -Wunknown-pragmas \
  $(PKG_CFLAGS) \
 -I .

CDEBUG   = -g3 -O0

CWARN = \
 -Wall \
 -Wpointer-arith \
 -Wformat-nonliteral \
 -Winit-self \
 -Wignored-qualifiers \
 -Wwrite-strings \
 -Wconversion \
 -Wpointer-arith \
 -Warray-bounds

    # -Wcast-align \
    # -Wmissing-declarations \
    # -Wpointer-arith \
    # -Warray-bounds -ftree-vrp \
    # -Wstrict-aliasing -fstrict-aliasing \
    # -Wstrict-overflow -fstrict-overflow \
    # -Wunreachable-code \
    # -Winvalid-pch \
	#    -Wundef \

CXXWARN = $(CWARN) -Wsign-promo -Weffc++

CFLAGS   = $(CPPFLAGS) $(CWARN) $(CDEBUG) $(DEFS) -fvisibility=hidden

CXXFLAGS = $(CFLAGS) $(CXXWARN) $(CDEBUG) $(DEFS)

LDFLAGS  = -Xlinker --Map=burro.map \
	-Wl,--gc-sections -Wl,--as-needed -Wl,--demangle

LIBS = \
 -latk-1.0 \
 -lcairo \
 -lcairo-gobject \
 -lgc \
 -lgdk-3 \
 -lgdk_pixbuf-2.0 \
 -lgio-2.0 \
 -lglib-2.0 \
 -lgobject-2.0 \
 -lgtk-3 \
 -lguile-2.0 \
 -lm \
 -lpango-1.0 \
 -lpangocairo-1.0 \
 -lpulse \
 -lpulse-mainloop-glib

# LIBS = \
#  `pkg-config cairo gtk+-3.0 gdk-3.0 guile-2.0 glib-2.0 libpulse --libs` \
#  -lm


################################################################
# TOP-LEVEL SOURCES

SRCS_X_H = \
 x/xcairo.h \
 x/xgdk.h \
 x/xgdk-pixbuf.h \
 x/xgio.h \
 x/xgtk.h \
 x/xguile.h \
 x/xpulseaudio.h

SRCS_Y_H = \
 y/audio_model.h \
 y/backdrop.h \
 y/bg.h \
 y/console.h \
 y/draw.h \
 y/ecma48.h \
 y/eng.h \
 y/guile.h \
 y/lineedit.h \
 y/lisp.h \
 y/loop.h \
 y/matrix.h \
 y/obj.h \
 y/proc.h \
 y/pulseaudio.h \
 y/rand.h \
 y/repl.h \
 y/sheet.h \
 y/vram.h \
 y/wave.h

SRCS_C   = \
 x/xcairo.c \
 x/xgdk.c \
 x/xgdk-pixbuf.c \
 x/xgio.c \
 x/xglib.c \
 x/xgtk.c \
 x/xguile.c \
 x/xpulseaudio.c \
 y/audio_model.c \
 y/backdrop.c \
 y/bg.c \
 y/console.c \
 y/draw.c \
 y/ecma48.c \
 y/eng.c \
 y/guile.c \
 y/lineedit.c \
 y/lisp.c \
 y/loop.c \
 y/matrix.c \
 y/obj.c \
 y/pulseaudio.c \
 y/rand.c \
 y/repl.c \
 y/sheet.c \
 y/vram.c \
 y/wave.c \
 main.c

################################################################
# ALL

.PHONY:  all
all: burro

################################################################
# GENERATED SOURCES


################################################################
# OBJECTS

OBJS     = \
	$(SRCS_C:.c=.o) \
	$(SRCS_CPP:.cpp=.o)

DEPENDS = ${OBJS:.o=.d}
-include ${DEPENDS}

# %.o : %.c
# 	$(CC) $(CFLAGS) -c $*.c -o $*.o
# 	$(CC) $(CPPFLAGS) -M $*.c > $*.d

# %.x : %.c
# 	guile-snarf -o $*.x $*.c $(CPPFLAGS)

%.o : %.c
	guile-snarf -o $*.x $*.c $(CPPFLAGS)
	$(CC) $(CPPFLAGS) -MM $*.c > $*.d
	$(CC) $(CFLAGS) -c $*.c -o $*.o

%.o : %.cpp
	$(CXX) $(CPPFLAGS) -MM $*.cpp > $*.d
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $*.o

################################################################
# MAJOR TARGETS

burro: $(OBJS) x.pch y.pch
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

x.pch: x.h $(SRCS_X_H)
	(CC) -o $@ $(CFLAGS) $<

y.pch: y.h $(SRCS_Y_H)
	$(CC) -o $@ $(CFLAGS) $<

################################################################
# MINOR TARGETS

.PHONY:  clean
clean:
	-rm -f $(OBJS) $(DEPENDS) bdf-convert burro burro.iso core

.PHONY: check-syntax
check-syntax:
	$(CC) $(CFLAGS) -fsyntax-only $(SRCS_C)

TAGS: $(SRCS_C) $(SRCS_CPP)
	etags $(SRCS_C) $(SRCS_CPP)

################################################################
# GENERATED SOURCES

# sprites.png sprites.jsn: $(SPRITES_ASE)
# 	 aseprite --batch --sheet sprites.png --data sprites.jsn $(SPRITES_ASE)

################################################################
# OBJECTS

# burro.iso: $(SRCS_JS) $(MAPS_TMX) $(TILESHEETS_PNG) sprites.png sprites.jsn
# 	genisoimage -o $@ $^


################################################################

# VERSION = 0.0
# distdir = $(PACKAGE)-$(VERSION)
# tar --format=posix --create --gzip --verbose --sort=name --file=$(distdir).tar.gz --transform 's,^,$(distdir),' $(DISTFILES)

