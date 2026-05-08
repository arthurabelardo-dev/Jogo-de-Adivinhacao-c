CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -I./include
LDFLAGS =
TARGET  = ccriminal
TEST_TARGET = test_pistas
SRC     = src/main.c src/jogo.c src/historico.c src/utils.c src/pistas.c
TEST_SRC = tests/test_pistas.c src/pistas.c src/utils.c

.PHONY: all clean run test

all: $(TARGET)
	@echo "✓ Compilacao concluida: $(TARGET)"

$(TARGET):
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

test: $(TEST_TARGET)
	@echo "✓ Executando testes..."
	./$(TEST_TARGET)

$(TEST_TARGET):
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

run: $(TARGET)
	./$(TARGET)
