local io = require "io"
local nixio  = require "nixio"
local os = os

module "wt.sys"

function getpasswd(username)
	local pwe = nixio.getsp and nixio.getsp(username) or nixio.getpw(username)
	local pwh = pwe and (pwe.pwdp or pwe.passwd)
	if not pwh or #pwh < 1 or pwh == "!" or pwh == "x" then
		return nil, pwe
	else
		return pwh, pwe
	end
end

function checkpasswd(username, pass)
	local pwh, pwe = getpasswd(username)
	if pwe then
		return (pwh == nil or nixio.crypt(pass, pwh) == pwh)
	end
	return false
end

function setpasswd(username, password)
	if password then
		password = password:gsub("'", [['"'"']])
	end

	if username then
		username = username:gsub("'", [['"'"']])
	end

	return os.execute(
		"(echo '" .. password .. "'; sleep 1; echo '" .. password .. "') | " ..
		"passwd '" .. username .. "' >/dev/null 2>&1"
	)
end

function reboot()
	return os.execute("reboot >/dev/null 2>&1")
end

