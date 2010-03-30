COMPILER = g++
DFLAGS = -ansi -pedantic -Wall -g
CFLAGS = -c -O3
OFLAGS = -O3 -o 
EXECNAME = main 
SRCS = main.cpp 
OBJS = main.o 

suffixtree:	${OBJS}
	${COMPILER} ${DFLAGS} ${LFLAGS} ${OBJS} ${OFLAGS} ${EXECNAME}

.SUFFIXES: .cpp .o
.cpp.o:
	${COMPILER} ${DFLAGS} ${CFLAGS} $<

clean: 
	rm -f *.o 
	rm -f ${EXECNAME}

veryclean:	clean
	rm -f Makefile.bak

main.o: main.cpp suffixarray.h 

antal.o: mainantal.cpp suffixarray.h
