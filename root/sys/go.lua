function go(pathplus)
    local pathplus = pathplus or ""
    local fs = filesystem
    local out
    local start, opts = pathplus:match("^([^?]*)%??([^\\/]-)$")
    start = start or ""
    if start:find(">") then
        return "Cannot process from>to path specification."
    end
    if opts:find("?") then
        out = [[
go: go to drive and/or folder
Examples: go "usb:" ; go "sd:/sys" ; go "foo?q"
Options after ?:
    q - suppress output of result path
]]
    else
        local quiet = opts:find("q")
        local device, colon, path = start:match("^([^:]-(:?))([^:]*)$")
        path = path or ""
        opts = opts or ""
        local ok = fs.checkdir(device)
        -- print("Start: "..start.." Device: "..tostring(device).." DevOK: "..tostring(ok).." Colon: "..tostring(colon).." Path: "..path.." Opts: "..opts)
        if colon and not ok then
            io.stderr:write("ERROR: Unknown device: '" .. device .. "'\n")
            io.stderr:flush()
        else
            if colon then ok = fs.drive(device) end
            ok = fs.chdir(path)
            if not ok then
                io.stderr:write("ERROR: Unknown path: '" .. path .. "'")
                if drive then
                    io.stderr:write(" on device: '" .. drive .. "'")
                end
                io.stderr:write("\n")
            elseif not quiet then
                out = fs.getpath()
            end
        end
    end
    ::exit::
    return out and (out:gsub("^(.-)%s*$", "%1")) or nil
end
