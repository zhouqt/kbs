PYTHON?=python

PROG=big2gb
OBJS=main.o gb.mb.o big.mb.o s2t.mb.o t2s.mb.o v2z.mb.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)

gb.mb.c : mkgb.py mkunitab.py CP936.TXT
	$(PYTHON) mkgb.py

big.mb.c : mkbig.py mkunitab.py CP950.TXT
	$(PYTHON) mkbig.py

s2t.mb.c t2s.mb.c: mkstv.py Unihan.txt
	grep '^[^#]*k\(Simplified\|Traditional\)Variant' Unihan.txt |$(PYTHON) mkstv.py

v2z.mb.c: mkstv.py Unihan.txt
	grep '^[^#]*kZVariant' Unihan.txt |$(PYTHON) mkzv.py

Unihan.txt CP936.TXT CP950.TXT:
	@echo Please read README file before you build this software.
	@exit 1

clean:
	rm -f big2gb *.o *~ *.bak core *.core

clean-all: clean
	rm -f *.mb.c *.pyc
