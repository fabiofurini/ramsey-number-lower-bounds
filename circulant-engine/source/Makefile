CXX ?= g++
# install prefix of the BitGraph / coptBG libraries (not redistributed, see ../../NOTICE.md)
COPT_ROOT ?= /path/to/coptBG/install

CPPFLAGS := -I$(COPT_ROOT)/include/BitGraph -I$(COPT_ROOT)/include/COPT_BG
CXXFLAGS ?= -O3 -march=native -mpopcnt -std=c++14 -Wall -Wextra -Wpedantic -Wno-deprecated -DNDEBUG -DLOGGER_ERROR_LEVEL
CXXFLAGS += -DPATH_RAMSEY_GRAPHS=\"./\"
LDLIBS := -L$(COPT_ROOT)/lib -lcopt -lgraph -lbitscan -lutils -lm -ldl -pthread

TARGET := hybrid_gencyc
SOURCES := src/main.cpp src/copt_oracle.cpp
OBJECTS := $(SOURCES:.cpp=.o)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

src/%.o: src/%.cpp src/dynamic_bitset.h src/copt_oracle.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

bitset_tests: tests/test_dynamic_bitset.cpp src/dynamic_bitset.h
	$(CXX) -O2 -std=c++14 -Wall -Wextra -Wpedantic -Isrc $< -o $@

copt_large_tests: tests/test_copt_large.cpp src/copt_oracle.o src/dynamic_bitset.h src/copt_oracle.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Isrc -o $@ tests/test_copt_large.cpp src/copt_oracle.o $(LDLIBS)

test: $(TARGET) bitset_tests copt_large_tests
	bash tests/run_tests.sh

clean:
	rm -f $(OBJECTS) $(TARGET) bitset_tests copt_large_tests copt_over128_tests
