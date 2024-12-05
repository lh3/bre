CC=			gcc
CFLAGS=		-g -Wall -Wc++-compat -std=c99 -O3
CPPFLAGS=
INCLUDES=
PROG=		examples/toy examples/dna6print
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

examples/dna6print:examples/dna6print.c bre.c bre.h
		$(CC) $(CFLAGS) -I. $< bre.c -o $@ $(LIBS)

examples/toy:examples/toy.c bre.c bre.h
		$(CC) $(CFLAGS) -I. $< bre.c -o $@ $(LIBS)

bre.pdf:bre.tex
		pdflatex $<

clean:
		rm -fr *.o examples/*.o a.out $(PROG) *~ *.a examples/*.dSYM bre.pdf bre.log bre.aux test.bre

depend:
		(LC_ALL=C; export LC_ALL; makedepend -Y -- $(CFLAGS) $(CPPFLAGS) -- *.c)

# DO NOT DELETE

bre.o: bre.h
