include version

.DEFAULT_GOAL:=all

OUTPUT_DIR 	= output/
REVISIONS 	= LIB
PLATFORMS 	= 86 64
SYSTEMS		= Linux

PLATFORM 	?= 64
SYSTEM 		?= Linux

.NOTPARALLEL:
.PHONY: all clean $(PLATFORMS) $(SYSTEMS)
all: $(PLATFORMS)

$(REVISIONS):
	@$(MAKE) --no-print-directory $(SYSTEMS) REVISION=$@
	@cd $(OUTPUT_DIR) ; tar -cJf apptesthash_$@_$(VERSION).tar.xz --exclude=apptesthash_$@_$(VERSION).tar.xz $@/*
	
$(SYSTEMS):
	@$(MAKE) --no-print-directory $(PLATFORMS) SYSTEM=$@ 

$(PLATFORMS):
	@$(MAKE) -B --no-print-directory -C src \
		-f $(SYSTEM).mk \
			REVISION=$(REVISION) \
			PLATFORM=$@ \
			clean all
	@mkdir -p $(OUTPUT_DIR)$(REVISION)/$(SYSTEM)/$@
	@rm -rf $(OUTPUT_DIR)$(REVISION)/$(SYSTEM)/$@/*
	@echo "Coping to $(OUTPUT_DIR)$(REVISION)/$(SYSTEM)/$@"
	@$(MAKE) --no-print-directory $(SYSTEM)_copy_$@ PLATFORM=$@	

Linux_copy_86 Linux_copy_64:
	@cp src/apptesthash \
		src/*.so \
		$(OUTPUT_DIR)$(REVISION)/$(SYSTEM)/$(PLATFORM)

clean:
	@$(MAKE) -C src -f Linux.mk clean
	@rm -rf $(OUTPUT_DIR)
