globals
endglobals

function main takes nothing returns nothing
endfunction

function config takes nothing returns nothing
    call SetMapName("TRIGSTR_004")
    call SetMapDescription("TRIGSTR_006")
    call SetPlayers(2)
    call SetTeams(2)
    call SetGamePlacement(MAP_PLACEMENT_TEAMS_TOGETHER)
    call DefineStartLocation(0, 2752.0, -3008.0)
    call DefineStartLocation(1, 2112.0, -2240.0)
    call InitCustomPlayerSlots()
    call SetPlayerSlotAvailable(Player(0), MAP_CONTROL_USER)
    call SetPlayerSlotAvailable(Player(1), MAP_CONTROL_USER)
    call InitGenericPlayerSlots()
    call InitAllyPriorities()
endfunction
