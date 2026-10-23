# shellcheck shell=bash
# Sourced by prep.sh and prep-wheel-deps.sh: the pinned versions of the dependencies (dependencies.env), and how
# a clone is brought to one. For one run the environment can name another tag or branch:
#   EVOAI_REF=master bash .devops/prep.sh native

_deps_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
_pinned() { sed -n "s/^$1=//p" "$_deps_dir/dependencies.env" | head -n 1 | tr -d '\r'; }
JSONBOX_REF="${JSONBOX_REF:-$(_pinned JSONBOX_REF)}"
EVOAI_REF="${EVOAI_REF:-$(_pinned EVOAI_REF)}"
if [ -z "$JSONBOX_REF" ] || [ -z "$EVOAI_REF" ]; then
    echo "$_deps_dir/dependencies.env has to pin JSONBOX_REF and EVOAI_REF" >&2
    exit 1
fi

# fetch_dependency <url> <directory> <ref>: afterwards the directory holds <ref>, cloned now or earlier. An
# existing clone (a CI cache, one made by hand) is moved to it as well: a pin that only applied to a fresh
# clone would be ignored everywhere a clone already exists.
fetch_dependency() {
    local url="$1" dir="$2" ref="$3"
    if [ ! -d "$dir" ]; then
        git -c advice.detachedHead=false clone --quiet --branch "$ref" --depth 1 "$url" "$dir"
    elif [ "$(git -C "$dir" describe --tags --exact-match 2>/dev/null)" != "$ref" ]; then
        echo "moving $dir to $ref"
        if git -C "$dir" fetch --quiet --depth 1 origin "refs/tags/$ref:refs/tags/$ref" 2>/dev/null; then
            git -C "$dir" -c advice.detachedHead=false checkout --quiet "$ref"
        else # not a tag: a branch, to try one
            git -C "$dir" fetch --quiet --depth 1 origin "$ref"
            git -C "$dir" -c advice.detachedHead=false checkout --quiet FETCH_HEAD
        fi
    fi
}
