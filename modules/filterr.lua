_filterr = function(str)
  local x = string.gsub(str, ".*:%d+:%s", "")
  return string.find(x, "assertion failed") and "ERR internal error" or x
end
