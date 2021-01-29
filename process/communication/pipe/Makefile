pipe_com: pipe_com.o
	$(CXX) build/pipe_com.o -o out/pipe_com

pipe_com.o: pipe_com.cpp
	$(CXX) -c pipe_com.cpp -o build/pipe_com.o


pipe_redirect: pipe_redirect.o
	$(CXX) build/pipe_redirect.o -o out/pipe_redirect

pipe_redirect.o: pipe_redirect.cpp
	$(CXX) -c pipe_redirect.cpp -o build/pipe_redirect.o


.PHONY: clean
clean:
	rm build/*