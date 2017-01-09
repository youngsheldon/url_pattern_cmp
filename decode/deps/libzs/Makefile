CPPFLAGS = -Wall -g -D_DEBUG -m64 -fPIC -I.
LDFLAGS = -shared -m64 

OBJ =commonlib.o cryptodes.o
		
BIN = libzs5_64.so
LNK = libzs.so

all : $(BIN) 
	ln -s $(BIN) $(LNK)
	
$(BIN) : $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^
	
	
.PHONY : clean build rebuild
clean :
	@ -rm -rf 64
	@ -rm -f $(OBJ) $(BIN) $(LNK)
	
build :
	$(MAKE)	
	mkdir -p 64/include
	mkdir -p 64/lib
	ln -s $(BIN) $(LNK)
	cp *.h 64/include
	cp $(BIN) $(LNK) 64/lib	
	
	
rebuild :
	$(MAKE)	clean
	$(MAKE) build

