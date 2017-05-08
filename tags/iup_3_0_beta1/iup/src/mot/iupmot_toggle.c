/** \file
 * \brief Toggle Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <Xm/Xm.h>
#include <Xm/ToggleB.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_attrib.h"
#include "iup_dialog.h"
#include "iup_str.h"
#include "iup_toggle.h"
#include "iup_drv.h"
#include "iup_image.h"
#include "iup_key.h"

#include "iupmot_drv.h"
#include "iupmot_color.h"


void iupdrvToggleAddCheckBox(int *x, int *y)
{
  (*x) += 15+6+3;
  if ((*y) < 15+6) (*y) = 12+6; /* minimum height */
  (*y) += 6;
}


/*********************************************************************************/


static int motToggleSetBgColorAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_TOGGLE_TEXT)
  {
    char* parent_value = iupAttribGetStrNativeParent(ih, "BGCOLOR");
    if (!parent_value)
    {
      /* if not defined at a native parent, 
         then change the toggle button color to the given color instead using the default */
      if (iupdrvBaseSetBgColorAttrib(ih, value))  /* let XmChangeColor do its job */
      {
        parent_value = IupGetGlobal("DLGBGCOLOR");
        XtVaSetValues(ih->handle, XmNbackground, iupmotColorGetPixelStr(parent_value), NULL);  /* reset just the background */

        if (ih->data->radio)
          XtVaSetValues(ih->handle, XmNselectColor, iupmotColorGetPixel(0, 0, 0), NULL);
        XtVaSetValues(ih->handle, XmNunselectColor, iupmotColorGetPixelStr(value), NULL);
        return 1;
      }
    }
    else
    {
      /* ignore given value, must use only from parent */
      if (iupdrvBaseSetBgColorAttrib(ih, parent_value))
      {
        if (ih->data->radio)
          XtVaSetValues(ih->handle, XmNselectColor, iupmotColorGetPixel(0, 0, 0), NULL);
        XtVaSetValues(ih->handle, XmNunselectColor, iupmotColorGetPixelStr(parent_value), NULL);
        return 1;
      }
    }
    return 0; 
  }
  else
    return iupdrvBaseSetBgColorAttrib(ih, value); 
}

static int motToggleSetBackgroundAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_TOGGLE_TEXT)
  {
    Pixel color;

    /* ignore given value, must use only from parent */
    value = iupAttribGetStrNativeParent(ih, "BACKGROUND");

    color = iupmotColorGetPixelStr(value);
    if (color != (Pixel)-1)
    {
      XtVaSetValues(ih->handle, XmNbackground, color, NULL);
      return 1;
    }
    else
    {
      Pixmap pixmap = (Pixmap)iupImageGetImage(value, ih, 0, "BACKGROUND");
      if (pixmap)
      {
        XtVaSetValues(ih->handle, XmNbackgroundPixmap, pixmap, NULL);
        return 1;
      }
    }
  }
  return 0;
}

static int motToggleSetTitleAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_TOGGLE_TEXT)
  {
    iupmotSetMnemonicTitle(ih, value);
    return 1;
  }

  return 0;
}

static int motToggleSetAlignmentAttrib(Ihandle* ih, const char* value)
{
  unsigned char align;
  char value1[30]="", value2[30]="";

  if (ih->data->type == IUP_TOGGLE_TEXT)
    return 0;

  iupStrToStrStr(value, value1, value2, ':');   /* value2 is ignored, NOT supported in Motif */

  if (iupStrEqualNoCase(value1, "ARIGHT"))
    align = XmALIGNMENT_END;
  else if (iupStrEqualNoCase(value1, "ACENTER"))
    align = XmALIGNMENT_CENTER;
  else /* "ALEFT" */
    align = XmALIGNMENT_BEGINNING;

  XtVaSetValues (ih->handle, XmNalignment, align, NULL);
  return 1;
}

static int motToggleSetImageAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_TOGGLE_IMAGE)
  {
    iupmotSetPixmap(ih, value, XmNlabelPixmap, 0, "IMAGE");

    if (!iupAttribGetStr(ih, "IMINACTIVE"))
    {
      /* if not active and IMINACTIVE is not defined 
         then automaticaly create one based on IMAGE */
      iupmotSetPixmap(ih, value, XmNlabelInsensitivePixmap, 1, "IMINACTIVE"); /* make_inactive */
    }
    return 1;
  }
  else
    return 0;
}

static int motToggleSetImInactiveAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_TOGGLE_IMAGE)
  {
    iupmotSetPixmap(ih, value, XmNlabelInsensitivePixmap, 0, "IMINACTIVE");
    return 1;
  }
  else
    return 0;
}

static int motToggleSetImPressAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_TOGGLE_IMAGE)
  {
    iupmotSetPixmap(ih, value, XmNselectPixmap, 0, "IMPRESS");
    return 1;
  }
  else
    return 0;
}

static int motToggleSetValueAttrib(Ihandle* ih, const char* value)
{
  Ihandle *radio;
  unsigned char check;

  if (iupStrEqualNoCase(value,"NOTDEF"))
    check = XmINDETERMINATE;
  else if (iupStrBoolean(value))
    check = XmSET;
  else
    check = XmUNSET;

  /* This is necessary because Motif toggle does not have support for radio. 
     It is implemented using an external RadioBox that we do not use. */
  radio = iupRadioFindToggleParent(ih);
  if (radio)
  {
    Ihandle* last_tg;
    unsigned char oldcheck;

    XtVaGetValues(ih->handle, XmNset, &oldcheck, NULL);

    last_tg = (Ihandle*)iupAttribGetStr(radio, "_IUPMOT_LASTTOGGLE");
    if (check)
    {
      if (iupObjectCheck(last_tg) && last_tg != ih)
        XtVaSetValues(last_tg->handle, XmNset, XmUNSET, NULL);
      iupAttribSetStr(radio, "_IUPMOT_LASTTOGGLE", (char*)ih);
    }

    if (last_tg != ih && oldcheck != check)
      XtVaSetValues(ih->handle, XmNset, check, NULL);
  }
  else
    XtVaSetValues(ih->handle, XmNset, check, NULL);

  return 0;
}

static char* motToggleGetValueAttrib(Ihandle* ih)
{
  unsigned char check = 0;
  XtVaGetValues (ih->handle, XmNset, &check, NULL);
  if (check == XmINDETERMINATE)
    return "NOTDEF";
  else if (check == XmSET)
    return "ON";
  else
    return "OFF";
}

static int motToggleSetPaddingAttrib(Ihandle* ih, const char* value)
{
  iupStrToIntInt(value, &ih->data->horiz_padding, &ih->data->vert_padding, 'x');
  if (ih->handle && ih->data->type == IUP_TOGGLE_IMAGE)
  {
    XtVaSetValues(ih->handle, XmNmarginHeight, ih->data->vert_padding,
                              XmNmarginWidth, ih->data->horiz_padding, NULL);
  }
  return 0;
}

static char* motToggleGetSelectColorAttrib(Ihandle* ih)
{
  unsigned char r, g, b;
  Pixel color;
  char* str = iupStrGetMemory(20);
  XtVaGetValues(ih->handle, XmNselectColor, &color, NULL); 
  iupmotColorGetRGB(color, &r, &g, &b);
  sprintf(str, "%d %d %d", (int)r, (int)g, (int)b);
  return str;
}

static int motToggleSetSelectColorAttrib(Ihandle* ih, const char *value)
{
  Pixel color = iupmotColorGetPixelStr(value);
  if (color != (Pixel)-1)
    XtVaSetValues(ih->handle, XmNselectColor, color, NULL);
  return 1;
}

static void motToggleValueChangedCallback(Widget w, Ihandle* ih, XmToggleButtonCallbackStruct* call_data)
{
  Ihandle *radio;
  IFni cb;
  int check = call_data->set;

  /* Must manually hide the tip if the toggle is pressed. */
  iupmotTipLeaveNotify();

  /* This is necessary because Motif toggle does not have support for radio. 
     It is implemented using an external RadioBox that we do not use. */
  radio = iupRadioFindToggleParent(ih);
  if (radio)
  {
    if (check)
    {
      Ihandle* last_tg = (Ihandle*)iupAttribGetStr(radio, "_IUPMOT_LASTTOGGLE");
      if (iupObjectCheck(last_tg) && last_tg != ih)
      {
        cb = (IFni) IupGetCallback(last_tg, "ACTION");
        if (cb && cb(last_tg, 0) == IUP_CLOSE)
            IupExitLoop();

        XtVaSetValues(last_tg->handle, XmNset, XmUNSET, NULL);
      }
      iupAttribSetStr(radio, "_IUPMOT_LASTTOGGLE", (char*)ih);

      if (last_tg != ih)
      {
        cb = (IFni)IupGetCallback(ih, "ACTION");
        if (cb && cb (ih, 1) == IUP_CLOSE)
            IupExitLoop();
      }
    }
    else
    {
      /* Force stay checked */
      XtVaSetValues(ih->handle, XmNset, XmSET, NULL);
    }
  }
  else
  {
    if (check == XmINDETERMINATE)
       check = -1;

    cb = (IFni)IupGetCallback(ih, "ACTION");
    if (cb)
    {
      if (cb(ih, check) == IUP_CLOSE) 
        IupExitLoop();
    }
  }

  (void)w;
}

static int motToggleMapMethod(Ihandle* ih)
{
  Ihandle* radio = iupRadioFindToggleParent(ih);
  char* value;
  int num_args = 0;
  Arg args[40];

  if (radio)
    ih->data->radio = 1;

  value = iupAttribGetStr(ih, "IMAGE");
  if (value)
  {
    ih->data->type = IUP_TOGGLE_IMAGE;
    iupmotSetArg(args[num_args++], XmNlabelType, XmPIXMAP) 
  }
  else
  {
    ih->data->type = IUP_TOGGLE_TEXT;
    iupmotSetArg(args[num_args++], XmNlabelType, XmSTRING) 
  }

  /* Core */
  iupmotSetArg(args[num_args++], XmNmappedWhenManaged, False);  /* not visible when managed */
  iupmotSetArg(args[num_args++], XmNx, 0);  /* x-position */
  iupmotSetArg(args[num_args++], XmNy, 0);  /* y-position */
  iupmotSetArg(args[num_args++], XmNwidth, 10);  /* default width to avoid 0 */
  iupmotSetArg(args[num_args++], XmNheight, 10); /* default height to avoid 0 */
  /* Primitive */
  if (iupStrBoolean(iupAttribGetStrDefault(ih, "CANFOCUS")))
    iupmotSetArg(args[num_args++], XmNtraversalOn, True)
  else
    iupmotSetArg(args[num_args++], XmNtraversalOn, False)
  iupmotSetArg(args[num_args++], XmNhighlightThickness, 2);
  iupmotSetArg(args[num_args++], XmNnavigationType, XmTAB_GROUP);
  /* Label */
  iupmotSetArg(args[num_args++], XmNrecomputeSize, False);  /* no automatic resize from text */
  iupmotSetArg(args[num_args++], XmNmarginHeight, 0);  /* default padding */
  iupmotSetArg(args[num_args++], XmNmarginWidth, 0);
  iupmotSetArg(args[num_args++], XmNmarginTop, 0);     /* no extra margins */
  iupmotSetArg(args[num_args++], XmNmarginLeft, 0);
  iupmotSetArg(args[num_args++], XmNmarginBottom, 0);
  iupmotSetArg(args[num_args++], XmNmarginRight, 0);

  if (radio)
  {
    iupmotSetArg(args[num_args++], XmNtoggleMode, XmTOGGLE_BOOLEAN);
    iupmotSetArg(args[num_args++], XmNindicatorType, XmONE_OF_MANY_ROUND);

    if (!iupAttribGetStr(radio, "_IUPMOT_LASTTOGGLE"))
    {
      /* this is the first toggle in the radio, and the last toggle with VALUE=ON */
      iupAttribSetStr(ih, "VALUE","ON");
    }
  }
  else
  {
    if (ih->data->type == IUP_TOGGLE_TEXT && iupAttribGetIntDefault(ih, "3STATE"))
      iupmotSetArg(args[num_args++], XmNtoggleMode, XmTOGGLE_INDETERMINATE)
    else
      iupmotSetArg(args[num_args++], XmNtoggleMode, XmTOGGLE_BOOLEAN)
    iupmotSetArg(args[num_args++], XmNindicatorType, XmN_OF_MANY);
  }

  if (ih->data->type == IUP_TOGGLE_IMAGE)
  {
    iupmotSetArg(args[num_args++], XmNindicatorOn, XmINDICATOR_NONE);
    iupmotSetArg(args[num_args++], XmNalignment, XmALIGNMENT_CENTER);
    iupmotSetArg(args[num_args++], XmNshadowThickness, 2);
  }
  else
  {
    iupmotSetArg(args[num_args++], XmNspacing, 3);
    iupmotSetArg(args[num_args++], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
    iupmotSetArg(args[num_args++], XmNalignment, XmALIGNMENT_BEGINNING);
    if (radio)
    {
      iupmotSetArg(args[num_args++], XmNindicatorSize, 13);
      iupmotSetArg(args[num_args++], XmNselectColor, iupmotColorGetPixel(0, 0, 0));
    }
    else
      iupmotSetArg(args[num_args++], XmNindicatorSize, 15)

    iupmotSetArg(args[num_args++], XmNshadowThickness, 0);
    iupmotSetArg(args[num_args++], XmNdetailShadowThickness, 2);
  }

  ih->handle = XtCreateManagedWidget(
    iupDialogGetChildIdStr(ih),  /* child identifier */
    xmToggleButtonWidgetClass,     /* widget class */
    iupChildTreeGetNativeParentHandle(ih), /* widget parent */
    args, num_args);

  if (!ih->handle)
    return IUP_ERROR;

  ih->serial = iupDialogGetChildId(ih); /* must be after using the string */

  XtAddCallback(ih->handle, XmNhelpCallback, (XtCallbackProc)iupmotHelpCallback, (XtPointer)ih);

  XtAddEventHandler(ih->handle, EnterWindowMask, False, (XtEventHandler)iupmotEnterLeaveWindowEvent, (XtPointer)ih);
  XtAddEventHandler(ih->handle, LeaveWindowMask, False, (XtEventHandler)iupmotEnterLeaveWindowEvent, (XtPointer)ih);

  XtAddEventHandler(ih->handle, FocusChangeMask, False, (XtEventHandler)iupmotFocusChangeEvent, (XtPointer)ih);
  XtAddEventHandler(ih->handle, KeyPressMask,    False, (XtEventHandler)iupmotKeyPressEvent,    (XtPointer)ih);

  XtAddCallback(ih->handle, XmNvalueChangedCallback, (XtCallbackProc)motToggleValueChangedCallback, (XtPointer)ih);

  /* Disable Drag Source */
  iupmotDisableDragSource(ih->handle);

  /* initialize the widget */
  XtRealizeWidget(ih->handle);

  if (ih->data->type == IUP_TOGGLE_TEXT)
    iupmotSetString(ih->handle, XmNlabelString, "");

  return IUP_NOERROR;
}

void iupdrvToggleInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = motToggleMapMethod;

  /* Driver Dependent Attribute functions */

  /* Overwrite Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", iupmotGetBgColorAttrib, motToggleSetBgColorAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "BACKGROUND", NULL, motToggleSetBackgroundAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);

  /* Special */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, iupdrvBaseSetFgColorAttrib, "0 0 0", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "TITLE", NULL, motToggleSetTitleAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);

  /* IupToggle only */
  iupClassRegisterAttribute(ic, "ALIGNMENT", NULL, motToggleSetAlignmentAttrib, "ACENTER:ACENTER", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGE", NULL, motToggleSetImageAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMINACTIVE", NULL, motToggleSetImInactiveAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMPRESS", NULL, motToggleSetImPressAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", motToggleGetValueAttrib, motToggleSetValueAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTCOLOR", motToggleGetSelectColorAttrib, motToggleSetSelectColorAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);

  iupClassRegisterAttribute(ic, "PADDING", iupToggleGetPaddingAttrib, motToggleSetPaddingAttrib, "0x0", IUP_NOT_MAPPED, IUP_INHERIT);
}