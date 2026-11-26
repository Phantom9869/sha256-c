#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "sha256.h"

static void usage(FILE *fp)
{
    fprintf(fp, "Usage: sha256 [FILE...]\n");
    fprintf(fp, "Print the SHA-256 digest of each FILE, or of standard input\n");
    fprintf(fp, "if no files are given. A FILE of - means standard input.\n");
}

static int hash_stream(FILE *fp, uint8_t digest[SHA256_DIGEST_SIZE])
{
    sha256_ctx ctx;
    uint8_t buf[8192];
    size_t n;

    sha256_init(&ctx);
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        sha256_update(&ctx, buf, n);
    if (ferror(fp))
        return -1;
    sha256_final(&ctx, digest);
    return 0;
}

static void print_digest(const uint8_t digest[SHA256_DIGEST_SIZE], const char *name)
{
    int i;
    for (i = 0; i < SHA256_DIGEST_SIZE; i++)
        printf("%02x", digest[i]);
    printf("  %s\n", name);
}

static int is_help(const char *arg)
{
    return strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0;
}

int main(int argc, char **argv)
{
    uint8_t digest[SHA256_DIGEST_SIZE];
    int status = 0;
    int i;

    /* Options stop at --, so a file named --help can be hashed as: sha256 -- --help */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0)
            break;
        if (is_help(argv[i])) {
            usage(stdout);
            return 0;
        }
        /* A lone "-" is stdin, not an option. Anything else starting with
           '-' would otherwise be treated as a filename and fail confusingly. */
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            fprintf(stderr, "sha256: unknown option: %s\n", argv[i]);
            usage(stderr);
            return 1;
        }
    }

    if (argc < 2) {
        if (hash_stream(stdin, digest) != 0) {
            perror("stdin");
            return 1;
        }
        print_digest(digest, "-");
        return 0;
    }

    {
        int end_opts = 0;
        int hashed = 0;

    for (i = 1; i < argc; i++) {
        FILE *fp;
        struct stat st;

        if (!end_opts && strcmp(argv[i], "--") == 0) {
            end_opts = 1;
            continue;
        }
        hashed++;

        if (strcmp(argv[i], "-") != 0) {
            if (stat(argv[i], &st) == 0 && S_ISDIR(st.st_mode)) {
                fprintf(stderr, "sha256: %s: Is a directory\n", argv[i]);
                status = 1;
                continue;
            }
        }

        fp = strcmp(argv[i], "-") == 0 ? stdin : fopen(argv[i], "rb");
        if (!fp) {
            fprintf(stderr, "sha256: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }
        if (hash_stream(fp, digest) != 0) {
            fprintf(stderr, "sha256: %s: read error\n", argv[i]);
            status = 1;
        } else {
            print_digest(digest, argv[i]);
        }
        if (fp != stdin)
            fclose(fp);
    }

        /* sha256 -- with no files after it reads standard input, like sha256sum. */
        if (hashed == 0) {
            if (hash_stream(stdin, digest) != 0) {
                perror("stdin");
                return 1;
            }
            print_digest(digest, "-");
        }
    }
    return status;
}
