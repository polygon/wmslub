CXX=gcc
CXXFLAGS=-I/usr/local/include `pkg-config --cflags gai` `xml2-config --cflags` `curl-config --cflags` `pkg-config --cflags sqlite3` -g -DDEBUG
LDFLAGS=-L/usr/local/lib `pkg-config --libs gai` `curl-config --libs` `xml2-config --libs` `pkg-config --libs sqlite3` -g
EXECUTABLE=wmslub
SOURCES=main.c booklist.c database.c dockapp.c
OBJECTS=$(SOURCES:.c=.o)

all: $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(EXECUTABLE) $(OBJECTS)

%.o: %.c
	$(CXX) $(CXXFLAGS) -o $@ -c $<
