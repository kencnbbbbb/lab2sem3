CXX = c++
PYTHON ?= python3

all: go cpp

go:
	go build -o laba2 .

cpp:
	$(MAKE) -C cpp CXX="$(CXX)"

test-go:
	go test ./...

test-cpp: cpp
	$(MAKE) -C cpp test CXX="$(CXX)"

cross-test: all
	$(PYTHON) cpp/tests/verify_cli.py --cpp ./laba2-cpp --go ./laba2

test: test-go test-cpp cross-test

check: cpp
	go vet ./...

run: go
	./laba2

experiment: go
	./laba2 --task 6.1 > HASH_RESULTS.csv

experiment-cpp: cpp
	mkdir -p cpp/reports
	./laba2-cpp --task 6.1 > cpp/reports/hash-results.csv

clean:
	$(MAKE) -C cpp clean
	rm -f laba2

.PHONY: all go cpp test test-go test-cpp cross-test check run clean experiment experiment-cpp
