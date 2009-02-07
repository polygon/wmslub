CXX=g++
CXXFLAGS=-I/usr/local/include `xml2-config --cflags` `curl-config --cflags` -g -DDEBUG
LDFLAGS=-L/usr/local/lib `curl-config --libs` `xml2-config --libs` -lsqlite3 -ldockapp -g
EXECUTABLE=wmslub
SOURCES=main.c booklist.c database.c
OBJECTS=$(SOURCES:.c=.o)

all: $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(EXECUTABLE) $(OBJECTS)

%.o: %.c
	$(CXX) $(CXXFLAGS) -o $@ -c $<
