#!/bin/bash
# Generates the Visual Studio solution/project files (.sln + .vcxproj + .csproj)
# with premake. JetBrains Rider opens this .sln directly on macOS and Linux,
# giving full C# support (Coral.Managed, Shado-script-core, game projects) and
# navigation for the C++ engine.
#
# After running, open  shado-opengl-api.sln  in Rider.
#
# Note: to actually BUILD on macOS/Linux, use scripts/unix_compile_run_editor.sh
# (premake gmake2 + make + dotnet). The .sln is primarily for the IDE.
#
# Usage: scripts/unix_setup.sh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

command -v premake5 >/dev/null || {
  echo "premake5 not found. Install it (macOS: 'brew install premake', Linux: package manager or https://premake.github.io)." >&2
  exit 1
}

echo ">> Generating Visual Studio solution for Rider (premake vs2022)..."
premake5 vs2022

SLN="$(ls -1 "$ROOT_DIR"/*.sln 2>/dev/null | head -1 || true)"
if [[ -n "$SLN" ]]; then
  echo ">> Done. Open in Rider:"
  echo "     $SLN"
else
  echo ">> premake finished but no .sln was found in $ROOT_DIR" >&2
  exit 1
fi
