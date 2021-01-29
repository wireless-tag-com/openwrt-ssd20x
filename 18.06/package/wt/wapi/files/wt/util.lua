local io = require "io"
local _ubus = require "ubus"
local nixio = require "nixio"
local _ubus_connection = nil
local type, assert = type, assert
local ipairs, pairs, os = ipairs, pairs, os

module "wt.util"

function file_exists(filename)
	local file = io.open(filename, "r")
	if (file) then
		file:close()
		return true
	end

	return false
end

function call(...)
	return os.execute(...) / 256
end

function call_quiet(call_cmd)
	local os_cmd = call_cmd .. " >/dev/null 2>&1"
	return os.execute(os_cmd) / 256
end

function split(str, pat, max, regex)
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

function trim(str)
	return (str:gsub("^%s*(.-)%s*$", "%1"))
end

function exec(command)
	local pp   = io.popen(command)
	local data = pp:read("*a")
	pp:close()

	return data
end

function execi(command)
	local pp = io.popen(command)

	return pp and function()
		local line = pp:read()

		if not line then
			pp:close()
		end

		return line
	end
end

-- Deprecated
function execl(command)
	local pp   = io.popen(command)
	local line = ""
	local data = {}

	while true do
		line = pp:read()
		if (line == nil) then break end
		data[#data+1] = line
	end
	pp:close()

	return data
end

function fork_exec(command)
	local pid = nixio.fork()
	if pid > 0 then
		return
	elseif pid == 0 then
		-- change to root dir
		nixio.chdir("/")

		-- patch stdin, out, err to /dev/null
		local null = nixio.open("/dev/null", "w+")
		if null then
			nixio.dup(null, nixio.stderr)
			nixio.dup(null, nixio.stdout)
			nixio.dup(null, nixio.stdin)
			if null:fileno() > 2 then
				null:close()
			end
		end

		-- replace with target command
		nixio.exec("/bin/sh", "-c", command)
	end
end

function ubus(object, method, data)
	if not _ubus_connection then
		_ubus_connection = _ubus.connect()
		assert(_ubus_connection, "Unable to establish ubus connection")
	end

	if object and method then
		if type(data) ~= "table" then
			data = { }
		end
		return _ubus_connection:call(object, method, data)
	elseif object then
		return _ubus_connection:signatures(object)
	else
		return _ubus_connection:objects()
	end
end

function contains(table, value)
	for k, v in pairs(table) do
		if value == v then
			return k
		end
	end
	return false
end

function check_val_switch(switch)
    local switch_range = {"0", "1"}
    if switch then
        for _, val in ipairs(switch_range) do
            if val == switch then
                return true
            end
        end
    end

    return false
end

function ipv4tonumber(ipv4addr)
	local o1,o2,o3,o4 = ipv4addr:match("(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)")

	local num = 2^24*o1 + 2^16*o2 + 2^8*o3 + o4

	return num
end
