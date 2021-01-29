thread: thread.o
	$(CXX) build/thread.o -o out/thread -lpthread

thread.o: thread.cpp
	$(CXX) -c thread.cpp -o build/thread.o


.PHONY: clean
clean:
	rm build/*