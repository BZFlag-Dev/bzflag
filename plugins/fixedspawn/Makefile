
# The flags needed to compile all
# 
CXXFLAGS = -fPIC -frtti -I../../include -O

# we need to link in the dll handling library

LDLIBS = -ldl

all : fixedspawn.so

# link the shared libraries with the -shared flag.

fixedspawn.so : fixedspawn.o
	$(CXX) $(CXXFLAGS) -shared -rdynamic  $^ -o $@	

clean :
	-rm *.so 
	
