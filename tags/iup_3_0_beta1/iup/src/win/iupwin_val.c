/** \file
 * \brief Valuator Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_val.h"
#include "iup_drv.h"

#include "iupwin_drv.h"
#include "iupwin_handle.h"
#include "iupwin_draw.h"


#ifndef SHRT_MAX
#define SHRT_MAX 32767
#endif

void iupdrvValGetMinSize(Ihandle* ih, int *w, int *h)
{
  int ticks_size = 0;
  if (ih->data->show_ticks)
  {
    char* tickspos = iupAttribGetStr(ih, "TICKSPOS");
    if(iupStrEqualNoCase(tickspos, "BOTH"))
      ticks_size = 2*8;
    else 
      ticks_size = 8;
  }

  if (ih->data->type == IVAL_HORIZONTAL)
  {
    *w = 35;
    *h = 30+ticks_size;
  }
  else
  {
    *w = 30+ticks_size;
    *h = 35;
  }
}

static int winValSetStepAttrib(Ihandle* ih, const char* value)
{
  int linesize;
  ih->data->step = atof(value);
  linesize = (int)(ih->data->step*SHRT_MAX);
  SendMessage(ih->handle, TBM_SETLINESIZE, 0, linesize);
  return 0; /* do not store value in hash table */
}

static int winValSetPageStepAttrib(Ihandle* ih, const char* value)
{
  int pagesize;
  ih->data->pagestep = atof(value);
  pagesize = (int)(ih->data->pagestep*SHRT_MAX);
  SendMessage(ih->handle, TBM_SETPAGESIZE, 0, pagesize);
  return 0; /* do not store value in hash table */
}

static int winValSetShowTicksAttrib(Ihandle* ih, const char* value)
{
  int tick_freq, show_ticks;

  if (!ih->data->show_ticks)  /* can only set if already not zero */
    return 0;

  show_ticks = atoi(value);
  if (!show_ticks)
    return 0;

  ih->data->show_ticks = show_ticks;
  if (ih->data->show_ticks<2) ih->data->show_ticks=2;

  /* Defines the interval frequency for tick marks */
  tick_freq = SHRT_MAX/(ih->data->show_ticks-1);
  SendMessage(ih->handle, TBM_SETTICFREQ, tick_freq, 0);
  return 0;
}

static int winValSetValueAttrib(Ihandle* ih, const char* value)
{
  int ival;

  ih->data->val = atof(value);
  iupValCropValue(ih);

  ival = (int)(((ih->data->val-ih->data->vmin)/(ih->data->vmax - ih->data->vmin))*SHRT_MAX);
  if (ih->data->inverted)
    ival = SHRT_MAX-ival;

  SendMessage(ih->handle, TBM_SETPOS, TRUE, ival);
  return 0; /* do not store value in hash table */
}


/*********************************************************************************************/


static int winValCtlColor(Ihandle* ih, HDC hdc, LRESULT *result)
{
  COLORREF cr;
  if (iupwinGetParentBgColor(ih, &cr))
  {
    SetDCBrushColor(hdc, cr);
    *result = (LRESULT)GetStockObject(DC_BRUSH);
    return 1;
  }
  return 0;
}

static int winValCustomScroll(Ihandle* ih, int msg)
{
  int ival;
  IFnd cb = NULL;

  switch (msg)
  {
    case TB_BOTTOM:
    case TB_TOP:
    case TB_LINEDOWN:
    case TB_LINEUP:
    case TB_PAGEDOWN:
    case TB_PAGEUP:
    {
      cb = (IFnd) IupGetCallback(ih, "BUTTON_PRESS_CB");
      break;
    }
    case TB_THUMBPOSITION:
    {
      cb = (IFnd) IupGetCallback(ih, "BUTTON_RELEASE_CB");
      break;
    }
    case TB_THUMBTRACK:
    {
      cb = (IFnd) IupGetCallback(ih, "MOUSEMOVE_CB");
      break;
    }
  }

  if (!cb)
    cb = (IFnd)IupGetCallback(ih, "CHANGEVALUE_CB");

  ival = (int)SendMessage(ih->handle, TBM_GETPOS, 0, 0);
  if (ih->data->inverted)
    ival = SHRT_MAX-ival;

  ih->data->val = (((double)ival/(double)SHRT_MAX)*(ih->data->vmax - ih->data->vmin)) + ih->data->vmin;
  iupValCropValue(ih);

  if (cb)
    cb(ih, ih->data->val);

  return 0; /* not used */
}

static void winValIncPageValue(Ihandle *ih, int dir)
{
  int pagesize, ival;
  pagesize = (int)(ih->data->pagestep*SHRT_MAX);

  ival = (int)SendMessage(ih->handle, TBM_GETPOS, 0, 0);
  ival += dir*pagesize;
  if (ival < 0) ival = 0;
  if (ival > SHRT_MAX) ival = SHRT_MAX;
  SendMessage(ih->handle, TBM_SETPOS, TRUE, ival);

  winValCustomScroll(ih, 0);
}

static int winValProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  (void)lp;

  switch (msg)
  {
  case WM_ERASEBKGND:
    {
      RECT rect;
      HDC hDC = (HDC)wp;
      GetClientRect(ih->handle, &rect); 
      iupwinDrawParentBackground(ih, hDC, &rect);

      /* return non zero value */
      *result = 1;
      return 1;
    }
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
    {
      if (GetKeyState(VK_CONTROL) & 0x8000)  /* handle Ctrl+Arrows */
      {
        if (wp == VK_UP || wp == VK_LEFT)
        {
          winValIncPageValue(ih, -1);
          *result = 0;
          return 1;
        }
        if (wp == VK_RIGHT || wp == VK_DOWN)
        {
          winValIncPageValue(ih, 1);
          *result = 0;
          return 1;
        }
      }
    }
  }

  return iupwinBaseProc(ih, msg, wp, lp, result);
}


/*********************************************************************************************/


static int winValMapMethod(Ihandle* ih)
{
  DWORD dwStyle = WS_CHILD | TBS_AUTOTICKS;
  int show_ticks;

  if (!ih->parent)
    return IUP_ERROR;

  /* Track bar Orientation */
  if (ih->data->type == IVAL_HORIZONTAL)
    dwStyle |= TBS_HORZ;
  else
    dwStyle |= TBS_VERT;

  if (iupStrBoolean(iupAttribGetStrDefault(ih, "CANFOCUS")))
    dwStyle |= WS_TABSTOP;

  /* Track bar Ticks */
  show_ticks = IupGetInt(ih, "SHOWTICKS");
  if (!show_ticks)
  {
    dwStyle |= TBS_NOTICKS;    /* No show_ticks */
  }
  else
  {
    char* tickspos;

    if (show_ticks<2) show_ticks=2;
    ih->data->show_ticks = show_ticks; /* non zero value, can be changed later, but not to zero */

    /* Defines the position of tick marks */
    tickspos = iupAttribGetStr(ih, "TICKSPOS");
    if(iupStrEqualNoCase(tickspos, "BOTH"))
      dwStyle |= TBS_BOTH;
    else if(iupStrEqualNoCase(tickspos, "REVERSE"))
      dwStyle |= TBS_BOTTOM;  /* same as TBS_RIGHT */
    else /* NORMAL */
      dwStyle |= TBS_TOP;     /* same as TBS_LEFT  */
  }

  if (!iupwinCreateWindowEx(ih, TRACKBAR_CLASS, 0, dwStyle))
    return IUP_ERROR;

  /* Process Keyboard */
  IupSetCallback(ih, "_IUPWIN_CTRLPROC_CB", (Icallback)winValProc);

  /* Process Val Scroll commands */
  IupSetCallback(ih, "_IUPWIN_CUSTOMSCROLL_CB", (Icallback)winValCustomScroll);

  /* Process background color */
  IupSetCallback(ih, "_IUPWIN_CTLCOLOR_CB", (Icallback)winValCtlColor);

  /* ensure the default values, that are different from the native ones */
  SendMessage(ih->handle, TBM_SETRANGEMIN, FALSE, 0);
  SendMessage(ih->handle, TBM_SETRANGEMAX, FALSE, SHRT_MAX);
  winValSetPageStepAttrib(ih, iupAttribGetStrDefault(ih, "PAGESTEP"));
  winValSetStepAttrib(ih, iupAttribGetStrDefault(ih, "STEP"));

  if (ih->data->inverted)
    SendMessage(ih->handle, TBM_SETPOS, FALSE, SHRT_MAX);  /* default initial position is at MIN */

  return IUP_NOERROR;
}

void iupdrvValInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = winValMapMethod;

  /* IupVal only */
  iupClassRegisterAttribute(ic, "VALUE", iupValGetValueAttrib, winValSetValueAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);  
  iupClassRegisterAttribute(ic, "SHOWTICKS", iupValGetShowTicksAttrib, winValSetShowTicksAttrib, "0", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "PAGESTEP", iupValGetPageStepAttrib, winValSetPageStepAttrib, "0.1", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "STEP", iupValGetStepAttrib, winValSetStepAttrib, "0.01", IUP_MAPPED, IUP_NO_INHERIT);
}