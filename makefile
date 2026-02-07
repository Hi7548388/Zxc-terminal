.RECIPEPREFIX = >

APP_NAME = Zxc-terminal

CC = gcc
CFLAGS = -Wall -Wextra -g `pkg-config --cflags gtk+-3.0 vte-2.91`
LIBS = `pkg-config --libs gtk+-3.0 vte-2.91`

SRC = main.c
OBJ = $(SRC:.c=.o)

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

all: $(APP_NAME)

$(APP_NAME): $(OBJ)
> $(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
> $(CC) $(CFLAGS) -c $< -o $@

install: $(APP_NAME)
> sudo install -m 755 $(APP_NAME) $(BINDIR)/

uninstall:
> sudo rm -f $(BINDIR)/$(APP_NAME)

clean:
> rm -f $(OBJ) $(APP_NAME)

.PHONY: all clean install uninstall

