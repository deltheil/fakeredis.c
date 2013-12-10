_tokenize = function(cmd)
  local args = {}
  for tok in string.gmatch(cmd, "%S+") do
    args[#args+1] = tok
  end
  return args
end
