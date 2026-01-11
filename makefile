.RECIPEPREFIX = >

APP_NAME = Zxc-terminal

CC = gcc
CFLAGS = -Wall -Wextra -g `pkg-config --cflags gtk+-3.0 vte-2.91`
LIBS = `pkg-config --libs gtk+-3.0 vte-2.91`

SRC = main.c
OBJ = $(SRC:.c=.o)

all: $(APP_NAME)

$(APP_NAME): $(OBJ)
> $(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
> $(CC) $(CFLAGS) -c $< -o $@

clean:
> rm -f $(OBJ) $(APP_NAME)

.PHONY: all clean

