#!/usr/bin/env bash
. /usr/share/libubox/jshn.sh

json_load "$(ubus call session create)"
json_get_var sid ubus_rpc_session


json_init
json_add_string ubus_rpc_session "$sid"
json_add_array "objects"
json_add_array ""
json_add_string "" "session"
json_add_string "" "list"
json_close_array
json_close_array

ubus call session grant "$(json_dump)"

echo "Session: $sid"
echo "Request 1"
wget -q -O- \
	--post-data='{
		"jsonrpc": "2.0",
		"method" : "call",
		"params" : [
			"'$sid'",
			"session",
			"test",
			{},
		]
	}' "http://localhost:8080/ubus"
echo "Request 2"
wget -q -O- \
	--post-data='[
	{
		"jsonrpc": "2.0",
		"method" : "call",
		"params" : [
			"'$sid'",
			"session",
			"list",
			{},
		]
	},
	{
		"jsonrpc": "2.0",
		"method" : "call",
		"params" : [
			"'$sid'",
			"session",
			"test",
			{},
		]
	},
	]' "http://localhost:8080/ubus"
