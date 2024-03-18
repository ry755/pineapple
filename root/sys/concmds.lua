-- lupos console commands
con = console
function rgba(r, g, b, a) return ((a << 24) + (r << 16) + (g << 8) + b) end
function clear()
    io.write("\027[H\027[J")
    io.flush()
end
