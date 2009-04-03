
# find the mono compiler
COMPILER=$(shell which smcs || which gmcs2 || which gmcs || which mcs)

LIBS=-r:System.Windows.Forms -r:System.Drawing -r:System.Data

SRCS=frmMain.cs BZFSPoller.cs

EXEC=BZFSLauncher.exe


all:	$(EXEC)

$(EXEC):	$(SRCS) Makefile
	@if [ -z $(COMPILER) ]; then \
		echo "Could not find the mono compiler"; \
		exit 1; \
	fi
	$(COMPILER) -out:$(EXEC) $(LIBS) $(SRCS)

clean:
	rm -f $(EXEC) *~

fresh:
	$(MAKE) clean
	$(MAKE) all
