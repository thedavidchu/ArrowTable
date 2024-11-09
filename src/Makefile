all: build

build:
	gcc main.c arrow.c
	python3 generate_trace.py

test: build
	./a.out

clean:
	rm -rf a.out trace.txt

help:
	@echo "Usage: make {build,test,help,clean}. Default: 'make' => 'make build'."
	@echo "    - build: build the 'trace.txt' and compile the test executable"
	@echo "    - test: execute 'make build' and run the test executable"
	@echo "    - clean: remove 'trace.txt' and the executable"
	@echo "    - help: print this help message"

