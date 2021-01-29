local fs = require "nixio.fs"
local ip = require "luci.ip"
local math = require "math"
local bit    = require "nixio".bit

local tonumber, tostring, type, unpack, select = tonumber, tostring, type, unpack, select

module("wt.datatypes", package.seeall)

local function split(str, pat, max, regex)
    pat = pat or "\n"
    max = max or #str

    local t = {}
    local c = 1

    if #str == 0 then
        return {""}
    end

    if #pat == 0 then
        return nil
    end

    if max == 0 then
        return str
    end

    repeat
        local s, e = str:find(pat, c, not regex)
        max = max - 1
        if s and max < 0 then
            t[#t+1] = str:sub(c)
        else
            t[#t+1] = str:sub(c, s and s - 1)
        end
        c = e and e + 1 or #str + 1
    until not s or max < 0

    return t
end

function bool(val)
    if val == "1" or val == "yes" or val == "on" or val == "true" then
        return true
    elseif val == "0" or val == "no" or val == "off" or val == "false" then
        return true
    elseif val == "" or val == nil then
        return true
    end

    return false
end

function uinteger(val)
    local n = tonumber(val)
    if n ~= nil and math.floor(n) == n and n >= 0 then
        return true
    end

    return false
end

function integer(val)
    local n = tonumber(val)
    if n ~= nil and math.floor(n) == n then
        return true
    end

    return false
end

function ufloat(val)
    local n = tonumber(val)
    return ( n ~= nil and n >= 0 )
end

function float(val)
    return ( tonumber(val) ~= nil )
end

function ipaddr(val)
    return ip4addr(val) or ip6addr(val)
end

function ip4addr(val)
    if val then
        return ip.IPv4(val) and true or false
    end

    return false
end

function ip4prefix(val)
    val = tonumber(val)
    return ( val and val >= 0 and val <= 32 )
end

function ip6addr(val)
    if val then
        return ip.IPv6(val) and true or false
    end

    return false
end

function ip6prefix(val)
    val = tonumber(val)
    return ( val and val >= 0 and val <= 128 )
end

function port(val)
    val = tonumber(val)
    return ( val and val >= 0 and val <= 65535 )
end

function portrange(val)
    local p1, p2 = val:match("^(%d+)%-(%d+)$")
    if p1 and p2 and port(p1) and port(p2) then
        return true
    else
        return port(val)
    end
end

function macaddr(val)
    if val and val:match(
        "^[a-fA-F0-9]+:[a-fA-F0-9]+:[a-fA-F0-9]+:" ..
         "[a-fA-F0-9]+:[a-fA-F0-9]+:[a-fA-F0-9]+$"
    ) then
        local parts = split( val, ":" )

        for i = 1,6 do
            parts[i] = tonumber( parts[i], 16 )
            if parts[i] < 0 or parts[i] > 255 then
                return false
            end

            -- check multicast/broadcast
            if i == 1 then
                if bit.band(parts[i], 0x1) == 1 then
                    return false
                end
            end
        end

        return true
    end

    return false
end

function hostname(val)
    if val and (#val < 254) and (
       val:match("^[a-zA-Z_]+$") or
       (val:match("^[a-zA-Z0-9_][a-zA-Z0-9_%-%.]*[a-zA-Z0-9]$") and
        val:match("[^0-9%.]"))
    ) then
        return true
    end
    return false
end

function host(val)
    return hostname(val) or ipaddr(val)
end

function network(val)
    return uciname(val) or host(val)
end

function wpakey(val)
    if #val == 64 then
        return (val:match("^[a-fA-F0-9]+$") ~= nil)
    else
        return (#val >= 8) and (#val <= 63)
    end
end

function wepkey(val)
    if val:sub(1, 2) == "s:" then
        val = val:sub(3)
    end

    if (#val == 10) or (#val == 26) then
        return (val:match("^[a-fA-F0-9]+$") ~= nil)
    else
        return (#val == 5) or (#val == 13)
    end
end

function string(val)
    return true        -- Everything qualifies as valid string
end

function directory( val, seen )
    local s = fs.stat(val)
    seen = seen or { }

    if s and not seen[s.ino] then
        seen[s.ino] = true
        if s.type == "dir" then
            return true
        elseif s.type == "lnk" then
            return directory( fs.readlink(val), seen )
        end
    end

    return false
end

function file( val, seen )
    local s = fs.stat(val)
    seen = seen or { }

    if s and not seen[s.ino] then
        seen[s.ino] = true
        if s.type == "reg" then
            return true
        elseif s.type == "lnk" then
            return file( fs.readlink(val), seen )
        end
    end

    return false
end

function device( val, seen )
    local s = fs.stat(val)
    seen = seen or { }

    if s and not seen[s.ino] then
        seen[s.ino] = true
        if s.type == "chr" or s.type == "blk" then
            return true
        elseif s.type == "lnk" then
            return device( fs.readlink(val), seen )
        end
    end

    return false
end

function uciname(val)
    return (val:match("^[a-zA-Z0-9_]+$") ~= nil)
end

function range(val, min, max)
    val = tonumber(val)
    min = tonumber(min)
    max = tonumber(max)

    if val ~= nil and min ~= nil and max ~= nil then
        return ((val >= min) and (val <= max))
    end

    return false
end

function min(val, min)
    val = tonumber(val)
    min = tonumber(min)

    if val ~= nil and min ~= nil then
        return (val >= min)
    end

    return false
end

function max(val, max)
    val = tonumber(val)
    max = tonumber(max)

    if val ~= nil and max ~= nil then
        return (val <= max)
    end

    return false
end

function rangelength(val, min, max)
    val = tostring(val)
    min = tonumber(min)
    max = tonumber(max)

    if val ~= nil and min ~= nil and max ~= nil then
        return ((#val >= min) and (#val <= max))
    end

    return false
end

function minlength(val, min)
    val = tostring(val)
    min = tonumber(min)

    if val ~= nil and min ~= nil then
        return (#val >= min)
    end

    return false
end

function maxlength(val, max)
    val = tostring(val)
    max = tonumber(max)

    if val ~= nil and max ~= nil then
        return (#val <= max)
    end

    return false
end

function phonedigit(val)
    return (val:match("^[0-9\*#]+$") ~= nil)
end

function is_empty(val)
    if val == nil then
        return true
    elseif type(val) == "string" then
        return #val == 0
    else
        return false
    end
end

function is_not_empty(val)
    return not is_empty(val)
end

function ip_to_number(ip)
    local b1, b2, b3, b4 = ip:match("^(%d+)%.(%d+)%.(%d+)%.(%d+)$")
    b1 = tonumber(b1)
    b2 = tonumber(b2)
    b3 = tonumber(b3)
    b4 = tonumber(b4)
    if b1 and b1 <= 255
        and b2 and b2 <= 255
        and b3 and b3 <= 255
        and b4 and b4 <= 255
    then
        return bit.lshift(b4, 0)
            + bit.lshift(b3, 8)
            + bit.lshift(b2, 16)
            + bit.lshift(b1, 24)
    else
        return false
    end
end

function unicast_ip4addr(val)
    if not ip4addr(val) then
        return false
    end
    local b1, b2, b3, b4 = val:match("^(%d+)%.(%d+)%.(%d+)%.(%d+)$")
    b1 = tonumber(b1)
    b2 = tonumber(b2)
    b3 = tonumber(b3)
    b4 = tonumber(b4)

    if b1 >= 224 or b1 == 127 or b1 == 0
    then
        return false
    end

    if b4 >=255 or b4 == 0 then
        return false
    end

    return true
end

function number_to_ip(n)
    local b4 = bit.band(bit.rshift(n, 0), 0xFF)
    local b3 = bit.band(bit.rshift(n, 8), 0xFF)
    local b2 = bit.band(bit.rshift(n, 16), 0xFF)
    local b1 = bit.band(bit.rshift(n, 24), 0xFF)

    return "%d.%d.%d.%d" % {b1, b2, b3, b4}
end

function ip4netmask(val, loose)
    local uint32_max = 0xffffffff
    local mask = ip_to_number(val)
    if mask then
        if mask == 0 then
            return false
        end

        if not loose and mask == uint32_max then
            return false
        end

        mask = bit.band(bit.bnot(mask), uint32_max)
        return bit.band(mask + 1, mask) == 0
    else
        return false
    end
end

function ip4_same_network(ip1, ip2, mask)
    ip1 = luci.ip.IPv4(ip1, mask)
    ip2 = luci.ip.IPv4(ip2, mask)
    return ip1 and ip2 and ip1:network() == ip2:network()
end

function ip4_network_conflict(ip1, mask1, ip2, mask2)
    local mask = luci.ip.IPv4(mask1):lower(luci.ip.IPv4(mask2)) and mask1 or mask2
    return ip4_same_network(ip1, ip2, mask)
end

function netproto(val)
    if val == "tcp" or val == "udp" or val == "tcpudp" then
        return true
    end

    return false
end

function check_mtu(val)
    local n = tonumber(val)
    if n ~= nil and math.floor(n) == n and n >= 0 and n <=1500 then
        return true
    end

    return false
end
