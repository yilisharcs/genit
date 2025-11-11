if vim.g.loaded_GENIT_NVIM == 1 then return end
vim.g.loaded_GENIT_NVIM = 1

---@toc_entry CONFIGURATION
---@tag GENIT_NVIM-configuration
---@class GENIT_NVIM.Config
---
---@usage >lua
---     vim.g.GENIT_NVIM = {
---             --
---     }
--- <
local DEFAULTS = {}

---@type GENIT_NVIM.Config
vim.g.GENIT_NVIM = vim.tbl_deep_extend("force", DEFAULTS, vim.g.GENIT_NVIM or {})
