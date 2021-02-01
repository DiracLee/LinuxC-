simplest: simplest.o
	$(CXX) build/simplest.o -o out/simplest -lpthread

simplest.o: simplest.cpp
	$(CXX) -c simplest.cpp -o build/simplest.o


with_args: with_args.o
	$(CXX) build/with_args.o -o out/with_args -lpthread

with_args.o: with_args.cpp
	$(CXX) -c with_args.cpp -o build/with_args.o


with_return: with_return.o
	$(CXX) build/with_return.o -o out/with_return -lpthread 

with_return.o: with_return.cpp
	$(CXX) -c with_return.cpp -o build/with_return.o -fpermissive




.PHONY: clean
clean:
	rm build/*