/** \file
 * \brief IupMatrix Expansion Library.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_array.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_matrixex.h"


static void iMatrixExStrCopyNoSepTXT(char* buffer, const char* str, char sep)
{
  while (*str)
  {
    if (*str==sep || *str=='\n')
      *buffer = ' ';
    else
      *buffer = *str;

    buffer++;
    str++;
  }
  *buffer = 0;
}

static void iMatrixExStrCopyNoSepHTML(char* buffer, const char* str)
{
  while (*str)
  {
    if (*str=='\n')
    {
      *buffer = '<'; buffer++;
      *buffer = 'B'; buffer++;
      *buffer = 'R'; buffer++;
      *buffer = '>'; 
    }
    else
      *buffer = *str;

    buffer++;
    str++;
  }
  *buffer = 0;
}

static void iMatrixExStrCopyNoSepLaTeX(char* buffer, const char* str)
{
  while (*str)
  {
    if (*str=='\n' || *str=='_')
      *buffer = ' ';
    else if (*str=='%')
    {
      *buffer = '\\'; buffer++;
      *buffer = '%'; 
    }
    else
      *buffer = *str;

    buffer++;
    str++;
  }
  *buffer = 0;
}

static void iMatrixExCopyTXT(Ihandle *ih, FILE* file, int num_lin, int num_col, char* buffer)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  int lin, col;
  int add_sep;
  char* str, sep = '\t';

  iupMatrixExInitCellAccess(matex_data);

  str = IupGetAttribute(ih, "TEXTSEPARATOR");
  if (str) sep = *str;

  str = iupAttribGetStr(ih, "COPYCAPTION");
  if (str)
    fprintf(file,"%s\n",str);

  /* Here includes the title cells */
  for(lin = 0; lin <= num_lin; ++lin)
  {
    add_sep = 0;

    if (iupMatrixExIsLineVisible(ih, lin))
    {
      for(col = 0; col <= num_col; ++col)
      {
        if (iupMatrixExIsColumnVisible(ih, col))
        {
          if (add_sep)
            fprintf(file, "%c", sep);

          str = iupMatrixExGetCell(matex_data, lin, col);
          if (str)
          {
            iMatrixExStrCopyNoSepTXT(buffer, str, sep);
            fprintf(file, "%s", buffer);
          }
          else
            fprintf(file, "%s", " ");

          add_sep = 1;
        }
      }
    }

    if (add_sep)
      fprintf(file, "%s", "\n");
  }
}

static char* iMatrixExGetCellAttrib(Ihandle* ih, const char* attrib, int lin, int col)
{
  char* value = NULL;

  /* 1 -  check for this cell */
  value = iupAttribGetId2(ih, attrib, lin, col);

  if (!value)
  {
    /* 2 - check for this line, if not title col */
    if (col != 0)
      value = iupAttribGetId2(ih, attrib, lin, IUP_INVALID_ID);

    if (!value)
    {
      /* 3 - check for this column, if not title line */
      if (lin != 0)
        value = iupAttribGetId2(ih, attrib, IUP_INVALID_ID, col);
    }
  }

  return value;
}

static char* iMatrixExGetCellFormat(Ihandle *ih, int lin, int col, char* format)
{
  char* value, *init = "style=\"";

#define _STRCATFORMAT {if (value) { if (init) {strcpy(format, init); init=NULL;} strcat(format, value); }}

  *format = 0;

  value = iupAttribGetId(ih, "ALIGNMENT", col);
  if (value)
  {
    if (iupStrEqualNoCase(value, "ARIGHT"))
      value = "text-align: right; ";
    else if(iupStrEqualNoCase(value, "ACENTER"))
      value = "text-align: center; ";
    else if(iupStrEqualNoCase(value, "ALEFT"))
      value = "text-align: left; ";
    else
      value = NULL;

    _STRCATFORMAT;
  }

  value = iMatrixExGetCellAttrib(ih, "BGCOLOR", lin, col);
  if (value)
  {
    char rgb[50];
    unsigned char r, g, b;
    iupStrToRGB(value, &r, &g, &b);
    sprintf(rgb, "background-color: #%2x%2x%2x; ", (int)r, (int)g, (int)b);

    value = rgb;
    _STRCATFORMAT;
  }

  value = iMatrixExGetCellAttrib(ih, "FGCOLOR", lin, col);
  if (value)
  {
    char rgb[50];
    unsigned char r, g, b;
    iupStrToRGB(value, &r, &g, &b);
    sprintf(rgb, "color: #%2x%2x%2x; ", (int)r, (int)g, (int)b);

    value = rgb;
    _STRCATFORMAT;
  }

  value = iMatrixExGetCellAttrib(ih, "FONT", lin, col);
  if (value)
  {
    if (strstr(value, "Bold")||strstr(value, "BOLD"))
    {
      value = "font-weight: bold; ";
      _STRCATFORMAT;
    }

    if (strstr(value, "Italic")||strstr(value, "ITALIC"))
    {
      value = "font-weight: bold; ";
      _STRCATFORMAT;
    }

    /* Leave this out for now:
       font-size: %dpt; 
       font-family: %s; */
  }

  if (format[0]!=0)
    strcat(format, "\"");

  return format;
}

static void iMatrixExCopyHTML(Ihandle *ih, FILE* file, int num_lin, int num_col, char* buffer)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  int lin, col;
  char* str, *caption, f[512];

  int add_format = iupAttribGetInt(ih, "HTMLADDFORMAT");

  char* table = iupAttribGetStr(ih, "HTML<TABLE>");
  char* tr = iupAttribGetStr(ih, "HTML<TR>");
  char* th = iupAttribGetStr(ih, "HTML<TH>");
  char* td = iupAttribGetStr(ih, "HTML<TD>");
  caption = iupAttribGetStr(ih, "HTML<CAPTION>");
  if (!table) table = "";
  if (!tr) tr = "";
  if (!th) th = "";
  if (!td) td = "";
  if (!caption) caption = "";

  iupMatrixExInitCellAccess(matex_data);

  fprintf(file,"<!-- File automatically generated by IUP -->\n");
  fprintf(file,"<TABLE%s>\n", table);

  str = iupAttribGetStr(ih, "COPYCAPTION");
  if (str)
    fprintf(file,"<CAPTION%s>%s</CAPTION>\n", caption, str);

  /* Here includes the title cells */
  for(lin = 0; lin <= num_lin; ++lin)
  {
    if (iupMatrixExIsLineVisible(ih, lin))
    {
      fprintf(file,"<TR%s> ", tr);

      for(col = 0; col <= num_col; ++col)
      {
        if (iupMatrixExIsColumnVisible(ih, col))
        {           
          if (lin==0||col==0)
            fprintf(file,"<TH%s%s>", th, add_format? iMatrixExGetCellFormat(ih, lin, col, f): "");
          else
            fprintf(file,"<TD%s%s>", td, add_format? iMatrixExGetCellFormat(ih, lin, col, f): "");

          str = iupMatrixExGetCell(matex_data, lin, col);
          if (str)
          {
            iMatrixExStrCopyNoSepHTML(buffer, str);
            fprintf(file, "%s", buffer);
          }
          else
            fprintf(file, "%s", " ");

          if (lin==0||col==0)
            fprintf(file,"</TH> ");
          else
            fprintf(file,"</TD> ");
        }
      }

      fprintf(file,"</TR>\n");
    }
  }

  fprintf(file,"</TABLE>\n");
}

static int iMatrixExIsBoldLine(Ihandle* ih, int lin)
{
  char* value;

  if (lin==0)
    return 0;

  value = iupAttribGetId2(ih, "FONT", lin, IUP_INVALID_ID);
  if (value)
  {
    if (strstr(value, "Bold")||strstr(value, "BOLD"))
      return 1;
  }

  return 0;
}

static void iMatrixExCopyLaTeX(Ihandle *ih, FILE* file, int num_lin, int num_col, char* buffer)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  int lin, col;
  int add_sep;
  char* str;

  iupMatrixExInitCellAccess(matex_data);

  fprintf(file,"%% File automatically generated by IUP \n");

  fprintf(file,"\\begin{table}\n");
  fprintf(file,"\\begin{center}\n");
  fprintf(file,"\\begin{tabular}{");

  for(col = 0; col <= num_col; ++col)
  {
    if (iupMatrixExIsColumnVisible(ih, col))
      fprintf(file,"|r");
  }
  fprintf(file,"|} \\hline\n");

  /* Here includes the title cells */
  for(lin = 0; lin <= num_lin; ++lin)
  {
    add_sep = 0;

    if (iupMatrixExIsLineVisible(ih, lin))
    {
      int is_bold = iMatrixExIsBoldLine(ih, lin);

      for(col = 0; col <= num_col; ++col)
      {
        if (iupMatrixExIsColumnVisible(ih, col))
        {    
          if (add_sep)
            fprintf(file,"& ");

          if (is_bold)
            fprintf(file,"\\bf{");

          str = iupMatrixExGetCell(matex_data, lin, col);
          if (str)
          {
            iMatrixExStrCopyNoSepLaTeX(buffer, str);
            fprintf(file, "%s", buffer);
          }
          else
            fprintf(file, "%s", " ");

          if (is_bold)
            fprintf(file,"}");

          add_sep = 1;
        }
      }

      fprintf(file,"\\\\ \\hline\n");
    }
  }

  fprintf(file,"\\end{tabular}\n");

  str = iupAttribGetStr(ih, "COPYCAPTION");
  if (str) fprintf(file,"\\caption{%s.}\n", str);

  str = iupAttribGetStr(ih, "LATEXLABEL");
  if (str) fprintf(file,"\\label{tab:%s}\n", str);

  fprintf(file,"\\end{center}\n");
  fprintf(file,"\\end{table}\n");
}

static int iMatrixExSetCopyFileAttrib(Ihandle *ih, const char* value)
{
  int num_lin, num_col;
  char buffer[1024];
  char* format;

  FILE *file = fopen(value, "wb");
  if (!file)
  {
    iupAttribSetStr(ih, "LASTERROR", "INVALIDFILENAME");
    return 0;
  }

  /* reset error state */
  iupAttribSetStr(ih, "LASTERROR", NULL);

  num_lin = IupGetInt(ih, "NUMLIN");
  num_col = IupGetInt(ih, "NUMCOL");

  format = iupAttribGetStr(ih, "TEXTFORMAT");
  if (iupStrEqualNoCase(format, "HTML"))
    iMatrixExCopyHTML(ih, file, num_lin, num_col, buffer);
  else if (iupStrEqualNoCase(format, "LaTeX"))
    iMatrixExCopyLaTeX(ih, file, num_lin, num_col, buffer);
  else
    iMatrixExCopyTXT(ih, file, num_lin, num_col, buffer);

  fclose(file);
  return 0;
}

void iupMatrixExRegisterExport(Iclass* ic)
{
  iupClassRegisterAttribute(ic, "COPYFILE", NULL, iMatrixExSetCopyFileAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "COPYCAPTION", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LATEXLABEL", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HTML<TABLE>", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HTML<TR>", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HTML<TH>", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HTML<TD>", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HTML<CAPTION>", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
}
