module("wt.auth", package.seeall)

local dbg = require "wt.debug"
local util = require "wt.util"
local uci_r = require "uci".cursor()

function auth(sid)
    local noatuh = uci_r:get("uhttpd", "main", "no_ubusauth")

    if noatuh == "1" then
        return true
    end

    local param = {
        ubus_rpc_session = sid
    }

    if sid == "00000000000000000000000000000000" then
        return false
    end

    local data = util.ubus("session", "get", param)

    if not data then
        return false
    else
        return true
    end
end

