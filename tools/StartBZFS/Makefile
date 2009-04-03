

.PHONY:	compiler_check

# find the mono compiler
COMPILER = $(shell     \
	which smcs  || \
	which gmcs2 || \
	which gmcs  || \
	which mcs)

LIBS = \
	-r:System.Windows.Forms \
	-r:System.Drawing \
	-r:System.Data

SRCS = $(shell /bin/ls -R *.cs)


EXEC = StartBZFS.exe



all:	$(EXEC)

compiler_check:
	@if [ -z $(COMPILER) ]; then \
		echo "Could not find the mono compiler"; \
		exit 1; \
	fi

$(EXEC):	compiler_check $(SRCS) Makefile
	$(COMPILER) -out:$(EXEC) $(LIBS) $(SRCS)

clean:
	rm -f $(EXEC) *~

fresh:
	$(MAKE) clean
	$(MAKE) all
