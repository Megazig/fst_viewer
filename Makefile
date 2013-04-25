OBJECTS = src/main.o
LIBS = 
OUTPUT = fst_viewer
all: $(OUTPUT)
%.o: %.c
	gcc -fno-strict-aliasing -g -c -o $@ $<
$(OUTPUT): $(OBJECTS)
	gcc  -g -o $(OUTPUT) $(OBJECTS) $(LIBS)
clean:
	rm -f $(OUTPUT) $(OBJECTS)

