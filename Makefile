CC=cc
# CC=clang
NAME=minesweep
OUT=bin/$(NAME)
OUT_DEBUG=bin/$(NAME)-debug
CFLAGS=-Wall -Wextra -pedantic -DPROJECT_DIR='"$(shell pwd)"'
CLIBS= -lncurses -lm

sources=main.c

.PHONY: default debug clean count run

default: $(OUT)

clean:
	-rm -Rf obj dep bin

run: $(OUT)
	./$(OUT) $(ARGS)

debug: CFLAGS+=-g
debug: OUT=$(OUT_DEBUG)
debug:  | obj dep bin
	$(CC) $(CFLAGS) $(addprefix src/, $(sources)) -o $(OUT) $(CLIBS)
	gdb $(OUT)

count:
	cloc src/*

test: clean | dep obj bin
	$(CC) $(CFLAGS) $(addprefix src/, $(filter-out main.c, $(sources))) src/test.c -o bin/test $(CLIBS)
	./bin/test

ifeq (,$(filter $(MAKECMDGOALS), clean count test))
include $(addprefix dep/, $(sources:.c=.d))
endif

dep/%.d: src/%.c | dep
	$(CC) $(CFLAGS) -M $^ > $@

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c -o $@ $(realpath $<)

obj dep bin:
	mkdir -p $@

$(OUT): $(addprefix obj/, $(sources:.c=.o)) | bin
	$(CC) $(CFLAGS) -o $@ $^ $(CLIBS)
