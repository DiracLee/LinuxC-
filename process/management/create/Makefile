system: system.o
	$(CXX) -o out/system build/system.o

system.o: system.cpp
	$(CXX) -c system.cpp -o build/system.o


fork: fork.o
	$(CXX) -o out/fork build/fork.o

fork.o: fork.cpp
	$(CXX) -c fork.cpp -o build/fork.o


.PHONY: clean
clean:
	rm build/*