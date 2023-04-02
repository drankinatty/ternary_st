## application names
TSTCODE := ternary_st
LIBNAME := libternary_st
TESTCPY := tst_test_cpy
TESTREF := tst_test_ref
TESTVAL := tst_validate
## compiler
CC	:= gcc
CCLD    := $(CC)
## output/object/include/source directories
BINDIR  := bin
OBJDIR  := obj
INCLUDE	:= include
SRCDIR  := src
## compiler and linker flags
CFLAGS  := -Wall -Wextra -pedantic -finline-functions -std=c11 -Wshadow
CFLAGS	+= -I$(INCLUDE)
ifeq ($(debug),-DDEBUG)
  CFLAGS  += -g
else
  CFLAGS  += -Ofast
endif
LDFLAGS :=
## libraries
LIBS    :=
## source/include/object variables
SOURCES	:= $(wildcard $(SRCDIR)/tst*.c)
INCLUDES := $(wildcard $(INCLUDE)/*.h)
OBJECTS := $(OBJDIR)/$(TSTCODE).o

all:    $(TESTCPY) $(TESTREF) $(TESTVAL) $(LIBNAME)

$(TESTCPY):     $(OBJECTS)
	@mkdir -p $(@D)/$(BINDIR)
	$(CCLD) -o $(BINDIR)/$(TESTCPY) $(SRCDIR)/$(TESTCPY).c $(OBJDIR)/$(TSTCODE).o $(CFLAGS) $(LDFLAGS) $(LIBS)

$(TESTREF):     $(OBJECTS)
	@mkdir -p $(@D)/$(BINDIR)
	$(CCLD) -o $(BINDIR)/$(TESTREF) $(SRCDIR)/$(TESTREF).c $(OBJDIR)/$(TSTCODE).o $(CFLAGS) $(LDFLAGS) $(LIBS)

$(TESTVAL):     $(OBJECTS)
	@mkdir -p $(@D)/$(BINDIR)
	$(CCLD) -o $(BINDIR)/$(TESTVAL) $(SRCDIR)/$(TESTVAL).c $(OBJDIR)/$(TSTCODE).o $(CFLAGS) $(LDFLAGS) $(LIBS)

## strip only if -DDEBUG not set
ifneq ($(debug),-DDEBUG)
	strip -s $(BINDIR)/*
endif

$(LIBNAME):
	# call shared-object lib makefile
	$(MAKE) -f Makefile.lib

## define object file for $(TSTCODE)
$(OBJDIR)/$(TSTCODE).o:	$(INCLUDE)/$(TSTCODE).h

## create object dir/compile objects
$(OBJECTS):	$(SRCDIR)/$(TSTCODE).c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/$(TSTCODE).o $(SRCDIR)/$(TSTCODE).c

## install library
install:
	$(MAKE) -f Makefile.lib install

## clean source directory
clean:
	if [ -d $(BINDIR) ]; then rm -rf $(BINDIR); fi
	if [ -d $(OBJDIR) ]; then rm -rf $(OBJDIR); fi
	$(MAKE) -f Makefile.lib clean
