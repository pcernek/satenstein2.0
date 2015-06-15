/*

      ##  ##  #####    #####   $$$$$   $$$$   $$$$$$    
      ##  ##  ##  ##  ##      $$      $$  $$    $$      
      ##  ##  #####   ##       $$$$   $$$$$$    $$      
      ##  ##  ##  ##  ##          $$  $$  $$    $$      
       ####   #####    #####  $$$$$   $$  $$    $$      
  ======================================================
  SLS SAT Solver from The University of British Columbia
  ======================================================
  ...Developed by Dave Tompkins (davet [@] cs.ubc.ca)...
  ------------------------------------------------------
  .......consult legal.txt for legal information........
  ......consult revisions.txt for revision history......
  ------------------------------------------------------
  ... project website: http://www.satlib.org/ubcsat ....
  ------------------------------------------------------
  .....e-mail ubcsat-help [@] cs.ubc.ca for support.....
  ------------------------------------------------------

*/

#include "paws.h"

UINT32 iPAWSMaxInc;
PROBABILITY iPAWSFlatMove;

UINT32 iPawsSmoothCounter;


/***** Trigger PenClauseList *****/

void CreatePenClauseList();
void InitPenClauseList();

UINT32 *aPenClauseList;
UINT32 *aPenClauseListPos;
UINT32 iNumPenClauseList;

void CreatePenClauseList() {
  aPenClauseList = AllocateRAM(iNumClauses*sizeof(UINT32));
  aPenClauseListPos = AllocateRAM(iNumClauses*sizeof(UINT32));
}

void InitPenClauseList() {
  iNumPenClauseList = 0;
  iPawsSmoothCounter = 0;
}



void AddPAWS() {

  ALGORITHM *pCurAlg;

  pCurAlg = CreateAlgorithm("paws","",FALSE,
    "PAWS: Pure Additive Weighting Scheme",
    "Thornton, Pham, Bain, Ferreira [AAAI 04]",
    "PickPAWS,PostFlipPAWS",
    "DefaultProcedures,Flip+MBPINT+FCL+VIF,PenClauseList,VarLastChange",
    "default","default");
  
  AddParmUInt(&pCurAlg->parmList,"-maxinc","frequency of penalty reductions [default %s]","reduce (smooth) all clause penalties by 1~after every INT increases","",&iPAWSMaxInc,10);
  AddParmProbability(&pCurAlg->parmList,"-pflat","flat move probabilty [default %s]","when a local minimum is encountered,~take a 'flat' (sideways) step with probability PR","",&iPAWSFlatMove,0.15);

  CreateTrigger("PickPAWS",ChooseCandidate,PickPAWS,"","");
  CreateTrigger("PostFlipPAWS",PostFlip,PostFlipPAWS,"","");

  CreateTrigger("CreatePenClauseList",CreateStateInfo,CreatePenClauseList,"","");
  CreateTrigger("InitPenClauseList",InitStateInfo,InitPenClauseList,"","");
  CreateContainerTrigger("PenClauseList","CreatePenClauseList,InitPenClauseList");

}

void PickPAWS() {
  
  UINT32 j;
  UINT32 iVar;
  SINT32 iScore;
  SINT32 iBestScore;
  UINT32 iLoopEnd;
  UINT32 iTabuCutoff;
  iNumCandidates = 0;
  iBestScore = 0x7FFFFFFF;
     if(bTabu)
  {
      if (iStep > iTabuTenure) {
       iTabuCutoff = iStep - iTabuTenure;
      if (iVarLastChangeReset > iTabuCutoff) {
       iTabuCutoff = iVarLastChangeReset;
     }
   } else {
    iTabuCutoff = 1;
   }
  }  
  /* look at all variables that appear in false clauses */
  if(bVarInFalse){
  if(!bTabu){
  for (j=0;j<iNumVarsInFalseList;j++) {
    iVar = aVarInFalseList[j];

    /* use cached value of breakcount - makecount */
    switch(iScoringMeasure){

    case 1:
    iScore = aBreakPenaltyINT[iVar] - aMakePenaltyINT[iVar];

    /* build candidate list of best vars */

    if (iScore <= iBestScore) {
      if (iScore < iBestScore) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];
      
      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }
     
     break;

     case 2:
        iScore =  -aMakePenaltyINT[iVar];

    /* build candidate list of best vars */

    
     if (iScore <= iBestScore) {
      if (iScore < iBestScore) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];

      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }




     break;
     case 3: 
     iScore = aBreakPenaltyINT[iVar] - aMakePenaltyINT[iVar];

    /* build candidate list of best vars */

    if (iScore <= iBestScore) {
      if ((iScore < iBestScore)||(aVarLastChange[iVar]<aVarLastChange[*aCandidateList])) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];

      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }

     break;

     
    }
  }
 }
 // If Tabu 
 else{
   for (j=0;j<iNumVarsInFalseList;j++) {
    iVar = aVarInFalseList[j];
    if (aVarLastChange[j] < iTabuCutoff) { 
    /* use cached value of breakcount - makecount */
    switch(iScoringMeasure){

    case 1:
    iScore = aBreakPenaltyINT[iVar] - aMakePenaltyINT[iVar];

    /* build candidate list of best vars */

    if (iScore <= iBestScore) {
      if (iScore < iBestScore) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];

      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }

     break;

     case 2:
        iScore =  -aMakePenaltyINT[iVar];

    /* build candidate list of best vars */


     if (iScore <= iBestScore) {
      if (iScore < iBestScore) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];

      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }




     break;
     case 3:
     iScore = aBreakPenaltyINT[iVar] - aMakePenaltyINT[iVar];

    /* build candidate list of best vars */

    if (iScore <= iBestScore) {
      if ((iScore < iBestScore)||(aVarLastChange[iVar]<aVarLastChange[*aCandidateList])) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];

      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }

     break;


    }
  }
}
}
 
}
  else{
  if(!bTabu){
  for (j=0;j<iNumVars;j++) {
    iVar = j;

    /* use cached value of breakcount - makecount */
    switch(iScoringMeasure){

    case 1:
    iScore = aBreakPenaltyINT[iVar] - aMakePenaltyINT[iVar];

    /* build candidate list of best vars */

    if (iScore <= iBestScore) {
      if (iScore < iBestScore) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];

      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }

     break;

     case 2:
        iScore =  -aMakePenaltyINT[iVar];

    /* build candidate list of best vars */


     if (iScore <= iBestScore) {
      if (iScore < iBestScore) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];

      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }




     break;
     case 3:
     iScore = aBreakPenaltyINT[iVar] - aMakePenaltyINT[iVar];

    /* build candidate list of best vars */

    if (iScore <= iBestScore) {
      if ((iScore < iBestScore)||(aVarLastChange[iVar]<aVarLastChange[*aCandidateList])) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];

      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }

     break;


    }
  }
 }
 // If Tabu
 else{
   for (j=0;j<iNumVars;j++) {
    iVar = j;
    if (aVarLastChange[j] < iTabuCutoff) {
    /* use cached value of breakcount - makecount */
    switch(iScoringMeasure){

    case 1:
    iScore = aBreakPenaltyINT[iVar] - aMakePenaltyINT[iVar];

    /* build candidate list of best vars */

    if (iScore <= iBestScore) {
      if (iScore < iBestScore) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];

      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }

     break;

     case 2:
        iScore =  -aMakePenaltyINT[iVar];

    /* build candidate list of best vars */


     if (iScore <= iBestScore) {
      if (iScore < iBestScore) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];

      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }




     break;
     case 3:
     iScore = aBreakPenaltyINT[iVar] - aMakePenaltyINT[iVar];

    /* build candidate list of best vars */

    if (iScore <= iBestScore) {
      if ((iScore < iBestScore)||(aVarLastChange[iVar]<aVarLastChange[*aCandidateList])) {
        iNumCandidates = 0;
        iBestScore = iScore;
      }

      /* using the "Monte Carlo" method */

      iLoopEnd = iNumCandidates + aMakeCount[iVar];

      if (iLoopEnd >= iMaxCandidates) {
        ReportPrint1(pRepErr,"Unexpected Error: increase iMaxCandidates [%u]\n",iMaxCandidates);
        AbnormalExit();
      }

      while (iNumCandidates < iLoopEnd) {
        aCandidateList[iNumCandidates++] = iVar;
      }
     }

     break;


    }
  }
}
}

}

  iFlipCandidate = 0;

  if (iBestScore < 0) {

    /* select flip candidate uniformly from candidate list */
    //iFlipCandidate = TieBreaking();
     
    if (iNumCandidates > 1) {
     // iFlipCandidate = aCandidateList[RandomInt(iNumCandidates)];
        iFlipCandidate = TieBreaking();
    } else {
      iFlipCandidate = *aCandidateList;
    }
    
  } else {
    
    if (iBestScore == 0) {

      /* with probability (iPAWSFlatMove) flip candidate from candidate list,
         otherwise it's a null flip */
    
      if (RandomProb(iPAWSFlatMove)) {
       // iFlipCandidate = aCandidateList[RandomInt(iNumCandidates)]; 
        
        if (iNumCandidates > 1) {
          //iFlipCandidate = aCandidateList[RandomInt(iNumCandidates)]; 
           iFlipCandidate = TieBreaking();
        } else {
          iFlipCandidate = *aCandidateList;
        }
       
      }
    }
  }
}

void SmoothPAWS() {
  
  UINT32 j;
  UINT32 k;
  UINT32 iClause;
  UINT32 iLoopMax;
  LITTYPE *pLit;

  iLoopMax = iNumPenClauseList;

  iTotalPenaltyINT -= iNumPenClauseList;

  for (j=0;j<iLoopMax;j++) {
    
    iClause = aPenClauseList[j];

    aClausePenaltyINT[iClause]--;

    if (aClausePenaltyINT[iClause]==1) {
      aPenClauseList[aPenClauseListPos[iClause]] = aPenClauseList[--iNumPenClauseList];
      aPenClauseListPos[aPenClauseList[iNumPenClauseList]] = aPenClauseListPos[iClause];
    }

    if (aNumTrueLit[iClause]==0) { 
      pLit = pClauseLits[iClause];
      for (k=0;k<aClauseLen[iClause];k++) {
        aMakePenaltyINT[GetVarFromLit(*pLit)]--;
        pLit++;
      }
    }
    if (aNumTrueLit[iClause]==1) {
      aBreakPenaltyINT[aCritSat[iClause]]--;
    }
  }
}

void ScalePAWS() {
  UINT32 j;
  UINT32 k;
  UINT32 iClause;
  LITTYPE *pLit;

  /* for each false clause, increae the clause penalty by 1 */

  iTotalPenaltyINT += iNumFalse;

  for(j=0;j<iNumFalse;j++) {
    
    iClause = aFalseList[j];

    aClausePenaltyINT[iClause]++;

    if (aClausePenaltyINT[iClause]==2) {

      aPenClauseList[iNumPenClauseList] = iClause;
      aPenClauseListPos[iClause] = iNumPenClauseList++;

    }

    /* update cached values */

    pLit = pClauseLits[iClause];
    for (k=0;k<aClauseLen[iClause];k++) {
      aMakePenaltyINT[GetVarFromLit(*pLit)]++;
      pLit++;
    }
  }
}


void PostFlipPAWS() {

  if (iFlipCandidate)
    return;

  /* if a 'null flip' */

  /* Scale penalties */

  ScalePAWS();

  /* smooth every iPAWSMaxInc Null Flips */

  iPawsSmoothCounter++;
  if (iPawsSmoothCounter > iPAWSMaxInc) {
    SmoothPAWS();
    iPawsSmoothCounter = 0;
  }

}

