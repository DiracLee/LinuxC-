exec: exec.o
	$(CXX) -o out/exec build/exec.o

exec.o: exec.cpp
	$(CXX) -c exec.cpp -o build/exec.o

.PHONY: clean
clean:
	rm build/*