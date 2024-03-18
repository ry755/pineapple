_banner = "Pineapple - Lua-based Fantasy OS\n"
_prompt = "repl> "

package.path = "?.lua;/sys/?.lua;usb:/sys/?.lua"

dofile("/sys/repl.lua")
dofile("/sys/fscmds.lua")
dofile("/sys/misccmds.lua")
dofile("/sys/concmds.lua")
dofile("/sys/ple.lua")

repl(_banner, _prompt)
con.mode("sane")
