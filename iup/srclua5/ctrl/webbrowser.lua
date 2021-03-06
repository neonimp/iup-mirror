------------------------------------------------------------------------------
-- WebBrowser class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "webbrowser",
  parent = iup.WIDGET,
  creation = "",
  funcname = "WebBrowser",
  subdir = "ctrl",
  callback = {
    navigate_cb = "s",
    newwindow_cb = "s",
    error_cb = "s",
    completed_cb = "s",
    update_cb = "",
  },
  include = "iupweb.h",
  extracode = [[ 
  
#include "iupluaweb.h"
  
IUPLUAWEB_API int iupweblua_open(lua_State* L)
{
  if (iuplua_opencall_internal(L))
    IupWebBrowserOpen();
    
  iuplua_get_env(L);
  iupwebbrowserlua_open(L);
  return 0;
}

/* obligatory to use require"iupluaweb" */
IUPLUAWEB_API int luaopen_iupluaweb(lua_State* L)
{
  return iupweblua_open(L);
}

]]
}

function ctrl.createElement(class, param)
  return iup.WebBrowser()
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iupWidget")
