thread_id: thread_id.o
	$(CXX) build/thread_id.o -o out/thread_id -lpthread

thread_id.o: thread_id.cpp
	$(CXX) -c thread_id.cpp -o build/thread_id.o


thread_attr: thread_attr.o
	$(CXX) build/thread_attr.o -o out/thread_attr -lpthread

thread_attr.o: thread_attr.cpp
	$(CXX) -c thread_attr.cpp -o build/thread_attr.o


.PHONY: clean
clean:
	rm build/*