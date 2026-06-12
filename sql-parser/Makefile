CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = sql_parser
SRCDIR   = src
SRCS     = $(SRCDIR)/main.cpp \
           $(SRCDIR)/lexer.cpp \
           $(SRCDIR)/error_reporter.cpp
OBJS     = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

test: all
	@bash tests/run_tests.sh

.PHONY: all clean test
