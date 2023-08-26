#
# Simple top-level make file for Koko
#

# check installation prefix
ifndef PREFIX
  PREFIX = /usr/local
endif

ifndef DATAPREFIX
  DATAPREFIX = ${HOME}
endif	     

export PREFIX

# check variables for sub-makes
ifdef NATIVE
  export NATIVE
endif
ifdef DEBUG
  export DEBUG
endif
ifdef OS
  export OS
endif

.PHONY: all clean install

# build all the components
all:
	make -C ./Src
	make -C ./QtGui -f MakeGui koko-gui
	@echo
	@echo "*****************************************************************************"
	@echo
	@echo "It's all done !"
	@echo
	@echo "Optional next step:"
	@echo "     make strip          ( remove debugging information from koko-cli )"
	@echo
	@echo "Now install the programs with (requires superuser privileges):"
	@echo "     make install        ( full installation )"
	@echo "     make install-exec   ( install executables only )"
	@echo "     make install-conf   ( install system-wide configuration file )"
	@echo "     make install-data   ( install program data )"
	@echo
	@echo "     make clean          ( remove intermediate files )"
	@echo
	@echo "See INSTALL.md for details and for installation instructions as a user"
	@echo "without superuser privileges."
	@echo
	@echo "******************************************************************************"
	@echo

# install executables and system-wide config file
install: install-exec install-conf

# install executable files only
install-exec:
	@echo "Installing executable files ..."
	@echo "==============================="
	make -C ./Src install
	make -C ./QtGui -f MakeGui install
	install -D koko.desktop $(PREFIX)/share/applications/koko.desktop
	install -D ./QtGui/images/koko.png $(PREFIX)/share/icons/hicolor/512x512/apps/koko.png
	update-desktop-database

# install system-wide configuration file
install-conf:
	@echo "Installing system-wide configuration file"
	@echo "========================================="
	install --backup=numbered -m 644 ./kokorc /etc

# install data
install-data:
	mkdir -p $(DATAPREFIX)/KODS
	cd ./Libs && cp -R * $(DATAPREFIX)/KODS

# clean up
clean:
	make -C ./Src clean
	make -C ./QtGui -f MakeGui clean

# remove debugging symbols from koko-cli
strip:
	make -C ./Src strip
