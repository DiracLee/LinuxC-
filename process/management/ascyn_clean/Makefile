async_clean: async_clean.o
	$(CXX) -o out/async_clean build/async_clean.o

async_clean.o: async_clean.cpp
	$(CXX) -o build/async_clean.o -c async_clean.cpp

.PHONY: clean
clean:
	rm build/*