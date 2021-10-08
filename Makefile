include version

.DEFAULT_GOAL:=all

OUTPUT_DIR 	= output/
PLATFORMS 	= 86 64
SYSTEMS		= Linux

PLATFORM 	?= 86
SYSTEM 		?= Linux

.NOTPARALLEL:
.PHONY: all clean $(PLATFORMS)
all: $(PLATFORMS)

$(PLATFORMS):
	@$(MAKE) -B --no-print-directory -C src \
		-f $(SYSTEM).mk \
			PLATFORM=$@ \
			clean all
	@mkdir -p $(OUTPUT_DIR)$(SYSTEM)/$@
	@rm -rf $(OUTPUT_DIR)$(SYSTEM)/$@/*
	@echo "Coping to $(OUTPUT_DIR)$(SYSTEM)/$@"
	@$(MAKE) --no-print-directory $(SYSTEM)_$@ PLATFORM=$@	
	@cp src/apptesthash \
		src/*.so \
		$(OUTPUT_DIR)$(SYSTEM)/$(PLATFORM)

clean:
	@$(MAKE) -C src -f Linux.mk clean
	@rm -rf $(OUTPUT_DIR)
