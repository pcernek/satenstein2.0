//
// Created by pcernek on 6/5/15.
//

#include "DccaTest.h"

SINT32* aVarScore;
UINT32* aVarLastChange;
UINT32 iFlipCandidate;

TEST_F(DccaTest, hello) {
  EXPECT_EQ("hello", "hello");
}

TEST_F(DccaTest, pickCSDvar) {
  iNumCSDvars = _numCSDvars;
  csdVarsList = _csdVarsList;

  PickDCCA();

  EXPECT_EQ(6, iFlipCandidate);
}

TEST_F(DccaTest, pickNVDvar) {
  iNumCSDvars = 0;
  csdVarsList = _csdVarsList;

  iNumNVDvars = _numNVDvars;
  nvdVarsList = _nvdVarsList;

  PickDCCA();

  EXPECT_EQ(1, iFlipCandidate);
}

TEST_F(DccaTest, pickSDvar) {
  iNumCSDvars = 0;
  csdVarsList = _csdVarsList;

  iNumNVDvars = 0;
  nvdVarsList = _nvdVarsList;

  iNumSDvars = _numSDvars;
  sdVarsList = _sdVarsList;

  PickDCCA();

  EXPECT_EQ(4, iFlipCandidate);
}

DccaTest::DccaTest() {
  iFlipCandidate = 0;
  initializeArrays();
}

void DccaTest::initializeArrays() {
  SINT32 tempVarScores[] =     {0, -3,  0, -3, -8,  2, -8};
  UINT32 tempVarLastChange[] = {0,  1,  2,  3,  4,  5,  1};

  aVarScore = (SINT32 *) malloc( (_numVars + 1) * sizeof(SINT32));
  aVarLastChange = (UINT32 *) malloc( (_numVars + 1) * sizeof(UINT32));

  copyArray(tempVarScores, aVarScore, _numVars + 1);
  copyArray(tempVarLastChange, aVarLastChange, _numVars + 1);

  _numCSDvars = 4;
  UINT32 tempCSDvarsList[] = {1,3,4,6};
  _csdVarsList = (UINT32 *) malloc( _numCSDvars * sizeof(UINT32));
  copyArray(tempCSDvarsList, _csdVarsList, _numCSDvars);

  _numNVDvars = 2;
  UINT32 tempNVDvarsList[] = {1,3};
  _nvdVarsList = (UINT32 *) malloc( _numNVDvars * sizeof(UINT32));
  copyArray(tempNVDvarsList, _nvdVarsList, _numNVDvars);

  _numSDvars = 5;
  UINT32 tempSDvarsList[] = {1,2,3,4,5};
  _sdVarsList = (UINT32 *) malloc( _numSDvars * sizeof(UINT32));
  copyArray(tempNVDvarsList, _sdVarsList, _numSDvars);

}

template <typename T>
void DccaTest::copyArray(T *src, T *dst, UINT32 size) {
  for (int i = 0; i < size; i++) {
    dst[i] = src[i];
  }
}