CC     = gcc
CFLAGS = -Wall -std=c11 -I./include
TARGET = ccriminal
SRC    = src/main.c src/jogo.c src/historico.c src/utils.c src/tui.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
