ROOTDIR=../../../../
ifneq ("$(wildcard $(ROOTDIR)/aflpp/afl-clang-fast)","")
    AFLCC=$(ROOTDIR)/aflpp/afl-clang-fast
else ifneq ("$(wildcard $(ROOTDIR)/aflpp/afl-clang)","")
    AFLCC=$(ROOTDIR)/aflpp/afl-clang
else ifeq (, $(shell which afl-clang-fast))
    AFLCC=afl-clang-fast
else ifeq (, $(shell which afl-clang))
    AFLCC=afl-clang
else
    $(error Need some form of afl-cc installed.)
endif

ifneq ("$(wildcard $(ROOTDIR)/aflpp/afl-fuzz)","")
    AFLFUZZ=$(ROOTDIR)/aflpp/afl-fuzz
else ifeq (, $(shell which afl-fuzz))
    AFLFUZZ=afl-fuzz
else
    $(error Need some form of afl-fuzz installed.)
endif


CC=clang
# AFLCC=afl-clang-fast

CFLAGS=
#-fsanitize=address
# AFLCFLAGS=-fsanitize=fuzzer
AFLCFLAGS=

OBJDIR=obj
BINDIR=bin
CRASHDIR=crashes
FUZZINPUTS=fuzzinputs
FUZZOUTPUTS=fuzzoutputs
AFLCRASHDIR=$(FUZZOUTPUTS)/default/crashes/

# Build a specific hash, but try to keep it general
# this could loop through subdirs or something
APIDIR=jh_ref

$(OBJDIR)/%.api.o: $(APIDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.api.afl.o: $(APIDIR)/%.c
	$(AFLCC) $(AFLCFLAGS) -c $< -o $@

# Compiling utilities
UTILDIR=../../utilities
$(OBJDIR)/%.util.o: $(UTILDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.util.afl.o: $(UTILDIR)/%.c
	$(AFLCC) $(AFLCFLAGS) -c $< -o $@

# Compiling objects in .

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.afl.o: %.c
	$(AFLCC) $(AFLCFLAGS) -c $< -o $@

# Executables

$(BINDIR)/GenInput.out: $(OBJDIR)/serialize.o $(OBJDIR)/Call.o $(OBJDIR)/GenInput.o $(OBJDIR)/fmt_str_parser.util.o $(OBJDIR)/bufutils.util.o $(OBJDIR)/types.util.o
	$(CC) $(CFLAGS) -o $(BINDIR)/GenInput.out $^

$(BINDIR)/Match.out: $(OBJDIR)/serialize.o $(OBJDIR)/Call.o $(OBJDIR)/fmt_str_parser.util.o $(OBJDIR)/bufutils.util.o $(OBJDIR)/types.util.o
	$(CC) $(CFLAGS) -DSTAND_ALONE Match.c -o $(BINDIR)/Match.out $^

$(BINDIR)/Match.afl.out: $(OBJDIR)/serialize.afl.o $(OBJDIR)/Call.afl.o $(OBJDIR)/fmt_str_parser.util.afl.o $(OBJDIR)/bufutils.util.afl.o $(OBJDIR)/types.util.afl.o
	$(AFLCC) $(AFLCFLAGS) Match.c -o $(BINDIR)/Match.afl.out $^

$(BINDIR)/Maul.so: Maul.c serialize.c ${UTILDIR}/fmt_str_parser.c $(UTILDIR)/bufutils.c $(UTILDIR)/types.c
	$(CC) -fpic -shared -undefined dynamic_lookup -o $(BINDIR)/Maul.so $^

$(BINDIR)/GenInput.a: $(OBJDIR)/serialize.o $(OBJDIR)/Call.o $(OBJDIR)/GenInput.o $(OBJDIR)/fmt_str_parser.util.o $(OBJDIR)/bufutils.util.o $(OBJDIR)/types.util.o
	ar rcs $(BINDIR)/GenInput.a $^

$(BINDIR)/Match.afl.a: $(OBJDIR)/Match.afl.o $(OBJDIR)/serialize.afl.o $(OBJDIR)/Call.afl.o $(OBJDIR)/fmt_str_parser.util.afl.o $(OBJDIR)/bufutils.util.afl.o $(OBJDIR)/types.util.afl.o $(OBJDIR)/approx_memcmp.util.afl.o
	ar rcs $(BINDIR)/Match.afl.a $^

$(BINDIR)/Match.afl.so: Maul.c serialize.c ${UTILDIR}/fmt_str_parser.c $(UTILDIR)/bufutils.c $(UTILDIR)/types.c Call.c Match.c
	$(AFLCC) $(AFLCFLAGS) -fpic -shared -undefined dynamic_lookup -o $(BINDIR)/Match.afl.so $^

$(BINDIR)/ParseInput.a: $(OBJDIR)/serialize.o $(OBJDIR)/Call.o $(OBJDIR)/ParseInput.o $(OBJDIR)/fmt_str_parser.util.o $(OBJDIR)/bufutils.util.o $(OBJDIR)/types.util.o
	ar rcs $(BINDIR)/ParseInput.a $^

all: dirs $(BINDIR)/GenInput.out $(BINDIR)/Match.out $(BINDIR)/Match.afl.out $(BINDIR)/Maul.so

libs: dirs $(BINDIR)/GenInput.a $(BINDIR)/Maul.so $(BINDIR)/Match.afl.a $(BINDIR)/ParseInput.a
# Tests

$(BINDIR)/serialize.test: $(OBJDIR)/types.util.o $(OBJDIR)/bufutils.util.o
	$(CC) $(CFLAGS) -DUNIT_TESTS -o $@ serialize.c $^

tests: dirs $(BINDIR)/serialize.test

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	mkdir -p $(FUZZINPUTS)
	mkdir -p $(CRASHDIR)

clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
	rm -rf $(FUZZINPUTS)
	rm -rf $(FUZZOUTPUTS)
	rm -rf $(CRASHDIR)
