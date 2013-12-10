_exec = function(rds, cmd)
  args = _tokenize(cmd)
  cmd = string.lower(table.remove(args, 1))
  return _fmt(rds[cmd](rds, unpack(args)))
end
