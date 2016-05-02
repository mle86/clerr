CFLAGS=-O3 -I. -Wall -Wextra -pedantic -Wno-unused-result -std=c99 -D_POSIX_C_SOURCE=199309L
BIN=clerr
DEST=/usr/local/bin/clerr
CHOWN=root:root
SRC=src/clerr.c

MAN=man/clerr.1
MANDEST=/usr/local/share/man/man1/


default: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ -DPROGNAME='"$(BIN)"' $^

install:
	strip $(BIN)
	cp $(BIN) $(DEST)
	chown $(CHOWN) $(DEST)
	
	mkdir -p $(MANDEST)
	cp $(MAN) $(MANDEST)
	chown $(CHOWN) $(MANDEST)/$(MAN)
	chmod 0644 $(MANDEST)/$(MAN)

clean:
	rm -f $(BIN) *~

