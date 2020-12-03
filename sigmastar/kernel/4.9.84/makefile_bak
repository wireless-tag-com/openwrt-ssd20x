SSTAR_CHIP_FILE := '.sstar_chip.txt'

# workaround: make -c [folder] clean
# additional $(MAKEFLAGS) with 'w' passed
# use $(filter-out w,$(MAKEFLAGS)) instead of $(MAKEFLAGS)

%: force 
	@$(MAKE) -f Makefile $(filter-out w,$(MAKEFLAGS)) $@

%_defconfig: force
	@echo "Extract CHIP NAME ($(firstword $(subst _, ,$@))) to $(SSTAR_CHIP_FILE)"
	@if [[ "$(shell which git)" != "" ]];then if [[ "$(shell git ls-remote 2>&1 | grep fatal)" == "" ]];then git update-index --assume-unchanged $(SSTAR_CHIP_FILE);fi;fi
	@echo $(strip $(firstword $(subst _, ,$@))) > $(SSTAR_CHIP_FILE)
	@$(MAKE) -f Makefile $(filter-out w,$(MAKEFLAGS)) $@

GNUmakefile: force 
	@$(MAKE) -f Makefile $(filter-out w,$(MAKEFLAGS)) $(filter-out GNUmakefile,$@)

makefile:;

force:;
