-- Status code reply (true -> "OK", ping -> "PONG")
local _sreply = {
  ping = "PONG",
  set = true,
  mset = true,
  flushall = true,
  flushdb = true,
  hmset = true,
  lset = true,
  ltrim = true,
  rename = true
}

-- Integer reply (true -> "(integer) 1", false -> "(integer) 0")
local _ireply = {
  exists = true,
  hset = true,
  hsetnx = true,
  hexists = true,
  renamenx = true,
  setnx = true,
  msetnx = true,
  sismember = true,
  smove = true
}

local is_integer = function(x)
  return (type(x) == "number") and (math.floor(x) == x)
end

_fmtreply = function(cmd, res)
  -- specific cases
  if _sreply[cmd] then
    return (_sreply[cmd] == true) and "OK" or _sreply[cmd]
  elseif _ireply[cmd] then
    return "(integer) " .. (res and "1" or "0") -- res is a boolean
  end
  -- generic format reply
  if is_integer(res) then
    return "(integer) " .. res
  elseif type(res) == "nil" then
    return "(nil)"
  elseif type(res) == "table" then
    local t = {}
    for k, v in pairs(res) do
      if type(k) == "number" then
        if type(v) == "table" then -- zadd et al. return {value, score} pairs
          t[#t+1] = (2*k-1) .. ") " .. _rawfmt(v[1])
          t[#t+1] = (2*k)   .. ") " .. _rawfmt(v[2])
        else
          t[#t+1] = k .. ") " .. _rawfmt(v)
        end
      else
        t[#t+1] = #t+1 .. ") " .. _rawfmt(k)
        t[#t+1] = #t+1 .. ") " .. _rawfmt(v)
      end
    end
    return (#t > 0) and table.concat(t, "\n") or "(empty list or set)"
  else
    return _rawfmt(tostring(res))
  end
end
