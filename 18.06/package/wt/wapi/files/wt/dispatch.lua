
module("wt.dispatch", package.seeall)

local g_errcode = require "wt.errcode"
local g_util = require "wt.util"
local cjson = require "cjson"
local string = string

function throw_error(param, op_code)
	local err_data = param

	err_data["errcode"] = op_code or g_errcode.WT_ERRCODE_PARAMS_INVALID

	return err_data
end

function dispatch_request(m_type, param)
	local iparam = param
	local api_dir = "api"
	local response_data = {}

	local op_sid = nil
	local op_remote_addr = nil
	local op_topic = nil
	local response_topic = nil

	if m_type == "web" then
		op_sid = iparam["sid"]
		op_remote_addr = iparam["remote_addr"]
		if not op_sid then
			return throw_error(iparam, g_errcode.WT_ERRCODE_PARAMS_INVALID);
		end
	elseif m_type == "mqtt" then
		op_topic = param["topic"]
		iparam = cjson.decode(param["payload"])
		if op_topic then
			t_op = string.sub(op_topic, 1, 1)
			if t_op ~= "/" then
				return throw_error(iparam, g_errcode.WT_ERRCODE_PARAMS_INVALID);
			end

			local t_parts = g_util.split(string.sub(op_topic, 2), "/")

			response_topic = "/" .. t_parts[1] .. "/" .. t_parts[2] .. "/report"
		end
	end

	local op_version = iparam["ver"]
	local op_module = iparam["module"]
	local op_api = iparam["api"]
	local op_param = iparam["param"]
	local op_mid = iparam["mid"]

	if not op_module or not op_api then
		return throw_error(iparam, g_errcode.WT_ERRCODE_PARAMS_INVALID);
	end

	local mod = "wt." .. api_dir .. "." .. op_module
	local mod_path = "/usr/lib/lua/wt/" .. api_dir .. "/" .. op_module .. ".lua"

	if not g_util.file_exists(mod_path) then
		return throw_error(iparam, g_errcode.WT_ERRCODE_MODULE_NOT_FOUND)
	end

	mod = require(mod)

	if not mod then
		return throw_error(iparam, g_errcode.WT_ERRCODE_MODULE_NOT_FOUND)
	end

	local dispatch_op_api = op_module .. "_" .. op_api
	local dispatch_api = mod[dispatch_op_api]

	if not dispatch_api then
		return throw_error(iparam, g_errcode.WT_ERRCODE_MODULE_API_NOT_FOUND)
	end

	local api_return_data
	api_return_data = dispatch_api(op_param, op_sid, op_remote_addr)

	if type(api_return_data) ~= "table" then
		return throw_error(iparam, g_errcode.WT_ERRCODE_API_FAILED)
	end

	if op_sid then
		response_data.sid = op_sid
	end

	if op_version then
		response_data.ver = op_version
	end

	if op_mid then
		response_data.mid = op_mid
	end

	if response_topic then
		print(response_topic)
	end

	response_data.module = op_module
	response_data.api = op_api
	response_data.result = api_return_data.result
	response_data.errcode = api_return_data.errcode

	return response_data
end
