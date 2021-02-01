transaction: transaction.o
	$(CXX) build/transaction.o -o out/transaction -lpthread

transaction.o: transaction.cpp
	$(CXX) -c transaction.cpp -o build/transaction.o


.PHONY: clean
clean:
	rm build/*