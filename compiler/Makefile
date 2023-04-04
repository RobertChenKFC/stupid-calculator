FLEX=flex
BISON=bison
BISON_FLAGS=-d -v
CXX=g++
CXX_FLAGS=-O3 -Wall -Wpedantic -std=c++17
BIN_FILE=compiler

.PHONY: clean

$(BIN_FILE): main.o parser.o lexer.o driver.o
	$(CXX) $(CXX_FLAGS) -o $(BIN_FILE) $^

%.o: %.cc
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

%.cc: %.ll
	$(FLEX) -o $@ $<

%.cc %.hh: %.yy
	$(BISON) $(BISON_FLAGS) -o $*.cc $<

main.o: parser.hh
parser.o: parser.hh
lexer.o: parser.hh
driver.o: driver.hh

clean:
	rm -f *.o $(BIN_FILE) parser.hh parser.cc lexer.cc
