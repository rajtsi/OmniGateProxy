CXX = clang++
CXXFLAGS = -std=c++11 -Wall

all:
	mkdir -p bin
	clang++ -std=c++17 -Wall src/main.cpp src/server.cpp src/thread_pool.cpp src/httpparser.cpp src/layer7.cpp -o bin/omni_gate

clean:
	rm -rf bin/