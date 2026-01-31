#!/bin/zsh
cd "$(dirname "$0")"

echo "------------------------------------------"
echo "Running Post-Build: prepare-firmware.py"
echo "------------------------------------------"

python3 prepare-firmware.py
