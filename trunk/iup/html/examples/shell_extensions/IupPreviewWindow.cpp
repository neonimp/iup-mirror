
#include "PreviewHandler.h"

#include <iup.h>

//#define USE_OPEN_GL
#define USE_IM

#ifdef USE_OPEN_GL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h> 
#include <iupgl.h>
#else
#include <cd.h>
#include <cdiup.h>
#include <cdcgm.h>
#include <cdemf.h>
#endif

#ifdef USE_IM
#include <im.h>
#include <im_image.h>
#include <im_convert.h>
#endif

static void imImageViewFitRect(int cnv_width, int cnv_height, int img_width, int img_height, int *w, int *h)
{
  double rView, rImage;
  int correct = 0;

  *w = cnv_width;
  *h = cnv_height;

  rView = ((double)cnv_height) / cnv_width;
  rImage = ((double)img_height) / img_width;

  if ((rView <= 1 && rImage <= 1) || (rView >= 1 && rImage >= 1)) /* view and image are horizontal rectangles */
  {
    if (rView > rImage)
      correct = 2;
    else
      correct = 1;
  }
  else if (rView < 1 && rImage > 1) /* view is a horizontal rectangle and image is a vertical rectangle */
    correct = 1;
  else if (rView > 1 && rImage < 1) /* view is a vertical rectangle and image is a horizontal rectangle */
    correct = 2;

  if (correct == 1)
    *w = (int)(cnv_height / rImage);
  else if (correct == 2)
    *h = (int)(cnv_width * rImage);
}


static int canvas_redraw(Ihandle* ih)
{
  char* filename = IupGetAttribute(ih, "PATHFILE");

#ifdef USE_OPEN_GL
  int cnv_w, cnv_h;
  IupGetIntInt(ih, "DRAWSIZE", &cnv_w, &cnv_h);

  IupGLMakeCurrent(ih);

  glViewport(0, 0, cnv_w, cnv_h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, cnv_w, 0, cnv_h, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(1.0, 1.0, 1.0, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glColor3f(0.0,1.0,0.0);  /* green in OpenGL */

  glBegin(GL_LINES);
  glVertex2i(0, 0);
  glVertex2i(cnv_w - 1, cnv_h - 1);
  glEnd();

  glBegin(GL_LINES);
  glVertex2i(0, cnv_h - 1);
  glVertex2i(cnv_w - 1, 0);
  glEnd();

#ifdef USE_IM
  int error;
  imImage* image = imFileImageLoadBitmap(filename, 0, &error);

  int glformat;
  void* glImage = imImageGetOpenGLData(image, &glformat);

  int x, y, w, h;
  imImageViewFitRect(cnv_w, cnv_h, image->width, image->height, &w, &h);
  x = (cnv_w - w) / 2;
  y = (cnv_h - h) / 2;

  glPixelZoom((GLfloat)w / (GLfloat)image->width, (GLfloat)h / (GLfloat)image->height);
  glRasterPos2d(x, y);
  glDrawPixels(image->width, image->height, glformat, GL_UNSIGNED_BYTE, glImage);

  imImageDestroy(image);
#endif

  IupGLSwapBuffers(ih);
#else
//  cdCanvas* canvas = (cdCanvas*)IupGetAttribute(ih, "_CD_CANVAS");
  cdCanvas* canvas = (cdCanvas*)IupGetAttribute(ih, "_CD_CANVAS_DBUFFER");

  cdCanvasActivate(canvas);
  cdCanvasBackground(canvas, CD_WHITE);
  cdCanvasClear(canvas);

  int cnv_w, cnv_h;
  cdCanvasGetSize(canvas, &cnv_w, &cnv_h, 0, 0);

  cdCanvasForeground(canvas, CD_RED);
  cdCanvasLine(canvas, 0, 0, cnv_w - 1, cnv_h - 1);
  cdCanvasLine(canvas, 0, cnv_h - 1, cnv_w - 1, 0);

#ifdef USE_IM
  int error;
  imImage* image = imFileImageLoadBitmap(filename, 0, &error);

  int x, y, w, h;
  imImageViewFitRect(cnv_w, cnv_h, image->width, image->height, &w, &h);
  x = (cnv_w - w) / 2;
  y = (cnv_h - h) / 2;

  imcdCanvasPutImage(canvas, image, x, y, w, h, 0, 0, 0, 0);

  imImageDestroy(image);
#else
  //  cdCanvasPlay(canvas, CD_CGM, 0, cnv_w - 1, 0, cnv_h - 1, filename);
  cdCanvasPlay(canvas, CD_EMF, 0, cnv_w - 1, 0, cnv_h - 1, filename);
#endif

//  cdCanvasText(canvas, 0, 0, "CD_IUP");
  cdCanvasText(canvas, 0, 0, "CD_IUPDBUFFER");

  cdCanvasFlush(canvas);
#endif
  return IUP_DEFAULT;
}

#ifndef USE_OPEN_GL
static int canvas_map(Ihandle* ih)
{
//  cdCreateCanvas(CD_IUP, ih);
  cdCreateCanvas(CD_IUPDBUFFER, ih);
  return IUP_DEFAULT;
}

static int canvas_unmap(Ihandle* ih)
{
//  cdCanvas* canvas = (cdCanvas*)IupGetAttribute(ih, "_CD_CANVAS");
  cdCanvas* canvas = (cdCanvas*)IupGetAttribute(ih, "_CD_CANVAS_DBUFFER");
  cdKillCanvas(canvas);
  return IUP_DEFAULT;
}
#endif

static Ihandle* IupPreviewCanvasCreate()
{
#ifdef USE_OPEN_GL
  IupGLCanvasOpen();

  Ihandle* cnv = IupGLCanvas(NULL);
  IupSetAttribute(cnv, "BUFFER", "DOUBLE");
#else
  Ihandle* cnv = IupCanvas(0);
  IupSetCallback(cnv, "MAP_CB", canvas_map);
  IupSetCallback(cnv, "UNMAP_CB", canvas_unmap);
#endif

  IupSetCallback(cnv, "ACTION", canvas_redraw);

  return cnv;
}


extern "C" void iupwinSetInstance(HINSTANCE hInstance);

// Create the preview window based on the recipe information.
HRESULT PreviewHandler::CreatePreviewWindow(HINSTANCE hInstance, int width, int height)
{
  HRESULT hr = S_OK;

  iupwinSetInstance(hInstance);

  IupOpen(NULL, NULL);

  Ihandle* cnv = IupPreviewCanvasCreate();

  Ihandle* m_dialog = IupDialog(cnv);
  IupSetAttribute(m_dialog, "BORDER", "NO");
  IupSetAttribute(m_dialog, "MAXBOX", "NO");
  IupSetAttribute(m_dialog, "MINBOX", "NO");
  IupSetAttribute(m_dialog, "MENUBOX", "NO");
  IupSetAttribute(m_dialog, "RESIZE", "NO");
  IupSetAttribute(m_dialog, "CONTROL", "YES");

  char str[10240];
  size_t size;
  wcstombs_s(&size, str, 10240, m_pPathFile, 10240);
  IupSetStrAttribute(m_dialog, "PATHFILE", str);

  IupSetAttribute(m_dialog, "NATIVEPARENT", (char*)m_hwndParent);

  IupSetStrf(m_dialog, "RASTERSIZE", "%dx%d", width, height);
  IupMap(m_dialog);
  IupSetAttribute(m_dialog, "RASTERSIZE", NULL);

  m_hwndPreview = (HWND)IupGetAttribute(m_dialog, "HWND");

  // Set the preview window position.
  SetWindowPos(m_hwndPreview, NULL, m_rcParent.left, m_rcParent.top,
               width, height,
               SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

  ShowWindow(m_hwndPreview, SW_SHOW);

  return hr;
}

// MessageBox(NULL, L"ShowWindow-Fail", L"IUP", MB_OK);
