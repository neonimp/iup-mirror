/** \file
 * \brief GL Drawing functions.
 *
 * See Copyright Notice in "iup.h"
 */

#ifdef WIN32
#include <windows.h>
#endif

#if defined (__APPLE__) || defined (OSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif


#include <stdio.h>
#include <stdlib.h>

#include "iup.h"

#include "iup_assert.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_array.h"
#include "iup_image.h"

#include "iup_glcontrols.h"


void iupGLDrawLine(Ihandle* ih, double x1, double y1, double x2, double y2, float width, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (width == 0)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);

  /* y is oriented top to bottom in IUP */
  y1 = ih->currentheight - 1 - y1;
  y2 = ih->currentheight - 1 - y2;

  if (!active)
    iupGLColorMakeInactive(&r, &g, &b);
  glColor4ub(r, g, b, a);

  glLineWidth(width);

  glBegin(GL_LINES);
  glVertex2d(x1, y1);
  glVertex2d(x2, y2);
  glEnd();
}

void iupGLDrawRect(Ihandle* ih, double xmin, double xmax, double ymin, double ymax, float width, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (width == 0 || xmin == xmax || ymin == ymax)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);

  /* y is oriented top to bottom in IUP */
  ymin = ih->currentheight - 1 - ymin;
  ymax = ih->currentheight - 1 - ymax;

  if (!active)
    iupGLColorMakeInactive(&r, &g, &b);
  glColor4ub(r, g, b, a);

  glLineWidth(width);

  glBegin(GL_LINE_LOOP);
  glVertex2d(xmin, ymin);
  glVertex2d(xmax, ymin);
  glVertex2d(xmax, ymax);
  glVertex2d(xmin, ymax);
  glEnd();
}

void iupGLDrawBox(Ihandle* ih, double xmin, double xmax, double ymin, double ymax, const char* color)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (!color || xmin == xmax || ymin == ymax)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);

  /* y is oriented top to bottom in IUP */
  ymin = ih->currentheight - 1 - ymin;
  ymax = ih->currentheight - 1 - ymax;

  glColor4ub(r, g, b, a);

  glBegin(GL_QUADS);
  glVertex2d(xmin, ymin);
  glVertex2d(xmax, ymin);
  glVertex2d(xmax, ymax);
  glVertex2d(xmin, ymax);
  glEnd();
}

void iupGLDrawText(Ihandle* ih, double x, double y, const char* str, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (!str)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);

  if (!active)
    iupGLColorMakeInactive(&r, &g, &b);
  glColor4ub(r, g, b, a);

  if (str[0])
  {
    int len, lineheight, ascent, stipple = 0, baseline;
    const char *nextstr;
    const char *curstr = str;

    iupGLFontGetDim(ih, NULL, &lineheight, &ascent, NULL);
    baseline = lineheight - ascent;

    if (glIsEnabled(GL_POLYGON_STIPPLE))
    {
      stipple = 1;
      glDisable(GL_POLYGON_STIPPLE);
    }

    /* y is at top and oriented top to bottom in IUP */
    /* y is at baseline and oriented bottom to top in OpenGL */
    y = y + lineheight - baseline;  /* move to baseline */
    y = ih->currentheight - 1 - y; /* orient bottom to top */

    glPushMatrix();
    glTranslated(x, y, 0.0);

    do
    {
      nextstr = iupStrNextLine(curstr, &len);
      if (len)
        iupGLFontRenderString(ih, curstr, len);

      glTranslated(0.0, (double)-lineheight, 0.0);

      curstr = nextstr;
    } while (*nextstr);

    glPopMatrix();

    if (stipple)
      glEnable(GL_POLYGON_STIPPLE);
  }
}

void iupGLDrawImage(Ihandle* ih, double x, double y, const char* name, int active)
{
  Ihandle* image = iupGLIconGetImageHandle(ih, name, active);
  if (image)
  {
    unsigned char* gldata = iupGLImageGetData(image, active);
    int depth = iupAttribGetInt(image, "GL_DEPTH");
    int format = GL_RGB;
    if (depth == 4)
      format = GL_RGBA;

    /* y is at top and oriented top to bottom in IUP */
    /* y is at bottom and oriented bottom to top in OpenGL */
    y = y + image->currentheight - 1;  /* move to bottom */
    y = ih->currentheight - 1 - y; /* orient bottom to top */

    glRasterPos2d(x, y);
    glDrawPixels(image->currentwidth, image->currentheight, format, GL_UNSIGNED_BYTE, gldata);
  }
}
