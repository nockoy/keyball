# Keyball QMK Flash Makefile

.PHONY: flash-keyball44-nockoy

# Flash keyball44 with nockoy_no_aml keymap
flash-keyball44-nockoy:
	cd ~/qmk && qmk flash -kb keyball/keyball44 -km nockoy_no_aml

# Alternative target with shorter name
flash:
	cd ~/qmk && qmk flash -kb keyball/keyball44 -km nockoy_no_aml

help:
	@echo "Available targets:"
	@echo "  flash-keyball44-nockoy  - Flash keyball44 with nockoy_no_aml keymap"
	@echo "  flash                   - Same as flash-keyball44-nockoy (shortcut)"
	@echo "  help                    - Show this help message" 