#include "sha256t.h"
#include "sha256.h"

void sha256t_hash(const uint8_t *header, size_t len, uint8_t out[32]) {
    uint8_t h1[32], h2[32];
    sha256(header, len, h1);
    sha256(h1, 32, h2);
    sha256(h2, 32, out);
}
