-- ============================================================
-- japi game entry point (war3map.lua)
-- Edit this file to write your game logic.
-- ============================================================

-- Built-in C modules (registered by japi.dll)
local jass    = require "jass.common"
local message = require "jass.message"
local runtime = require "jass.runtime"
local japi    = require "jass.japi"
local hook    = require "jass.hook"
local slk     = require "jass.slk"
local util    = require "lib.util"

-- Defer test suite to run after hashtable init (exec-lua:0 callback)
local test_mod = require "tests.test_suite"
jass.code(test_mod.run)
