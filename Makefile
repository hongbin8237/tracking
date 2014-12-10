CXXFLAGS += -g
CXXFLAGS += $(shell pkg-config --cflags opencv)
LIBS = -lm
LIBS += $(shell pkg-config --libs opencv)

all: track track_reference
track: track.o 
	$(CXX) -o track track.o $(CXXFLAGS) $(LIBS) 
track_reference: track_reference.o
	$(CXX) -o track_reference track_reference.o $(CXXFLAGS) $(LIBS)
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o