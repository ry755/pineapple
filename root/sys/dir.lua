function dir(pathplus)
    local dir_columns = dir_columns or 5
    local out = ""
    local fs = filesystem
    local dir, path, mask, pattern, opts = fsparse(pathplus, "*")
    if not dir then return "No such volume or path " .. pathplus end
    if type(path) == "table" then
        return "Cannot process from>to file specification."
    end

    -- print("Path: "..path.." Mask: "..mask.." Pat: "..pattern.." Opts: "..opts)
    local n = 0
    local showall = opts:find("a")
    local long = opts:find("l")
    local single = opts:find("o")
    local nodirs = opts:find("f")
    local dironly = opts:find("F")
    local archonly = opts:find("A")
    local sysonly = opts:find("S")
    local hiddenonly = opts:find("H")
    local roonly = opts:find("R")
    showall = showall or sysonly or hiddenonly
    local reverse = opts:find("r")
    local sort = "name"
    if opts:find("s") then sort = "size" end
    if opts:find("t") then sort = "modified" end
    if opts:find("?") then
        out = [[
dir: list directory of folder
Examples: dir "sd:/sys?l" ; dir "*.lua?osr"
Options after ?: 
    l - long form
    o - one column
    s - sort by size (not name)
    t - sort by date/time
    r - sort reverse
    a - show all files/folders
    f - files only
    F - folders only
    A - with archive attribute only
    S - only system files
    H - only hidden files
    R - only read-only files

    The FARHS options can be culminated
]]
    else
        local names = {}
        for f, t in pairs(dir) do table.insert(names, f) end
        table.sort(names, function(a, b)
            if reverse then
                return dir[a][sort] > dir[b][sort]
            else
                return dir[a][sort] < dir[b][sort]
            end
        end)
        for i, f in ipairs(names) do
            local s = dir[f];
            local isarch = (s.attributes & 0x20) > 0
            local isdir = (s.attributes & 0x10) > 0
            local issys = (s.attributes & 0x04) > 0
            local ishidden = (s.attributes & 0x02) > 0
            local isro = (s.attributes & 0x01) > 0
            local show = f:match(pattern)
            show = show and
                       ((dironly and isdir) or (nodirs and not isdir) or
                           (not (dironly or nodirs))) -- no opt, show both
            show = show and (not archonly or isarch)
            show = show and (not sysonly or issys)
            show = show and (not hiddenonly or ishidden)
            show = show and (not roonly or isro)
            if show then
                if showall or (s.attributes & 0x02) == 0 then
                    if not (long or single) and #f > 19 then
                        f = f:sub(1, 17) .. ".."
                    end
                    n = n + 1
                    if long then
                        out = out .. os.date("%d.%m.%y %H:%M:%S ", s.modified)
                        if (s.attributes & 0x10) > 0 then
                            out = out .. string.rep(' ', 9) .. "- "
                        else
                            out = out .. string.format("%10d ", s.size)
                        end
                        out = out .. fsattr2str(s.attributes) .. " "
                    end
                    out = out .. f
                    if single or long or n % dir_columns == 0 then
                        out = out .. "\n"
                    else
                        out = out .. string.rep(" ", 20 - #f)
                    end
                end
            end
        end
        if n < 1 then
            out = "No "
            if not dironly or nodirs then out = out .. "files " end
            if dironly == nodirs or (dironly and nodirs) then
                out = out .. "or "
            end -- xor 
            if not nodirs or dironly then out = out .. "folders " end
            out = out .. "found in "
            out = out ..
                      ((#path > 0) and "'" .. path .. "' " or "current path ")
            out = out .. "matching '" .. mask .. "'."
        end
    end
    ::exit::
    return (out:gsub("^(.-)%s*$", "%1"))
end
return "ok"
