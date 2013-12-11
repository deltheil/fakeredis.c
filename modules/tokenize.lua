_tokenize = function(cmd)
  local args = {}
  for tok in string.gmatch(string.gsub(cmd, "\"", ""), "%S+") do
    args[#args+1] = tok
  end
  return args
end
