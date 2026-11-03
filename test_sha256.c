#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sha256.h"

static int failures;

static void to_hex(const uint8_t *digest, char *out)
{
    int i;
    for (i = 0; i < SHA256_DIGEST_SIZE; i++)
        sprintf(out + i * 2, "%02x", digest[i]);
}

static void check(const char *name, const uint8_t *digest, const char *expected)
{
    char hex[SHA256_DIGEST_SIZE * 2 + 1];
    to_hex(digest, hex);
    if (strcmp(hex, expected) == 0) {
        printf("ok    %s\n", name);
    } else {
        printf("FAIL  %s\n  got      %s\n  expected %s\n", name, hex, expected);
        failures++;
    }
}

static void check_str(const char *name, const char *msg, const char *expected)
{
    uint8_t digest[SHA256_DIGEST_SIZE];
    sha256((const uint8_t *)msg, strlen(msg), digest);
    check(name, digest, expected);
}

int main(void)
{
    uint8_t digest[SHA256_DIGEST_SIZE];
    sha256_ctx ctx;
    uint8_t chunk[1000];
    const char *long_msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    size_t i;

    check_str("empty", "",
              "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    check_str("abc", "abc",
              "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    check_str("448 bits", long_msg,
              "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    check_str("896 bits",
              "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno"
              "ijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
              "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1");

    /* Feed one byte at a time to exercise the buffering in sha256_update. */
    sha256_init(&ctx);
    for (i = 0; i < strlen(long_msg); i++)
        sha256_update(&ctx, (const uint8_t *)long_msg + i, 1);
    sha256_final(&ctx, digest);
    check("448 bits, byte at a time", digest,
          "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");

    memset(chunk, 'a', sizeof(chunk));
    sha256_init(&ctx);
    for (i = 0; i < 1000; i++)
        sha256_update(&ctx, chunk, sizeof(chunk));
    sha256_final(&ctx, digest);
    check("one million a", digest,
          "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");

    if (failures) {
        printf("%d test(s) failed\n", failures);
        return EXIT_FAILURE;
    }
    printf("all tests passed\n");
    return EXIT_SUCCESS;
}
