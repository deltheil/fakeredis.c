_exec = function(rds, cmd)
  local args = _tokenize(cmd)
  local cmd = string.lower(table.remove(args, 1))
  return rds[cmd] and _fmt(rds[cmd](rds, unpack(args)))
end
