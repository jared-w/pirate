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
 * Copyright 2021 Two Six Labs, LLC.  All rights reserved.
 */

#pragma once

#include <iostream>

typedef enum TargetLanguage {
    UNKNOWN,
    C_LANG,
    CPP_LANG,
    DFDL_LANG
} target_t;

std::string target_as_string(TargetLanguage target);
int parse(std::istream &istream, std::ostream &ostream, std::ostream &estream, target_t target, bool packed);
