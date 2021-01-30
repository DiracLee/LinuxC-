pointer_param: pointer_param.o
	$(CXX) build/pointer_param.o -o out/pointer_param -lpthread

pointer_param.o: pointer_param.cpp
	$(CXX) -c pointer_param.cpp -o build/pointer_param.o


future: future.o
	$(CXX) build/future.o -o out/future -lpthread

future.o: future.cpp
	$(CXX) -c future.cpp -o build/future.o


promise: promise.o
	$(CXX) build/promise.o -o out/promise -lpthread

promise.o: promise.cpp
	$(CXX) -c promise.cpp -o build/promise.o


.PHONY: clean
clean:
	rm build/*