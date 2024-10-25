CC=			gcc
CFLAGS=		-g -Wall -Wc++-compat -O3
CPPFLAGS=
INCLUDES=
PROG=		example
LIBS=

ifneq ($(asan),)
	CFLAGS+=-fsanitize=address
	LIBS+=-fsanitize=address -ldl
endif

.SUFFIXES:.c .o
.PHONY:all clean depend

.c.o:
		$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) $< -o $@

all:$(PROG)

example:bre.o example.o
		$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

bre.pdf:bre.tex
		pdflatex $<

clean:
		rm -fr *.o a.out $(PROG) *~ *.a *.dSYM bre.pdf bre.log bre.aux test.bre

depend:
		(LC_ALL=C; export LC_ALL; makedepend -Y -- $(CFLAGS) $(CPPFLAGS) -- *.c)

# DO NOT DELETE

bre.o: bre.h
example.o: bre.h
