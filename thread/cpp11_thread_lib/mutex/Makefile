mutex: mutex.o
	$(CXX) build/mutex.o -o out/mutex -lpthread

mutex.o: mutex.cpp
	$(CXX) -c mutex.cpp -o build/mutex.o


lock_guard: lock_guard.o
	$(CXX) build/lock_guard.o -o out/lock_guard -lpthread

lock_guard.o: lock_guard.cpp
	$(CXX) -c lock_guard.cpp -o build/lock_guard.o


transaction: transaction.o
	$(CXX) build/transaction.o -o out/transaction -lpthread

transaction.o: transaction.cpp
	$(CXX) -c transaction.cpp -o build/transaction.o


.PHONY: clean
clean:
	rm build/*