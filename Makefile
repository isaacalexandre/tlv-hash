include version

.DEFAULT_GOAL:=all

OUTPUT_DIR 	= output/
PLATFORMS 	= 64 86

PLATFORM 	?= 64

.NOTPARALLEL:
.PHONY: all clean $(PLATFORMS)
all: $(PLATFORMS)

$(PLATFORMS):
	@$(MAKE) -B --no-print-directory -C src -f Linux.mk PLATFORM=$@ clean all
	@mkdir -p $(OUTPUT_DIR)Linux/$@
	@rm -rf $(OUTPUT_DIR)Linux/$@/*
	@echo "Coping to $(OUTPUT_DIR)Linux/$@"
	@cp src/apptesthash src/*.so $(OUTPUT_DIR)Linux/$@

clean:
	@$(MAKE) -C src -f Linux.mk clean
	@rm -rf $(OUTPUT_DIR)
