CXXFLAGS += -MMD -g

Makefile: $(PROJ_INPUTS)
        @echo "        Regenerating Makefile..."
        @proj9 update

# Use OBJECTS, not FILES, so we have only one file list (namely, OBJECTS) in the Makefile.
DEPFILES := $(OBJECTS_$(TARGETS))

depend.mk: Makefile
	@echo "        Regenerating depend.mk..."
	@for i in $(DEPFILES); do echo "-include $${i%o}d"; done > depend.mk

include depend.mk
