CXX=gcc
CXXFLAGS=-I/usr/local/include -I/usr/include/libxml2 -g -DDEBUG
LDFLAGS=-L/usr/local/lib -lcurl -lxml2 -g
EXECUTABLE=wmslub
SOURCES=main.c booklist.c
OBJECTS=$(SOURCES:.c=.o)

all: $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(EXECUTABLE) $(OBJECTS)

%.o: %.c
	$(CXX) $(CXXFLAGS) -o $@ -c $<
