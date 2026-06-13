#!/bin/bash
# Script to build the Essentia library in the build directory.
#
# Prerequisites:
# - Ensure all Essentia dependencies are installed prior to running this script.
# - Python dependencies are not required for this build process.
#
# For more information on installing dependencies, refer to:
# https://essentia.upf.edu/installing.html#installing-dependencies-on-macos
#
# This script uses the Waf build system. For detailed documentation on Waf, visit:
# https://waf.io/book/

set -euo pipefail # safer bash
IFS=$'\n\t'

# ――― Helpers ――― ------------------------------------------------------------
EXEC_CMDS=() # collects the commands we run

run() {
  # Build a single‑line, shell‑quoted version of the command
  local cmd_str
  printf -v cmd_str '%q ' "$@" # quote+join with spaces
  cmd_str=${cmd_str% }         # trim trailing space

  echo ">> $cmd_str"      # one‑liner marker in log
  EXEC_CMDS+=("$cmd_str") # remember for the summary
  "$@"                    # execute the command
}

# ――― Locate project root ――― ------------------------------------------------
SCRIPT_DIR="$(
  cd -- "$(dirname "$0")" >/dev/null 2>&1
  pwd -P
)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# ――― Define configuration (previously in .env file) ――― ---------------------
EXTERNAL_DIR=external
ESSENTIA_DIR=${EXTERNAL_DIR}/essentia

# ――― Resolve paths ――― ------------------------------------------------------
ESSENTIA_PATH="$PROJECT_DIR/$ESSENTIA_DIR"
BUILD_PATH="$ESSENTIA_PATH/build"
INSTALL_PATH="$PROJECT_DIR/build_external/install"

# ――― Build steps ――― --------------------------------------------------------
echo "Building Essentia from: $ESSENTIA_PATH"
echo "Output directory      : $BUILD_PATH"
echo "Install directory     : $INSTALL_PATH"
mkdir -p "$BUILD_PATH"
cd "$ESSENTIA_PATH"

echo "Configuring Essentia build…"
run python3 waf configure \
  --build-static \
  --static-dependencies \
  --fft=KISS \
  --lightweight= \
  --out="$BUILD_PATH" \
  --prefix="$INSTALL_PATH"

echo "Building and installing Essentia…"
run python3 waf build --progress
run python3 waf install --progress

echo "Essentia build completed successfully!"

# Summary (paths + executed commands)
printf '\n%s\n' '---------- Build summary ----------'
printf 'Essentia source : %s\n' "$ESSENTIA_PATH"
printf 'Build directory : %s\n' "$BUILD_PATH"
printf 'Install prefix  : %s\n' "$INSTALL_PATH"
printf '\nCommands executed (%d):\n' "${#EXEC_CMDS[@]}"
for cmd in "${EXEC_CMDS[@]}"; do
  printf '  %s\n' "$cmd"
done
printf '%s\n' '---------- End of summary ----------'
