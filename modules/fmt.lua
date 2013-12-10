_fmt = function(res)
  if type(res) == "number" then
    return "(integer) " .. res
  elseif type(res) == "nil" then
    return "(nil)"
  elseif type(res) == "table" then
    local t = {}
    for k, v in pairs(res) do
      if type(k) == "number" then
        t[#t+1] = k .. ") " .. v
      else
        t[#t+1] = #t+1 .. ") " .. k
        t[#t+1] = #t+1 .. ") " .. v
      end
    end
    return table.concat(t, "\n")
  else
    return tostring(res)
  end
end
