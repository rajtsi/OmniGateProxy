CXX = clang++
CXXFLAGS = -std=c++11 -Wall

all:
	mkdir -p bin
	$(CXX) $(CXXFLAGS) src/main.cpp src/server.cpp src/thread_pool.cpp -o bin/omni_gate

clean:
	rm -rf bin/