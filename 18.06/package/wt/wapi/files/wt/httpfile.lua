local io = io
local os = os
local table = table
local type = type
local string = string
local pairs = pairs

local fs = require "nixio.fs"
local socket = require "socket"
local http = require "socket.http"
local https = require "ssl.https"
local ltn12 = require "ltn12"
local l_md5file = require "wt.md5file"
local c_debug = require "wt.debug"

module ("wt.httpfile")

ERROR_CODE = {
  E_OKAY = 0,
  E_FORMAT = 1,         --Image Format error
  E_MD5 = 2,            --MD5 sum dismatched.
  E_HTTP = 3,           --Something went wrong during downloading firmware, refers to http(s) errors.
  E_BUSY = 4,           --upgrade is running.
}

local e = ERROR_CODE

ERROR_STRING = {
  [e.E_OKAY] = "ok",
  [e.E_FORMAT] = "image format error",
  [e.E_MD5] = "image md5 dismatched",
  [e.E_HTTP] = "http download error",
  [e.E_BUSY] = "upgrade is runnig",
}

function is_ssl(url)
	if string.sub(url, 1, 5) == "https" then
		return true
	else
		return false
	end
end

local function __http_download(param, sink)
	if type(param) == "string" then
		param = {
			url = param
		}
	end
	param['sink'] = sink

	if is_ssl(param['url']) then
		param['capath'] = param['capath'] or '/etc/ssl/certs'
		return https.request(param)
	else
		return http.request(param)
	end
end

local function http_download_file(param, file)
	local sink = ltn12.sink.file(io.open(file, 'w'))
	local r, c = __http_download(param, sink)
	if r ~= 1 then
		sink = ltn12.sink.file(io.open(file, 'w'))
		if type(param) == "string" then
			param = { url = param }
		end
		local headers = param['headers']
		if headers == nil then
			headers = { ['connection'] = "Keep-Alive" }
		else
			headers['connection'] = "Keep-Alive"
		end
		param['headers'] = headers
		r, c = __http_download(param, sink)
	end
	return c
end

local function http_download_and_verify(param, file, md5)
  local ret
  for i = 0, 3, 1 do
    ret = http_download_file(param, file)
    if type(ret) == "number" and ret == 200 then
      if md5 == nil then
        break
      end

      local actual_md5 = l_md5file.md5file(file)
      if actual_md5 == md5 then
        break
      end
    end

    socket.select(nil, nil, 3)
    if i == 2 then
      param = param .. "?param=" .. os.time()
    end
  end

  if type(ret) ~= "number" or ret ~= 200 then
    c_debug.syslog("info", "Failed to download firmware", 1)
    return e.E_HTTP, ret
  end

  if md5 ~= nil then
    local actual_md5 = l_md5file.md5file(file)
    if actual_md5 ~= md5 then
      c_debug.syslog("info", "The MD5SUM from downloaded firmware is different from the server's")
      return e.E_MD5, string.format("Size: %d, MD5: %s", fs.stat(file, "size"), actual_md5)
    end
  end

  return e.E_OKAY, "Download OK."
end

function http_download(url, filename, md5)
	local tmp_name = os.tmpname()

	ret, err_text = http_download_and_verify(url, tmp_name, md5)

	if ret == e.E_OKAY then
		local mv_cmd = "mv " .. tmp_name .. " " .. filename
		os.execute(mv_cmd)
	end

	fs.remove(tmp_name)

	return ret, err_text
end