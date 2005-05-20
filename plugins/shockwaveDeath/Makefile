
# The flags needed to compile all
CXXFLAGS = -fPIC -frtti -O

# we need to link in the dll handling library

LDLIBS = -ldl


all : shockwavedeath.so

# link the shared libraries with the -shared flag.

shockwavedeath.so : shockwaveDeath.o
	$(CPP) $(CXXFLAGS) -shared  $^ -o $@	

clean :
	-rm *.o 
	
realclean : clean
	-rm shockwavedeath *.so 
	






		
