all: unit_test thread_test

unit_test: test_buff_aggr.cpp
	     g++ --std=c++11 -g -o unit_test test_buff_aggr.cpp

thread_test: test_multithreaded.cpp
	     g++ --std=c++11 -g -pthread -o thread_test test_multithreaded.cpp


clean:
	rm -f ./unit_test
	rm -f ./thread_test
