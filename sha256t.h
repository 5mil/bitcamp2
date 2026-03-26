#ifndef SHA256T_H
#define SHA256T_H

#include <stdint.h>
#include <stddef.h>

void sha256t_hash(const uint8_t *header, size_t len, uint8_t out[32]);

#endif
