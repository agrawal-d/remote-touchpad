CXX := g++
CXXFLAGS := -O0 -Wall -Wextra -pedantic -std=gnu++17
LDFLAGS := -lws2_32
SOURCES := main.cpp LocalServer.cpp RemoteTouchpad.cpp
TARGET := main.exe

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
