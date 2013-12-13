local is_integer = function(x)
  return (type(x) == "number") and (math.floor(x) == x)
end

_fmtreply = function(res)
  if is_integer(res) then
    return "(integer) " .. res
  elseif type(res) == "boolean" then
    return res and "true" or "false"
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
