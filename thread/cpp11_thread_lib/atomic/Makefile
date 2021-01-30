atomic: atomic.o
	$(CXX) build/atomic.o -o out/atomic -lpthread

atomic.o: atomic.cpp
	$(CXX) -c atomic.cpp -o build/atomic.o


.PHONY: clean
clean:
	rm build/*