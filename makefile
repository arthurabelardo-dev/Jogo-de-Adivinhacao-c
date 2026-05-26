CC     = gcc
CFLAGS = -Wall -std=c11 -I./include
TARGET = ccriminal
SRC    = src/main.c src/jogo.c src/historico.c src/utils.c src/tui.c
TEST_TARGET = ccriminal_test
TEST_SRC = src/reputacao_test.c src/jogo.c src/historico.c src/utils.c src/tui.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

test:
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)
