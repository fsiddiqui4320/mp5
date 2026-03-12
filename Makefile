CC      = gcc
CFLAGS  = -g -Wall -Wextra -std=c11

# Build all workload .so files plus the test harness
all: tester workloads/mytest.so

# Test harness (provided by course; compile testharness.c if present)
tester: testharness.c allocator.c
	$(CC) $(CFLAGS) -o $@ $^ -ldl

# Each workload is a shared library: pattern rule covers provided .c files
workloads/%.so: workloads/%.c allocator.c allocator.h
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

# mytest.so from mytest.c
workloads/mytest.so: mytest.c allocator.h
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

# Standalone debug build of mytest (no harness needed)
mytest_standalone: mytest.c allocator.c allocator.h
	$(CC) $(CFLAGS) -DSTANDALONE -o $@ mytest.c allocator.c

# Run all workloads through the harness
test: tester $(wildcard workloads/*.so)
	./tester workloads/*.so

clean:
	rm -f tester mytest_standalone workloads/*.so

.PHONY: all test clean
