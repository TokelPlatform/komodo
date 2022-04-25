/******************************************************************************
* Copyright � 2014-2019 The SuperNET Developers.                             *
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

// encode decode tokens opret 
// make token cryptoconditions and vouts
// This code was moved to a separate source file to enable linking libcommon.so (with importcoin.cpp which depends on some token functions)

#include "key_io.h"
#include "CCtokens.h"

CScript EncodeTokenCreateOpRetV1(const std::vector<uint8_t> &origpubkey, const std::string &name, const std::string &description, const std::vector<vscript_t> &oprets)
{        
    CScript opret;
    uint8_t evalcode = EVAL_TOKENS;
    uint8_t funcid = 'C'; // 'C' indicates opreturn version 1 (with the version field)
    uint8_t version = TOKENS_OPRETURN_VERSION;

    opret << OP_RETURN << E_MARSHAL(ss << evalcode << funcid << version << origpubkey << name << description;
    for (const auto &o : oprets) {
        ss << o;
    });
    return(opret);
}

CScript EncodeTokenCreateOpRetV2(const std::vector<uint8_t> &origpubkey, const std::string &name, const std::string &description, const std::vector<vscript_t> &oprets)
{        
    CScript opret;
    uint8_t evalcode = EVAL_TOKENSV2;
    uint8_t funcid = 'c'; 
    uint8_t version = TOKENS_OPRETURN_VERSION;

    opret << OP_RETURN << E_MARSHAL(ss << evalcode << funcid << version << origpubkey << name << description;
    for (const auto &o : oprets) {
        ss << o;
    });
    return(opret);
}

// v1 format with no opretid (evalcode is used instead)
CScript EncodeTokenOpRetV1(uint256 tokenid, const std::vector<CPubKey> &voutPubkeys, const std::vector<vscript_t> &oprets)
{
    CScript opret;
    uint8_t tokenFuncId = 'T'; // 'T' indicates opreturn version 1 (with the version field)
    uint8_t evalCodeInOpret = EVAL_TOKENS;
    uint8_t version = TOKENS_OPRETURN_VERSION;

    tokenid = revuint256(tokenid);

    uint8_t pkCount = voutPubkeys.size();
    if (voutPubkeys.size() > 2)
    {    
        pkCount = 2;
        LOGSTREAM(cctokens_log, CCLOG_DEBUG2, stream << "EncodeTokenOpRet voutPubkeys.size()=" << voutPubkeys.size() << " not supported" << std::endl);
    }

    opret << OP_RETURN << E_MARSHAL(ss << evalCodeInOpret << tokenFuncId << version << tokenid << pkCount;
                            if (pkCount >= 1) ss << voutPubkeys[0];
                            if (pkCount == 2) ss << voutPubkeys[1];
                            for (const auto &o : oprets) {
                                ss << o;
                            });

    return opret;
}

// tokens 2 opret
CScript EncodeTokenOpRetV2(uint256 tokenid, const std::vector<vscript_t> &oprets)
{
    CScript opret;
    uint8_t tokenFuncId = 't'; 
    uint8_t evalCodeInOpret = EVAL_TOKENSV2;
    uint8_t version = TOKENS_OPRETURN_VERSION;

    tokenid = revuint256(tokenid);

    opret << OP_RETURN << E_MARSHAL(ss << evalCodeInOpret << tokenFuncId << version << tokenid;
                            for (const auto &o : oprets) {
                                ss << o;
                            });

    return opret;
}


// overload for fungible tokens (no additional data in opret):
uint8_t DecodeTokenCreateOpRetV1(const CScript &scriptPubKey, std::vector<uint8_t> &origpubkey, std::string &name, std::string &description) 
{
    std::vector<vscript_t>  opretsDummy;
    return DecodeTokenCreateOpRetV1(scriptPubKey, origpubkey, name, description, opretsDummy);
}

uint8_t DecodeTokenCreateOpRetV1(const CScript &scriptPubKey, std::vector<uint8_t> &origpubkey, std::string &name, std::string &description, std::vector<vscript_t> &oprets)
{
    vscript_t vopret, vblob;
    uint8_t dummyEvalcode, funcid, version;

    oprets.clear();

    GetOpReturnData(scriptPubKey, vopret);
    if (vopret.size() > 2 && vopret[0] == EVAL_TOKENS && vopret[1] == 'C')
    {
        if (E_UNMARSHAL(vopret, ss >> dummyEvalcode; ss >> funcid; ss >> version; ss >> origpubkey; ss >> name; ss >> description;
            while (!ss.eof()) {
                ss >> vblob;
                oprets.push_back(vblob);     // put oprets               
            }))
        {
            return 'c'; // convert to old-style funcid
        }   
    }
    LOGSTREAMFN(cctokens_log, CCLOG_DEBUG1, stream << "incorrect token create opret" << std::endl);
    return (uint8_t)0;
}

// decode token opret for mixed mode cc
uint8_t DecodeTokenCreateOpRetV2(const CScript &scriptPubKey, std::vector<uint8_t> &origpubkey, std::string &name, std::string &description, std::vector<vscript_t> &oprets)
{
    vscript_t vopret, vblob;
    uint8_t dummyEvalcode, funcid, version;

    oprets.clear();


    GetOpReturnData(scriptPubKey, vopret);
    if (vopret.size() >= 3 && vopret[0] == EVAL_TOKENSV2 && vopret[1] == 'c' && vopret[2] == 1)
    {
        if (E_UNMARSHAL(vopret, ss >> dummyEvalcode; ss >> funcid; ss >> version; ss >> origpubkey; ss >> name; ss >> description;
            while (!ss.eof()) {
                ss >> vblob;
                oprets.push_back(vblob);     // put other cc modules' oprets               
            }))
        {
            return funcid; 
        }   
    }
    LOGSTREAMFN(cctokens_log, CCLOG_DEBUG1, stream << "incorrect token2 create opret" << std::endl);
    return (uint8_t)0;
}

// decode token opret: 
// for 't' returns all data from opret, vopretExtra contains other contract's data (currently only assets'). 
// for 'c' returns only funcid. NOTE: nonfungible data is not returned
uint8_t DecodeTokenOpRetV1(const CScript scriptPubKey, uint256 &tokenid, std::vector<CPubKey> &voutPubkeys, std::vector<vscript_t>  &oprets)
{
    vscript_t vopret, vblob, vorigPubkey, vnonfungibleDummy;
    uint8_t funcId = 0, evalCode, dummyEvalCode, evalCodeOld, dummyFuncId, pkCount, version;
    std::string dummyName; std::string dummyDescription;
    uint256 dummySrcTokenId;
    CPubKey voutPubkey1, voutPubkey2;

    oprets.clear();

    GetOpReturnData(scriptPubKey, vopret);
    // tokenid = zeroid; this was incorrect: cleared the passed tokenid if creation tx

    if (vopret.size() > 2)
    {
        voutPubkeys.clear();
        evalCode = vopret[0];
        if (evalCode != EVAL_TOKENS) {
            LOGSTREAMFN(cctokens_log, CCLOG_INFO, stream << "incorrect evalcode in tokens opret" << std::endl);
            return (uint8_t)0;
        }

        funcId = vopret[1];
        LOGSTREAMFN(cctokens_log, CCLOG_DEBUG2, stream << "decoded funcId=" << (char)(funcId ? funcId : ' ') << std::endl);

        switch (funcId)
        {
        case 'C': 
            funcId = DecodeTokenCreateOpRetV1(scriptPubKey, vorigPubkey, dummyName, dummyDescription, oprets);
            if (funcId != 0)    {
                // add orig pubkey
                voutPubkeys.push_back(pubkey2pk(vorigPubkey));
            }
            return funcId;  // should be converted to old-style funcid

        case 'T':           
            if (E_UNMARSHAL(vopret, ss >> dummyEvalCode; ss >> dummyFuncId; ss >> version; ss >> tokenid; ss >> pkCount;
                    if (pkCount >= 1) ss >> voutPubkey1;
                    if (pkCount >= 2) ss >> voutPubkey2;  // pkCountshould not be > 2
                    while (!ss.eof()) {
                        ss >> vblob;
                        oprets.push_back(vblob);
                    }))
            {
                tokenid = revuint256(tokenid);
                if (voutPubkey1.IsValid())
                    voutPubkeys.push_back(voutPubkey1);
                if (voutPubkey2.IsValid())
                    voutPubkeys.push_back(voutPubkey2);
                return 't'; // convert to old style funcid
            }
            LOGSTREAMFN(cctokens_log, CCLOG_INFO, stream << "bad opret format for 'T'," << " pkCount=" << (int)pkCount << " tokenid=" <<  revuint256(tokenid).GetHex() << std::endl);
            return (uint8_t)0;
            
        default:
            LOGSTREAMFN(cctokens_log, CCLOG_INFO, stream << "illegal funcid=" << (int)funcId << std::endl);
            return (uint8_t)0;
        }
    }
    else {
        LOGSTREAMFN(cctokens_log, CCLOG_INFO, stream << "empty opret, could not parse" << std::endl);
    }
    return (uint8_t)0;
}

// decode token v2 (mixed mode cc) opret: 
// for 't' returns all data from opret, vopretExtra contains other contract's data (currently only assets'). 
// for 'c' returns only funcid. NOTE: nonfungible data is not returned
uint8_t DecodeTokenOpRetV2(const CScript scriptPubKey, uint256 &tokenid, std::vector<vscript_t> &oprets)
{
    vscript_t vopret, vblob, vorigPubkey, vnonfungibleDummy;
    uint8_t funcId = 0, evalCode, version;
    std::string dummyName; std::string dummyDescription;

    oprets.clear();

    GetOpReturnData(scriptPubKey, vopret);
    // tokenid = zeroid; this was incorrect: cleared the passed tokenid if creation tx

    if (vopret.size() >= 3)
    {
        evalCode = vopret[0];
        funcId = vopret[1];
        version = vopret[2];

        if (evalCode != EVAL_TOKENSV2) {
            LOGSTREAMFN(cctokens_log, CCLOG_DEBUG1, stream << "incorrect evalcode in tokens v2 opret" << std::endl);
            return (uint8_t)0;
        }
        if (version != TOKENS_OPRETURN_VERSION) {
            LOGSTREAMFN(cctokens_log, CCLOG_DEBUG1, stream << "incorrect version in tokens v2 opret" << std::endl);
            return (uint8_t)0;
        }
        switch (funcId)
        {
        case 'c': 
            funcId = DecodeTokenCreateOpRetV2(scriptPubKey, vorigPubkey, dummyName, dummyDescription, oprets);
            return funcId;  // should be converted to old-style funcid

        case 't':           
            if (E_UNMARSHAL(vopret, ss >> evalCode; ss >> funcId; ss >> version; ss >> tokenid; 
                    while (!ss.eof()) {
                        ss >> vblob;
                        oprets.push_back(vblob);
                    }))
            {
                tokenid = revuint256(tokenid);
                return 't'; 
            }
            LOGSTREAMFN(cctokens_log, CCLOG_DEBUG1, stream << "bad opret format for 't' tokenid=" <<  revuint256(tokenid).GetHex() << std::endl);
            return (uint8_t)0;
            
        default:
            LOGSTREAMFN(cctokens_log, CCLOG_DEBUG1, stream << "illegal funcid=" << (int)funcId << std::endl);
            return (uint8_t)0;
        }
    }
    else {
        LOGSTREAMFN(cctokens_log, CCLOG_DEBUG1, stream << "empty token opret" << std::endl);
    }
    return (uint8_t)0;
}




// make three-eval (token+evalcode+evalcode2) 1of2 cryptocondition:
CC *MakeTokensCCcond1of2(uint8_t evalcode, uint8_t evalcode2, CPubKey pk1, CPubKey pk2)
{
    // make 1of2 sigs cond 
    std::vector<CC*> pks;
    pks.push_back(CCNewSecp256k1(pk1));
    pks.push_back(CCNewSecp256k1(pk2));

    std::vector<CC*> thresholds;
    if (evalcode != 0)
        thresholds.push_back(CCNewEval(E_MARSHAL(ss << evalcode)));
    if (evalcode != EVAL_TOKENS)	                                                // if evalCode == EVAL_TOKENS, it is actually MakeCCcond1of2()!
        thresholds.push_back(CCNewEval(E_MARSHAL(ss << (uint8_t)EVAL_TOKENS)));	    // this is eval token cc
    if (evalcode2 != 0)
        thresholds.push_back(CCNewEval(E_MARSHAL(ss << evalcode2)));                // add optional additional evalcode
    thresholds.push_back(CCNewThreshold(1, pks));		                            // this is 1 of 2 sigs cc

    return CCNewThreshold(thresholds.size(), thresholds);
}
// overload to make two-eval (token+evalcode) 1of2 cryptocondition:
CC *MakeTokensCCcond1of2(uint8_t evalcode, CPubKey pk1, CPubKey pk2) {
    return MakeTokensCCcond1of2(evalcode, 0, pk1, pk2);
}

// make three-eval (token+evalcode+evalcode2) cryptocondition:
CC *MakeTokensCCcond1(uint8_t evalcode, uint8_t evalcode2, CPubKey pk)
{
    std::vector<CC*> pks;
    pks.push_back(CCNewSecp256k1(pk));

    std::vector<CC*> thresholds;
    if (evalcode != 0)
        thresholds.push_back(CCNewEval(E_MARSHAL(ss << evalcode)));
    if (evalcode != EVAL_TOKENS)                                                    // if evalCode == EVAL_TOKENS, it is actually MakeCCcond1()!
        thresholds.push_back(CCNewEval(E_MARSHAL(ss << (uint8_t)EVAL_TOKENS)));	    // this is eval token cc
    if (evalcode2 != 0)
        thresholds.push_back(CCNewEval(E_MARSHAL(ss << evalcode2)));                // add optional additional evalcode
    thresholds.push_back(CCNewThreshold(1, pks));			                        // signature

    return CCNewThreshold(thresholds.size(), thresholds);
}
// overload to make two-eval (token+evalcode) cryptocondition:
CC *MakeTokensCCcond1(uint8_t evalcode, CPubKey pk) {
    return MakeTokensCCcond1(evalcode, 0, pk);
}

// make three-eval (token+evalcode+evalcode2) 1of2 cc vout:
CTxOut MakeTokensCC1of2vout(uint8_t evalcode, uint8_t evalcode2, CAmount nValue, CPubKey pk1, CPubKey pk2, std::vector<vscript_t>* pvvData)
{
    CTxOut vout;
    CC *payoutCond = MakeTokensCCcond1of2(evalcode, evalcode2, pk1, pk2);
    vout = CTxOut(nValue, CCPubKey(payoutCond));
    if (pvvData)
    {
        //std::vector<std::vector<unsigned char>> vtmpData = std::vector<std::vector<unsigned char>>(vData->begin(), vData->end());
        std::vector<CPubKey> vPubKeys = std::vector<CPubKey>();
        //vPubKeys.push_back(pk);   // Warning: if add a pubkey here, the Solver function will add it to vSolutions and ExtractDestination might use it to get the spk address (such result might not be expected)
        COptCCParams ccp = COptCCParams(COptCCParams::VERSION_1, evalcode, 1, 1, vPubKeys, (*pvvData));
        vout.scriptPubKey << ccp.AsVector() << OP_DROP;
    }
    cc_free(payoutCond);
    return(vout);
}
// overload to make two-eval (token+evalcode) 1of2 cc vout:
CTxOut MakeTokensCC1of2vout(uint8_t evalcode, CAmount nValue, CPubKey pk1, CPubKey pk2, std::vector<vscript_t>* pvvData) {
    return MakeTokensCC1of2vout(evalcode, 0, nValue, pk1, pk2, pvvData);
}

// make three-eval (token+evalcode+evalcode2) cc vout:
CTxOut MakeTokensCC1vout(uint8_t evalcode, uint8_t evalcode2, CAmount nValue, CPubKey pk, std::vector<vscript_t>* pvvData)
{
    CTxOut vout;
    CC *payoutCond = MakeTokensCCcond1(evalcode, evalcode2, pk);
    vout = CTxOut(nValue, CCPubKey(payoutCond));
    if (pvvData)
    {
        //std::vector<std::vector<unsigned char>> vtmpData = std::vector<std::vector<unsigned char>>(vData->begin(), vData->end());
        std::vector<CPubKey> vPubKeys = std::vector<CPubKey>();
        //vPubKeys.push_back(pk);   // Warning: if add a pubkey here, the Solver function will add it to vSolutions and ExtractDestination might use it to get the spk address (such result might not be expected)
        COptCCParams ccp = COptCCParams(COptCCParams::VERSION_1, evalcode, 1, 1, vPubKeys, (*pvvData));
        vout.scriptPubKey << ccp.AsVector() << OP_DROP;
    }
    cc_free(payoutCond);
    return(vout);
}
// overload to make two-eval (token+evalcode) cc vout:
CTxOut MakeTokensCC1vout(uint8_t evalcode, CAmount nValue, CPubKey pk, std::vector<vscript_t>* pvvData) {
    return MakeTokensCC1vout(evalcode, 0, nValue, pk, pvvData);
}

// token v2 'mixed' vouts:

// make three-eval (token+evalcode+evalcode2) 1of2 cryptocondition with pubkeys or keyids:
CC *MakeTokensv2CCcondMofNDest(uint8_t evalcode1, uint8_t evalcode2, uint8_t M, std::vector<CTxDestination> dests)
{
    // make MofN sigs cond with destinations
    std::vector<CC*> conddests;
    for (auto const &dest : dests) {
        CC *ccSig;
        if (dest.which() == TX_PUBKEY)
            ccSig = CCNewSecp256k1(boost::get<CPubKey>(dest));
        else if (dest.which() == TX_PUBKEYHASH)
            ccSig = CCNewSecp256k1Hash(boost::get<CKeyID>(dest));
        else 
            return nullptr;
        conddests.push_back(ccSig);
    }

    std::vector<CC*> thresholds;
    if (evalcode1 != 0)
        thresholds.push_back(CCNewEval(E_MARSHAL(ss << evalcode1)));
    if (evalcode1 != EVAL_TOKENSV2)	                                                // if evalCode == EVAL_TOKENSV2, it is actually MakeCCcond1of2()!
        thresholds.push_back(CCNewEval(E_MARSHAL(ss << (uint8_t)EVAL_TOKENSV2)));	// this is eval token cc
    if (evalcode2 != 0)
        thresholds.push_back(CCNewEval(E_MARSHAL(ss << evalcode2)));                // add optional additional evalcode
    thresholds.push_back(CCNewThreshold(M, conddests));		                            // this is 1 of 2 sigs cc

    return CCNewThreshold(thresholds.size(), thresholds);
}

// make three-eval (token+evalcode+evalcode2) 1of2 cryptocondition with pubkeys:
CC *MakeTokensv2CCcondMofN(uint8_t evalcode1, uint8_t evalcode2, uint8_t M, std::vector<CPubKey> pks)
{
    // convert pks to dests
    std::vector<CTxDestination> dests;
    for (auto const &pk : pks)
        dests.push_back(pk);
    // make MofN sigs cond 
    return MakeTokensv2CCcondMofNDest(evalcode1, evalcode2, M, dests);
}


// overload to make two-eval (token+evalcode) 1of2 cryptocondition:
CC *MakeTokensv2CCcond1of2(uint8_t evalcode, CPubKey pk1, CPubKey pk2) {
    return MakeTokensv2CCcondMofN(evalcode, 0, 1, { pk1, pk2 });
}

// make three-eval (token+evalcode+evalcode2) 1of2 cryptocondition:
CC *MakeTokensv2CCcond1of2(uint8_t evalcode1, uint8_t evalcode2, CPubKey pk1, CPubKey pk2)
{
    return MakeTokensv2CCcondMofN(evalcode1, evalcode2, 1, { pk1, pk2 });
}

// make three-eval (token+evalcode+evalcode2) cryptocondition:
CC *MakeTokensv2CCcond1(uint8_t evalcode1, uint8_t evalcode2, CPubKey pk)
{
    return MakeTokensv2CCcondMofN(evalcode1, evalcode2, 1, { pk });
}
// overload to make two-eval (token+evalcode) cryptocondition:
CC *MakeTokensv2CCcond1(uint8_t evalcode, CPubKey pk) {
    return MakeTokensv2CCcondMofN(evalcode, 0, 1, { pk });
}

CTxOut MakeTokensCCMofNDestVoutMixed(uint8_t evalcode1, uint8_t evalcode2, CAmount nValue, uint8_t M, const std::vector<CTxDestination> &dests, vscript_t* pvData)
{
    CTxOut vout;
    CCwrapper payoutCond( MakeTokensv2CCcondMofNDest(evalcode1, evalcode2, M, dests) );
    //if (!CCtoAnon(payoutCond.get())) 
    //    return vout;

    bool hasSecHash = std::find_if(dests.begin(), dests.end(), [](const CTxDestination &dest){ return dest.which() == TX_PUBKEYHASH; }) != dests.end();

    vout = CTxOut(nValue, CCPubKey(payoutCond.get(), hasSecHash ? CC_MIXED_MODE_SECHASH_SUBVER_1 : CC_MIXED_MODE_SUBVER_0) );

    {
        std::vector<vscript_t> vvData;
        if (pvData)
            vvData.push_back(*pvData);

        // convert to pubkeys to show them in opdrop
        std::vector<CPubKey> pks;
        for (auto const &dest : dests)
            if (dest.which() == TX_PUBKEY)
                pks.push_back(boost::get<CPubKey>(dest));

        COptCCParams ccp = COptCCParams(COptCCParams::VERSION_2, evalcode1, M, pks.size(), pks, vvData);  // ver2 -> add pks
        vout.scriptPubKey << ccp.AsVector() << OP_DROP;
    }
    //if (pvData)
    //    vout.scriptPubKey << *pvData << OP_DROP;
    return vout;
}

// make three-eval (token+evalcode+evalcode2) MofN cc vout:
CTxOut MakeTokensCCMofNvoutMixed(uint8_t evalcode1, uint8_t evalcode2, CAmount nValue, uint8_t M, const std::vector<CPubKey> &pks, vscript_t* pvData)
{
    // convert pks to dests
    std::vector<CTxDestination> dests;
    for (auto const &pk : pks)
        dests.push_back(pk);
    return MakeTokensCCMofNDestVoutMixed(evalcode1, evalcode2, nValue, M, dests, pvData);
}

// make three-eval (token+evalcode+evalcode2) cc vout:
CTxOut MakeTokensCC1voutMixed(uint8_t evalcode1, uint8_t evalcode2, CAmount nValue, CPubKey pk, vscript_t* pvData)
{
    return MakeTokensCCMofNvoutMixed(evalcode1, evalcode2, nValue, 1, { pk }, pvData);
}
// overload to make two-eval (token+evalcode) cc vout:
CTxOut MakeTokensCC1voutMixed(uint8_t evalcode, CAmount nValue, CPubKey pk, vscript_t* pvData) {
    return MakeTokensCCMofNvoutMixed(evalcode, 0, nValue, 1, { pk }, pvData);
}

// overload to make two-eval (token+evalcode) 1of2 cc vout:
CTxOut MakeTokensCC1of2voutMixed(uint8_t evalcode, CAmount nValue, CPubKey pk1, CPubKey pk2, vscript_t* pvData) {
    return MakeTokensCCMofNvoutMixed(evalcode, 0, nValue, 1, { pk1, pk2 }, pvData);
}

// overload to make two-eval (token+evalcode) 1of2 cc vout:
CTxOut MakeTokensCC1of2voutMixed(uint8_t evalcode1, uint8_t evalcode2, CAmount nValue, CPubKey pk1, CPubKey pk2, vscript_t* pvData) {
    return MakeTokensCCMofNvoutMixed(evalcode1, evalcode2, nValue, 1, { pk1, pk2}, pvData);
}

// decodes token opret version, current values: 
// 0 before NN HF, 
// 1 after NN HF (also EVAL_TOKENSV2 mixed mode)
// if funcid == c or t it is version 0
// if funcid == C or T it is version 1
uint8_t DecodeTokenOpretVersion(const CScript &scriptPubKey)
{
    uint8_t funcId, evalCode, version = 0xFF;
    vscript_t vopret;

    GetOpReturnData(scriptPubKey, vopret);
    // tokenid = zeroid; this was incorrect: cleared the passed tokenid if creation tx

    if (vopret.size() >= 2)
    {
        evalCode = vopret[0];
        funcId = vopret[1];
        if (evalCode == EVAL_TOKENS)   {
            if (funcId == 'c' || funcId == 't') {
                version = 0;
                return version;
            }
            if (funcId == 'C' || funcId == 'T')   {
                if (vopret.size() >= 3)  {
                    version = vopret[2];
                    return version;
                }
            }
        }
        if (evalCode == EVAL_TOKENSV2)   {
            if (vopret.size() >= 3)  {
                version = vopret[2];
                return version;
            }
        }
    }
    return version;
}

// get token indexkey for pubkey for old tokens:
std::vector<std::string> GetTokenV1IndexKeys(const CPubKey &pk)
{
    std::vector<std::string> tokenindexkeys;
    char tokenindexkeyPK[KOMODO_ADDRESS_BUFSIZE];
	struct CCcontract_info *cp, C; 
	cp = CCinit(&C, EVAL_TOKENS);

    GetTokensCCaddress(cp, tokenindexkeyPK, pk, false);  
    tokenindexkeys.push_back(tokenindexkeyPK);
    
    return tokenindexkeys;
}

std::vector<CCwrapper> GetTokenV2Conds(const CPubKey &pk)
{
    std::vector<CCwrapper> tokenconds;
    // get token cond from the pubkey:
    char tokenindexkeyPK[KOMODO_ADDRESS_BUFSIZE];
	struct CCcontract_info *cp, C; 
	cp = CCinit(&C, EVAL_TOKENSV2);

    CCwrapper tokenCondPK( MakeTokensv2CCcondMofNDest(EVAL_TOKENSV2, 0, 1, { pk }) );
    tokenconds.push_back(tokenCondPK);
    
    // get token conds from the Raddress
    char normaladdr[KOMODO_ADDRESS_BUFSIZE];
    Getscriptaddress(normaladdr, CScript() << vuint8_t(pk.begin(), pk.end()) << OP_CHECKSIG); 
    CTxDestination dest = DecodeDestination(normaladdr);  // get normal dest
    for (CC_SUBVER ccSubVersion = CC_MIXED_MODE_SUBVER_0; ccSubVersion <= CC_MIXED_MODE_SUBVER_MAX; ccSubVersion = (CC_SUBVER)(ccSubVersion + 1))  {
        CCwrapper tokenCond( MakeTokensv2CCcondMofNDest(EVAL_TOKENSV2, 0, 1, { dest }) ); // use cc creator for destinations
        tokenconds.push_back(tokenCond);
    }
    return tokenconds;
}

// get all token indexkeys for pubkey for tokens mixed mode:
std::vector<std::string> GetTokenV2IndexKeys(const CPubKey &pk)
{
    std::vector<std::string> tokenindexkeys;

    // get indexkey from pubkey:
    CTxDestination pkdest = pk;

    // get indexkey from Raddress
    char normaladdr[KOMODO_ADDRESS_BUFSIZE];
    char tokenindexkeyR[KOMODO_ADDRESS_BUFSIZE];
    Getscriptaddress(normaladdr, CScript() << vuint8_t(pk.begin(), pk.end()) << OP_CHECKSIG); 
    CTxDestination dest = DecodeDestination(normaladdr);  // get normal dest

    std::vector<CTxDestination> dests{ pkdest, dest };

    int i = 0;
    for (CC_SUBVER ccSubVersion = CC_MIXED_MODE_SUBVER_0; i < dests.size() && ccSubVersion <= CC_MIXED_MODE_SUBVER_MAX; ccSubVersion = (CC_SUBVER)(ccSubVersion + 1), i ++)  {
        CCwrapper tokenCond( MakeTokensv2CCcondMofNDest(EVAL_TOKENSV2, 0, 1, { dests[i] }) );
        //if (!CCtoAnon(tokenCond.get())) { std::cerr << __func__ << " CCtoAnon failed" << std::endl; continue; }  // now in CCPubKey()
        Getscriptaddress(tokenindexkeyR, CCPubKey(tokenCond.get(), ccSubVersion));
        tokenindexkeys.push_back(tokenindexkeyR);
    }
    return tokenindexkeys;
}