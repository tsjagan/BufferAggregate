all: unit_test thread_test

unit_test: test_buff_aggr.cpp
	     g++ --std=c++11 -I../include -g -o unit_test test_buff_aggr.cpp

thread_test: test_block_cache.cpp
	     g++ --std=c++11 -I../include -g -pthread -o test_block_cache test_block_cache.cpp


clean:
	rm -f ./unit_test
	rm -f ./test_block_cache

rmcore:
	rm -f \"core*
