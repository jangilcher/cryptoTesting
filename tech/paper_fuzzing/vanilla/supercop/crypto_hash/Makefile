ROOTDIR=../../../../../
ifneq ("$(wildcard $(ROOTDIR)/aflpp/afl-clang-fast)","")
    AFLCC=$(ROOTDIR)/aflpp/afl-clang-fast
else ifneq ("$(wildcard $(ROOTDIR)/aflpp/afl-clang)","")
    AFLCC=$(ROOTDIR)/aflpp/afl-clang
else ifneq (, $(shell which afl-clang-fast))
    AFLCC=afl-clang-fast
else ifneq (, $(shell which afl-clang))
    AFLCC=afl-clang
else
    $(error Need some form of afl-cc installed.)
endif

ifneq ("$(wildcard $(ROOTDIR)/aflpp/afl-fuzz)","")
    AFLFUZZ=$(ROOTDIR)/aflpp/afl-fuzz
else ifneq (, $(shell which afl-fuzz))
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
UTILDIR=../../../utilities
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
$(BINDIR)/gen_input.o: gen_input.c
	$(AFLCC) -c $(AFLCFLAGS) gen_input.c -o $(BINDIR)/gen_input.o $^

$(BINDIR)/fuzz_harness.afl.o: fuzz_harness.c
	$(AFLCC) -c $(AFLCFLAGS) -DALG_ID=$(DESIRED_ALG_TO_FUZZ) fuzz_harness.c -o $(BINDIR)/fuzz_harness.afl.o $^

$(BINDIR)/ParseInput.o: ParseInput.c
	$(AFLCC) -c $(AFLCFLAGS) -DALG_ID=$(DESIRED_ALG_TO_FUZZ) ParseInput.c -o $(BINDIR)/ParseInput.o $^

$(BINDIR)/ParseInput.a: $(OBJDIR)/ParseInput.o $(OBJDIR)/bufutils.util.o $(OBJDIR)/types.util.o
	ar rcs $(BINDIR)/ParseInput.a $^


all: dirs $(BINDIR)/gen_input.o $(BINDIR)/fuzz_harness.afl.o

libs: dirs $(BINDIR)/ParseInput.a

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
