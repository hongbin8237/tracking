
CXXFLAGS += -g -Wall
CXXFLAGS += $(shell pkg-config --cflags opencv)
LIBS = -lm
LIBS += $(shell pkg-config --libs opencv)

track: track.o 
	$(CXX) -o track track.o $(CXXFLAGS) $(LIBS) 
#trackJH: trackJH.o
#	$(CXX) -o trackJH trackJH.o $(CXXFLAGS) $(LIBS)
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o

