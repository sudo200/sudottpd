OUTDIR:=out
OUTFILE:=server

SRCDIR:=./src

DBG:="gdb"
MEMCHECK:="valgrind"

CX_FLAGS:=-I./include -g #-O2
C_FLAGS:=
CPP_FLAGS:=
LD_FLAGS:=-lpthread -lm

all:

	if [ -d "$(OUTDIR)" ]; then true; else mkdir -p "$(OUTDIR)" ; fi
	if ls $(SRCDIR) | grep '^.*\.cpp$$'; then $(CXX) $(CX_FLAGS) $(CPP_FLAGS) -c $(SRCDIR)/*.cpp; fi
	if ls $(SRCDIR) | grep '^.*\.c$$'; then $(CC) $(CX_FLAGS) $(C_FLAGS) -c $(SRCDIR)/*.c; fi
	if ls | grep '^.*\.o$$'; then $(CC) -o$(OUTDIR)/$(OUTFILE) $(LD_FLAGS) *.o; fi
	if ls | grep '^.*\.o$$'; then rm -rf *.o; fi
	if [ -f "$(OUTDIR)/$(OUTFILE)" ]; then objcopy --only-keep-debug "$(OUTDIR)/$(OUTFILE)" "$(OUTDIR)/$(OUTFILE).dbg"; fi
	if [ -f "$(OUTDIR)/$(OUTFILE).dbg" ]; then objcopy --strip-unneeded "$(OUTDIR)/$(OUTFILE)"; fi
	if [ -f "$(OUTDIR)/$(OUTFILE)" -a -f "$(OUTDIR)/$(OUTFILE).dbg" ]; then objcopy --add-gnu-debuglink="$(OUTDIR)/$(OUTFILE).dbg" "$(OUTDIR)/$(OUTFILE)" ; fi
	if [ -f "$(OUTDIR)/$(OUTFILE).dbg" ]; then chmod -x "$(OUTDIR)/$(OUTFILE).dbg"; fi

clean:
	rm -rf *.o
	rm -rf $(OUTDIR)

build: all

test: build
	"$(OUTDIR)/$(OUTFILE)"

debug: build
	$(DBG) "$(OUTDIR)/$(OUTFILE)"

memcheck: build
	$(MEMCHECK) "$(OUTDIR)/$(OUTFILE)"

.PHONY: clean build test memcheck

