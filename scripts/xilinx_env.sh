#!/usr/bin/env bash
# Source this helper; override the settings-file location on another workstation.
case "${1:-}" in
  vivado) ranc_settings=${VIVADO_SETTINGS:-/tools/Xilinx/Vivado/2024.2/settings64.sh} ;;
  vitis) ranc_settings=${VITIS_SETTINGS:-/tools/Xilinx/Vitis/2024.2/settings64.sh} ;;
  *) echo 'Expected tool: vivado or vitis' >&2; return 1 ;;
esac
if [ ! -f "$ranc_settings" ]; then
  echo "Missing $ranc_settings; set VIVADO_SETTINGS or VITIS_SETTINGS to your 2024.2 settings64.sh." >&2
  return 1
fi
source "$ranc_settings"
unset ranc_settings
