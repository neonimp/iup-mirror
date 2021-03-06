/** \file
 * \brief iupim utilities
 *
 * See Copyright Notice in iup.h
 *  */

#include <im.h>
#include <im_convert.h>
#include <im_counter.h>
#include <im_util.h>

#include "iup.h"
#include "iupim.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#if (IUP_VERSION_NUMBER < 300000)
void* iupGetImageData(Ihandle* self);
#else
static void* iupGetImageData(Ihandle* self)
{
  return IupGetAttribute(self, "WID");
}
#endif

static void PrintError(int error)
{
  char* lang = IupGetLanguage();
  char* msg;
  if (strcmp(lang, "ENGLISH")==0)
  {
    switch (error)
    {
    case IM_ERR_OPEN:
      msg = "Error Opening File.\n";
      break;
    case IM_ERR_MEM:
      msg = "Insuficient memory.\n";
      break;
    case IM_ERR_ACCESS:
      msg = "Error Accessing File.\n";
      break;
    case IM_ERR_DATA:
      msg = "Image type not Suported.\n";
      break;
    case IM_ERR_FORMAT:
      msg = "Invalid Format.\n";
      break;
    case IM_ERR_COMPRESS:
      msg = "Invalid or unsupported compression.\n";
      break;
    default:
      msg = "Unknown Error.\n";
    }
  }
  else
  {
    switch (error)
    {
    case IM_ERR_OPEN:
      msg = "Erro Abrindo Arquivo.\n";
      break;
    case IM_ERR_MEM:
      msg = "Mem?ria Insuficiente.\n";
      break;
    case IM_ERR_ACCESS:
      msg = "Erro Acessando Arquivo.\n";
      break;
    case IM_ERR_DATA:
      msg = "Tipo de Imagem n?o Suportado.\n";
      break;
    case IM_ERR_FORMAT:
      msg = "Formato Inv?lido.\n";
      break;
    case IM_ERR_COMPRESS:
      msg = "Compress?o Inv?lida ou n?o Suportada.\n";
      break;
    default:
      msg = "Erro Desconhecido.\n";
    }
  }

  IupMessage("Error", msg);
}

Ihandle* IupLoadImage(const char* file_name)
{
  long palette[256];
  int i, error, width, height, color_mode, flags,
      data_type, palette_count, has_alpha = 0;
  Ihandle* iup_image = NULL;
  const unsigned char* transp_index;
  void* image_data = NULL;

  imCounterCallback old_callback = imCounterSetCallback(NULL, NULL);

  imFile* ifile = imFileOpen(file_name, &error);
  if (error)
    goto load_finish;

  error = imFileReadImageInfo(ifile, 0, &width, &height, &color_mode, &data_type);
  if (error)
    goto load_finish;

  flags = IM_TOPDOWN;
#if (IUP_VERSION_NUMBER >= 300000)
  flags |= IM_PACKED;
  if (imColorModeHasAlpha(color_mode))
  {
    has_alpha = 1;
    flags |= IM_ALPHA;
  }
#endif

  color_mode = imColorModeToBitmap(color_mode);
  data_type = IM_BYTE;

  image_data = malloc(imImageDataSize(width, height, flags|color_mode, data_type));
  if (!image_data)
    goto load_finish;

  error = imFileReadImageData(ifile, image_data, 1, flags);
  if (error)
    goto load_finish;

  if (color_mode == IM_RGB)
  {
#if (IUP_VERSION_NUMBER < 300000)
    int plane_size = width*height;
    palette_count = 256;
    imConvertRGB2Map(width, height, (unsigned char*)image_data, 
                             (unsigned char*)image_data + plane_size, 
                             (unsigned char*)image_data + 2*plane_size, 
                             (unsigned char*)image_data, palette, &palette_count);
    iup_image = IupImage(width, height, (unsigned char*)image_data);
#else
    if (has_alpha)
      iup_image = IupImageRGBA(width, height, (unsigned char*)image_data);
    else
      iup_image = IupImageRGB(width, height, (unsigned char*)image_data);
    palette_count = 0;
#endif
  }
  else
  {
    imFileGetPalette(ifile, palette, &palette_count);
    iup_image = IupImage(width, height, (unsigned char*)image_data);
  }

  for (i = 0; i < palette_count; i++)
  {
    char attr[6], color[30];
    unsigned char r, g, b;

    sprintf(attr, "%d", i);
    imColorDecode(&r, &g, &b, palette[i]);
    sprintf(color, "%d %d %d", (int)r, (int)g, (int)b);

    IupStoreAttribute(iup_image, attr, color); 
  }

  transp_index = imFileGetAttribute(ifile, "TransparencyIndex", NULL, NULL);
  if (transp_index)
  {
    char attr[6];
    sprintf(attr, "%d", (int)(*transp_index));
    IupSetAttribute(iup_image, attr, "BGCOLOR"); 
  }

load_finish:
  imCounterSetCallback(NULL, old_callback);
  if (ifile) imFileClose(ifile);
  if (image_data) free(image_data);
  if (error) PrintError(error);
  return iup_image;
}

int IupSaveImage(Ihandle* image, const char* file_name, const char* format)
{
  int width, height, i, bpp;
  unsigned char* data;
  int error;
  long palette[256];

  imFile* ifile = imFileNew(file_name, format, &error);
  if (!ifile)
  {
    PrintError(error);
    return 0;
  }

  data = (unsigned char*)iupGetImageData(image);

  width = IupGetInt(image, "WIDTH");
  height = IupGetInt(image, "HEIGHT");
  bpp = IupGetInt(image, "BPP");

  if (bpp == 24)
    error = imFileWriteImageInfo(ifile, width, height, IM_RGB|IM_TOPDOWN|IM_PACKED, IM_BYTE);
  else if (bpp == 32)
    error = imFileWriteImageInfo(ifile, width, height, IM_RGB|IM_TOPDOWN|IM_PACKED|IM_ALPHA, IM_BYTE);
  else /* bpp == 8 */
  {
    for(i = 0; i < 256; i++)
    {
      unsigned int r, g, b;
      char str[20];
      char* color;

      sprintf(str, "%d", i);
      color = IupGetAttribute(image, str);
      if (!color)
        break;

      if (strcmp(color, "BGCOLOR") == 0)
      {
        unsigned char transp_index = (unsigned char)i;
        imFileSetAttribute(ifile, "TransparencyIndex", IM_BYTE, 1, &transp_index);
        palette[i] = imColorEncode(0, 0, 0);
      }
      else
      {
        sscanf(color, "%d %d %d", &r, &g, &b);
        palette[i] = imColorEncode((unsigned char)r, (unsigned char)g, (unsigned char)b);
      }
    }

    imFileSetPalette(ifile, palette, i);

    error = imFileWriteImageInfo(ifile, width, height, IM_MAP|IM_TOPDOWN, IM_BYTE);
  }

  if (error == IM_ERR_NONE)
    error = imFileWriteImageData(ifile, data);

  imFileClose(ifile); 

  if (error)
    PrintError(error);

  return error != IM_ERR_NONE? 1: 0;
}

