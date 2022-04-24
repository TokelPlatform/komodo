/******************************************************************************
 * Copyright © 2021 The SuperNET Developers.                             *
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

#include <iostream>
#include "CCupgrades.h"

namespace CCUpgrades {

    class CUpgradesContainer {
    private: 
        void addUpgradeActive(const std::string &chainName, UPGRADE_ID upgradeId, int32_t nHeight)
        {
            ChainUpgrades oUpgrade;
            oUpgrade.setActivationHeight(upgradeId, nHeight, UPGRADE_ACTIVE);
            mChainUpgrades[chainName] = oUpgrade;
        }
    public:
        CUpgradesContainer()  {

            // default upgrades: always enable all fixes
            defaultUpgrades.IsAllEnabled = true;

            // CCASSETS_OPDROP_VALIDATE_FIX activation
            addUpgradeActive("TOKEL", CCASSETS_OPDROP_VALIDATE_FIX, CCASSETS_OPDROP_FIX_TOKEL_HEIGHT);
            addUpgradeActive("TKLTEST", CCASSETS_OPDROP_VALIDATE_FIX, CCASSETS_OPDROP_FIX_TKLTEST_HEIGHT);

            // CCMIXEDMODE_SUBVER_1 activation
            addUpgradeActive("TOKEL", CCMIXEDMODE_SUBVER_1, CCMIXEDMODE_SUBVER_1_TOKEL_HEIGHT);
            addUpgradeActive("TKLTEST", CCMIXEDMODE_SUBVER_1, CCMIXEDMODE_SUBVER_1_TKLTEST_HEIGHT);
            addUpgradeActive("DIMXY24", CCMIXEDMODE_SUBVER_1, CCMIXEDMODE_SUBVER_1_DIMXY24_HEIGHT);
            addUpgradeActive("TKLTEST2", CCMIXEDMODE_SUBVER_1, CCMIXEDMODE_SUBVER_1_TKLTEST2_HEIGHT);

            // add more chains here...
            // ...
        }

    public:
        std::map<std::string, ChainUpgrades> mChainUpgrades;
        ChainUpgrades defaultUpgrades;
    } ccChainsUpgrades;

    static const ChainUpgrades *pSelectedUpgrades = &ccChainsUpgrades.defaultUpgrades;

    // return ref to chain upgrades list by chain name: 
    void SelectUpgrades(const std::string &chainName) {
        std::map<std::string, ChainUpgrades>::const_iterator it = ccChainsUpgrades.mChainUpgrades.find(chainName);
        if (it != ccChainsUpgrades.mChainUpgrades.end())  {
            pSelectedUpgrades = &it->second;
        }
        else {
            pSelectedUpgrades = &ccChainsUpgrades.defaultUpgrades;
        }
    }

    const ChainUpgrades &GetUpgrades()
    {
        return *pSelectedUpgrades;
    }


    bool IsUpgradeActive(int32_t nHeight, const ChainUpgrades &chainUpgrades, UPGRADE_ID id) {
        if (chainUpgrades.IsAllEnabled)
            return true;
        else {
            std::map<UPGRADE_ID, UpgradeInfo>::const_iterator it = chainUpgrades.mUpgrades.find(id);
            if (it != chainUpgrades.mUpgrades.end())
                return nHeight >= it->second.nActivationHeight ? it->second.status == UPGRADE_ACTIVE : false;
            return false;
        }
    }

}; // namespace CCUpgrades
