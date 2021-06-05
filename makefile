CC=gcc
DEPS=abstract-tree symbols-table symbol hashtable translation translation_utils sem-err lex.yy
OBJ=$(DEPS:%=obj/%.o)
CFLAGS=-Wall -ly -lfl -pedantic -g
EXEC=tpcc
PARSER=parser
LEXER=lexer

all : makedirs bin/$(EXEC)

makedirs:
	@mkdir -p bin
	@mkdir -p obj

bin/$(EXEC): src/$(PARSER).tab.c $(OBJ) 
	$(CC) src/$(PARSER).tab.c $(OBJ) -o bin/$(EXEC)

obj/lex.yy.o: src/lex.yy.c
	$(CC) -c src/lex.yy.c -o obj/lex.yy.o $(CFLAGS)

src/lex.yy.c: src/$(LEXER).l
	flex -o $@ src/$(LEXER).l

obj/%.o: src/%.c src/%.h
	$(CC) -c $< -o $@ $(CFLAGS)

src/$(PARSER).tab.c: src/$(PARSER).y
	bison -d src/$(PARSER).y
	@mv $(PARSER).tab.c src/
	@mv $(PARSER).tab.h src/

clean:
	rm -f src/lex.yy.* src/$(PARSER).tab.* $(PARSER).output obj/*

mrproper: clean
	rm -f testoutputs.txt bin/*