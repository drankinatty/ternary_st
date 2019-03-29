# application names
TSTCODE := ternary_st
TESTCPY := tst_test_cpy
TESTREF := tst_test_ref
TESTVAL := tst_validate
# compiler
CC	:= gcc
CCLD    := $(CC)
# output/object/include/source directories
BINDIR  := bin
OBJDIR  := obj
INCLUDE	:= include
SRCDIR  := src
# compiler and linker flags
CFLAGS  := -Wall -Wextra -pedantic -finline-functions -std=c11 -Wshadow
CFLAGS	+= -I$(INCLUDE)
ifeq ($(debug),-DDEBUG)
CFLAGS  += -g
else
CFLAGS  += -Ofast
endif
LDFLAGS :=
# libraries
LIBS    :=
# # source/include/object variables
SOURCES	:= $(wildcard $(SRCDIR)/tst*.c)
INCLUDES := $(wildcard $(INCLUDE)/*.h)
OBJECTS := obj/$(TSTCODE).o

# all:	$(OBJECTS)
# 	@mkdir -p $(@D)/bin
# 	$(CCLD) -o $(BINDIR)/$(TESTCPY) $(SRCDIR)/$(TESTCPY).c obj/$(TSTCODE).o $(CFLAGS) $(LDFLAGS) $(LIBS)
# 	$(CCLD) -o $(BINDIR)/$(TESTREF) $(SRCDIR)/$(TESTREF).c obj/$(TSTCODE).o $(CFLAGS) $(LDFLAGS) $(LIBS)
# 	$(CCLD) -o $(BINDIR)/$(TESTVAL) $(SRCDIR)/$(TESTVAL).c obj/$(TSTCODE).o $(CFLAGS) $(LDFLAGS) $(LIBS)

all:    $(TESTCPY) $(TESTREF) $(TESTVAL)

# $(TESTCPY):     $(OBJECTS)
$(TESTCPY):
	@mkdir -p $(@D)/bin
	$(CCLD) -o $(BINDIR)/$(TESTCPY) $(SRCDIR)/$(TESTCPY).c obj/$(TSTCODE).o $(CFLAGS) $(LDFLAGS) $(LIBS)

# $(TESTREF):     $(OBJECTS)
$(TESTREF):
	@mkdir -p $(@D)/bin
	$(CCLD) -o $(BINDIR)/$(TESTREF) $(SRCDIR)/$(TESTREF).c obj/$(TSTCODE).o $(CFLAGS) $(LDFLAGS) $(LIBS)

# $(TESTVAL):     $(OBJECTS)
$(TESTVAL):
	@mkdir -p $(@D)/bin
	$(CCLD) -o $(BINDIR)/$(TESTVAL) $(SRCDIR)/$(TESTVAL).c obj/$(TSTCODE).o $(CFLAGS) $(LDFLAGS) $(LIBS)

# strip only if -DDEBUG not set
ifneq ($(debug),-DDEBUG)
	strip -s $(BINDIR)/*
endif

# define object file for $(TSTCODE)
$(OBJDIR)/$(TSTCODE).o:	$(INCLUDE)/$(TSTCODE).h

# create object dir/compile objects
$(OBJECTS):	$(OBJDIR)/$(TSTCODE).o $(SRCDIR)/$(TSTCODE).c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/$(TSTCODE).o $(SRCDIR)/$(TSTCODE).c

clean:
	rm -rf $(BINDIR) $(OBJDIR)
