_exec = function(redis, cmd)
  local args = _tokenize(cmd)
  local cmd = string.lower(args[1])
  return redis[cmd] and _fmtreply(cmd, redis[cmd](redis, unpack(args, 2)))
end
