.PHONY : default test install clean

CFLAGS=-O3 -I. -Wall -Wextra -pedantic -Wno-unused-result -std=c99 -D_POSIX_C_SOURCE=199309L
BIN=clerr
DEST=/usr/local/bin/clerr
CHOWN=root:root
SRC=src/clerr.c

MAN=clerr.1
MANDEST=/usr/local/share/man/man1/


default: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ -DPROGNAME='"$(BIN)"' $^

README.md: man/*
	perl man/to-readme.pl <man/clerr.1 >README.md

test:
	test/run-all-tests.sh

install:
	strip $(BIN)
	cp $(BIN) $(DEST)
	chown $(CHOWN) $(DEST)
	
	mkdir -p $(MANDEST)
	cp man/$(MAN) $(MANDEST)
	chown $(CHOWN) $(MANDEST)/$(MAN)
	chmod 0644 $(MANDEST)/$(MAN)
	gzip -f $(MANDEST)/$(MAN)

clean:
	rm -f $(BIN) *~ README.md

