CC=gcc
OBJ=abstract-tree.o symbols-table.o symbol.o hashtable.o translation.o translation_utils.o lex.yy.o
CFLAGS=-Wall -ly -lfl -pedantic -g
EXEC=tpcc
PARSER=parser
LEXER=lexer

all : $(EXEC)

$(EXEC): $(PARSER).tab.c $(OBJ) 
	$(CC) src/$(PARSER).tab.c $(OBJ:%.o=obj/%.o) -o bin/$(EXEC)

lex.yy.o: lex.yy.c
	$(CC) -c src/lex.yy.c -o obj/lex.yy.o $(CFLAGS)

lex.yy.c: src/$(LEXER).l
	flex -o src/$@ src/$(LEXER).l

%.o: src/%.c src/%.h
	$(CC) -c $< -o obj/$@ $(CFLAGS)

$(PARSER).tab.c: src/$(PARSER).y
	bison -d src/$(PARSER).y
	@mv $(PARSER).tab.c src/
	@mv $(PARSER).tab.h src/

clean:
	rm -f src/lex.yy.* src/$(PARSER).tab.* $(PARSER).output obj/*

mrproper: clean
	rm -f testoutputs.txt bin/*