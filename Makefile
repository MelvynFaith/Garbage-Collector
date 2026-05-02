# ============================================================================
# Makefile — SnakeLang GC Project
#
# Targets:
#   make          → build the test binary (default)
#   make run      → build and run
#   make valgrind → build and run under Valgrind (leak check)
#   make clean    → remove build artefacts
# ============================================================================

CC      = gcc
# -Wall -Wextra:    catch common mistakes
# -Wpedantic:       enforce strict ISO C
# -g:               include DWARF debug info for GDB / Valgrind
# -fsanitize=...:   enable AddressSanitizer + UBSanitizer (clang/gcc ≥ 4.8)
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g

TARGET  = snakelang

SRCS    = main.c        \
          stack.c       \
          snake_object.c \
          frame.c       \
          vm.c          \
          gc.c

OBJS    = $(SRCS:.c=.o)

# --------------------------------------------------------------------------
# Default target
# --------------------------------------------------------------------------
.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# --------------------------------------------------------------------------
# Run
# --------------------------------------------------------------------------
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# --------------------------------------------------------------------------
# Valgrind — definitive memory leak check
# Requires: sudo apt install valgrind  (or equivalent)
# --------------------------------------------------------------------------
.PHONY: valgrind
valgrind: $(TARGET)
	valgrind \
	  --leak-check=full \
	  --show-leak-kinds=all \
	  --track-origins=yes \
	  --error-exitcode=1 \
	  ./$(TARGET)

# --------------------------------------------------------------------------
# AddressSanitizer build (alternative to Valgrind, faster)
# --------------------------------------------------------------------------
.PHONY: asan
asan: CFLAGS += -fsanitize=address,undefined
asan: clean $(TARGET)
	./$(TARGET)

# --------------------------------------------------------------------------
# Clean
# --------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)