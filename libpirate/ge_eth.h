/*
 * This work was authored by Two Six Labs, LLC and is sponsored by a subcontract
 * agreement with Galois, Inc.  This material is based upon work supported by
 * the Defense Advanced Research Projects Agency (DARPA) under Contract No.
 * HR0011-19-C-0103.
 *
 * The Government has unlimited rights to use, modify, reproduce, release,
 * perform, display, or disclose computer software or computer software
 * documentation marked with this legend. Any reproduction of technical data,
 * computer software, or portions thereof marked with this legend must also
 * reproduce this marking.
 *
 * Copyright 2020 Two Six Labs, LLC.  All rights reserved.
 */

#ifndef __PIRATE_CHANNEL_GE_ETH_H
#define __PIRATE_CHANNEL_GE_ETH_H

#include "libpirate.h"

typedef struct {
    int sock;
    uint8_t *buf;
} ge_eth_ctx;

int pirate_ge_eth_parse_param(char *str, pirate_ge_eth_param_t *param);
int pirate_ge_eth_open(int gd, int flags, pirate_ge_eth_param_t *param, ge_eth_ctx *ctx);
int pirate_ge_eth_close(ge_eth_ctx *ctx);
ssize_t pirate_ge_eth_read(pirate_ge_eth_param_t *param, ge_eth_ctx *ctx, void *buf, size_t count);
ssize_t pirate_ge_eth_write(pirate_ge_eth_param_t *param, ge_eth_ctx *ctx, const void *buf, size_t count);

#endif /* __PIRATE_CHANNEL_GE_ETH_H */