# Rules for Unix toolchain

INIT_DEFAULT_OUTPUT = Makefile
INIT_KEYWORDS       = unix

INIT_VAR_CXX    = g++
INIT_VAR_CC     = gcc
INIT_VAR_LD     = ld
INIT_VAR_STRIP  = strip
INIT_VAR_RM     = rm -f
INIT_VAR_AR     = ar
INIT_VAR_RANLIB = :


### Standard Application
# Files can be source files.
# Dependencies can be lib, dll, obj, subproject, prebuild-lib.
# Produces binary <name>.
begin app
OUTPUT_$(NAME) := $(NAME)
OBJECT_EXT     := o

$(OUTPUT): $(OBJECTS) $(OUTPUT_$(DEPEND)) $(LIBDEPEND)
	@echo "        Linking $(OUTPUT)..."
	@$(CXX) $(LDFLAGS) -o $(OUTPUT) $(OBJECTS) $(LINK_$(DEPEND)) $(LIBS)

clean:
	$(RM) $(OBJECTS)

distclean:
	$(RM) $(OUTPUT)

ALL_TARGETS += $(OUTPUT)
end


### Static Library
# Files can be source files.
# Dependencies can be obj, subproject.
# Produces library lib<name>.a.
begin lib
OUTPUT_$(NAME) := lib$(NAME).a
OBJECT_EXT     := o
LINK_$(NAME)   := -L. -l$(NAME)

$(OUTPUT): $(OBJECTS) $(OUTPUT_$(DEPEND))
	@echo "        Archiving $(OUTPUT)..."
	@$(AR) cru $(OUTPUT) $(OBJECTS) $(OUTPUT_$(DEPEND))

distclean:
	$(RM) $(OUTPUT)

clean:
	$(RM) $(OBJECTS)

ALL_TARGETS += $(OUTPUT)
end


### DLL
# Files can be source files.
# Dependencies can be obj, subproject.
# Produces library lib<name>.so.
begin dll
OUTPUT_$(NAME) := lib$(NAME).so
OBJECT_EXT     := lo
LINK_$(NAME)   := -L. -l$(NAME)

$(OUTPUT): $(OBJECTS) $(OUTPUT_$(DEPEND))
	@echo "        Linking $(OUTPUT)..."
	@$(CC) -o $(OUTPUT) -shared $(OBJECTS) $(OUTPUT_$(DEPEND))

distclean:
	$(RM) $(OUTPUT)

clean:
	$(RM) $(OBJECTS)

ALL_TARGETS += $(OUTPUT)
end


### Object file
# Files can be source files.
# Dependencies can be obj, subproject.
# Produces object file <name>.o.
begin obj
OUTPUT_$(NAME) := $(NAME).o
OBJECT_EXT     := o
LINK_$(NAME)   := $(OUTPUT)

$(OUTPUT): $(OBJECTS) $(OUTPUT_$(DEPEND))
	@echo "        Linking $(OUTPUT)..."
	@$(LD) -r -o $(OUTPUT) $(OBJECTS) $(OUTPUT_$(DEPEND))

distclean:
	$(RM) $(OUTPUT)

clean:
	$(RM) $(OBJECTS)

ALL_TARGETS += $(OUTPUT)
end


### Subproject
# Files can be source files.
# Dependencies can be obj, subproject.
# Produces nothing; instead, just passes this group of object files to its user.
begin subproject
OBJECT_EXT     := o
OUTPUT_$(NAME) := $(OBJECTS) $(OUTPUT_$(DEPEND))
LINK_$(NAME)   := $(OBJECTS) $(LINK_$(DEPEND))

clean:
	$(RM) $(OBJECTS)
end


### Prebuilt library
# Define LIBDIR to point to the library.
# Define LIBEXTRA with optional additional options.
begin prebuilt-lib
OUTPUT_$(NAME) :=
LINK_$(NAME) := -L$(LIBDIR) -l$(NAME) $(LIBEXTRA)

.PHONY: $(NAME)
end


### Basic Rules

# C compiler:
.c.o:
	@echo "        Compiling $<..."
	@$(CC) $(CFLAGS) -o $@ -c $<

.c.s:
	$(CC) $(CFLAGS) -o $@ -S $<

.c.lo:
	$(CC) -fPIC $(CXXFLAGS) -o $@ -c $<

# C++ compiler:
.cc.o:
	@echo "        Compiling $<..."
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

.cc.lo:
	$(CXX) -fPIC $(CXXFLAGS) -o $@ -c $<

.cpp.o:
	@echo "        Compiling $<..."
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

.cpp.lo:
	$(CXX) -fPIC $(CXXFLAGS) -o $@ -c $<

.cc.s:
	$(CXX) $(CXXFLAGS) -o $@ -S $<

.cpp.s:
	$(CXX) $(CXXFLAGS) -o $@ -S $<

# Administrativa:
distclean: clean

.PHONY: clean distclean
