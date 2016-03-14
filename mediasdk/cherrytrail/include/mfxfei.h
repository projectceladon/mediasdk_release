/******************************************************************************* *\

Copyright (C) 2013-2014 Intel Corporation.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
- Neither the name of Intel Corporation nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL INTEL CORPORATION BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

File Name: mfxfei.h

*******************************************************************************/
#ifndef __MFXFEI_H__
#define __MFXFEI_H__
#include "mfxdefs.h"
#include "mfxvstructures.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */



//1 PreENC input
typedef struct { /* VAStatsStatisticsParameter16x16 */
    mfxExtBuffer    Header;  /* MFX_EXTBUFF_FEI_xxx */

    mfxU16    Qp;
    mfxU16    LenSP;
    mfxU16    MaxLenSP;
    mfxU16    SubMBPartMask;
    mfxU16    SubPelMode;
    mfxU16    InterSAD;
    mfxU16    IntraSAD;
    mfxU16    AdaptiveSearch;
    mfxU16    MVPredictor; /*0 -disable, 1 - past, 2 future, 3 both */
    mfxU16    MBQp;
    mfxU16    FTEnable;
    mfxU16    RefWidth;
    mfxU16    RefHeight;
    mfxU16    SearchWindow;

    mfxU16    BkwMVPresent;
    mfxU16    FrwMVPresent;

    mfxU16    DisableMVOutput;    
    mfxU16    DisableStatisticsOutput;   
} mfxExtFeiPreEncCtrl;

typedef struct { /* VAMotionVector */
    mfxExtBuffer    Header;
    mfxU32  reserved[13];
    mfxU32  NumMBAlloc; /* size of allocated memory in number of macroblocks */

    struct  mfxMB{
        mfxI16Pair MV[2]; /* 0 for L0 and 1 for L1 */
    } *MB;
} mfxExtFeiPreEncMVPredictors;

typedef struct {
    mfxExtBuffer    Header;
    mfxU32  reserved[13];
    mfxU32  NumQPAlloc; /* size of allocated memory in number of QPs value*/

    mfxU16    *QP;
} mfxExtFeiPreEncQPs;

//1 PreENC output
/* Layout is exactly the same as mfxExtFeiEncMVs, this buffer may be removed in future */
typedef struct {
    mfxExtBuffer    Header;
    mfxU32  reserved[13];
    mfxU32  NumMBAlloc;

    struct  {
        mfxI16Pair MV[16][2];
    } *MB;
} mfxExtFeiPreEncMV;

typedef struct {
    mfxExtBuffer    Header;
    mfxU32 reserved0[13];
    mfxU32  NumMBAlloc;
    
    struct  {
        struct  {
            mfxU16  BestDistortion;
            mfxU16  Mode ;
        } Inter[2]; /*0 -L0, 1 - L1*/

        mfxU16  BestIntraDistortion;
        mfxU16  IntraMode ;

        mfxU16  NumOfNonZeroCoef;
        mfxU16  reserved1;

        mfxU32  SumOfCoef;

        mfxU32  Variance;
        mfxU32  PixelAverage;
    } *MB;
} mfxExtFeiPreEncMBStat;

//1  ENC_PAK input
typedef struct {
    mfxExtBuffer    Header;

    mfxU16    MaxLenSP;
    mfxU16    LenSP;
    mfxU16    SubMBPartMask;
    mfxU16    IntraPartMask;
    mfxU16    MultiPredL0;
    mfxU16    MultiPredL1;
    mfxU16    SubPelMode;
    mfxU16    InterSAD;
    mfxU16    IntraSAD;
    mfxU16    DistortionType;
    mfxU16    RepartitionCheckEnable;
    mfxU16    AdaptiveSearch;
    mfxU16    MVPredictor;
    mfxU16    NumMVPredictors;
    mfxU16    PerMBQp;
    mfxU16    PerMBInput;
    mfxU16    MBSizeCtrl;
    mfxU16    RefWidth;
    mfxU16    RefHeight;
    mfxU16    SearchWindow;
} mfxExtFeiEncFrameCtrl;

typedef struct { /* VAEncMVPredictorH264 */
    mfxExtBuffer    Header;
    mfxU32  reserved[13];
    mfxU32  NumMBAlloc; /* size of allocated memory in number of macroblocks */

    struct  mfxMB{
        struct {
            mfxU8   L0: 4;
            mfxU8   L1: 4;
        } RefIdx[4]; /* index is predictor number */
        mfxU32      Reserved;
        mfxI16Pair MV[4][2]; /* first index is predictor number, second is 0 for L0 and 1 for L1 */
    } *MB;
} mfxExtFeiEncMVPredictors;

typedef struct {
    mfxExtBuffer    Header;
    mfxU32  reserved[13];
    mfxU32  NumMBAlloc;

    struct  mfxMB{
        mfxU16    QpY;
        mfxU16    QpCb;
        mfxU16    QpCr;
        mfxU16    ForceToSkip;
        mfxU16    ForceToIntra;
        mfxU16    MaxSizeInWord;
        mfxU16    TargetSizeInWord;
    } *MB;
} mfxExtFeiEncMBCtrl;


//1 ENC_PAK output
/* Buffer holds 32 MVs per MB. MVs are located in zigzag scan order.
Number in diagram below shows location of MV in memory.
For example, MV for right top 4x4 sub block is stored in 5-th element of the array.
========================
|| 00 | 01 || 04 | 05 ||
------------------------
|| 02 | 03 || 06 | 07 ||
========================
|| 08 | 09 || 12 | 13 ||
------------------------
|| 10 | 11 || 14 | 15 ||
========================
*/
typedef struct {
    mfxExtBuffer    Header;
    mfxU32  reserved[13];
    mfxU32  NumMBAlloc;

    struct  mfxMB{
        mfxI16Pair MV[16][2]; /* first index is block (4x4 pixels) number, second is 0 for L0 and 1 for L1 */
    } *MB;
} mfxExtFeiEncMVs;

typedef struct {
    mfxExtBuffer    Header;
    mfxU32  reserved[13];
    mfxU32  NumMBAlloc;

    struct mfxMB{
        mfxU16  InterDistortion[16];
        mfxU16  BestInterDistortion;
        mfxU16  BestIntraDistortion;
    } *MB;
} mfxExtFeiEncMBStat;

typedef struct {
    mfxU32    reserved0[4];

    //dword 3
    mfxU32    InterMbMode         : 2;
    mfxU32    MBSkipFlag          : 1;
    mfxU32    Reserved00          : 1;
    mfxU32    IntraMbMode         : 2;
    mfxU32    Reserved01          : 1;
    mfxU32    FieldMbPolarityFlag : 1;
    mfxU32    MbType              : 5;
    mfxU32    IntraMbFlag         : 1;
    mfxU32    FieldMbFlag         : 1;
    mfxU32    Transform8x8Flag    : 1;
    mfxU32    Reserved02          : 1;
    mfxU32    DcBlockCodedCrFlag  : 1;
    mfxU32    DcBlockCodedCbFlag  : 1;
    mfxU32    DcBlockCodedYFlag   : 1;
    mfxU32    Reserved03          :12;

    //dword 4
    mfxU8     HorzOrigin;
    mfxU8     VertOrigin;
    mfxU16    CbpY;

    //dword 5
    mfxU16    CbpCb;
    mfxU16    CbpCr;

    //dword 6
    mfxU32    QpPrimeY            : 8;
    mfxU32    Reserved30          :17;
    mfxU32    MbSkipConvDisable   : 1;
    mfxU32    IsLastMB            : 1;
    mfxU32    EnableCoefficientClamp : 1;
    mfxU32    Direct8x8Pattern    : 4;

    union {
        struct {// Intra MBs
            //dword 7,8
            mfxU16   LumaIntraPredModes[4];

            //dword 9
            mfxU32   MbIntraStruct : 8;
            mfxU32   Reserved60    : 24;
        };
        struct {// Inter MBs
            //dword 7
            mfxU8    SubMbShapes;
            mfxU8    SubMbPredModes;
            mfxU16   Reserved40;

            //dword 8, 9
            mfxU8    RefIdx[2][4]; /* first index is 0 for L0 and 1 for L1 */
        };
    };

    //dword 10
    mfxU16    Reserved70;
    mfxU8     TargetSizeInWord;
    mfxU8     MaxSizeInWord;

    mfxU32     reserved2[4];
}mfxFeiPakMBCtrl;

typedef struct {
    mfxExtBuffer    Header;
    mfxU32  reserved[13];
    mfxU32  NumMBAlloc;

    mfxFeiPakMBCtrl *MB;
} mfxExtFeiPakMBCtrl;




//1 functions
typedef enum {
    MFX_FEI_FUNCTION_PREENC     =1,
    MFX_FEI_FUNCTION_ENCPAK     =2,
    MFX_FEI_FUNCTION_ENC        =3,
    MFX_FEI_FUNCTION_PAK        =4
} mfxFeiFunction;

enum {
    MFX_EXTBUFF_FEI_PARAM          = MFX_MAKEFOURCC('F','E','P','R'),
    MFX_EXTBUFF_FEI_PREENC_CTRL    = MFX_MAKEFOURCC('F','P','C','T'),
    MFX_EXTBUFF_FEI_PREENC_MV_PRED = MFX_MAKEFOURCC('F','P','M','P'),
    MFX_EXTBUFF_FEI_PREENC_QP      = MFX_MAKEFOURCC('F','P','Q','P'),
    MFX_EXTBUFF_FEI_PREENC_MV      = MFX_MAKEFOURCC('F','P','M','V'),
    MFX_EXTBUFF_FEI_PREENC_MB      = MFX_MAKEFOURCC('F','P','M','B'),
    MFX_EXTBUFF_FEI_ENC_CTRL       = MFX_MAKEFOURCC('F','E','C','T'),
    MFX_EXTBUFF_FEI_ENC_MV_PRED    = MFX_MAKEFOURCC('F','E','M','P'),
    MFX_EXTBUFF_FEI_ENC_MB         = MFX_MAKEFOURCC('F','E','M','B'),
    MFX_EXTBUFF_FEI_ENC_MV         = MFX_MAKEFOURCC('F','E','M','V'),
    MFX_EXTBUFF_FEI_ENC_MB_STAT    = MFX_MAKEFOURCC('F','E','S','T'),
    MFX_EXTBUFF_FEI_PAK_CTRL       = MFX_MAKEFOURCC('F','K','C','T')
};

/* shoud be attached to mfxVideoParam during initialization to indicate FEI function */
typedef struct {
    mfxExtBuffer    Header; /* MFX_EXTBUFF_FEI_PARAM */
    mfxFeiFunction  Func;
    mfxU32 reserved[29];
} mfxExtFeiParam;


/* PAK */
typedef struct {
    mfxU32  reserved[32];

    mfxFrameSurface1 *InSurface;

    mfxU16  NumFrameL0;
    mfxFrameSurface1 *L0Surface;
    mfxU16  NumFrameL1;
    mfxFrameSurface1 *L1Surface;

    mfxU16  NumExtParam;
    mfxExtBuffer    **ExtParam;
} mfxPAKInput;

typedef struct {
    mfxBitstream *Bs;

    mfxU16  NumExtParam;
    mfxExtBuffer    **ExtParam;
} mfxPAKOutput;

//mfxStatus MFX_CDECL MFXVideoPAK_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
//mfxStatus MFX_CDECL MFXVideoPAK_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request);
mfxStatus MFX_CDECL MFXVideoPAK_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoPAK_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoPAK_Close(mfxSession session);

//mfxStatus MFX_CDECL MFXVideoPAK_GetVideoParam(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoPAK_ProcessFrameAsync(mfxSession session, mfxPAKInput *in, mfxPAKOutput *out,  mfxSyncPoint *syncp);


#ifdef __cplusplus
} // extern "C"
#endif /* __cplusplus */


#endif

