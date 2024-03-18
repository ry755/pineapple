-- lua repl with line editing and history
local term = require "plterm"

local byte, char, rep = string.byte, string.char, string.rep
local col, keys = term.colors, term.keys

local nextk = term.input()

local hist = {""}
local hmax = repl_history_size or 20
local hpos = 1
local hfrm = string.format("%%%dd %%s\r\n", math.floor(math.log10(hmax)) + 1)

local function write_error(err)
    local ok, str = pcall(tostring, err)
    if not ok then str = 'error object is a (' .. type(err) .. ')' end
    io.stderr:write(str)
    io.stderr:write('\n')
end

local function call_func(fun)
    local res = table.pack(pcall(fun))
    if res[1] then
        if #res > 1 then
            print(table.unpack(res, 2, #res))
        else
            print("nil")
        end
    else
        write_error(res[2])
    end
end

local function exec_str(line)
    local fun, err
    fun, err = load('return ' .. line)
    if err then fun, err = load(line) end
    if fun then
        call_func(fun)
    else
        write_error(err)
    end
end

local function repl_loop(prompt)
    replvl = replvl and replvl + 1 or 1
    local prompt = prompt or (repl_prompt or 'L' .. replvl .. '>')
    local buf = ""
    local quit = nil
    local pos = 1
    local r, c = term.getcurpos()
    io.write(prompt)
    io.flush()

    while not quit do
        local k = nextk()
        if k == 10 or k == 13 then
            io.write("\r\n")
            if #buf > 0 then
                if buf == "??" then
                    print("Use 'help' for help!");
                else
                    local cmd = buf
                    if _ENV[buf] and type(_ENV[buf]) == "function" then
                        cmd = cmd .. "()";
                    end
                    exec_str(cmd)
                end
                hpos = #hist
                if hpos == 1 or hist[hpos - 1] ~= buf then
                    hist[hpos] = buf
                    table.insert(hist, "")
                    hpos = hpos + 1
                end
                buf = hist[hpos]
                if #hist > hmax + 1 then
                    table.remove(hist, 1)
                    hpos = #hist
                end
            end
            pos = 1
            r, c = term.getcurpos()
        elseif (k == byte 'P' - 64 or k == keys.kup) and hpos > 1 and #hist > 0 then
            hist[hpos] = buf
            hpos = hpos - 1
            buf = hist[hpos]
            pos = #buf + 1
        elseif (k == byte 'N' - 64 or k == keys.kdown) and hpos < #hist and
            #hist > 0 then
            hist[hpos] = buf
            hpos = hpos + 1
            buf = hist[hpos]
            pos = #buf + 1
        elseif k == byte 'L' - 64 then
            io.write("\r")
            term.cleareol()
            for i = 1, #hist - 1 do
                io.write(string.format(hfrm, i, hist[i]))
            end
            r, c = term.getcurpos()
        elseif k == byte 'R' - 64 then
            hidx = tonumber(buf)
            if hidx and hidx > 0 and hidx < hmax then
                hpos = hidx
                buf = hist[hpos]
                pos = #buf + 1
            end
        elseif (k == byte 'B' - 64 or k == keys.kleft) and pos > 1 then
            pos = pos - 1
        elseif (k == byte 'F' - 64 or k == keys.kright) and pos <= #buf then
            pos = pos + 1
        elseif k == byte 'A' - 64 or k == keys.khome then
            pos = 1
        elseif k == byte 'E' - 64 or k == keys.kend then
            pos = #buf + 1
        elseif (k == 8 or k == 127) and pos > 1 then -- backspace
            if pos > #buf then
                buf = buf:sub(1, -2)
            else
                buf = buf:sub(1, pos - 2) .. buf:sub(pos, -1)
            end
            pos = pos - 1
        elseif k == byte 'C' - 64 then
            print "^C"
            r, c = term.getcurpos()
            pos = 1
            buf = ""
        elseif k == byte 'D' - 64 or k == keys.kdel then
            if #buf < 1 and pos == 1 then
                quit = true
                break
            elseif pos == #buf then
                buf = buf:sub(1, -2)
            else
                buf = buf:sub(1, pos - 1) .. buf:sub(pos + 1, -1)
            end
        elseif ((k >= 32 and k < 127) or (k > 127 and k < 256) or (k == 9)) then
            local ch = char(k)
            if pos > #buf then -- Append input.
                buf = buf .. ch
            elseif pos == 1 then -- Prepend input.
                buf = ch .. buf
            else -- Insert in the middle
                buf = buf:sub(1, pos - 1) .. ch .. buf:sub(pos, -1)
            end
            pos = pos + 1
        else
            -- print("<"..k..">")
        end
        io.flush()
        io.write("\r")
        term.cleareol()
        io.write(prompt .. buf)
        term.golc(r, pos + #prompt)
        io.flush()
    end
    replvl = replvl - 1
end

function repl(banner, prompt, byebye)
    local banner = banner or ""
    local byebye = byebye or ""
    local savemode = term.savemode()
    term.setrawmode()
    io.write(banner)
    io.flush()
    local ok, msg = xpcall(repl_loop, debug.traceback, prompt)
    io.write("\r\n")
    term.restoremode(savemode)
    if not ok then -- display traceback in case of error
        print(msg)
    end
    io.write(byebye)
    io.flush()
end
