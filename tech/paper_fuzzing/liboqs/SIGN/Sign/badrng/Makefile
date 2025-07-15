ROOTDIR=../../../../../../
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

DIRNAME?=cur_liboqs
# DIRNAME?=mid_liboqs
# DIRNAME?=old_liboqs

CC?=gcc

BASEDIR=../../../
LIBOQSDIR=$(BASEDIR)../../../$(DIRNAME)
LIBOQSFLAGS=-I $(LIBOQSDIR)/build/include -L $(LIBOQSDIR)/build/lib -loqs -lcrypto -lssl -lm

CHES=ches_liboqs
ifeq ($(DIRNAME), $(CHES))
    LIBOQSFLAGS+=-loqs-internal -lpthread -DCHES_VERSION
endif

CFLAGS=-fsanitize=address -Wno-unused-command-line-argument
# AFLCFLAGS=-fsanitize=fuzzer
AFLCFLAGS=-Wno-unused-command-line-argument

OBJDIR=obj
BINDIR=bin
CRASHDIR=crashes
FUZZINPUTS=fuzzinputs
FUZZOUTPUTS=fuzzoutputs
AFLCRASHDIR=$(FUZZOUTPUTS)/default/crashes

# Build a specific hash, but try to keep it general
# this could loop through subdirs or something
# APIDIR=jh_ref

# $(OBJDIR)/%.api.o: $(APIDIR)/%.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# $(OBJDIR)/%.api.afl.o: $(APIDIR)/%.c
# 	$(AFLCC) $(AFLCFLAGS) -c $< -o $@

# Compiling utilities
UTILDIR=$(BASEDIR)../utilities
$(OBJDIR)/%.util.o: $(UTILDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

$(OBJDIR)/%.util.afl.o: $(UTILDIR)/%.c
	$(AFLCC) $(AFLCFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

# Compiling objects in .
$(OBJDIR)/%.base.o: $(BASEDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

$(OBJDIR)/%.base.afl.o: $(BASEDIR)/%.c
	$(AFLCC) $(AFLCFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

$(OBJDIR)/%.afl.o: %.c
	$(AFLCC) $(AFLCFLAGS) -c $< -o $@ $(LIBOQSFLAGS)

# Executables

$(BINDIR)/GenInput.out: $(OBJDIR)/serialize.base.o $(OBJDIR)/Call.o $(OBJDIR)/GenInput.o $(OBJDIR)/buf_list.util.o $(OBJDIR)/fmt_str_parser.util.o $(OBJDIR)/bufutils.util.o $(OBJDIR)/types.util.o $(OBJDIR)/liboqs_prng.util.o
	$(CC) $(CFLAGS) -o $(BINDIR)/GenInput.out $^ $(LIBOQSFLAGS)

$(BINDIR)/Match.out: $(OBJDIR)/serialize.base.o $(OBJDIR)/Call.o $(OBJDIR)/buf_list.util.o $(OBJDIR)/fmt_str_parser.util.o $(OBJDIR)/bufutils.util.o $(OBJDIR)/types.util.o $(OBJDIR)/liboqs_prng.util.o
	$(CC) $(CFLAGS) -DSTAND_ALONE $(BASEDIR)/Match.c -o $(BINDIR)/Match.out $^ $(LIBOQSFLAGS)

$(BINDIR)/Match.afl.out: $(OBJDIR)/serialize.base.afl.o $(OBJDIR)/Call.afl.o $(OBJDIR)/buf_list.util.afl.o $(OBJDIR)/fmt_str_parser.util.afl.o $(OBJDIR)/bufutils.util.afl.o $(OBJDIR)/types.util.afl.o $(OBJDIR)/liboqs_prng.util.afl.o
	$(AFLCC) $(AFLCFLAGS) $(BASEDIR)/Match.c -o $(BINDIR)/Match.afl.out $^ $(LIBOQSFLAGS)

$(BINDIR)/Maul.so: $(BASEDIR)/Maul.c $(BASEDIR)/serialize.c $(UTILDIR)/buf_list.c $(UTILDIR)/fmt_str_parser.c $(UTILDIR)/bufutils.c $(UTILDIR)/types.c
	$(CC) -fpic -shared -undefined,dynamic_lookup -o $(BINDIR)/Maul.so $^ $(LIBOQSFLAGS)

$(BINDIR)/list.out:
	make dirs
	$(CC) $(CFLAGS) -o $@ $(BASEDIR)/list.c $^ $(LIBOQSFLAGS) -Dalg_identifier=OQS_KEM_alg_identifier

all: dirs $(BINDIR)/GenInput.out $(BINDIR)/Match.out $(BINDIR)/Match.afl.out $(BINDIR)/Maul.so $(BINDIR)/ParseInput.out $(BINDIR)/list.out
	ln -s -f $(BASEDIR)/run_all.py

clone:
	ln -s -f $(BASEDIR)/clone.sh

# Tests

$(BINDIR)/serialize.test: $(OBJDIR)/types.util.o $(OBJDIR)/buf_list.util.o $(OBJDIR)/bufutils.util.o
	$(CC) $(CFLAGS) -DUNIT_TESTS -o $@ serialize.c $^

# Tools

$(BINDIR)/ParseInput.out: $(BASEDIR)/serialize.c $(UTILDIR)/fmt_str_parser.c $(OBJDIR)/Call.o $(OBJDIR)/types.util.o $(OBJDIR)/buf_list.util.o $(OBJDIR)/bufutils.util.o $(OBJDIR)/liboqs_prng.util.o
	$(CC) $(CFLAGS) -o $@ ParseInput.c $^ $(LIBOQSFLAGS)

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
	rm -rf __pycache__
	rm -f *.log

# loops in makefiles: https://stackoverflow.com/a/1490961

GENINPUTCMD=$(BINDIR)/GenInput.out $(FUZZINPUTS)/test.bin
FUZZCMD=$(AFLFUZZ) -i $(FUZZINPUTS)/ -o $(FUZZOUTPUTS) -G 100000000 -t 100000000 -D -- $(BINDIR)/Match.afl.out @@
# -V 90 above would set a time limit to afl-fuzz

# setting default overwritable value for DESIRED_ALG_TO_FUZZ
DESIRED_ALG_TO_FUZZ?=0

# timeout after which to kill GenInput
GITIMEOUT?=10

run_python: export PYTHONPATH=$(shell pwd)
run_python: export AFL_PYTHON_MODULE=Maul
run_python: export AFL_CUSTOM_MUTATOR_ONLY=1
run_python: export AFL_SKIP_CPUFREQ=1
run_python: export AFL_DISABLE_TRIM=1
run_python: export AFL_EXIT_WHEN_DONE=1
run_python: export AFL_EXIT_ON_TIME=1
run_python: export AFL_DEBUG_CHILD=0
run_python: export AFL_NO_UI=0
run_python: export AFL_INPUT_LEN_MAX=100000000
run_python: export AFL_TESTCACHE_SIZE=200
run_python: export BASEDIR_LVL=$(BASEDIR)
run_python: dirs all
	-timeout -s SIGKILL $(GITIMEOUT)s $(BINDIR)/GenInput.out $(FUZZINPUTS)/clean_input.bin $(DESIRED_ALG_TO_FUZZ); \
	if [ $$? -eq 137 ]; then \
		mkdir -p $(FUZZOUTPUTS)/default/crashes; \
		mkdir -p $(FUZZOUTPUTS)/default/hangs; \
		touch $(FUZZOUTPUTS)/default/hangs/GenInput; \
	else \
		$(FUZZCMD) ; \
	fi
	ls -l $(FUZZOUTPUTS)/default/crashes
	ls -l $(FUZZOUTPUTS)/default/hangs


run_c: export AFL_CUSTOM_MUTATOR_LIBRARY=$(CURDIR)/$(BINDIR)/Maul.so
run_c: export AFL_CUSTOM_MUTATOR_ONLY=1
run_c: export AFL_SKIP_CPUFREQ=1
run_c: export AFL_DISABLE_TRIM=1
run_c: export AFL_EXIT_WHEN_DONE=1
run_c: export AFL_EXIT_ON_TIME=1
run_c: export AFL_DEBUG_CHILD=0
run_c: export AFL_NO_UI=0
run_c: dirs all
	min=0; max=60; for bits in `seq $$min $$max`; do $(GENINPUTCMD) $$bits; $(FUZZCMD); cp $(AFLCRASHDIR)/* $(CRASHDIR); done

AFLFUZZCMD=$(AFLFUZZ) -i $(FUZZINPUTS)/ -o $(FUZZOUTPUTS) -D -- $(BINDIR)/Match.afl.out clean_input.bin @@
# -V 90 above would set a time limit to afl-fuzz

# run_afl: export AFL_PYTHON_MODULE=Maul
# run_afl: export AFL_CUSTOM_MUTATOR_ONLY=1
-include ../../../MakefileCommonVars.mk
run_afl: export PYTHONPATH=$(shell pwd)
run_afl: export AFL_SKIP_CPUFREQ=1
run_afl: export AFL_DISABLE_TRIM=1
run_afl: export AFL_EXIT_WHEN_DONE=1
run_afl: export AFL_EXIT_ON_TIME=1
run_afl: export AFL_DEBUG_CHILD=0
run_afl: export AFL_INPUT_LEN_MAX=100000000
run_afl: export AFL_TESTCACHE_SIZE=200
run_afl: export BASEDIR_LVL=$(BASEDIR)
run_afl: dirs all
	$(BINDIR)/GenInput.out clean_input.bin $(DESIRED_ALG_TO_FUZZ) $(FUZZINPUTS)/zero_mask.bin
	-$(AFLFUZZCMD)
	ls -l $(FUZZOUTPUTS)/default/crashes
	ls -l $(FUZZOUTPUTS)/default/hangs

-include ../../../MakefileCommonTargets.mk
