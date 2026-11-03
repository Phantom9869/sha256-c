#!/bin/sh
# Compare ./sha256 against the system sha256sum on a few inputs.
set -e

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

: > "$tmp/empty"
printf 'abc' > "$tmp/abc"
printf 'hello world\n' > "$tmp/hello"
head -c 55 /dev/urandom > "$tmp/55"
head -c 56 /dev/urandom > "$tmp/56"
head -c 64 /dev/urandom > "$tmp/64"
head -c 100000 /dev/urandom > "$tmp/random"

fail=0
for f in "$tmp"/*; do
    ours=$(./sha256 "$f" | cut -d' ' -f1)
    theirs=$(sha256sum "$f" | cut -d' ' -f1)
    if [ "$ours" = "$theirs" ]; then
        echo "ok    $(basename "$f")"
    else
        echo "FAIL  $(basename "$f"): $ours != $theirs"
        fail=1
    fi
done

ours=$(printf 'abc' | ./sha256 | cut -d' ' -f1)
theirs=$(printf 'abc' | sha256sum | cut -d' ' -f1)
[ "$ours" = "$theirs" ] && echo "ok    stdin" || { echo "FAIL  stdin"; fail=1; }

exit $fail
