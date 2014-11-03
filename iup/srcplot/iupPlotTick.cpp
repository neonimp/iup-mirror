
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "iupPlot.h"


const double kTickValueVeryBig = 1.0e4;// switch to scientific format
const double kTickValueVerySmall = 1.0e-3;
const double kLittleIncrease = 1.0001;
const double kLittleDecrease = 0.9999;
const double kEps = 1e-4;


/***********************************************************************************/


static double iPlotRoundSpan(double inSpan) 
{
  // round it to something producing readable tick labels
  // write it in the form inSpan = a*SafeExp10 (b)

  if (inSpan<=0)
    return (double)-1.234567;  // error

  int thePow = 0;
  double theSpan = inSpan;
  if (inSpan>1) 
  {
    while (theSpan>10) 
    {
      theSpan/=10;
      if (theSpan == inSpan)  // not a number
          return (double)-1.234567;
      thePow++;
    }
  }
  else 
  {
    while (theSpan<1) 
    {
      theSpan*=10;
      thePow--;      
    }
  }

  int theRoundedFirstDigit = iupPlotRound (theSpan);
  int thePreferredFirstDigit = 1;
  switch (theRoundedFirstDigit) 
  {
  case 1:
    thePreferredFirstDigit = 1;
    break;
  case 2:
  case 3:
  case 4:
    thePreferredFirstDigit = 2;
    break;
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
    thePreferredFirstDigit = 5;
    break;
  case 10:
    thePreferredFirstDigit = 1;
    thePow++;
    break;
  default:
    // error
    return (double)-1.234567;
    break;
  }
  double theRes = thePreferredFirstDigit*pow (10., thePow);
  return theRes;
}

static void iPlotMakeFormatString(double inValue, char* outFormatString) 
{
  if (inValue < 0)
    inValue = - inValue;
  
  if (inValue > kTickValueVeryBig || inValue < kTickValueVerySmall)
    strcpy(outFormatString, "%.1e");
  else 
  {
    int thePrecision = 0;
    if (inValue < 1) 
    {
      double theSpan = inValue;
      while (theSpan < 1 && thePrecision < 9)
      {
        thePrecision++;
        theSpan *= 10;
      }
    }

    char theBuf[128] = "%.0f";
    theBuf[2] = (char)('0' + thePrecision);
    strcpy(outFormatString, theBuf);
  }
}


/***********************************************************************************/


bool iupPlotTickIterLinear::Init() 
{
  if (!mAxis)
    return false;

  double theMin = mAxis->mMin;
  double theMajorTickSpan = mAxis->mTick.mMajorSpan;
  int theDiv = mAxis->mTick.mMinorDivision;
  mDelta = theMajorTickSpan/theDiv;
  mCount = (long)ceil(theMin/mDelta);
  mCurrentTick = mCount*mDelta;

  strcpy(mFormatString, mAxis->mTick.mFormatString);

  return true;

}

bool iupPlotTickIterLinear::GetNextTick (double &outTick, bool &outIsMajorTick, char* outFormatString) 
{
  if (!mAxis)
    return false;
  
  if (mCurrentTick>mAxis->mMax*kLittleIncrease)
    return false;
  
  outTick = mCurrentTick;
  outIsMajorTick = (mCount%mAxis->mTick.mMinorDivision == 0);
  if (outFormatString) strcpy(outFormatString, mFormatString);

  mCurrentTick += mDelta;
  mCount++;
  return true;
}

bool iupPlotTickIterLinear::InitFromRanges (double inParRange, double inDivGuess, iupPlotTick &ioTick) const 
{
  if (inDivGuess <= kFloatSmall)
    return false;
  
  double thePreferredSpan = iPlotRoundSpan(inParRange / inDivGuess);
  if (thePreferredSpan < 0)
    return false;

  double thePreferredNrOfTicks = inParRange/thePreferredSpan;
  if (thePreferredNrOfTicks <1)
    ioTick.mMajorSpan = inParRange;
  else
    ioTick.mMajorSpan = thePreferredSpan;

  ioTick.mMinorDivision = 5;

  iPlotMakeFormatString(ioTick.mMajorSpan, ioTick.mFormatString);
  return true;
}


/***********************************************************************************/


bool iupPlotTickIterLog::Init () 
{
  if (!mAxis)
    return false;

  double theMin = mAxis->mMin;
  double theMajorTickSpan = mAxis->mTick.mMajorSpan;
  int theDiv = mAxis->mTick.mMinorDivision;
  mDelta = theMajorTickSpan/theDiv;
  double theBase = mAxis->mLogBase;
  long thePowMin = (long)floor(iupPlotLog(theMin, theBase));
  mCurrentTick = iupPlotExp (thePowMin, theBase);
  mCount = 0;

  // walk to the first tick

  if (theMin<=0)
    return false;
  else 
  {
    // walk forward
    double theNext = mCurrentTick+mDelta*iupPlotExp (thePowMin, theBase);
    while (theNext<=theMin*kLittleDecrease) 
    {
      mCurrentTick = theNext;
      theNext += mDelta*iupPlotExp (thePowMin,theBase);
      mCount++;
    }
  }
  return true;
}

bool iupPlotTickIterLog::InitFromRanges (double inParRange, double inDivGuess, iupPlotTick &ioTick) const 
{
  if (inDivGuess<=kFloatSmall)
    return false;

  /* TODO check this
     There will be a compiler warning for inParRange unused. Leave it this way for now...

  double thePreferredSpan = iPlotRoundSpan(inParRange/inDivGuess);
  double thePreferredNrOfTicks = inParRange/thePreferredSpan;
  if (thePreferredNrOfTicks <1)
    ioTick.mMajorSpan = inParRange;
  else
    ioTick.mMajorSpan = thePreferredSpan;
  */

  double theBase = mAxis->mLogBase;
  ioTick.mMajorSpan = theBase-1;// relative

  ioTick.mMinorDivision = iupPlotRound (ioTick.mMajorSpan);

  strcpy(ioTick.mFormatString, "%.1e");
  return true;
}

bool iupPlotTickIterLog::GetNextTick (double &outTick, bool &outIsMajorTick, char* outFormatString) 
{
  if (!mAxis)
    return false;
  
  if (mCurrentTick>mAxis->mMax*kLittleIncrease)
    return false;

  outTick = mCurrentTick;
  outIsMajorTick = (mCount%mAxis->mTick.mMinorDivision == 0);
  if (outFormatString) iPlotMakeFormatString(outTick, outFormatString);
  double theBase = mAxis->mLogBase;
  double theLogNow = iupPlotLog(mCurrentTick, theBase);
  int thePowNow = (int)floor(theLogNow);
  outIsMajorTick = false;
  if (fabs (theLogNow-thePowNow)<kEps)
    outIsMajorTick = true;

  mCurrentTick += mDelta*iupPlotExp (thePowNow, theBase);
  mCount++;

  return true;
}

bool iupPlotTickIterLog::AdjustRange (double &ioMin, double &ioMax) const 
{
  double theBase = mAxis->mLogBase;
  if (mAxis->mMaxDecades > 0)
    ioMin = ioMax/iupPlotExp (mAxis->mMaxDecades, theBase);

  if (ioMin == 0 && ioMax == 0) 
  {
    ioMin = kLogMinClipValue;
    ioMax = 1.0;
  }
  if (ioMin <= 0 || ioMax<=0)
    return false;
  
  ioMin = RoundDown (ioMin*kLittleIncrease);
  ioMax = RoundUp (ioMax*kLittleDecrease);

  if (ioMin<kLogMinClipValue)
    ioMin = kLogMinClipValue;
  
  if (mAxis->mMaxDecades > 0)
    ioMin = ioMax/iupPlotExp (mAxis->mMaxDecades, theBase);
  
  return true;
}

double iupPlotTickIterLog::RoundUp (double inFloat) const 
{
  double theBase = mAxis->mLogBase;
  int thePow = (int)ceil(iupPlotLog(inFloat, theBase));
  return pow (theBase, thePow);
}

double iupPlotTickIterLog::RoundDown (double inFloat) const 
{
  double theBase = mAxis->mLogBase;
  int thePow = (int)floor(iupPlotLog(inFloat,theBase));
  return pow (theBase, thePow);
}


/***********************************************************************************/


bool iupPlotTickIterNamed::GetNextTick (double &outTick, bool &outIsMajorTick, char* outFormatString) 
{
  if (iupPlotTickIterLinear::GetNextTick (outTick, outIsMajorTick, outFormatString)) 
  {
    int theIndex = iupPlotRound (outTick);

    // TODO: improve this
    if (fabs(outTick - (double)theIndex) > 0.1 ) 
    {
      if (outFormatString) strcpy(outFormatString, "");
      return true;
    }

    if (theIndex >= 0 && theIndex < mStringData->GetCount())
    {
      char** data = mStringData->GetStrData();
      if (outFormatString) strcpy(outFormatString, data[theIndex]);
      return true;
    }
  }
  return false;
}

bool iupPlotTickIterNamed::InitFromRanges (double inParRange, double inDivGuess, iupPlotTick &outTick) const 
{
  if (iupPlotTickIterLinear::InitFromRanges (inParRange, inDivGuess, outTick)) 
  {
    outTick.mMinorDivision = 1;
    return true;
  }
  return false;
}

