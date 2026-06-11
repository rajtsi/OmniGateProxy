CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3 -I./include -I../include
LDFLAGS = -lpthread

BIN_DIR = bin

TARGET = $(BIN_DIR)/omni_gate

SRCS = $(wildcard src/*.cpp)

OBJS = $(patsubst src/%.cpp, $(BIN_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo "--------------------------------------------------------"
	@echo "Build successful! Everything is neatly placed inside '$(BIN_DIR)/'"
	@echo "Run your server using: cd bin && ./omni_gate"
	@echo "--------------------------------------------------------"

$(BIN_DIR)/%.o: src/%.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR)
	@echo "Cleaned up all binaries and object files from '$(BIN_DIR)/'."
