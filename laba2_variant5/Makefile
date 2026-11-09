.PHONY: all test check run clean experiment
all:
	go build -o laba2 .
test:
	go test ./...
check:
	go vet ./...
run:
	go run .
experiment: all
	./laba2 --task 6.1 > HASH_RESULTS.csv
clean:
	rm -f laba2
