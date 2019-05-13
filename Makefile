.POSIX:

BIN = ersatz
OBJ = main.o solver.o dimacs.o utils.o
CFLAGS += -D_POSIX_C_SOURCE=2 -ansi -W -Wall -Wextra -Wpedantic

VERSION = $(shell git describe --tags --always)\ \($(shell date -I)\)
CFLAGS += -DVERSION=\"$(VERSION)\"

.PHONY: all debug test clean

all: $(BIN)

# If `prove` is not available, use the following instead:
#test:
#	! ./test.sh ./$(BIN) | grep "not ok"
test:
	prove -f test.sh :: ./$(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LIBS) -o $@

clean:
	rm -f $(BIN) $(OBJ)

%.o: src/%.c
	$(CC) $(CFLAGS) -c $<
