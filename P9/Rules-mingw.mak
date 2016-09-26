# Rules for MinGW toolchain

INIT_DEFAULT_OUTPUT = Makefile.min
INIT_KEYWORDS       = win32 mingw

INIT_VAR_CXX    = g++
INIT_VAR_CC     = gcc
INIT_VAR_STRIP  = strip
INIT_VAR_RM     = rm -f
INIT_VAR_AR     = ar
INIT_VAR_RANLIB = :


### Standard Application
begin app
OUTPUT_$(NAME) := $(NAME).exe
OBJECT_EXT     := o

$(OUTPUT): $(OBJECTS) $(OUTPUT_$(DEPEND)) $(LIBDEPEND)
        $(CXX) $(LDFLAGS) $(LINKFLAGS_$(DEPEND)) -o $(OUTPUT) $(OBJECTS) $(LINK_$(DEPEND)) $(LIBS)
        $(STRIP) $(OUTPUT)

clean:
	$(RM) $(OBJECTS)

distclean:
        $(RM) $(OUTPUT)

ALL_TARGETS += $(OUTPUT)
end


### Static Library
begin lib
OUTPUT_$(NAME) := lib$(NAME).a
OBJECT_EXT     := o
LINK_$(NAME)   := $(OUTPUT)

$(OUTPUT): $(OBJECTS) $(OUTPUT_$(DEPEND))
        $(AR) cr $(OUTPUT) $(OBJECTS) $(OUTPUT_$(DEPEND))
        $(RANLIB) $(OUTPUT)

distclean:
        $(RM) $(OUTPUT)

clean:
	$(RM) $(OBJECTS)

ALL_TARGETS += $(OUTPUT)
end


### Subproject
begin subproject
OBJECT_EXT     := o
OUTPUT_$(NAME) := $(OBJECTS) $(OUTPUT_$(DEPEND))
LINK_$(NAME)   := $(OBJECTS) $(LINK_$(DEPEND))

clean:
        $(RM) $(OBJECTS)
end


### Prebuilt library
# Note: the use of "LINKFLAGS" is a special-purpose hack. We should
# have a way to postprocess "LINK" somehow to separate options and
# libraries for compilers that need it.
# FIXME: do we still need this hack? If so, we should add it to the Unix toolchain as well.
begin prebuilt-lib
OUTPUT_$(NAME) :=
LINK_$(NAME) := -l$(NAME) $(LIBEXTRA)
LINKFLAGS_$(NAME) := -L$(LIBDIR)
end


### Basic Rules

# C compiler:
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.c.s:
	$(CC) $(CFLAGS) -o $@ -S $<

# C++ compiler:
.cc.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cc.s:
	$(CXX) $(CXXFLAGS) -o $@ -S $<

.cpp.s:
	$(CXX) $(CXXFLAGS) -o $@ -S $<

# Administrativa:
distclean: clean

.PHONY: all clean distclean
