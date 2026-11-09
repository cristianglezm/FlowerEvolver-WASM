#!/usr/bin/env bash
# Fail immediately if a command errors or variables are unbound
set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <path_to_package.json> <path_to_pyproject.toml>" >&2
    exit 1
fi

PKG_JSON="$1"
PY_TOML="$2"

# Ensure both target files exist
if [ ! -f "$PKG_JSON" ] || [ ! -f "$PY_TOML" ]; then
    echo "Error: One or both files do not exist." >&2
    exit 1
fi

# 1. Parse JSON with node: it is already required by this repo, jq is not
NODE_VERSION=$(node -p "require(process.argv[1]).version || ''" "$(cd "$(dirname "$PKG_JSON")" && pwd)/$(basename "$PKG_JSON")")

# 2. Parse TOML version using MSYS2-native grep/sed
# Looks for version inside standard [project] or Poetry's [tool.poetry] definitions
PYTHON_VERSION=$(grep -E '^(version|\[project\]|\[tool\.poetry\])' "$PY_TOML" | \
    sed -n -E '/\[project\]/,/^\[/p;/\[tool\.poetry\]/,/^\[/p;/^version/p' | \
    grep '^version[[:space:]]*=' | head -n 1 | \
    sed -E 's/^version[[:space:]]*=[[:space:]]*"([^"]+)".*/\1/' | tr -d '[:space:]')

# Validation checks
if [ -z "$NODE_VERSION" ]; then
    echo "Error: 'version' field missing or null in $PKG_JSON" >&2
    exit 1
fi

if [ -z "$PYTHON_VERSION" ]; then
    echo "Error: 'version' field could not be found in $PY_TOML" >&2
    exit 1
fi

echo "Node Package Version   : $NODE_VERSION"
echo "Python Project Version : $PYTHON_VERSION"

# Perform string comparison
if [ "$NODE_VERSION" = "$PYTHON_VERSION" ]; then
    echo "Success: Versions match."
    exit 0
else
    echo "Failure: Version mismatch!" >&2
    exit 1
fi
