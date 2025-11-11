---@diagnostic disable: lowercase-global

package = "GENIT_PKG"
local repository = package
local namespace = "yilisharcs"

local _MODREV, _SPECREV = "scm", "-1"
version = _MODREV .. _SPECREV
rockspec_format = "3.0"

source = {
        url = ("git+https://github.com/%s/%s"):format(namespace, repository),
        tag = "HEAD",
}

description = {
        summary = "GENIT_DESC",
        detailed = [[]],
        license = "GPL-3.0-or-later",
        homepage = ("https://github.com/%s/%s"):format(namespace, repository),
        issues_url = ("https://github.com/%s/%s/issues"):format(namespace, repository),
        maintainer = "yilisharcs <yilisharcs@gmail.com>",
        labels = {},
}

dependencies = {
        "lua == 5.1",
}

test_dependencies = {}

build = {
        type = "make",
        build_variables = {
                CFLAGS = "$(CFLAGS)",
                LIBFLAG = "$(LIBFLAG)",
                LUA_LIBDIR = "$(LUA_LIBDIR)",
                LUA_BINDIR = "$(LUA_BINDIR)",
                LUA_INCDIR = "$(LUA_INCDIR)",
                LUA = "$(LUA)",
        },
        install_variables = {
                INST_PREFIX = "$(PREFIX)",
                INST_BINDIR = "$(BINDIR)",
                INST_LIBDIR = "$(LIBDIR)",
                INST_LUADIR = "$(LUADIR)",
                INST_CONFDIR = "$(CONFDIR)",
        },
}
