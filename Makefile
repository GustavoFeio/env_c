CC=cc

SRC=./src
MAIN=main

.PHONY: all clean
all: $(MAIN)

$(MAIN): $(SRC)/$(MAIN).c $(SRC)/env.h
	$(CC) -Wall -Wextra $< -o $@

clean:
	rm -frv $(MAIN)
