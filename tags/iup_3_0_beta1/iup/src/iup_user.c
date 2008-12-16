/** \file
 * \brief User Element.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_stdcontrols.h"


static int iUserSetClearAttributesAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  iupTableClear(ih->attrib);
  return 0;
}

Ihandle* IupUser(void)
{
  return IupCreate("user");
}

Iclass* iupUserGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "user";
  ic->format = NULL;  /* no parameters */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 0;

  iupClassRegisterAttribute(ic, "CLEARATTRIBUTES", NULL, iUserSetClearAttributesAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  return ic;
}
