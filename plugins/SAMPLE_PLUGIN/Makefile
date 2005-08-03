
# The flags needed to compile all
# 
CXXFLAGS = -fPIC -frtti -I../../include -O

# we need to link in the dll handling library

LDLIBS = -ldl

all : SAMPLE_PLUGIN.so

# link the shared libraries with the -shared flag.

SAMPLE_PLUGIN.so : SAMPLE_PLUGIN.o
	$(CXX) $(CXXFLAGS) -shared -rdynamic  $^ -o $@	

clean :
	-rm *.so 
	
