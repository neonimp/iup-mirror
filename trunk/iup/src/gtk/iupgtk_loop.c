/** \file
 * \brief GTK Message Loop
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>    
#include <string.h>    
#include <stdlib.h>

#include <gtk/gtk.h>

#include "iup.h"
#include "iupcbs.h"
#include "iup_loop.h"

#include "iup_str.h"

/* local variables */
static IFidle gtk_idle_cb = NULL;
static guint gtk_idle_id;

static gboolean gtkIdleFunc(gpointer data)
{
  (void)data;
  if (gtk_idle_cb)
  {
    int ret = gtk_idle_cb();
    if (ret == IUP_CLOSE)
    {
      gtk_idle_cb = NULL;
      IupExitLoop();
      return FALSE; /* removes the idle */
    }
    if (ret == IUP_IGNORE)
    {
      gtk_idle_cb = NULL;
      return FALSE; /* removes the idle */
    }

    return TRUE; /* keeps the idle */
  }

  return FALSE; /* removes the idle */
}

IUP_SDK_API void iupdrvSetIdleFunction(Icallback f)
{
  if (gtk_idle_cb)
    g_source_remove(gtk_idle_id);

  gtk_idle_cb = (IFidle)f;

  if (gtk_idle_cb)
    gtk_idle_id = g_idle_add(gtkIdleFunc, NULL);
}

IUP_API void IupExitLoop(void)
{
  char* exit_loop = IupGetGlobal("EXITLOOP");
  if (gtk_main_level() > 1 || !exit_loop || iupStrBoolean(exit_loop))
  {
    if (gtk_main_iteration_do(FALSE) == FALSE)
      gtk_main_quit();
  }
}

IUP_API int IupMainLoopLevel(void)
{
  return gtk_main_level();
}

IUP_API int IupMainLoop(void)
{
  static int loop_count = 0;
  static int has_done_entry = 0;
  loop_count++;
  if (0 == has_done_entry)
  {
    has_done_entry = 1;
    iupLoopCallEntryCb();
  }

  gtk_main();

  loop_count--;
  if( 0 == loop_count)
  {
    iupLoopCallExitCb();
  }
  return IUP_NOERROR;
}

IUP_API int IupLoopStepWait(void)
{
  if (gtk_main_iteration_do(TRUE))
    return IUP_CLOSE;
  return IUP_DEFAULT;
}

IUP_API int IupLoopStep(void)
{
  if (gtk_main_iteration_do(FALSE))
    return IUP_CLOSE;
  return IUP_DEFAULT;
}

IUP_API void IupFlush(void)
{
  int count = 0;

  IFidle old_gtk_idle_cb = NULL;
  if (gtk_idle_cb)
  {
    old_gtk_idle_cb = gtk_idle_cb;
    iupdrvSetIdleFunction(NULL);
  }

  while (count<100 && gtk_events_pending())
  {
    gtk_main_iteration();

    /* we detected that after destroying a popup dialog
       just after clicking in a button of the same dialog,
       sometimes a message gets lost and gtk_events_pending
       keeps returning TRUE */
    count++;
  }

  if (old_gtk_idle_cb)
    iupdrvSetIdleFunction((Icallback)old_gtk_idle_cb);
}


typedef struct {
  Ihandle* ih;
  const char* s;
  int i;
  double d;
} gtkPostMessageUserData;

static gint gtkPostMessageCallback(void *user_data)
{
  gtkPostMessageUserData* message_user_data = (gtkPostMessageUserData*)user_data;
  Ihandle* ih = message_user_data->ih;
  IFnsid post_message_callback = (IFnsid)IupGetCallback(ih, "POSTMESSAGE_CB");
  if (post_message_callback)
  {
    const char* s = message_user_data->s;
    int i = message_user_data->i;
    double d = message_user_data->d;
    post_message_callback(ih, (char*)s, i, d);
  }
  free(message_user_data);
  return FALSE; /* call only once */
}

IUP_API void IupPostMessage(Ihandle* ih, const char* s, int i, double d)
{
  gtkPostMessageUserData* user_data = (gtkPostMessageUserData*)malloc(sizeof(gtkPostMessageUserData));
  user_data->ih = ih;
  user_data->s = s;
  user_data->i = i;
  user_data->d = d;
  g_idle_add(gtkPostMessageCallback, user_data);  
}
