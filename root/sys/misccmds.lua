-- lupos misc cmds
function unload(name) package.loaded[name] = nil end
function reload(name)
    package.loaded[name] = nil
    _G[name] = require(name)
end
function run(name) return dofile(name .. ".lua") end
function list(file)
    local f = assert(io.open(file, "rb"))
    local content = f:read("*all")
    f:close()
    return content
end
function help() return list("/sys/help.txt") end
if not rawprint then
    rawprint = print
    function print(t)
        if (t and type(t) == "table") then
            for x, y in pairs(t) do rawprint(x, y) end
        else
            rawprint(t)
        end
    end
end
return "ok"
