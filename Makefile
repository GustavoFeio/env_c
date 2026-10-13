CC=cc

SRC=./src
MAIN=main

.PHONY: all clean
all: $(MAIN)

$(MAIN): $(SRC)/$(MAIN).c
	$(CC) -Wall -Wextra $< -o $@

clean:
	rm -frv $(MAIN)
