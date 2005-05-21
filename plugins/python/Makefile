CXXFLAGS = -fPIC -frtti -I../../include -O
LDLIBS = -ldl

all:		python.so
python.so:	python.o
	$(CXX) $(CXXFLAGS) -shared -rdynamic  $^ -o $@

clean:
	rm -rf *.so
