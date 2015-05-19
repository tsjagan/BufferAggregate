all: unit_test thread_test

unit_test: test_buff_aggr.cpp
	     g++ --std=c++11 -g -o unit_test test_buff_aggr.cpp

thread_test: block_cache.cpp
	     g++ --std=c++11 -g -pthread -o test_block_cache block_cache.cpp


clean:
	rm -f ./unit_test
	rm -f ./test_block_cache
