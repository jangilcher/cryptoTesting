ROOTDIR=../../../../../../../
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

DIRNAME?=cur_liboqs
# DIRNAME?=mid_liboqs
# DIRNAME?=old_liboqs

CC?=gcc

BASEDIR=../../../
LIBOQSDIR=$(BASEDIR)../../../../$(DIRNAME)
LIBOQSFLAGS=-I $(LIBOQSDIR)/build/include -L $(LIBOQSDIR)/build/lib -loqs -lcrypto -lssl -lm

CHES=ches_liboqs
AFL_CHES=ches_liboqs_afl
CHES2=ches2_liboqs
AFL_CHES2=ches2_liboqs_afl
ifeq ($(DIRNAME), $(CHES))
    LIBOQSFLAGS+=-loqs-internal -lpthread -DCHES_VERSION
else ifeq ($(DIRNAME), $(CHES2))
    LIBOQSFLAGS+=-loqs-internal -lpthread -DCHES_VERSION
else ifeq ($(DIRNAME), $(AFL_CHES))
    LIBOQSFLAGS+=-loqs-internal -lpthread -DCHES_VERSION
else ifeq ($(DIRNAME), $(AFL_CHES2))
    LIBOQSFLAGS+=-loqs-internal -lpthread -DCHES_VERSION
endif


CFLAGS=-Wno-unused-command-line-argument -fno-sanitize=address 
# AFLCFLAGS=-fsanitize=fuzzer
AFLCFLAGS=-Wno-unused-command-line-argument

OBJDIR=obj
BINDIR=bin
CRASHDIR=crashes
FUZZINPUTS=fuzzinputs
FUZZOUTPUTS=fuzzoutputs
AFLCRASHDIR=$(FUZZOUTPUTS)/default/crashes

# # Compiling utilities
UTILDIR=$(BASEDIR)../../utilities
$(OBJDIR)/%.util.o: $(UTILDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

$(OBJDIR)/%.util.afl.o: $(UTILDIR)/%.c
	$(AFLCC) $(AFLCFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

# # Compiling objects in .
# $(OBJDIR)/%.base.o: $(BASEDIR)/%.c
# 	$(CC) $(CFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

# $(OBJDIR)/%.base.afl.o: $(BASEDIR)/%.c
# 	$(AFLCC) $(AFLCFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

# $(OBJDIR)/%.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

# $(OBJDIR)/%.afl.o: %.c
# 	$(AFLCC) $(AFLCFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

# Executables

$(BINDIR)/fuzz_harness.afl.out: fuzz_harness.c
	$(AFLCC) $(AFLCFLAGS) -DALG_ID=$(DESIRED_ALG_TO_FUZZ) fuzz_harness.c -o $(BINDIR)/fuzz_harness.afl.out $(LIBOQSFLAGS)

$(BINDIR)/fuzz_harness_old.afl.out: fuzz_harness_old.c
	$(AFLCC) $(AFLCFLAGS) -DALG_ID=$(DESIRED_ALG_TO_FUZZ) fuzz_harness_old.c -o $(BINDIR)/fuzz_harness_old.afl.out $(LIBOQSFLAGS)

$(BINDIR)/gen_input.out: gen_input.c $(OBJDIR)/liboqs_prng.util.o
	$(AFLCC) $(AFLCFLAGS) gen_input.c -DALG_ID=$(DESIRED_ALG_TO_FUZZ) -o $(BINDIR)/gen_input.out $^ $(LIBOQSFLAGS)

$(BINDIR)/ParseInput.out: ParseInput.c $(OBJDIR)/bufutils.util.o
	$(AFLCC) $(AFLCFLAGS) -DALG_ID=$(DESIRED_ALG_TO_FUZZ) ParseInput.c -o $(BINDIR)/ParseInput.out $^ $(LIBOQSFLAGS)

	
$(BINDIR)/list.out:
	make dirs
	$(AFLCC) $(CFLAGS) -o $@ $(BASEDIR)/list.c $^ $(LIBOQSFLAGS) -Dalg_identifier=OQS_KEM_alg_identifier

all: dirs $(BINDIR)/list.out $(BINDIR)/gen_input.out $(BINDIR)/fuzz_harness.afl.out #$(BINDIR)/fuzz_harness.afl.asan.out $(BINDIR)/fuzz_harness.afl.msan.out $(BINDIR)/fuzz_harness.afl.cfisan.out
	ln -s -f $(BASEDIR)/run_all.py
	
clone:
	ln -s -f $(BASEDIR)/clone.sh

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
	rm -rf __pycache__
	rm -f Maul.py
	rm -f run_all.py
	rm -f serialize.py
	rm -f *.log

# loops in makefiles: https://stackoverflow.com/a/1490961

# setting default overwritable value for DESIRED_ALG_TO_FUZZ
DESIRED_ALG_TO_FUZZ?=0

# timeout after which to kill GenInput
GITIMEOUT?=10

AFLFUZZCMD=$(AFLFUZZ)  -V 900 -i $(FUZZINPUTS)/ -o $(FUZZOUTPUTS) -D -- $(BINDIR)/fuzz_harness.afl.out @@
# -V 90 above would set a time limit to afl-fuzz

# run_afl: export AFL_PYTHON_MODULE=Maul
# run_afl: export AFL_CUSTOM_MUTATOR_ONLY=1
-include ../../../MakefileCommonVars.mk
run_afl: export PYTHONPATH=$(shell pwd)
run_afl: export AFL_SKIP_CPUFREQ=1
run_afl: export AFL_DISABLE_TRIM=1
run_afl: export AFL_EXIT_WHEN_DONE=1
run_afl: export AFL_EXIT_ON_TIME=300 #1800
run_afl: export AFL_DEBUG_CHILD=0
run_afl: export AFL_INPUT_LEN_MAX=100000000
run_afl: export AFL_TESTCACHE_SIZE=200
run_afl: export BASEDIR_LVL=$(BASEDIR)
run_afl: dirs all
	$(BINDIR)/gen_input.out $(FUZZINPUTS)/input.bin
	-$(AFLFUZZCMD)
	ls -l $(FUZZOUTPUTS)/default/crashes
	ls -l $(FUZZOUTPUTS)/default/hangs

-include ../../../MakefileCommonTargets.mk
