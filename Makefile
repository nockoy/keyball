# Keyball QMK Flash Makefile

# Default keymap (can be overridden)
KEYMAP ?= nockoy_no_aml

.PHONY: flash-keyball44 flash help

# Flash keyball44 with specified keymap
flash-keyball44:
	cd ~/qmk && qmk flash -kb keyball/keyball44 -km $(KEYMAP)

# Alternative target with shorter name
flash:
	cd ~/qmk && qmk flash -kb keyball/keyball44 -km $(KEYMAP)

help:
	@echo "Available targets:"
	@echo "  flash-keyball44  - Flash keyball44 with specified keymap (default: $(KEYMAP))"
	@echo "  flash            - Same as flash-keyball44 (shortcut)"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make flash                    # Use default keymap ($(KEYMAP))"
	@echo "  make flash KEYMAP=default     # Use default keymap"
	@echo "  make flash KEYMAP=via         # Use via keymap" 