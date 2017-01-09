
CPPFLAGS = -Wall -g -D_DEBUG -m64 -fPIC -I. -I"./deps/libzs"
LDFLAGS = -m64 -lzs -L"./deps/libzs"  -liconv -lssl
OBJ = main.o

BIN = fun

all : $(BIN) 

$(BIN) : $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^
	
	
.PHONY : clean build rebuild
clean :
	@ -rm -rf build
	@ -rm -f $(OBJ) $(BIN)
	
build :
	$(MAKE)
	
rebuild :
	$(MAKE)	clean
	$(MAKE) build
