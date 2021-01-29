module("wt.debug", package.seeall)

console = "/dev/console"

--- Debug console output method
-- @param   string to display on console
-- @return  N/A 
function print(...)
    local file = io.open(console, "a+")
    for i = 1, select("#", ...) do
        local v = select(i, ...)

        file:write(tostring(v))
        file:write("\t")
    end
    file:write("\n")
    file:close()
end

-- For backward compatible.
printf = print

local function _print(self, ...)
    local info = debug.getinfo(2, "Sl")
    if tostring(info.source):sub(1,1) == "=" then
        self.print(...)
    else
        prefix = ("%s:%s:"):format(tostring(info.short_src),
        tostring(info.currentline))
        self.print(prefix, ...)
    end
end

getmetatable(_M).__call = _print

--- Recursively dumps a table to console, modified from luci.util.dumptable.
-- @param t	Table value to dump
-- @param maxdepth	Maximum depth
-- @return	Always nil
function dumptable(t, maxdepth, i, seen)
    i = i or 0
    seen = seen or setmetatable({}, {__mode="k"})

    for k,v in pairs(t) do
        print(string.rep("\t", i) .. tostring(k) .. "\t" .. tostring(v))
        if type(v) == "table" and (not maxdepth or i < maxdepth) then
            if not seen[v] then
                seen[v] = true
                dumptable(v, maxdepth, i+1, seen)
            else
                print(string.rep("\t", i) .. "*** RECURSION ***")
            end
        end
    end
end

function syslog(type, data)
    local nixio = require "nixio"

    nixio.openlog("wapi")
    nixio.syslog(type,data)
    nixio.closelog()
end
