CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -std=c99
SRC = $(wildcard src/*.c)
OUT = rpg_game

all: $(OUT)

$(OUT):
	$(CC) $(SRC) $(CFLAGS) -o $(OUT)

debug:
	$(CC) $(SRC) $(CFLAGS) -g -o $(OUT)

asan:
	$(CC) $(SRC) $(CFLAGS) -fsanitize=address -g -o $(OUT)

clean:
	rm -f $(OUT)

test:
	gcc tests/*.c src/*.c -Iinclude -o test_runner
	./test_runner
