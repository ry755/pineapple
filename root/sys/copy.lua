function copy(pathplus)
    local out = ""
    local fs = filesystem
    local dir, path, mask, pattern, opts = fsparse(pathplus)
    if not dir then
        return "Cannot find source volume or path for " .. pathplus
    end
    local dryrun = opts:find("d")
    local quiet = opts:find("q")
    local n = 0
    if opts:find("?") then
        out = [[
copy: copy file(s)
Examples: copy "foo.txt>bar.txt" ; copy "*.txt>archive" ; copy "sys/test.lua>."
Options after ?:
    d - dry run, does not copy
    q - quiet, no output
]]
    else
        if not type(path) == "table" then
            return "Cannot get souce and destination, FROM>TO needed."
        end
        path, dest = table.unpack(path)
        if not dest then return "Cannot get destination, FROM>TO needed." end
        local dstattr = fs.getstat(dest)
        local dstisdir = (dstattr and (dstattr.attributes & 0x10))
        dstisdir = dstisdir or (dest == '.') or (dest == '..')
        local destdir
        if dstisdir then destdir = dest end
        if mask:find("*") and not dstisdir then
            return "Destination '" .. dest .. "' is not a folder. Cannot " ..
                       myname .. " with wildcard source."
        end
        -- print("COPY: "..path.."+"..mask.." to "..dest.." ?opts: "..opts)
        for file, stat in pairs(dir) do
            if pattern and file:match(pattern) then
                n = n + 1
                local source = file
                if #path > 0 then source = path .. "/" .. source end
                if dstisdir then dest = destdir .. "/" .. file end
                if not dryrun then
                    ok, msg = fs.copy(source, dest)
                    if not ok then
                        io.stderr:write("Could not copy file: '" .. source ..
                                            "' to '" .. dest .. "'\n")
                    elseif not quiet then
                        out = out .. "Copied file: '" .. source .. "' to '" ..
                                  dest .. "'\n"
                    end
                elseif not quiet then
                    out = out .. "Did not copy file: '" .. source .. "' to '" ..
                              dest .. "'\n"
                end
            end
        end
        if not quiet and n < 1 then out = "No files to copy." end
    end
    return (out:gsub("^(.-)%s*$", "%1"))
end
return "ok"
