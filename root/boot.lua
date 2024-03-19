_vermajor, _verminor = pineapple.version()
_banner = "Pineapple - Lua-based Fantasy OS\nVersion " .. _vermajor .. "." .. _verminor .. "\n"
_prompt = "\027[36mrepl\027[0m> "

package.path = "?.lua;/sys/?.lua;usb:/sys/?.lua"

dofile("/sys/repl.lua")
dofile("/sys/fscmds.lua")
dofile("/sys/misccmds.lua")
dofile("/sys/concmds.lua")
dofile("/sys/ple.lua")

repl(_banner, _prompt)
con.mode("sane")
