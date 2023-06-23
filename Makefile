all: xwcode_stream

CC := g++
C := gcc
CCFLAGS := -std=gnu++2a -Wall -I./inc

xwcode_stream: 
	make -C src
	@mv -f src/xwcode_stream bin/
	@echo "completed"

.PHONY: clean
clean:
	make -C src clean
