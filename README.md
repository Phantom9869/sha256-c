# sha256-c

SHA-256 written from scratch in C99, following FIPS 180-4. No dependencies.

## Build

```
make
./sha256 file.txt
echo -n abc | ./sha256
```

Output matches the format of `sha256sum`.

## Tests

```
make test
```

This runs the NIST test vectors (empty string, "abc", the 448 and 896 bit
messages, one million 'a') and then compares `./sha256` with the system
`sha256sum` on a few files, including sizes around the 55/56/64 byte padding
edge.

To use the hash in other code, include `sha256.h` and call `sha256()` for a
single buffer, or `sha256_init` / `sha256_update` / `sha256_final` for
streaming input.
