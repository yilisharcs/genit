#!/usr/bin/env lua5.1

assert(loadfile("GENIT_PKG-scm-1.rockspec"))()

local root = debug.getinfo(1, "S").source:match("@?(.*)/[^/]+/[^/]+$")
local mansection = 1

local cmd = ("./scripts/manpage.nu --root %s --ver %s --name %s --bin %s --mansection %s"):format(
        root,
        ---@diagnostic disable-next-line: undefined-global
        version,
        ---@diagnostic disable-next-line: param-type-mismatch
        string.gsub(package, "^(.)", string.upper),
        package,
        mansection
)

os.execute(cmd)
