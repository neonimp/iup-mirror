require("iuplua")

function btn_exit_cb(self)
	-- Exits the main loop
	return iup.CLOSE	
end

label = iup.label{title = "Hello world from IUP.", padding = "10x20"}
button = iup.button{title = "OK", padding = "30x2"}

vbox = iup.vbox{
	label,
	button,
	alignment = "ARIGHT",
	gap = "10",
	margin = "10x10"
}
dlg = iup.dialog{
	vbox,
	title = "Hello World 5a",
  maxbox = "No",
  minbox = "No",
}
button.action = btn_exit_cb

dlg:showxy(iup.CENTER,iup.CENTER)

-- to be able to run this script inside another context
if (iup.MainLoopLevel()==0) then
  iup.MainLoop()
  iup.Close()
end
