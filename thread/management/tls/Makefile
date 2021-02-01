tls: tls.o
	$(CXX) build/tls.o -o out/tls -lpthread

tls.o: tls.cpp
	$(CXX) -c tls.cpp -o build/tls.o


.PHONY: clean
clean:
	rm build/*