
SHELL = /bin/sh
SRCDIR = ./src
OBJDIR = ./obj
INCLUDEDIR = ./include
BINDIR = ./bin
CFLAGS = -Wall -std=c99 -o2 -I $(INCLUDEDIR)


.SUFFIXES:
.SUFFIXES: .c .o
.PHONY: clean

$(BINDIR)/interpreter: $(OBJDIR)/interpreter.o $(OBJDIR)/lexeme.o $(OBJDIR)/lex.o $(OBJDIR)/darray.o \
 $(OBJDIR)/bigint.o $(OBJDIR)/bigfloat.o $(OBJDIR)/eval.o $(OBJDIR)/environment.o $(OBJDIR)/builtins.o \
 $(OBJDIR)/parser.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/bigfloat.o: $(SRCDIR)/bigfloat.c $(INCLUDEDIR)/bigfloat.h $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<
$(OBJDIR)/bigint.o: $(SRCDIR)/bigint.c $(INCLUDEDIR)/bigint.h 
	$(CC) $(CFLAGS) -c -o $@ $<
$(OBJDIR)/builtins.o: $(SRCDIR)/builtins.c $(INCLUDEDIR)/builtins.h $(INCLUDEDIR)/environment.h \
 $(INCLUDEDIR)/lexeme.h $(INCLUDEDIR)/bigint.h $(INCLUDEDIR)/bigfloat.h $(INCLUDEDIR)/eval.h 
	$(CC) $(CFLAGS) -c -o $@ $<
$(OBJDIR)/darray.o: $(SRCDIR)/darray.c $(INCLUDEDIR)/darray.h
	$(CC) $(CFLAGS) -c -o $@ $<
$(OBJDIR)/environment.o: $(SRCDIR)/environment.c $(INCLUDEDIR)/environment.h $(INCLUDEDIR)/lexeme.h \
 $(INCLUDEDIR)/bigint.h 
	$(CC) $(CFLAGS) -c -o $@ $<
$(OBJDIR)/eval.o: $(SRCDIR)/eval.c $(INCLUDEDIR)/eval.h $(INCLUDEDIR)/lexeme.h $(INCLUDEDIR)/bigint.h \
 $(INCLUDEDIR)/environment.h $(INCLUDEDIR)/prettyprinter.h $(INCLUDEDIR)/builtins.h 
	$(CC) $(CFLAGS) -c -o $@ $<
$(OBJDIR)/interpreter.o: $(SRCDIR)/interpreter.c $(INCLUDEDIR)/eval.h $(INCLUDEDIR)/lexeme.h \
 $(INCLUDEDIR)/bigint.h $(INCLUDEDIR)/parser.h $(INCLUDEDIR)/lex.h $(INCLUDEDIR)/environment.h \
 $(INCLUDEDIR)/prettyprinter.h 
	$(CC) $(CFLAGS) -c -o $@ $<
$(OBJDIR)/lexeme.o: $(SRCDIR)/lexeme.c $(INCLUDEDIR)/lexeme.h $(INCLUDEDIR)/bigint.h \
 $(INCLUDEDIR)/bigfloat.h 
	$(CC) $(CFLAGS) -c -o $@ $<
$(OBJDIR)/parser.o: $(SRCDIR)/parser.c $(INCLUDEDIR)/lex.h $(INCLUDEDIR)/lexeme.h $(INCLUDEDIR)/bigint.h \
 $(INCLUDEDIR)/parser.h 
	$(CC) $(CFLAGS) -c -o $@ $<
$(OBJDIR)/prettyprinter.o: $(SRCDIR)/prettyprinter.c $(INCLUDEDIR)/lex.h $(INCLUDEDIR)/lexeme.h \
 $(INCLUDEDIR)/bigint.h $(INCLUDEDIR)/bigfloat.h $(INCLUDEDIR)/prettyprinter.h 
	$(CC) $(CFLAGS) -c -o $@ $<
$(OBJDIR)/scanner.o: $(SRCDIR)/scanner.c $(INCLUDEDIR)/lex.h $(INCLUDEDIR)/lexeme.h $(INCLUDEDIR)/bigint.h \
 $(INCLUDEDIR)/bigfloat.h 
	$(CC) $(CFLAGS) -c -o $@ $<
$(OBJDIR)/lex.o: $(SRCDIR)/lex.c $(INCLUDEDIR)/lex.h $(INCLUDEDIR)/lexeme.h $(INCLUDEDIR)/bigint.h \
 $(INCLUDEDIR)/darray.h $(INCLUDEDIR)/bigfloat.h
	$(CC) $(CFLAGS) -c -o $@ $<
clean:
	$(RM) $(OBJDIR)/*.o $(BINDIR)/*
