function del(pathplus)
    local out = ""
    local fs = filesystem
    local dir, path, mask, pattern, opts = fsparse(pathplus)
    if not dir then return "No such volume or path " .. pathplus end
    if type(path) == "table" then
        return "Cannot process from>to file specification."
    end
    local dryrun = opts:find("d")
    local quiet = opts:find("q")
    local delall = opts:find("a")
    local n = 0
    if opts:find("?") then
        out = [[
del: delete files
Examples: del "*.txt" ; del "foo*?d"
Options after ?:
    a - also delete hidden/system files
    d - dry run, does not delete
    q - quiet, no output
]]
    else
        local savepath = fs.getpath()
        go(path)
        for file, stat in pairs(dir) do
            visible = (stat.attributes & 0x06) == 0
            delit = delall or visible;
            if delit and pattern and file:match(pattern) then
                n = n + 1
                if not dryrun then
                    ok, msg = fs.unlink(file)
                    if not ok then
                        io.stderr:write("Could not delete file: '" .. file ..
                                            "'\n")
                    elseif not quiet then
                        out = out .. "Deleted file: '" .. file .. "'\n"
                    end
                elseif not quiet then
                    out = out .. "Not deleted file: '" .. file .. "'\n"
                end
            end
        end
        go(savepath)
        if not quiet and n < 1 then out = "No files to delete." end
    end
    return (out:gsub("^(.-)%s*$", "%1"))
end
