# ==============================
#  Clang-RPG-Game Makefile
# ==============================
# make の既定の CC は "cc" なので、指定がなければ gcc を使う
ifeq ($(origin CC),default)
  CC := gcc
endif

# 最適化レベルなど、コマンドラインから変えてよいもの（例: make CFLAGS="-O0 -g"）
CFLAGS ?= -O2
# 常に付けるもの（警告・ハードニング）
BASE_FLAGS := -std=c99 -Iinclude \
              -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 \
              -fstack-protector-strong -D_FORTIFY_SOURCE=2
# CI では make WERROR=1 で警告をエラー扱いにする
ifeq ($(WERROR),1)
  BASE_FLAGS += -Werror
endif

ifeq ($(OS),Windows_NT)
  EXE := .exe
else
  EXE :=
endif

SRC       := $(wildcard src/*.c)
# main.c 以外 = テストやファジングからも使う部分
LIB_SRC   := $(filter-out src/main.c,$(SRC))
TARGET    := rpg$(EXE)
TEST_BIN  := test_runner$(EXE)
FUZZ_BIN  := fuzz_load$(EXE)

.PHONY: all run test fuzz lint clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(BASE_FLAGS) $(CFLAGS) $^ -o $@

run: $(TARGET)
	./$(TARGET)

# ユニットテスト：AddressSanitizer / UndefinedBehaviorSanitizer 付き（Linux・WSL 用）
test: $(LIB_SRC) tests/test_main.c
	$(CC) -std=c99 -Iinclude -g -O1 -Wall -Wextra \
	    -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer \
	    $^ -o $(TEST_BIN)
	./$(TEST_BIN)

# ファジング：clang が必要（Linux・WSL 用）
FUZZ_TIME ?= 60
fuzz: $(LIB_SRC) tests/fuzz_load.c tests/gen_seed.c
	clang -std=c99 -Iinclude -g -O1 -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION \
	    -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=all \
	    $(LIB_SRC) tests/fuzz_load.c -o $(FUZZ_BIN)
	clang -std=c99 -Iinclude $(LIB_SRC) tests/gen_seed.c -o gen_seed$(EXE)
	mkdir -p fuzz_corpus
	./gen_seed$(EXE) fuzz_corpus/seed.dat
	./$(FUZZ_BIN) fuzz_corpus -max_total_time=$(FUZZ_TIME)

# 静的解析
lint:
	cppcheck --enable=warning,style,performance,portability --std=c99 \
	    --error-exitcode=1 --inline-suppr -Iinclude --suppress=missingIncludeSystem src tests

clean:
	rm -f rpg rpg.exe test_runner test_runner.exe fuzz_load fuzz_load.exe gen_seed gen_seed.exe save.dat save.dat.tmp
	rm -rf fuzz_corpus
