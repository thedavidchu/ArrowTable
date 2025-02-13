CC=gcc
CFLAGS=-Wall -Werror -g
TRACE_FILE=trace.txt
EXE=arrow_exe

all: build trace

build:
	$(CC) $(CFLAGS) main.c arrow.c -o $(EXE)

trace:
	python3 generate_trace.py $(TRACE_FILE)

test: build trace
	./$(EXE) $(TRACE_FILE)

clean:
	rm -rf $(EXE) $(TRACE_FILE)

help:
	@echo "Usage: make {build,test,help,clean}. Default: 'make' => 'make build; make trace'."
	@echo "    - build: compile the test executable"
	@echo "    - trace: generate the '$(TRACE_FILE)'"
	@echo "    - test: execute 'make build; make trace' and run the test executable"
	@echo "    - clean: remove '$(TRACE_FILE)' and the executable"
	@echo "    - help: print this help message"
