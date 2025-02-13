all: build trace

build:
	gcc main.c arrow.c

trace:
	python3 generate_trace.py

test: build trace
	./a.out trace.txt

clean:
	rm -rf a.out trace.txt

help:
	@echo "Usage: make {build,test,help,clean}. Default: 'make' => 'make build; make trace'."
	@echo "    - build: compile the test executable"
	@echo "    - trace: generate the 'trace.txt'"
	@echo "    - test: execute 'make build; make trace' and run the test executable"
	@echo "    - clean: remove 'trace.txt' and the executable"
	@echo "    - help: print this help message"
