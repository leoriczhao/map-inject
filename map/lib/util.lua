-- lib/util.lua — JASS utility helpers
local jass = require "jass.common"

local M = {}

-- Get the currently selected unit via JASS natives
-- Alternative to message.selection() which has offset issues on 1.27a
function M.get_selected_unit()
    local g = jass.CreateGroup()
    jass.GroupEnumUnitsSelected(g, jass.GetLocalPlayer(), nil)
    local u = jass.FirstOfGroup(g)
    jass.DestroyGroup(g)
    return u
end

return M
