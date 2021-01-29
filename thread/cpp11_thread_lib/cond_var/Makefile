cond_var: cond_var.o
	$(CXX) build/cond_var.o -o out/cond_var -lpthread

cond_var.o: cond_var.cpp
	$(CXX) -c cond_var.cpp -o build/cond_var.o


.PHONY: clean
clean:
	rm build/*