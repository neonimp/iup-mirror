
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "iupPlot.h"
#include "iup_cdutil.h"
#include "iup_image.h"


static inline void iPlotSetLine(cdCanvas* canvas, int inLineStyle, int inLineWidth)
{
  cdCanvasLineStyle(canvas, inLineStyle);
  cdCanvasLineWidth(canvas, inLineWidth);
}

static inline void iPlotSetMark(cdCanvas* canvas, int inMarkStyle, int inMarkSize)
{
  cdCanvasMarkType(canvas, inMarkStyle);
  cdCanvasMarkSize(canvas, inMarkSize);
}

static inline void iPlotDrawText(cdCanvas* canvas, double inX, double inY, int inAlignment, const char* inString)
{
  cdCanvasTextAlignment(canvas, inAlignment);
  cdfCanvasText(canvas, inX, inY, inString);
}

static inline void iPlotDrawRotatedText(cdCanvas* canvas, double inX, double inY, double inDegrees, int inAlignment, const char *inString)
{
  cdCanvasTextAlignment(canvas, inAlignment);
  double theOldOrientation = cdCanvasTextOrientation(canvas, inDegrees);
  cdfCanvasText(canvas, inX, inY, inString);
  cdCanvasTextOrientation(canvas, theOldOrientation);
}

static inline void iPlotDrawRect(cdCanvas* canvas, int inX, int inY, int inW, int inH)
{
  cdCanvasRect(canvas, inX, inX + inW - 1, inY, inY + inH - 1);
}

static inline void iPlotDrawRect(cdCanvas* canvas, double inX, double inY, double inW, double inH)
{
  cdfCanvasRect(canvas, inX, inX + inW - 1, inY, inY + inH - 1);
}

static inline void iPlotDrawBox(cdCanvas* canvas, double inX, double inY, double inW, double inH)
{
  cdfCanvasBox(canvas, inX, inX + inW - 1, inY, inY + inH - 1);
}

static void iPlotFillArrow(cdCanvas* canvas, int inX1, int inY1, int inX2, int inY2, int inX3, int inY3)
{
  cdCanvasBegin(canvas, CD_FILL);
  cdCanvasVertex(canvas, inX1, inY1);
  cdCanvasVertex(canvas, inX2, inY2);
  cdCanvasVertex(canvas, inX3, inY3);
  cdCanvasEnd(canvas);
}

static void iPlotDrawArrow(cdCanvas* canvas, double inX, double inY, int inVertical, int inDirection, int inSize)
{
  int theX = iupPlotRound(inX);
  int theY = iupPlotRound(inY);

  inSize += 2; // to avoid too small sizes

  int theSizeDir = iupPlotRound(inSize * 0.7);
  if (inVertical)
  {
    cdfCanvasLine(canvas, inX, inY, inX, inY + inDirection*inSize);  // complement the axis

    int theY1 = iupPlotRound(inY + inDirection*inSize);
    int theY2 = theY1 - inDirection*theSizeDir;
    iPlotFillArrow(canvas, theX, theY1,
                   theX - theSizeDir, theY2,
                   theX + theSizeDir, theY2);
  }
  else
  {
    cdfCanvasLine(canvas, inX, inY, inX + inDirection*inSize, inY);

    int theX1 = iupPlotRound(inX + inDirection*inSize);
    int theX2 = theX1 - inDirection*theSizeDir;
    iPlotFillArrow(canvas, theX1, theY,
                   theX2, theY - theSizeDir,
                   theX2, theY + theSizeDir);
  }
}


/************************************************************************************/


void iupPlotBox::Draw(const iupPlotRect &inRect, cdCanvas* canvas) const
{
  cdCanvasSetForeground(canvas, mColor);
  iPlotSetLine(canvas, mLineStyle, mLineWidth);

  iPlotDrawRect(canvas, inRect.mX, inRect.mY, inRect.mWidth, inRect.mHeight);
}

bool iupPlotGrid::DrawX(iupPlotTickIterBase* inTickIter, iupPlotTrafoBase* inTrafo, const iupPlotRect &inRect, cdCanvas* canvas) const
{
  if (mShowX)
  {
    if (!inTickIter->Init())
      return false;

    double theX;
    bool theIsMajorTick;

    cdCanvasSetForeground(canvas, mColor);
    iPlotSetLine(canvas, mLineStyle, mLineWidth);

    while (inTickIter->GetNextTick(theX, theIsMajorTick, NULL))
    {
      if ((theIsMajorTick && mMajor) || (!theIsMajorTick && !mMajor))
      {
        double theScreenX = inTrafo->Transform(theX);
        cdfCanvasLine(canvas, theScreenX, inRect.mY, theScreenX, inRect.mY + inRect.mHeight - 1);
      }
    }
  }

  return true;
}

bool iupPlotGrid::DrawY(iupPlotTickIterBase* inTickIter, iupPlotTrafoBase* inTrafo, const iupPlotRect &inRect, cdCanvas* canvas) const
{
  if (mShowY)
  {
    if (!inTickIter->Init())
      return false;

    double theY;
    bool theIsMajorTick;
    iupPlotRect theTickRect;

    cdCanvasSetForeground(canvas, mColor);
    iPlotSetLine(canvas, mLineStyle, mLineWidth);

    while (inTickIter->GetNextTick(theY, theIsMajorTick, NULL))
    {
      if ((theIsMajorTick && mMajor) || (!theIsMajorTick && !mMajor))
      {
        double theScreenY = inTrafo->Transform(theY);
        cdfCanvasLine(canvas, inRect.mX, theScreenY, inRect.mX + inRect.mWidth - 1, theScreenY);
      }
    }
  }

  return true;
}

bool iupPlotAxis::DrawX(const iupPlotRect &inRect, cdCanvas* canvas, const iupPlotAxis& inAxisY) const
{
  if (!mShow)
    return true;

  cdCanvasSetForeground(canvas, mColor);
  iPlotSetLine(canvas, CD_CONTINUOUS, mLineWidth);

  double theTargetY = 0;
  if (!mCrossOrigin)
  {
    if (!inAxisY.mReverse)
      theTargetY = inAxisY.mMin;
    else
      theTargetY = inAxisY.mMax;
  }
  if (inAxisY.mDiscrete)
    theTargetY -= 0.5;

  double theScreenY = inAxisY.mTrafo->Transform(theTargetY);
  double theScreenX1 = inRect.mX;
  double theScreenX2 = theScreenX1 + inRect.mWidth;

  cdfCanvasLine(canvas, theScreenX1, theScreenY, theScreenX2, theScreenY);

  if (mShowArrow)
  {
    if (!mReverse)
      iPlotDrawArrow(canvas, theScreenX2, theScreenY, 0, 1, mTick.mMinorSize);
    else
      iPlotDrawArrow(canvas, theScreenX1, theScreenY, 0, -1, mTick.mMinorSize);
  }

  if (mTick.mShow)
  {
    if (!mTickIter->Init())
      return false;

    double theX;
    bool theIsMajorTick;
    char theFormatString[30];
    strcpy(theFormatString, mTick.mFormatString);

    if (mTick.mShowNumber)
      SetFont(canvas, mTick.mFontStyle, mTick.mFontSize);

    while (mTickIter->GetNextTick(theX, theIsMajorTick, theFormatString))
    {
      if (!DrawXTick(theX, theScreenY, theIsMajorTick, theFormatString, canvas))
        return false;
    }

    theScreenY -= mTick.mMajorSize;  // skip major tick
    if (mTick.mShowNumber)
    {
      if (mTick.mRotateNumber)
      {
        int theXTickNumberWidth;
        GetTickNumberSize(canvas, &theXTickNumberWidth, NULL);
        theScreenY -= theXTickNumberWidth;
      }
      else
      {
        int theXTickNumberHeight;
        GetTickNumberSize(canvas, NULL, &theXTickNumberHeight);
        theScreenY -= theXTickNumberHeight;
      }
    }
  }

  if (GetLabel())
  {
    SetFont(canvas, mFontStyle, mFontSize);

    if (mLabelSpacing == -1)
    {
      int theXFontHeight;
      cdCanvasGetFontDim(canvas, NULL, &theXFontHeight, NULL, NULL);
      theScreenY -= theXFontHeight / 10;  // default spacing
    }
    else
      theScreenY -= mLabelSpacing;

    if (mLabelCentered)
    {
      double theScreenX = theScreenX1 + inRect.mWidth / 2;
      iPlotDrawText(canvas, theScreenX, theScreenY, CD_NORTH, GetLabel());
    }
    else
    {
      double theScreenX = theScreenX2;
      iPlotDrawText(canvas, theScreenX, theScreenY, CD_NORTH_EAST, GetLabel());
    }
  }

  return true;
}

bool iupPlotAxis::DrawXTick(double inX, double inScreenY, bool inMajor, const char*inFormatString, cdCanvas* canvas) const
{
  int theTickSize;
  double theScreenX = mTrafo->Transform(inX);
  if (inMajor)
  {
    theTickSize = mTick.mMajorSize;

    if (mTick.mShowNumber)
    {
      char theBuf[128];
      iupStrPrintfDoubleLocale(theBuf, inFormatString, inX, IupGetGlobal("DEFAULTDECIMALSYMBOL"));

      double theScreenY = inScreenY - theTickSize - mTick.mMinorSize;  // Use minor size as spacing
      if (mTick.mRotateNumber)
        iPlotDrawRotatedText(canvas, theScreenX, theScreenY, mTick.mRotateNumberAngle, CD_EAST, theBuf);
      else
        iPlotDrawText(canvas, theScreenX, theScreenY, CD_NORTH, theBuf);
    }
  }
  else
    theTickSize = mTick.mMinorSize;

  cdfCanvasLine(canvas, theScreenX, inScreenY, theScreenX, inScreenY - theTickSize);
  return true;
}

bool iupPlotAxis::DrawY(const iupPlotRect &inRect, cdCanvas* canvas, const iupPlotAxis& inAxisX) const
{
  if (!mShow)
    return true;

  cdCanvasSetForeground(canvas, mColor);
  iPlotSetLine(canvas, CD_CONTINUOUS, mLineWidth);

  double theTargetX = 0;
  if (!mCrossOrigin)
  {
    if (!inAxisX.mReverse)
      theTargetX = inAxisX.mMin;
    else
      theTargetX = inAxisX.mMax;
  }
  if (inAxisX.mDiscrete)
    theTargetX -= 0.5;

  double theScreenX = inAxisX.mTrafo->Transform(theTargetX);
  double theScreenY1 = inRect.mY;
  double theScreenY2 = theScreenY1 + inRect.mHeight;

  cdfCanvasLine(canvas, theScreenX, theScreenY1, theScreenX, theScreenY2);

  if (mShowArrow)
  {
    if (!mReverse)
      iPlotDrawArrow(canvas, theScreenX, theScreenY2, 1, 1, mTick.mMinorSize);
    else
      iPlotDrawArrow(canvas, theScreenX, theScreenY1, 1, -1, mTick.mMinorSize);
  }

  if (mTick.mShow)
  {
    if (!mTickIter->Init())
      return false;

    double theY;
    bool theIsMajorTick;
    char theFormatString[30];
    strcpy(theFormatString, mTick.mFormatString);

    if (mTick.mShowNumber)
      SetFont(canvas, mTick.mFontStyle, mTick.mFontSize);

    while (mTickIter->GetNextTick(theY, theIsMajorTick, theFormatString))
    {
      if (!DrawYTick(theY, theScreenX, theIsMajorTick, theFormatString, canvas))
        return false;
    }

    theScreenX -= mTick.mMajorSize;  // skip major tick
    if (mTick.mShowNumber)
    {
      if (mTick.mRotateNumber)
      {
        int theYTickNumberHeight;
        GetTickNumberSize(canvas, NULL, &theYTickNumberHeight);
        theScreenX -= theYTickNumberHeight;
      }
      else
      {
        int theYTickNumberWidth;
        GetTickNumberSize(canvas, &theYTickNumberWidth, NULL);
        theScreenX -= theYTickNumberWidth;
      }
    }
  }

  if (GetLabel())
  {
    SetFont(canvas, mFontStyle, mFontSize);

    if (mLabelSpacing == -1)
    {
      int theYFontHeight;
      cdCanvasGetFontDim(canvas, NULL, &theYFontHeight, NULL, NULL);
      theScreenX -= theYFontHeight / 10;  // default spacing
    }
    else
      theScreenX -= mLabelSpacing;

    if (mLabelCentered)
    {
      double theScreenY = theScreenY1 + inRect.mHeight / 2;
      iPlotDrawRotatedText(canvas, theScreenX, theScreenY, 90, CD_SOUTH, GetLabel());
    }
    else
    {
      double theScreenY = theScreenY2;
      iPlotDrawRotatedText(canvas, theScreenX, theScreenY, 90, CD_SOUTH_EAST, GetLabel());
    }
  }

  return true;
}

bool iupPlotAxis::DrawYTick(double inY, double inScreenX, bool inMajor, const char* inFormatString, cdCanvas* canvas) const
{
  int theTickSize;
  double theScreenY = mTrafo->Transform(inY);
  if (inMajor)
  {
    theTickSize = mTick.mMajorSize;

    if (mTick.mShowNumber)
    {
      char theBuf[128];
      iupStrPrintfDoubleLocale(theBuf, inFormatString, inY, IupGetGlobal("DEFAULTDECIMALSYMBOL"));

      double theScreenX = inScreenX - theTickSize - mTick.mMinorSize;  // Use minor size as spacing
      if (mTick.mRotateNumber)
        iPlotDrawRotatedText(canvas, theScreenX, theScreenY, mTick.mRotateNumberAngle, CD_SOUTH, theBuf);
      else
        iPlotDrawText(canvas, theScreenX, theScreenY, CD_EAST, theBuf);
    }
  }
  else
    theTickSize = mTick.mMinorSize;

  cdfCanvasLine(canvas, inScreenX, theScreenY, inScreenX - theTickSize, theScreenY);
  return true;
}

void iupPlot::DrawCrossSamplesH(const iupPlotRect &inRect, const iupPlotDataBase *inXData, const iupPlotDataBase *inYData, cdCanvas* canvas) const
{
  int theCount = inXData->GetCount();
  if (theCount == 0)
    return;

  double theXTarget = mAxisX.mTrafo->TransformBack((double)mCrossHairX);
  bool theFirstIsLess = inXData->GetSample(0) < theXTarget;

  for (int i = 0; i < theCount; i++)
  {
    double theX = inXData->GetSample(i);
    bool theCurrentIsLess = theX < theXTarget;

    if (theCurrentIsLess != theFirstIsLess)
    {
      double theY = inYData->GetSample(i);
      int theScreenY = iupPlotRound(mAxisY.mTrafo->Transform(theY)); // transform to pixels
      // Draw a horizontal line at data Y coordinate
      cdfCanvasLine(canvas, inRect.mX, theScreenY, inRect.mX + inRect.mWidth - 1, theScreenY);

      theFirstIsLess = theCurrentIsLess;
    }
  }
}

void iupPlot::DrawCrossHairH(const iupPlotRect &inRect, cdCanvas* canvas) const
{
  cdCanvasSetForeground(canvas, mAxisY.mColor);
  iPlotSetLine(canvas, CD_CONTINUOUS, 1);

  // Draw a vertical line at cursor X coordinate
  cdCanvasLine(canvas, mCrossHairX, inRect.mY, mCrossHairX, inRect.mY + inRect.mHeight - 1);

  for (int ds = 0; ds < mDataSetListCount; ds++)
  {
    iupPlotDataSet* dataset = mDataSetList[ds];

    const iupPlotDataBase *theXData = dataset->GetDataX();
    const iupPlotDataBase *theYData = dataset->GetDataY();

    cdCanvasSetForeground(canvas, dataset->mColor);

    DrawCrossSamplesH(inRect, theXData, theYData, canvas);
  }
}

void iupPlot::DrawCrossSamplesV(const iupPlotRect &inRect, const iupPlotDataBase *inXData, const iupPlotDataBase *inYData, cdCanvas* canvas) const
{
  int theCount = inXData->GetCount();
  if (theCount == 0)
    return;

  double theYTarget = mAxisY.mTrafo->TransformBack((double)mCrossHairY);
  bool theFirstIsLess = inYData->GetSample(0) < theYTarget;

  for (int i = 0; i < theCount; i++)
  {
    double theY = inYData->GetSample(i);
    bool theCurrentIsLess = theY < theYTarget;

    if (theCurrentIsLess != theFirstIsLess)
    {
      double theX = inXData->GetSample(i);
      int theScreenX = iupPlotRound(mAxisX.mTrafo->Transform(theX)); // transform to pixels
      // Draw a vertical line at data X coordinate
      cdfCanvasLine(canvas, theScreenX, inRect.mY, theScreenX, inRect.mY + inRect.mHeight - 1);

      theFirstIsLess = theCurrentIsLess;
    }
  }
}

void iupPlot::DrawCrossHairV(const iupPlotRect &inRect, cdCanvas* canvas) const
{
  cdCanvasSetForeground(canvas, mAxisX.mColor);
  iPlotSetLine(canvas, CD_CONTINUOUS, 1);

  // Draw an horizontal line at cursor Y coordinate
  cdCanvasLine(canvas, inRect.mX, mCrossHairY, inRect.mX + inRect.mWidth - 1, mCrossHairY);

  for (int ds = 0; ds < mDataSetListCount; ds++)
  {
    iupPlotDataSet* dataset = mDataSetList[ds];

    const iupPlotDataBase *theXData = dataset->GetDataX();
    const iupPlotDataBase *theYData = dataset->GetDataY();

    cdCanvasSetForeground(canvas, dataset->mColor);

    DrawCrossSamplesV(inRect, theXData, theYData, canvas);
  }
}

void iupPlot::SetTitleFont(cdCanvas* canvas) const
{
  int theFontSize = mTitle.mFontSize;
  if (theFontSize == 0)
  {
    int size = IupGetInt(ih, "FONTSIZE");
    if (size > 0) size += 6;
    else size -= 8;

    theFontSize = size;
  }

  SetFont(canvas, mTitle.mFontStyle, theFontSize);
}

void iupPlot::DrawTitle(cdCanvas* canvas) const
{
  if (mTitle.GetText())
  {
    cdCanvasSetForeground(canvas, mTitle.mColor);

    SetTitleFont(canvas);

    cdCanvasTextAlignment(canvas, CD_NORTH);
    cdCanvasText(canvas, mTitle.mPosX, cdCanvasInvertYAxis(canvas, mTitle.mPosY), mTitle.GetText());
  }
}

void iupPlot::DrawBackground(cdCanvas* canvas) const
{
  cdCanvasSetForeground(canvas, mBack.mColor);
  cdCanvasBox(canvas, 0, mViewport.mWidth - 1, 0, mViewport.mHeight - 1);
}

void iupPlot::DrawInactive(cdCanvas* canvas) const
{
  long inactive_color = cdEncodeAlpha(CD_GRAY, 96);
  cdCanvasSetForeground(canvas, inactive_color);
  cdCanvasBox(canvas, 0, mViewport.mWidth - 1, 0, mViewport.mHeight - 1);
}

void iupPlot::DrawBackgroundImage(cdCanvas* canvas) const
{
  Ihandle* image = iupImageGetHandle(mBack.GetImage());
  if (image)
  {
    double theScreenMinX = mAxisX.mTrafo->Transform(mBack.mImageMinX);
    double theScreenMinY = mAxisY.mTrafo->Transform(mBack.mImageMinY);
    double theScreenMaxX = mAxisX.mTrafo->Transform(mBack.mImageMaxX);
    double theScreenMaxY = mAxisY.mTrafo->Transform(mBack.mImageMaxY);

    double theScreenW = theScreenMaxX - theScreenMinX + 1;
    double theScreenH = theScreenMaxY - theScreenMinY + 1;

    int theX = iupPlotRound(theScreenMinX);
    int theY = iupPlotRound(theScreenMinY);
    int theW = iupPlotRound(theScreenW);
    int theH = iupPlotRound(theScreenH);

    cdIupDrawImage(canvas, image, theX, theY, theW, theH, 0, mBack.mColor);
  }
}

bool iupPlot::DrawLegend(const iupPlotRect &inRect, cdCanvas* canvas, iupPlotRect &ioPos) const
{
  if (mLegend.mShow)
  {
    int ds;
    int theFontHeight;

    SetFont(canvas, mLegend.mFontStyle, mLegend.mFontSize);
    cdCanvasGetFontDim(canvas, NULL, &theFontHeight, NULL, NULL);

    int theMargin = theFontHeight / 2;
    if (mLegend.mPosition == IUP_PLOT_BOTTOMCENTER)
      theMargin = 0;
    int theTotalHeight = mDataSetListCount*theFontHeight + 2 * theMargin;
    int theLineSpace = 20;

    int theWidth, theMaxWidth = 0;
    for (ds = 0; ds < mDataSetListCount; ds++)
    {
      iupPlotDataSet* dataset = mDataSetList[ds];

      cdCanvasGetTextSize(canvas, dataset->GetName(), &theWidth, NULL);

      if (dataset->mMode == IUP_PLOT_MARK || dataset->mMode == IUP_PLOT_MARKLINE)
      {
        if (dataset->mMarkSize + 6 > theLineSpace)
          theLineSpace = dataset->mMarkSize + 6;
      }

      theWidth += theLineSpace;

      if (theWidth > theMaxWidth)
        theMaxWidth = theWidth;
    }

    if (theMaxWidth == 0)
      return false;

    theMaxWidth += 2 * theMargin;

    int theScreenX, theScreenY;

    if (mLegend.mPosition == IUP_PLOT_XY)
    {
      theScreenX = ioPos.mX;
      theScreenY = cdCanvasInvertYAxis(canvas, ioPos.mY);
    }
    else
    {
      theScreenX = inRect.mX;
      theScreenY = inRect.mY;

      switch (mLegend.mPosition)
      {
      case IUP_PLOT_TOPLEFT:
        theScreenX += 2;
        theScreenY += inRect.mHeight - theTotalHeight - 2;
        break;
      case IUP_PLOT_BOTTOMLEFT:
        theScreenX += 2;
        theScreenY += 2;
        break;
      case IUP_PLOT_BOTTOMRIGHT:
        theScreenX += inRect.mWidth - theMaxWidth - 2;
        theScreenY += 2;
        break;
      case IUP_PLOT_BOTTOMCENTER:
        theScreenX += (inRect.mWidth - theMaxWidth) / 2;
        theScreenY = theFontHeight / 4;
        break;
      default: // IUP_PLOT_TOPRIGHT
        theScreenX += inRect.mWidth - theMaxWidth - 2;
        theScreenY += inRect.mHeight - theTotalHeight - 2;
        break;
      }

      ioPos.mX = theScreenX;
      ioPos.mY = cdCanvasInvertYAxis(canvas, theScreenY);
    }

    ioPos.mWidth = theMaxWidth;
    ioPos.mHeight = theTotalHeight;

    // Clip to the legend box
    cdCanvasClipArea(canvas, theScreenX, theScreenX + theMaxWidth - 1,
                     theScreenY, theScreenY + theTotalHeight - 1);

    if (mLegend.mBoxShow)
    {
      cdCanvasSetForeground(canvas, mLegend.mBoxBackColor);
      iPlotDrawBox(canvas, theScreenX + 1, theScreenY + 1, theMaxWidth - 2, theTotalHeight - 2);

      cdCanvasSetForeground(canvas, mLegend.mBoxColor);
      iPlotSetLine(canvas, mLegend.mBoxLineStyle, mLegend.mBoxLineWidth);
      iPlotDrawRect(canvas, theScreenX, theScreenY, theMaxWidth, theTotalHeight);
    }

    for (ds = 0; ds < mDataSetListCount; ds++)
    {
      iupPlotDataSet* dataset = mDataSetList[ds];

      cdCanvasSetForeground(canvas, dataset->mColor);

      int theLegendX = theScreenX + theMargin;
      int theLegendY = theScreenY + (mDataSetListCount - 1 - ds)*theFontHeight + theMargin;

      theLegendY += theFontHeight / 2;

      if (dataset->mMode == IUP_PLOT_MARK || dataset->mMode == IUP_PLOT_MARKLINE)
      {
        iPlotSetMark(canvas, dataset->mMarkStyle, dataset->mMarkSize);
        cdCanvasMark(canvas, theLegendX + (theLineSpace - 3) / 2, theLegendY - theFontHeight / 8);
      }
      if (dataset->mMode != IUP_PLOT_MARK)
      {
        iPlotSetLine(canvas, dataset->mLineStyle, dataset->mLineWidth);
        cdCanvasLine(canvas, theLegendX, theLegendY - theFontHeight / 8,
                     theLegendX + theLineSpace - 3, theLegendY - theFontHeight / 8);
      }

      iPlotDrawText(canvas, theLegendX + theLineSpace, theLegendY, CD_WEST, dataset->GetName());
    }
  }

  return true;
}


/************************************************************************************/


void iupPlotDataSet::DrawDataLine(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, const iupPlotSampleNotify* inNotify, bool inShowMark, bool inErrorBar) const
{
  int theCount = mDataX->GetCount();
  cdCanvasBegin(canvas, CD_OPEN_LINES);

  for (int i = 0; i < theCount; i++)
  {
    double theX = mDataX->GetSample(i);
    double theY = mDataY->GetSample(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    if (inNotify)
      inNotify->cb(inNotify->ih, inNotify->ds, i, theX, theY, (int)mSelection->GetSampleBool(i));

    if (inShowMark)
    {
      if (mExtra)
      {
        if (inErrorBar)
          DrawErrorBar(inTrafoY, canvas, i, theY, theScreenX);
        else
          SetSampleExtraMarkSize(inTrafoY, canvas, i);
      }

      // No problem that will be drawn before the polygon, they both should have the same color
      cdfCanvasMark(canvas, theScreenX, theScreenY);
    }

    if (mSegment && mSegment->GetSampleBool(i))
    {
      cdCanvasEnd(canvas);
      cdCanvasBegin(canvas, CD_OPEN_LINES);
    }

    cdfCanvasVertex(canvas, theScreenX, theScreenY);
  }

  cdCanvasEnd(canvas);
}

void iupPlotDataSet::DrawErrorBar(const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, int index, double theY, double theScreenX) const
{
  double theError = mExtra->GetSample(index);
  double theScreenErrorY1 = inTrafoY->Transform(theY - theError);
  double theScreenErrorY2 = inTrafoY->Transform(theY + theError);

  double theBarWidth = (double)mMarkSize;  /* fixed size in screen coordinates */

  cdfCanvasLine(canvas, theScreenX, theScreenErrorY1, theScreenX, theScreenErrorY2);
  cdfCanvasLine(canvas, theScreenX - theBarWidth, theScreenErrorY1, theScreenX + theBarWidth, theScreenErrorY1);
  cdfCanvasLine(canvas, theScreenX - theBarWidth, theScreenErrorY2, theScreenX + theBarWidth, theScreenErrorY2);
}

void iupPlotDataSet::SetSampleExtraMarkSize(const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, int inSampleIndex) const 
{
  double theMarkSize = mExtra->GetSample(inSampleIndex);
  int theScreenSize = 1;
  if (theMarkSize != 0)
    theScreenSize = iupPlotRound(inTrafoY->Transform(theMarkSize));
  if (theScreenSize < 1) theScreenSize = 1;

  cdCanvasMarkSize(canvas, theScreenSize);
}

void iupPlotDataSet::DrawDataMark(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, const iupPlotSampleNotify* inNotify) const
{
  int theCount = mDataX->GetCount();
  for (int i = 0; i < theCount; i++)
  {
    double theX = mDataX->GetSample(i);
    double theY = mDataY->GetSample(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    if (inNotify)
      inNotify->cb(inNotify->ih, inNotify->ds, i, theX, theY, (int)mSelection->GetSampleBool(i));

    if (mExtra)
      SetSampleExtraMarkSize(inTrafoY, canvas, i);

    cdfCanvasMark(canvas, theScreenX, theScreenY);
  }
}

void iupPlotDataSet::DrawDataStem(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, const iupPlotSampleNotify* inNotify, bool inShowMark) const
{
  int theCount = mDataX->GetCount();
  for (int i = 0; i < theCount; i++)
  {
    double theX = mDataX->GetSample(i);
    double theY = mDataY->GetSample(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);
    double theScreenY0 = inTrafoY->Transform(0.0);

    if (inNotify)
      inNotify->cb(inNotify->ih, inNotify->ds, i, theX, theY, (int)mSelection->GetSampleBool(i));

    if (inShowMark)
    {
      if (mExtra)
        SetSampleExtraMarkSize(inTrafoY, canvas, i);

      cdfCanvasMark(canvas, theScreenX, theScreenY);
    }

    cdfCanvasLine(canvas, theScreenX, theScreenY0, theScreenX, theScreenY);
  }
}

void iupPlotDataSet::DrawDataArea(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, const iupPlotSampleNotify* inNotify) const
{
  int theCount = mDataX->GetCount();
  cdCanvasBegin(canvas, CD_FILL);
  double theScreenY0 = inTrafoY->Transform(0);
  double theLastX = 0;

  for (int i = 0; i < theCount; i++)
  {
    double theX = mDataX->GetSample(i);
    double theY = mDataY->GetSample(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    if (inNotify)
      inNotify->cb(inNotify->ih, inNotify->ds, i, theX, theY, (int)mSelection->GetSampleBool(i));

    if (i == 0)
      cdfCanvasVertex(canvas, theScreenX, theScreenY0);

    if (mSegment && mSegment->GetSampleBool(i))
    {
      cdfCanvasVertex(canvas, theLastX, theScreenY0);
      cdfCanvasVertex(canvas, theScreenX, theScreenY0);
    }

    cdfCanvasVertex(canvas, theScreenX, theScreenY);

    if (i == theCount - 1)
      cdfCanvasVertex(canvas, theScreenX, theScreenY0);

    theLastX = theScreenX;
  }

  cdCanvasEnd(canvas);
}

void iupPlotDataSet::DrawDataBar(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, const iupPlotSampleNotify* inNotify) const
{
  int theCount = mDataX->GetCount();
  double theScreenY0 = inTrafoY->Transform(0);

  double theMinX = mDataX->GetSample(0);
  double theScreenMinX = inTrafoX->Transform(theMinX);
  double theMaxX = mDataX->GetSample(theCount - 1);
  double theScreenMaxX = inTrafoX->Transform(theMaxX);

  double theBarWidth = (theScreenMaxX - theScreenMinX) / (theCount - 1);
  theBarWidth *= 1 - (double)mBarSpacingPercent/100.0;

  for (int i = 0; i < theCount; i++)
  {
    double theX = mDataX->GetSample(i);
    double theY = mDataY->GetSample(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    double theBarX = theScreenX - theBarWidth / 2;
    double theBarHeight = theScreenY - theScreenY0;

    if (inNotify)
      inNotify->cb(inNotify->ih, inNotify->ds, i, theX, theY, (int)mSelection->GetSampleBool(i));

    iPlotDrawBox(canvas, theBarX, theScreenY0, theBarWidth, theBarHeight);

    if (mBarShowOutline)
    {
      long oldColor = cdCanvasForeground(canvas, mBarOutlineColor);
      iPlotDrawRect(canvas, theBarX, theScreenY0, theBarWidth, theBarHeight);
      cdCanvasForeground(canvas, oldColor);
    }
  }
}

void iupPlotDataSet::DrawDataHorizontalBar(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, const iupPlotSampleNotify* inNotify) const
{
  int theCount = mDataX->GetCount();
  double theScreenX0 = inTrafoX->Transform(0);


  double theMinY = mDataY->GetSample(0);
  double theScreenMinY = inTrafoY->Transform(theMinY);
  double theMaxY = mDataY->GetSample(theCount - 1);
  double theScreenMaxY = inTrafoY->Transform(theMaxY);

  double theBarHeight = (theScreenMaxY - theScreenMinY) / (theCount - 1);
  theBarHeight *= 1 - (double)mBarSpacingPercent / 100.0;

  for (int i = 0; i < theCount; i++)
  {
    double theX = mDataX->GetSample(i);
    double theY = mDataY->GetSample(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    double theBarY = theScreenY - theBarHeight / 2;
    double theBarWidth = theScreenX - theScreenX0;

    if (inNotify)
      inNotify->cb(inNotify->ih, inNotify->ds, i, theX, theY, (int)mSelection->GetSampleBool(i));

    iPlotDrawBox(canvas, theScreenX0, theBarY, theBarWidth, theBarHeight);
  }
}

void iupPlotDataSet::DrawDataMultiBar(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, const iupPlotSampleNotify* inNotify) const
{
  int theCount = mDataX->GetCount();
  double theScreenY0 = inTrafoY->Transform(0);

  double theMinX = mDataX->GetSample(0);
  double theScreenMinX = inTrafoX->Transform(theMinX);
  double theMaxX = mDataX->GetSample(theCount - 1);
  double theScreenMaxX = inTrafoX->Transform(theMaxX);

  double theTotalBarWidth = (theScreenMaxX - theScreenMinX) / (theCount - 1);
  theTotalBarWidth *= 1 - (double)mBarSpacingPercent / 100.0;
  double theBarWidth = theTotalBarWidth / mMultibarCount;

  for (int i = 0; i < theCount; i++)
  {
    double theX = mDataX->GetSample(i);
    double theY = mDataY->GetSample(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    double theBarX = (theScreenX - theTotalBarWidth / 2) + (mMultibarIndex*theBarWidth);
    double theBarHeight = theScreenY - theScreenY0;

    if (inNotify)
      inNotify->cb(inNotify->ih, inNotify->ds, i, theX, theY, (int)mSelection->GetSampleBool(i));

    iPlotDrawBox(canvas, theBarX, theScreenY0, theBarWidth, theBarHeight);
  }
}

void iupPlotDataSet::DrawDataStep(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, const iupPlotSampleNotify* inNotify) const
{
  int theCount = mDataX->GetCount();
  cdCanvasBegin(canvas, CD_OPEN_LINES);
  double thePreviousScreenX = 0.;

  for (int i = 0; i < theCount; i++)
  {
    double theX = mDataX->GetSample(i);
    double theY = mDataY->GetSample(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    if (inNotify)
      inNotify->cb(inNotify->ih, inNotify->ds, i, theX, theY, (int)mSelection->GetSampleBool(i));

    if (mSegment && mSegment->GetSampleBool(i))
    {
      cdCanvasEnd(canvas);
      cdCanvasBegin(canvas, CD_OPEN_LINES);
    }

    if (i > 0)
      cdfCanvasVertex(canvas, thePreviousScreenX, theScreenY);

    cdfCanvasVertex(canvas, theScreenX, theScreenY);

    thePreviousScreenX = theScreenX;
  }

  cdCanvasEnd(canvas);
}

void iupPlotDataSet::DrawSelection(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, const iupPlotSampleNotify* inNotify) const
{
  int theCount = mDataX->GetCount();

  cdCanvasMarkSize(canvas, 7);

  for (int i = 0; i < theCount; i++)
  {
    if (mSelection->GetSampleBool(i))
    {
      double theX = mDataX->GetSample(i);
      double theY = mDataY->GetSample(i);
      double theScreenX = inTrafoX->Transform(theX);
      double theScreenY = inTrafoY->Transform(theY);

      if (inNotify)
      {
        int ret = inNotify->cb(inNotify->ih, inNotify->ds, i, theX, theY, (int)mSelection->GetSampleBool(i));
        if (ret == IUP_IGNORE)
          continue;
      }

      cdCanvasMarkType(canvas, CD_BOX);
      cdCanvasSetForeground(canvas, cdEncodeAlpha(CD_GRAY, 128));
      cdfCanvasMark(canvas, theScreenX, theScreenY);

      cdCanvasMarkType(canvas, CD_HOLLOW_BOX);
      cdCanvasSetForeground(canvas, cdEncodeAlpha(CD_BLACK, 128));
      cdfCanvasMark(canvas, theScreenX, theScreenY);
    }
  }
}

void iupPlotDataSet::DrawData(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas, const iupPlotSampleNotify* inNotify) const
{
  int theXCount = mDataX->GetCount();
  int theYCount = mDataY->GetCount();

  if ((theXCount == 0) || (theYCount == 0))
    return;

  if (theXCount != theYCount)
    return;

  cdCanvasSetForeground(canvas, mColor);
  iPlotSetLine(canvas, mLineStyle, mLineWidth);
  iPlotSetMark(canvas, mMarkStyle, mMarkSize);

  switch (mMode)
  {
  case IUP_PLOT_LINE:
    DrawDataLine(inTrafoX, inTrafoY, canvas, inNotify, false, false);
    break;
  case IUP_PLOT_MARK:
    DrawDataMark(inTrafoX, inTrafoY, canvas, inNotify);
    break;
  case IUP_PLOT_STEM:
    DrawDataStem(inTrafoX, inTrafoY, canvas, inNotify, false);
    break;
  case IUP_PLOT_MARKSTEM:
    DrawDataStem(inTrafoX, inTrafoY, canvas, inNotify, true);
    break;
  case IUP_PLOT_MARKLINE:
    DrawDataLine(inTrafoX, inTrafoY, canvas, inNotify, true, false);
    break;
  case IUP_PLOT_AREA:
    DrawDataArea(inTrafoX, inTrafoY, canvas, inNotify);
    break;
  case IUP_PLOT_BAR:
    DrawDataBar(inTrafoX, inTrafoY, canvas, inNotify);
    break;
  case IUP_PLOT_HORIZONTALBAR:
    DrawDataHorizontalBar(inTrafoX, inTrafoY, canvas, inNotify);
    break;
  case IUP_PLOT_MULTIBAR:
    DrawDataMultiBar(inTrafoX, inTrafoY, canvas, inNotify);
    break;
  case IUP_PLOT_ERRORBAR:
    DrawDataLine(inTrafoX, inTrafoY, canvas, inNotify, true, true);
    break;
  case IUP_PLOT_STEP:
    DrawDataStep(inTrafoX, inTrafoY, canvas, inNotify);
    break;
  }

  if (mHasSelected)
    DrawSelection(inTrafoX, inTrafoY, canvas, inNotify);
}
