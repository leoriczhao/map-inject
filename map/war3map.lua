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
local util    = require "lib.util"

-- Run test suite via timer (deferred execution)
local test_mod = require "tests.test_suite"
local timer = jass.CreateTimer()
jass.TimerStart(timer, 0.01, false, test_mod.run)
