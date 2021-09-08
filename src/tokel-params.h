/******************************************************************************
 * Copyright © 2014-2021 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

#ifndef TOKEL_PARAMS_H
#define TOKEL_PARAMS_H

#include <string>
#include "komodo_defs.h"
#include "amount.h"

// TOKEL params:
// -ac_name=TOKEL -ac_supply=100000000 -ac_eras=2 -ac_cbmaturity=1 -ac_reward=100000000,4250000000 -ac_end=80640,0 
// -ac_decay=0,77700000 -ac_halving=0,525600 -ac_cc=555 -ac_ccenable=236,245,246,247 -ac_adaptivepow=6 


const std::string TOKEL_AC_NAME = "TOKEL";
const CAmount TOKEL_AC_SUPPLY = 100000000LL;
const int TOKEL_AC_ERAS = 2;
const int TOKEL_AC_CBMATURITY = 1;
const std::string TOKEL_AC_REWARD = "100000000,4250000000";
const std::string TOKEL_AC_END = "80640,0";
const std::string TOKEL_AC_DECAY = "0,77700000";
const std::string TOKEL_AC_HALVING = "0,525600";
const std::string TOKEL_AC_CCENABLE = "236,245,246,247";
const int TOKEL_AC_ADAPTIVEPOW = 6;
const int TOKEL_AC_CC = 555;

#define TOKEL_DEFAULT_ADDRESSINDEX true
#define TOKEL_DEFAULT_SPENTINDEX true
#define TOKEL_DEFAULT_UNSPENTCCINDEX true
#define TOKEL_DEFAULT_NSPV true

inline bool IS_TOKEL_MAINCHAIN() { return ASSETCHAINS_SYMBOL == TOKEL_AC_NAME; }

#endif // TOKEL_PARAMS_H