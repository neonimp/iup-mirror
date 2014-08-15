/** \file
 * \brief iupmatrix control
 * memory allocation.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __IMMEM_H 
#define __IMMEM_H

#ifdef __cplusplus
extern "C" {
#endif

void iupmatMemAloc         (Tmat *mat);
void iupmatMemRealocLines  (Tmat *mat,int nlines, int nl);
void iupmatMemRealocColumns(Tmat *mat, int ncols, int nc);
void iupmatMemAlocCell     (Tmat *mat, int lin, int col, int numc);

#ifdef __cplusplus
}
#endif

#endif
