#!/bin/bash
# Builds the ENTIRE Shado project (native C/C++ via premake+make, and the C#
# managed assemblies via dotnet) and launches the editor. Works on macOS and
# Linux.
#
# Usage: scripts/unix_compile_run_editor.sh [--clean] [--no-build]
set -euo pipefail

# Resolve the project root. This script lives in <root>/scripts/, so the root is
# its parent directory. All paths below are relative to the project root.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

CONFIG=release
DO_BUILD=1
DO_CLEAN=0
for arg in "$@"; do
  case "$arg" in
    --clean) DO_CLEAN=1 ;;
    --no-build) DO_BUILD=0 ;;
  esac
done

# --- Platform detection ------------------------------------------------------
UNAME_S="$(uname -s)"
UNAME_M="$(uname -m)"
case "$UNAME_S" in
  Darwin) SYS=macosx ;;
  Linux)  SYS=linux ;;
  *) echo "Unsupported platform: $UNAME_S" >&2; exit 1 ;;
esac
# Map the machine arch onto premake's %{cfg.architecture} token.
case "$UNAME_M" in
  arm64|aarch64) ARCH=AARCH64 ;;
  x86_64|amd64)  ARCH=x86_64 ;;
  *) ARCH="$UNAME_M" ;;
esac
ARCHDIR="Release-${SYS}-${ARCH}"

# --- .NET toolchain ----------------------------------------------------------
# Prefer a Homebrew .NET 9 on macOS; otherwise rely on a system dotnet in PATH.
if [[ "$SYS" == "macosx" ]]; then
  for p in /opt/homebrew/opt/dotnet@9 /usr/local/opt/dotnet@9; do
    [[ -d "$p/bin" ]] && export PATH="$p/bin:$PATH"
  done
fi
# Locate DOTNET_ROOT (the directory containing host/fxr/*/libhostfxr.*) if not
# already set. Coral also searches standard locations at runtime.
if [[ -z "${DOTNET_ROOT:-}" ]]; then
  for c in /opt/homebrew/opt/dotnet@9/libexec /usr/local/opt/dotnet@9/libexec \
           /usr/lib/dotnet /usr/share/dotnet /usr/local/share/dotnet "$HOME/.dotnet"; do
    if [[ -d "$c/host/fxr" ]]; then export DOTNET_ROOT="$c"; break; fi
  done
fi
export DOTNET_ROOT="${DOTNET_ROOT:-}"

# --- Paths -------------------------------------------------------------------
CORAL_MANAGED="shado-opengl-api/vendor/Coral/Coral.Managed"
SCRIPT_CORE="Shado-script-core"
EDITOR_DIR="bin/$ARCHDIR/shado-editor"

if [[ "$DO_BUILD" == "1" ]]; then
  command -v premake5 >/dev/null || { echo "premake5 not found (brew install premake / apt install premake)"; exit 1; }
  command -v dotnet   >/dev/null || { echo "dotnet not found (need the .NET 9 SDK)"; exit 1; }

  echo ">> Generating project files (premake gmake2)..."
  premake5 gmake2 >/dev/null

  if [[ "$DO_CLEAN" == "1" ]]; then
    echo ">> Cleaning..."; make config=$CONFIG clean >/dev/null || true
  fi

  echo ">> Building native (engine + editor + deps)..."
  make config=$CONFIG shado-editor

  echo ">> Building C# managed assemblies (dotnet)..."
  dotnet build "$CORAL_MANAGED/Coral.Managed.csproj" -c Release -o "$CORAL_MANAGED/bin-managed" --nologo -v q
  dotnet build "$SCRIPT_CORE/Shado-script-core.csproj" -c Release -o "$SCRIPT_CORE/bin-core" --nologo -v q

  echo ">> Deploying managed assemblies to $EDITOR_DIR/DotNet ..."
  mkdir -p "$EDITOR_DIR/DotNet"
  cp "$CORAL_MANAGED/bin-managed/Coral.Managed.dll" \
     "$CORAL_MANAGED/bin-managed/Coral.Managed.runtimeconfig.json" \
     "$CORAL_MANAGED/bin-managed/Coral.Managed.deps.json" \
     "$SCRIPT_CORE/bin-core/Shado-script-core.dll" \
     "$EDITOR_DIR/DotNet/"
fi

# Locate the editor binary (fall back to a search if the arch token differs).
EDITOR="$EDITOR_DIR/shado-editor"
if [[ ! -x "$EDITOR" ]]; then
  EDITOR="$(find bin -type f -name shado-editor -path '*Release-*' 2>/dev/null | head -1 || true)"
fi
if [[ -z "$EDITOR" || ! -x "$EDITOR" ]]; then
  echo "Editor binary not found. Build it first: scripts/unix_compile_run_editor.sh" >&2
  exit 1
fi

echo ">> Launching editor ($SYS/$ARCH): $EDITOR"
exec "$EDITOR"
