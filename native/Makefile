CXX := g++
CXXFLAGS := -O2 -Wall -Wextra -pedantic -std=gnu++17
LDFLAGS := -lws2_32 -liphlpapi
SOURCES := main.cpp LocalServer.cpp RemoteTouchpad.cpp
TARGET := touchpad.exe

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
