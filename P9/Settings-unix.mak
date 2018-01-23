CXXFLAGS += -MMD -g -DTARGET_OS_POSIX
INSTDIR = /opt/c2ng
INSTALL_CMD  = $(PERL) scripts/install.pl -d -s $(INSTALL_FLAGS)
INSTALL_DATA = $(PERL) scripts/install.pl -d -m 644 $(INSTALL_FLAGS)

Makefile: $(PROJ_INPUTS)
        @echo "        Regenerating Makefile..."
        @proj9 update

# Use OBJECTS, not FILES, so we have only one file list (namely, OBJECTS) in the Makefile.
DEPFILES := $(OBJECTS_$(TARGETS))

depend.mk: Makefile
	@echo "        Regenerating depend.mk..."
	@for i in $(DEPFILES); do echo "-include $${i%o}d"; done > depend.mk

include depend.mk

# Install, main entry point
.PHONY: install
install: $(OUTPUT_$(PROJECTS_app))
	$(INSTALL_CMD) $(OUTPUT_$(PROJECTS_app)) $(INSTDIR)/bin
	$(MAKE) install-sdl-$(CONFIG_C2NG_HAVE_SDL)
	$(INSTALL_DATA) -R share $(INSTDIR)/share

# Installation of SDL apps, when enabled
.PHONY: install-sdl-yes
install-sdl-yes: $(OUTPUT_$(PROJECTS_guiapp))
	$(INSTALL_CMD) $(OUTPUT_$(PROJECTS_guiapp)) $(INSTDIR)/bin

# Skip installation of SDL apps when disabled
.PHONY: install-sdl-no
install-sdl-no:
