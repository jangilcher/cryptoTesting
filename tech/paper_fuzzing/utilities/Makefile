CC=clang

CFLAGS=-ggdb -fsanitize=address

OBJDIR=obj
BINDIR=bin

LIBOQSDIR=../../../liboqs
LIBOQSFLAGS=-I $(LIBOQSDIR)/build/include -L $(LIBOQSDIR)/build/lib -loqs -lcrypto

# Objects

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Executables

$(BINDIR)/fmt_str_parser.test: $(OBJDIR)/types.o
	$(CC) $(CFLAGS) -DUNIT_TESTS -o $@ fmt_str_parser.c $^

$(BINDIR)/buf_list.test: $(OBJDIR)/types.o
	$(CC) $(CFLAGS) -DUNIT_TESTS -o $@ buf_list.c $^

$(BINDIR)/types.test:
	$(CC) $(CFLAGS) -DUNIT_TESTS -o $@ types.c $^

$(BINDIR)/liboqs_prng.test: $(OBJDIR)/bufutils.o
	$(CC) $(CFLAGS) -DUNIT_TESTS -o $@ liboqs_prng.c $^ $(LIBOQSFLAGS)

tests: dirs $(BINDIR)/fmt_str_parser.test $(BINDIR)/types.test $(BINDIR)/buf_list.test $(BINDIR)/liboqs_prng.test

all: dirs tests

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
