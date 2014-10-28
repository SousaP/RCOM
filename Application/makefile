CC=gcc
CFLAGS= -c -g -Wall
LDFLAGS=
SOURCES= appLayer.c linkLayer.c main.c packets.c
HEADERS= appLayer.h linkLayer.h types.h 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=program

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@echo Linking file...
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	@echo
	@echo Compilation complete.

.c.o:
	@echo Compiling $@..
	@$(CC) $(CFLAGS) $< -o $@

clean:
	@echo Cleaning...
	@echo
	@rm *.o
	@rm $(EXECUTABLE)

r: clean all
	@./program r

w: clean all
	@./program w

compress: all clean
	@zip compressed.zip $(SOURCES) $(HEADERS) makefile
