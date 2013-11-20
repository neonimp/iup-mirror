PROJNAME = iup
LIBNAME  = iupgl
OPT = YES

ifdef DBG
  DEFINES += IUP_ASSERT
endif  

INCLUDES = ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup

USE_OPENGL = Yes
USE_MACOS_OPENGL = Yes

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRC = iup_glcanvas_win.c
else ifneq ($(findstring Haiku, $(TEC_SYSNAME)), )
  SRC = iup_glcanvas_haiku.c
else
  SRC = iup_glcanvas_x.c
  USE_X11 = Yes
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifdef USE_MACOS_OPENGL
    LFLAGS = -framework OpenGL
    USE_OPENGL :=
  endif
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
