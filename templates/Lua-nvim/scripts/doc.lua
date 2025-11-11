package.path = "./vendor/mini-doc/lua/?.lua" .. package.path
require("mini.doc").generate({
        "lua/GENIT_NVIM.lua",
        "plugin/GENIT_NVIM.lua",
}, "doc/GENIT_PKG.txt", {})
