all: build

build:
	gcc main.c arrow.c
	python3 generate_trace.py

test: build
	./a.out

