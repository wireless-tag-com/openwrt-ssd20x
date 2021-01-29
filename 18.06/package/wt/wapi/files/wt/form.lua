module("wt.form", package.seeall)

local cjson = require "cjson"
local ipairs, pairs, type, assert, print = ipairs, pairs, type, assert, print

function uci_tset(uci_r, config, section, values)
  local stat = true
  for k, v in pairs(values) do
    if k:sub(1, 1) ~= "." then
      stat = stat and uci_r:set(config, section, k, v)
    end
  end
  return stat
end

local function uci_section(uci_r, config, type, name, values)
  local stat = true
  if name then
    stat = uci_r:set(config, name, type)
  else
    name = uci_r:add(config, type)
    stat = name and true
  end

  if stat and values then
    stat = uci_tset(uci_r, config, name, values)
  end

  return stat and name
end

--- Find target item.
-- @param uci_r        uci.cursor()
-- @param cfg        UCI config name
-- @param stype        UCI section type
-- @param target    Target item
-- @param keys        Use to match items (optional)
-- @return            Section name of the target item
local function find_item(uci_r, cfg, stype, target, keys, filter)
    keys = keys

    local sname
    uci_r:foreach(cfg, stype,
                  function (section)
                      if filter then
                        u_filter = uci_r:get(cfg, section[".name"], filter)
                        if not u_filter then
                          return
                        end
                      end

                      if keys then
                          for _, k in pairs(keys) do
                              if target[k] ~= section[k] then
                                  return
                              end
                          end
                      else
                          local tic = 0
                          for k, v in pairs(target) do
                              tic = tic + 1
                              if v ~= section[k] then
                                  return
                              end
                          end

                          local sic = 0
                          for k, _ in pairs(section) do
                              if k:sub(1, 1) ~= "." then
                                  sic = sic + 1
                              end
                          end

                          if tic ~= sic then
                              return
                          end
                      end

                      -- Found it.
                      sname =  section[".name"]
                      return false
                  end
    )

    return sname
end

--- Insert a form item.
-- @param uci_r        uci.cursor()
-- @param cfg        UCI config name
-- @param stype        UCI section type
-- @param new        New item
-- @param keys        Use to match items (optional)
-- @return            New item if succeeded or false on error
function insert(uci_r, cfg, stype, new, keys, filter)
    assert(cfg and stype and new)

    local new = type(new) == "table" and new or cjson.decode(new)
    if not new then
        return false
    end

    if find_item(uci_r, cfg, stype, new, keys, filter) then
        return false
    end

    local sname = uci_section(uci_r, cfg, stype, nil, new)
    if sname then
    --  uci_r:reorder(cfg, sname, 0)
        return new
    end

    return false
end


--- Update form items.
-- @param uci_r        uci.cursor()
-- @param cfg        UCI config name
-- @param stype        UCI section type
-- @param old        Old item
-- @param new        New item
-- @param keys        Use to match items (optional)
-- @return            New item if succeeded or false on error
function update(uci_r, cfg, stype, old, new, keys, filter)
    assert(cfg and stype and old and new)

    local old = type(old) == "table" and old or cjson.decode(old)
    local new = type(new) == "table" and new or cjson.decode(new)
    if not new then
        return false
    end

    local sname = find_item(uci_r, cfg, stype, old, keys, filter)
    if not sname then
        return false
    end

    local sname_new = find_item(uci_r, cfg, stype, new, keys, filter)
    if sname_new and sname_new ~= sname then
        return false
    end

    if uci_section(uci_r, cfg, stype, sname, new) then
        return new
    end

    return false
end

--- Delete form items.
-- @param uci_r        uci.cursor()
-- @param cfg        UCI config name
-- @param stype        UCI section type
-- @param key        Key of items to be deleted
-- @param index        Index of items to be deleted
-- @return            Table indicates if succeeded or not
function delete(uci_r, cfg, stype, key, index, filter)
    assert(cfg and stype and key and index)

    local keys  = type(key) == "table" and key or {key}
    local indexes = type(index) == "table" and index or {index}

    local items = {}
    uci_r:foreach(cfg, stype,
        function(section)
            if filter then
              u_filter = uci_r:get(cfg, section[".name"], filter)
              if not u_filter then
                return
              end
            end
            items[#items + 1] = section[".name"]
        end
    )

    local data = {}
    for i, v in ipairs(indexes) do
        if v >= #items then 
            return
        end
        data[#data + 1] = {
            key     = keys[i],
            index   = v,
            success = uci_r:delete(cfg, items[v + 1]) and true or false
        }
    end

    return data
end

--- Count of form items.
-- @param uci_r        uci.cursor()
-- @param cfg        UCI config name
-- @param stype        UCI section type
-- @return            Number
function count(uci_r, cfg, stype, filter)
    assert(cfg and stype)

    local count = 0
    uci_r:foreach(cfg, stype,
                  function(section)
                      if filter then
                        u_filter = uci_r:get(cfg, section[".name"], filter)
                        if not u_filter then
                          return
                        end
                      end
                      count = count + 1
                  end
    )
    return count
end
