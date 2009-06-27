-- Creates an IupText that accepts only numbers.

require( "iuplua" )
require( "iupluacontrols" )

txt = iup.text{}
txt.MASK = "/d*"
dg = iup.dialog{txt;title="MASK Example"}
dg:show()

if (not iup.MainLoopLevel or iup.MainLoopLevel()==0) then
  iup.MainLoop()
end
