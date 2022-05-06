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

#ifndef CC_UPGRADES_H
#define CC_UPGRADES_H

#include <stdlib.h>
#include <string>
#include <map>
#include <vector>

#include "version.h"

namespace CCUpgrades  {

    // asset chain activation heights
    const int32_t CCASSETS_OPDROP_FIX_TOKEL_HEIGHT = 286359;  // 26 Nov + 90 days
    const int32_t CCASSETS_OPDROP_FIX_TKLTEST_HEIGHT = 243159;  // 26 Nov + 60 days

    const int32_t CCMIXEDMODE_SUBVER_1_TKLTEST_HEIGHT = 100000000;  // TBD
    const int32_t CCMIXEDMODE_SUBVER_1_TOKEL_HEIGHT   = 100000000;  // TBD
    const int32_t CCMIXEDMODE_SUBVER_1_DIMXY24_HEIGHT = 100000000;  // TBD
    const int32_t CCMIXEDMODE_SUBVER_1_DIMXY28_HEIGHT = 100000000;  // TBD
    const int32_t CCMIXEDMODE_SUBVER_1_DIMXY32_HEIGHT = 100000000;  // TBD
    const int32_t CCMIXEDMODE_SUBVER_1_TKLTEST2_HEIGHT = 57544;  // 25 apr 2022 4:10p.m
    // latest protocol version:
    const int     CCMIXEDMODE_SUBVER_1_PROTOCOL_VERSION = 170010; 
    // pre-upgrade protocol version:
    const int     CCOLDDEFAULT_PROTOCOL_VERSION = 170009;  
    const int     CCNEWCHAIN_PROTOCOL_VERSION = CCMIXEDMODE_SUBVER_1_PROTOCOL_VERSION;  

    enum UPGRADE_STATUS {
        UPGRADE_ACTIVE = 1,
    };

    enum UPGRADE_ID  {
        CCASSETS_INITIAL_CHAIN       = 0x00,
        CCASSETS_OPDROP_VALIDATE_FIX = 0x01,
        CCMIXEDMODE_SUBVER_1         = 0x02,  // new cc secp256k1 cond type and eval param, assets cc royalty fixes
    };

    struct UpgradeInfo {
        int32_t nActivationHeight;
        UPGRADE_STATUS status;
        int nProtocolVersion;   // used for disconnecting old nodes
    };

    class ChainUpgrades {
    public:
        ChainUpgrades() : defaultUpgrade({0, UPGRADE_ACTIVE, CCNEWCHAIN_PROTOCOL_VERSION}) { }
        void setActivationHeight(UPGRADE_ID upgId, int32_t nHeight, UPGRADE_STATUS upgStatus, int nProtocolVersion) {
            mUpgrades[upgId] = { nHeight, upgStatus, nProtocolVersion };
        }
        
    public:
        std::map<UPGRADE_ID, UpgradeInfo> mUpgrades;
        const UpgradeInfo defaultUpgrade;
    };

    void SelectUpgrades(const std::string &chainName);
    const ChainUpgrades &GetUpgrades();
    bool IsUpgradeActive(int32_t nHeight, const ChainUpgrades &chainUpgrades, UPGRADE_ID upgId);
    UpgradeInfo GetCurrentUpgradeInfo(int32_t nHeight, const ChainUpgrades &chainUpgrades);

}; // namespace CCUpgrades

#endif // #ifndef CC_UPGRADES_H

