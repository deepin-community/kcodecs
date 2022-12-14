/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

// for japanese encoding, observe characteristic:
// 1, kana character (or hankaku?) often have high frequency of appearance
// 2, kana character often exist in group
// 3, certain combination of kana is never used in japanese language

#include "nsEUCJPProber.h"

namespace kencodingprober
{
void nsEUCJPProber::Reset(void)
{
    mCodingSM->Reset();
    mState = eDetecting;
    mContextAnalyser.Reset();
    mDistributionAnalyser.Reset();
}

nsProbingState nsEUCJPProber::HandleData(const char *aBuf, unsigned int aLen)
{
    if (aLen == 0) {
        return mState;
    }

    nsSMState codingState;

    for (unsigned int i = 0; i < aLen; i++) {
        codingState = mCodingSM->NextState(aBuf[i]);
        if (codingState == eError) {
            mState = eNotMe;
            break;
        }
        if (codingState == eItsMe) {
            mState = eFoundIt;
            break;
        }
        if (codingState == eStart) {
            unsigned int charLen = mCodingSM->GetCurrentCharLen();

            if (i == 0) {
                mLastChar[1] = aBuf[0];
                mContextAnalyser.HandleOneChar(mLastChar, charLen);
                mDistributionAnalyser.HandleOneChar(mLastChar, charLen);
            } else {
                mContextAnalyser.HandleOneChar(aBuf + i - 1, charLen);
                mDistributionAnalyser.HandleOneChar(aBuf + i - 1, charLen);
            }
        }
    }

    mLastChar[0] = aBuf[aLen - 1];

    if (mState == eDetecting) {
        if (mContextAnalyser.GotEnoughData() && GetConfidence() > SHORTCUT_THRESHOLD) {
            mState = eFoundIt;
        }
    }

    return mState;
}

float nsEUCJPProber::GetConfidence(void)
{
    float contxtCf = mContextAnalyser.GetConfidence();
    float distribCf = mDistributionAnalyser.GetConfidence();

    return (contxtCf > distribCf ? contxtCf : distribCf);
}
}
