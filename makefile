CC = gcc
CFLAGS = -Wall -pedantic -g -Isrc -I$(OBJDIR)
DEPFLAGS = -MMD -MP

EXEC = tpcc
PARSER = parser
LEXER = lexer
OBJDIR = obj
BINDIR = bin

SRCS = abstract-tree.c symbols-table.c symbol.c hashtable.c \
	translation.c translation_utils.c sem-err.c main.c

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
SRCS += arm64_emit.c
else
SRCS += elf64_emit.c
endif

OBJS = $(SRCS:%.c=$(OBJDIR)/%.o) \
	$(OBJDIR)/$(PARSER).tab.o \
	$(OBJDIR)/lex.yy.o

.PHONY: all clean fullclean
.DELETE_ON_ERROR:
.SUFFIXES:

all: $(BINDIR)/$(EXEC)

$(BINDIR)/$(EXEC): $(OBJS) | $(BINDIR)
	$(CC) $(OBJS) -o $@

$(SRCS:%.c=$(OBJDIR)/%.o): $(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJDIR)/main.o: $(OBJDIR)/$(PARSER).tab.c

$(OBJDIR)/$(PARSER).tab.o: $(OBJDIR)/$(PARSER).tab.c | $(OBJDIR)
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJDIR)/lex.yy.o: $(OBJDIR)/lex.yy.c $(OBJDIR)/$(PARSER).tab.c | $(OBJDIR)
	$(CC) $(DEPFLAGS) $(CFLAGS) -w -c $< -o $@

$(OBJDIR)/$(PARSER).tab.c: src/$(PARSER).y | $(OBJDIR)
	bison -d -o $@ $<

$(OBJDIR)/lex.yy.c: src/$(LEXER).l | $(OBJDIR)
	flex -o $@ $<

$(OBJDIR) $(BINDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR)
	rm -f src/lex.yy.* src/$(PARSER).tab.* $(PARSER).output

fullclean: clean
	rm -rf $(BINDIR)
	rm -f testoutputs.txt *.asm

-include $(OBJS:.o=.d)
