CC=gcc
CFLAGS=-O3 -I. -Wall
BIN=clerr
DEST=/usr/local/bin/clerr
CHOWN=root:root
SRC=src/clerr.c

MAN=man/clerr.1
MANDEST=/usr/local/share/man/man1/


default:
	$(CC) $(CFLAGS) -o $(BIN) -DPROGNAME='"$(BIN)"' $(SRC)

install:
	strip $(BIN)
	cp $(BIN) $(DEST)
	chown $(CHOWN) $(DEST)
	
	mkdir -p $(MANDEST)
	cp $(MAN) $(MANDEST)
	chown $(CHOWN) $(MANDEST)/$(MAN)
	chmod 0644 $(MANDEST)/$(MAN)

complete: clean default install

clean:
	rm -f $(BIN) *.so *.o *.a *~ *.bak
