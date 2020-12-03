_WD := scripts/sstar/release
_ROOT_DIR := $(subst $(_WD),,$(shell pwd))
KCONFIG_DIR := $(_ROOT_DIR)scripts/kconfig
MCONF_BIN := $(KCONFIG_DIR)/mconf
ARCH_DIR := $(_ROOT_DIR)/arch/arm/mach-sstar/
KCONFIG := $(_ROOT_DIR)$(_WD)/Kconfig
DEFAULT_CONFIG := $(_ROOT_DIR)$(_WD)/.config
RELEASE_SCRIPT := $(_ROOT_DIR)$(_WD)/rel.sh
# enumerate chip names from $(ARCH_DIR)
CHIP_LIST := $(sort $(shell find $(ARCH_DIR) -mindepth 1 -maxdepth 1 -type d -exec basename {} \; | xargs))

# Kconfig select item template
CHIP_NAME_TEMP := _CHIP_TEMPLATE_
ITEM_TEMPLATE := menuconfig $(CHIP_NAME_TEMP)\n\tbool \"SStar SoC platform $(CHIP_NAME_TEMP)\"\n\tdefault n\n
# replace template string with chip names
SELECT_ITEMS := $(foreach chip,$(CHIP_LIST),$(subst $(CHIP_NAME_TEMP),$(chip),$(ITEM_TEMPLATE)))

.PHONY: all debug autogen menuconfig release

all: autogen menuconfig release
	@echo "------------------------------"
	@echo " Cutomer release done!"
	@echo "------------------------------"

debug:
	@echo "------------------------------"
	@echo " DEBUG"
	@echo "------------------------------"
	@echo $(_WD)
	@echo $(_ROOT_DIR)
	@echo $(MCONF_BIN)
	@echo $(CHIP_LIST)
	@printf "$(SELECT_ITEMS)"

autogen:
	@echo "------------------------------"
	@echo " Auto generate Kconfig..."
	@echo "------------------------------"
	@rm -f $(KCONFIG)
	@rm -f $(DEFAULT_CONFIG)
	@echo "mainmenu \"Customer Release Chips Configuration\"" > $(KCONFIG)
	@printf "$(SELECT_ITEMS)" >> $(KCONFIG)
	@cat $(KCONFIG)

menuconfig:
	@$(MCONF_BIN) $(KCONFIG)

release:
	@echo "------------------------------"
	@echo " Execute customer release..."
	@echo "------------------------------"
	@$(RELEASE_SCRIPT) $(shell cat .config | grep -e '^# CONFIG_' | sed 's/^# CONFIG_\(.*\) is not set/\1/g') 

