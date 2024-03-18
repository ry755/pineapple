-- lupos fs commands
fs = filesystem
path = fs.getpath
if not io.rawopen then
    io.rawopen = io.open
    io.open = function(fname, ...)
        fs.checkdir(fname)
        return io.rawopen(fname, ...)
    end
end
function fsparse(pathplus, maskdef, fromto)
    local pathplus = pathplus or ""
    local fs = filesystem
    local start, opts = pathplus:match("^([^?]*)%??([^\\/]-)$")
    start = start or ""
    local ds = fs.getstat(start)
    if ds and ds.attributes and ((ds.attributes & 0x10) > 0) then
        -- user is trying to list a folder
        start = start .. "/"
    end
    local target
    if start:find(">") then
        start, target = start:match("^([^>]*)>([^>]*)$")
        start = start or ""
        target = target or ""
    end
    local path, slash, mask = start:match("^(.-)([\\/]?)([^:\\/]*)$")
    path = path or ""
    mask = mask or ""
    opts = opts or ""
    if #mask == 0 then mask = maskdef end
    if #path == 0 and #slash > 0 then path = "/" end
    -- print("Start: "..start.." Path: "..path.." Slash: "..slash.." Mask: "..mask.." Opts: "..opts)
    local pattern = mask
    if pattern then
        pattern = "^" .. pattern:gsub("%.", "%%."):gsub("%*", ".*") .. "$"
    end
    -- print("Path: "..path.." Mask: "..mask.." Opts: "..opts)
    local dir, msg = fs.getdir(path)
    if not dir then
        -- io.stderr:write("Error:"..msg.."\n")
        return nil
    end
    if target then
        source = path
        path = {source, target}
    end
    return dir, path, mask, pattern, opts
end
function fsattr2str(a)
    local attrs = {'A', 'F', 'V', 'S', 'H', 'R'}
    local str = ""
    for bit = 0, 5 do
        local ach = "-"
        if (a & (1 << bit)) > 0 then
            pos = 6 - bit
            ach = attrs[pos]
        end
        str = ach .. str
    end
    return str
end
function free(pathplus)
    local pathplus = pathplus or ""
    local fs = filesystem
    local out = ""
    local path, opts = pathplus:match("^([^?]*)%??([^\\/]-)$")
    path = path or ""
    local savepath = fs.getpath()
    local blks = fs.getfree(path)
    if blks then
        if opts:find("b") then
            out = "Free space on " .. path .. " " ..
                      tostring(math.floor(blks * 512)) .. " Bytes"
        elseif opts:find("k") then
            out = "Free space on " .. path .. " " ..
                      tostring(math.floor(blks // 2)) .. " kB"
        elseif opts:find("m") then
            out = "Free space on " .. path .. " " ..
                      tostring(math.floor(blks // 2048)) .. " MB"
        else
            out = tostring(math.floor(blks))
        end
    else
        out = "Error geting free blocks from '" .. path .. "'"
    end
    go(savepath)
    return (out:gsub("^(.-)%s*$", "%1"))
end
function volumes(arg)
    local out = ""
    if arg == "??" then
        out = [[
volumes: list available volumes
]]
    else
        local vollist = fs.getvols()
        if vollist then
            for i, vol in ipairs(vollist) do
                local f = fs.getfree(vol .. ":")
                if f then out = out .. vol .. "\n" end
            end
        else
            out = "No volumes found."
        end
    end
    return (out:gsub("^(.-)%s*$", "%1"))
end
vol = volumes
function folder(pathplus)
    local pathplus = pathplus or ""
    local fs = filesystem
    local out = ""
    local path, opts = pathplus:match("^([^?]*)%??([^\\/]-)$")
    if opts:find("?") then
        out = [[
folder: create a new folder
Examples: folder "myfolder" ; folder "my/folder"
Options after ?:
    p - create all folders in path if needed (not implemented)
]]
    else
        path = path or ""
        out = fs.mkdir(path)
        out = out or "Could not create folder '" .. path .. "'"
    end
    return (out:gsub("^(.-)%s*$", "%1"))
end
function attr(pathplus)
    local pathplus = pathplus or ""
    local fs = filesystem
    local out = ""
    local path, opts = pathplus:match("^([^?]*)%??([^\\/]-)$")
    if opts:find("?") then
        out = [[
attr: show and/or set file attributes
Examples: attr "myfile.lua" ; attr "sys.lua?Asr"
Options after ?:
    a - set archive attribute
    A - delete archive attribute
    s - set system attribute
    S - delete system attribute
    h - set hidden attribute
    H - delete hidden attribute
    r - set read-only attribute
    R - delete read-only attribute
]]
    else
        path = path or ""
        local s = fs.getstat(path)
        out = fsattr2str(s.attributes)
        local amask, aflags = 0, 0
        if opts:find("a") then
            amask = amask | 0x20
            aflags = aflags | 0x20
        end
        if opts:find("A") then
            amask = amask | 0x20
            aflags = aflags & ~0x20
        end
        if opts:find("s") then
            amask = amask | 0x04
            aflags = aflags | 0x04
        end
        if opts:find("S") then
            amask = amask | 0x04
            aflags = aflags & ~0x04
        end
        if opts:find("h") then
            amask = amask | 0x02
            aflags = aflags | 0x02
        end
        if opts:find("H") then
            amask = amask | 0x02
            aflags = aflags & ~0x02
        end
        if opts:find("r") then
            amask = amask | 0x01
            aflags = aflags | 0x01
        end
        if opts:find("R") then
            amask = amask | 0x01
            aflags = aflags & ~0x01
        end
        if amask > 0 then
            ok = fs.attr(path, aflags, amask)
            if not ok then
                out = "Error setting attributes on '" .. path .. "'"
            else
                s = fs.getstat(path)
                out = out .. " -> " .. fsattr2str(s.attributes)
            end
        end
    end
    return (out:gsub("^(.-)%s*$", "%1"))
end
dofile("/sys/del.lua")
dofile("/sys/copy.lua")
dofile("/sys/rename.lua")
ren = rename
dofile("/sys/dir.lua")
dofile("/sys/go.lua")
function up() return go("..") end
return "ok"
-- EOF
