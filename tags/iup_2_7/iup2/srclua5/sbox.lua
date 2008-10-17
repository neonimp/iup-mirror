------------------------------------------------------------------------------
-- Sbox class 
------------------------------------------------------------------------------
local ctrl = {
	nick = "sbox",
  parent = WIDGET,
	creation = "i",
  callback = {},
  include="iupsbox.h"
}

function ctrl.createElement(class, arg)
   return Sbox(arg[1])
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
