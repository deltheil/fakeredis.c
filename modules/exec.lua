_exec = function(rds, cmd)
  local args = _tokenize(cmd)
  local cmd = string.lower(args[1])
  return rds[cmd] and _fmt(rds[cmd](rds, unpack(args, 2)))
end
