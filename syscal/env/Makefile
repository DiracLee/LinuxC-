arg_list: arg_list.o
	$(CXX) -o out/arg_list build/arg_list.o

arg_list.o: arg_list.cpp
	$(CXX) -c arg_list.cpp -o build/arg_list.o

env_var: env_var.o
	$(CXX) -o out/env_var build/env_var.o

env_var.o: env_var.cpp
	$(CXX) -c env_var.cpp -o build/env_var.o


.PHONY: clean
clean:
	rm build/*