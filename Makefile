OUTFILE:="server"

SRCDIR:=./src

DBG:="gdb"
MEMCHECK:="valgrind"

CX_FLAGS:=-I./include -O2
C_FLAGS:=
CPP_FLAGS:=
LD_FLAGS:=-lpthread -lm

all:

	if ls $(SRCDIR) | grep '^.*\.cpp$$'; then $(CXX) $(CX_FLAGS) $(CPP_FLAGS) -c $(SRCDIR)/*.cpp; fi
	if ls $(SRCDIR) | grep '^.*\.c$$'; then $(CC) $(CX_FLAGS) $(C_FLAGS) -c $(SRCDIR)/*.c; fi
	if ls | grep '^.*\.o$$'; then $(CC) -o$(OUTFILE) $(LD_FLAGS) *.o; fi
	if ls | grep '^.*\.o$$'; then rm -rf *.o; fi

clean:
	rm -rf *.o
	rm -rf $(OUTFILE)

build: all

test: build
	./$(OUTFILE)

debug: build
	$(DBG) ./$(OUTFILE)

memcheck: build
	$(MEMCHECK) ./$(OUTFILE)

.PHONY: clean build test memcheck

