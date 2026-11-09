#!/usr/bin/env bash
set -euo pipefail

TRACKED_HOOK="${1:?usage: $0 <tracked_hook> [git_hook_target]}"

# no .git (npm tarball, source archive): nothing to install into
if ! GIT_HOOK_TARGET="${2:-$(git rev-parse --git-path hooks/pre-commit 2>/dev/null)}" || [ -z "$GIT_HOOK_TARGET" ]; then
    exit 0
fi
mkdir -p "$(dirname "$GIT_HOOK_TARGET")"

# Calculate SHA512 hash of a file
get_file_hash() {
    local file_path="$1"
    # Extract only the hash string, ignore the filename output
    sha512sum "$file_path" | awk '{print $1}'
}

# Install or update the git hook cleanly
sync_git_hook() {
    local source_file="$1"
    local target_file="$2"

    # 1. Edge Case: Tracked file is missing from the repository clone
    if [ ! -f "$source_file" ]; then
        echo "Error: Source hook file not found at $source_file" >&2
        return 1
    fi

    # 2. Case: Hook doesn't exist locally yet
    if [ ! -f "$target_file" ]; then
        echo "Installing fresh pre-commit Git hook..."
        cp "$source_file" "$target_file"
        chmod +x "$target_file"
        return 0
    fi

    # 3. Case: Hook exists, check if hashes match
    local source_hash
    local target_hash
    source_hash=$(get_file_hash "$source_file")
    target_hash=$(get_file_hash "$target_file")

    if [ "$source_hash" != "$target_hash" ]; then
        echo "Tracked hook has updates. Updating your local Git hook..."
        cp "$source_file" "$target_file"
        chmod +x "$target_file"
    else
        echo "Pre-commit hook is up to date."
    fi
}

# Execute the hook sync
sync_git_hook "$TRACKED_HOOK" "$GIT_HOOK_TARGET"
