_fmt = function(res)
  if type(res) == "number" then
    return "(integer) " .. res
  elseif type(res) == "boolean" then
    return res and "true" or "false"
  elseif type(res) == "nil" then
    return "(nil)"
  elseif type(res) == "table" then
    local t = {}
    for k, v in pairs(res) do
      if type(k) == "number" then
        t[#t+1] = k .. ") " .. _rawfmt(v)
      else
        t[#t+1] = #t+1 .. ") " .. _rawfmt(k)
        t[#t+1] = #t+1 .. ") " .. _rawfmt(v)
      end
    end
    return table.concat(t, "\n")
  else
    return _rawfmt(tostring(res))
  end
end
