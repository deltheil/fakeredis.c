_exec = function(rds, cmd)
  args = _tokenize(cmd)
  cmd = string.lower(table.remove(args, 1))
  return rds[cmd] and _fmt(rds[cmd](rds, unpack(args))) or "ERR invalid command"
end
