CXX = g++
CXXFLAGS = -std=c++14 -O3 -march=native -Wall -Wextra
SRCDIR = src
OBJDIR = obj
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET = brahma

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

test: $(TARGET)
	./$(TARGET)

compile-common:
	$(CXX) $(CXXFLAGS) -c $(SRCDIR)/common.cpp -o $(OBJDIR)/common.o

compile-bbinit:
	$(CXX) $(CXXFLAGS) -c $(SRCDIR)/bbinit.cpp -o $(OBJDIR)/bbinit.o

syntax-check:
	$(CXX) $(CXXFLAGS) -fsyntax-only $(SRCDIR)/*.cpp