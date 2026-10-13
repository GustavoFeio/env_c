SRC=./src
MAIN=main

.PHONY: all clean
all: $(MAIN)

$(MAIN): $(SRC)/$(MAIN).c
	clang -Wall -Wextra $< -o $@

clean:
	rm -frv $(MAIN)
