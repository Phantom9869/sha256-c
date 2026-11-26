#!/bin/sh
# Check the CLI's help text and that a directory is refused.
set -e

fail=0
check() {
    name=$1
    shift
    if "$@"; then
        echo "ok    $name"
    else
        echo "FAIL  $name"
        fail=1
    fi
}

out=$(./sha256 --help)
check "help mentions Usage" sh -c "printf '%s\n' \"$out\" | grep -q '^Usage: sha256'"
out=$(./sha256 -h)
check "short help mentions Usage" sh -c "printf '%s\n' \"$out\" | grep -q '^Usage: sha256'"

# A directory must be refused with a clear error, not hashed as empty
# and not reported as a generic read error.
err=$(./sha256 . 2>&1) && { echo "FAIL  directory exit status was 0"; fail=1; } || true
check "directory mentions directory" sh -c "printf '%s\n' \"$err\" | grep -qi directory"
check "directory does not print a digest" sh -c "! printf '%s\n' \"$err\" | grep -q 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855'"

# Missing files still fail, and one bad path must not hide a later good one.
tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT
printf 'abc' > "$tmp"
err=$(./sha256 /no/such/sha256-c-missing "$tmp" 2>&1) && { echo "FAIL  missing file exit status was 0"; fail=1; } || true
check "missing file still errors" sh -c "printf '%s\n' \"$err\" | grep -q 'No such file'"
check "later file is still hashed" sh -c "printf '%s\n' \"$err\" | grep -q 'ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad'"

# A file whose name looks like an option is hashed only after --.
dir=$(mktemp -d)
trap 'rm -f "$tmp"; rm -rf "$dir"' EXIT
printf 'abc' > "$dir/--help"
out=$(./sha256 -- "$dir/--help")
check "hash a file named --help after --" sh -c "printf '%s\n' \"$out\" | grep -q 'ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad'"
# Without --, --help must still be help, not a file.
out=$(./sha256 --help "$dir/--help")
check "help still wins before --" sh -c "printf '%s\n' \"$out\" | grep -q '^Usage: sha256'"
check "help before -- does not hash" sh -c "! printf '%s\n' \"$out\" | grep -q 'ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad'"

# sha256 -- with no files reads stdin.
out=$(printf 'abc' | ./sha256 --)
check "dash-dash alone hashes stdin" sh -c "printf '%s\n' \"$out\" | grep -q 'ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad'"

# Unknown options are refused instead of being opened as filenames.
err=$(./sha256 -v 2>&1) && { echo "FAIL  unknown option exit status was 0"; fail=1; } || true
check "unknown option mentions the flag" sh -c "printf '%s\n' \"$err\" | grep -q 'unknown option: -v'"
check "unknown option prints usage" sh -c "printf '%s\n' \"$err\" | grep -q '^Usage: sha256'"
err=$(./sha256 --version 2>&1) && { echo "FAIL  --version exit status was 0"; fail=1; } || true
check "long unknown option is refused" sh -c "printf '%s\n' \"$err\" | grep -q 'unknown option: --version'"

# The same dash-prefixed name is hashed when it comes after --.
printf 'abc' > "$dir/-v"
out=$(./sha256 -- "$dir/-v")
check "hash a file named -v after --" sh -c "printf '%s\n' \"$out\" | grep -q 'ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad'"

exit $fail
