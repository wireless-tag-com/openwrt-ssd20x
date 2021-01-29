module("wt.network", package.seeall)

local ubus = require "ubus"
local ipc = require "luci.ip"

local ubus_r = ubus.connect()

function get_iface_status(iface)
	local u_cmd = "network.interface." .. iface
	local data = ubus_r:call(u_cmd, "status",{})

	return data
end

function get_wan_ipaddr()
	local result = {}
	local data = get_iface_status("wan")

	if not data or not data.up then
		return nil
	end

	if data.up == true then
		local ip4_addr = data["ipv4-address"]
		local mask_str = "0.0.0.0/" .. ip4_addr[1].mask
		result.ipv4addr = ip4_addr[1].address
		result.ipv4mask = ipc.IPv4(mask_str):mask():string()
		return result
	end

	return nil
end

function dhcp_leases_common(family)
    local rv = { }
    local nfs = require "nixio.fs"
    local leasefile = "/var/dhcp.leases"
    local dev_hostname = "unknown"

    local fd = io.open(leasefile, "r")
    if fd then
        while true do
            local ln = fd:read("*l")
            if not ln then
                break
            else
                local ts, mac, ip, name, duid = ln:match("^(%d+) (%S+) (%S+) (%S+) (%S+)")
                if ts and mac and ip and name and duid then
                    if family == 4 and not ip:match(":") then
                        if name ~= "*" then
                            dev_hostname = name
                        else
                            dev_hostname = "unknown"
                        end

                        rv[#rv+1] = {
                            macaddr  = mac,
                            hostname = dev_hostname
                        }
                    end
                end
            end
        end
        fd:close()
    end

    return rv
end
