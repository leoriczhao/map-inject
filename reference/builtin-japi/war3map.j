globals
//globals from JapiConstantLib:
constant boolean LIBRARY_JapiConstantLib=true
integer array i_1
integer array i_2
integer array i_3
integer array i_4
integer array i_5
integer array i_6
integer array i_7
integer array i_8
integer array i_9
integer array i_10
integer array i_11
integer array i_12
integer array i_13
integer array i_14
integer array i_15
integer array i_16
integer array i_17
integer array i_18
integer array i_19
integer array i_20
integer array i_21
integer array i_22
integer array i_23
integer array i_24
integer array i_25
integer array i_26
integer array i_27
integer array i_28
integer array i_29
integer array i_30
integer array i_31
integer array i_32

//endglobals from JapiConstantLib
//globals from japi:
constant boolean LIBRARY_japi=true
hashtable japi_ht=InitHashtable()
integer japi__key=StringHash("jass")
//endglobals from japi
//globals from LocalActionLib:
constant boolean LIBRARY_LocalActionLib=true
constant hashtable LocalActionLib___ht=japi_ht
constant integer LocalActionLib___key=StringHash("jass")
//endglobals from LocalActionLib
//globals from d3d:
constant boolean LIBRARY_d3d=true
constant hashtable d3d__ht=japi_ht
constant integer d3d__key=StringHash("jass")
//endglobals from d3d
//globals from text:
constant boolean LIBRARY_text=true
integer font
string imp="ReplaceableTextures\\CommandButtons\\BTNStormBolt.blp"
//endglobals from text
//globals from texture:
constant boolean LIBRARY_texture=true
integer mouse_texture
timer mouse_timer=CreateTimer()
//endglobals from texture
    // User-defined
unit udg_unit= null
item udg_item= null
destructable udg_destructable= null
    // Generated
trigger gg_trg_japi_________u= null
trigger gg_trg____japi___u= null
trigger gg_trg____d3d___u= null
trigger gg_trg_______japi___u= null
trigger gg_trg___________________u= null
trigger gg_trg_d3d_____________u= null
trigger gg_trg_d3d_____________mouse= null
trigger gg_trg_lua____________u= null
trigger gg_trg_______lua______u= null
trigger gg_trg__________________________u= null
trigger gg_trg_start= null
trigger gg_trg_main_lua= null
trigger gg_trg________________1= null
trigger gg_trg______________________A= null
trigger gg_trg______________________B= null
unit gg_unit_Hamg_0025= null
unit gg_unit_Hblm_0002= null

trigger l__library_init

//JASSHelper struct globals:
constant integer si__LOGFONT=1
integer si__LOGFONT_F=0
integer si__LOGFONT_I=0
integer array si__LOGFONT_V
integer array s__LOGFONT_font
trigger st__LOGFONT_onDestroy
integer f__arg_this

endglobals
    native SetHeroLevels takes code f returns nothing 
    native TeleportCaptain takes real x, real y returns nothing
    native GetUnitGoldCost takes integer unitid returns integer


//Generated method caller for LOGFONT.onDestroy
function sc__LOGFONT_onDestroy takes integer this returns nothing
    set f__arg_this=this
    call TriggerEvaluate(st__LOGFONT_onDestroy)
endfunction

//Generated allocator of LOGFONT
function s__LOGFONT__allocate takes nothing returns integer
 local integer this=si__LOGFONT_F
    if (this!=0) then
        set si__LOGFONT_F=si__LOGFONT_V[this]
    else
        set si__LOGFONT_I=si__LOGFONT_I+1
        set this=si__LOGFONT_I
    endif
    if (this>8190) then
        return 0
    endif

    set si__LOGFONT_V[this]=-1
 return this
endfunction

//Generated destructor of LOGFONT
function sc__LOGFONT_deallocate takes integer this returns nothing
    if this==null then
        return
    elseif (si__LOGFONT_V[this]!=-1) then
        return
    endif
    set f__arg_this=this
    call TriggerEvaluate(st__LOGFONT_onDestroy)
    set si__LOGFONT_V[this]=si__LOGFONT_F
    set si__LOGFONT_F=this
endfunction

//library JapiConstantLib:
    function JapiConstantLib_init_memory takes nothing returns nothing
         set i_1[8191]=0
 set i_2[8191]=0
 set i_3[8191]=0
 set i_4[8191]=0
 set i_5[8191]=0
 set i_6[8191]=0
 set i_7[8191]=0
 set i_8[8191]=0
 set i_9[8191]=0
 set i_10[8191]=0
 set i_11[8191]=0
 set i_12[8191]=0
 set i_13[8191]=0
 set i_14[8191]=0
 set i_15[8191]=0
 set i_16[8191]=0
 set i_17[8191]=0
 set i_18[8191]=0
 set i_19[8191]=0
 set i_20[8191]=0
 set i_21[8191]=0
 set i_22[8191]=0
 set i_23[8191]=0
 set i_24[8191]=0
 set i_25[8191]=0
 set i_26[8191]=0
 set i_27[8191]=0
 set i_28[8191]=0
 set i_29[8191]=0
 set i_30[8191]=0
 set i_31[8191]=0
 set i_32[8191]=0

    endfunction
    function JapiConstantLib_init takes nothing returns integer
        call ExecuteFunc("JapiConstantLib_init_memory")
        return 1
    endfunction

//library JapiConstantLib ends
//library japi:



    
    
     function Call takes string str returns nothing
        call UnitId(str)
    endfunction
    //获取鼠标在地图中的x轴
     function GetMouseX takes nothing returns real
        call SaveStr(japi_ht, japi__key, 0, "()R")
        call UnitId(("GetMouseX")) // INLINED!!
        return LoadReal(japi_ht, japi__key, 0)
    endfunction
    //获取鼠标在地图中的y轴
     function GetMouseY takes nothing returns real
        call SaveStr(japi_ht, japi__key, 0, "()R")
        call UnitId(("GetMouseY")) // INLINED!!
        return LoadReal(japi_ht, japi__key, 0)
    endfunction
    
    
    
    
    //==========================================================================
     function EXGetUnitAbility takes unit u,integer abilityId returns integer
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(u))
        call SaveInteger(japi_ht, japi__key, 2, abilityId)
        call SaveStr(japi_ht, japi__key, 0, "(II)I")
        call UnitId(("EXGetUnitAbility")) // INLINED!!
        return LoadInteger(japi_ht, japi__key, 0)
    endfunction
    
    // yd japi ==================================================================
    // 技能----------------------------------------------------
    
    ///<summary>技能属性 [JAPI]</summary>
  function YDWEGetUnitAbilityState takes unit u,integer abilcode,integer data_type returns real
        call SaveInteger(japi_ht, japi__key, 1, EXGetUnitAbility(u , abilcode))
        call SaveInteger(japi_ht, japi__key, 2, data_type)
        call SaveStr(japi_ht, japi__key, 0, "(II)R")
        call UnitId(("EXGetAbilityState")) // INLINED!!
		return LoadReal(japi_ht, japi__key, 0)
	endfunction
	///<summary>技能数据 (整数) [JAPI]</summary>
  function YDWEGetUnitAbilityDataInteger takes unit u,integer abilcode,integer level,integer data_type returns integer
        call SaveInteger(japi_ht, japi__key, 1, EXGetUnitAbility(u , abilcode))
        call SaveInteger(japi_ht, japi__key, 2, level)
        call SaveInteger(japi_ht, japi__key, 3, data_type)
        call SaveStr(japi_ht, japi__key, 0, "(III)I")
        call UnitId(("EXGetAbilityDataInteger")) // INLINED!!
		return LoadInteger(japi_ht, japi__key, 0)
	endfunction
	///<summary>技能数据 (实数) [JAPI]</summary>
  function YDWEGetUnitAbilityDataReal takes unit u,integer abilcode,integer level,integer data_type returns real
        call SaveInteger(japi_ht, japi__key, 1, EXGetUnitAbility(u , abilcode))
        call SaveInteger(japi_ht, japi__key, 2, level)
        call SaveInteger(japi_ht, japi__key, 3, data_type)
        call SaveStr(japi_ht, japi__key, 0, "(III)R")
        call UnitId(("EXGetAbilityDataReal")) // INLINED!!
		return LoadReal(japi_ht, japi__key, 0)
    endfunction
	///<summary>技能数据 (字符串) [JAPI]</summary>
  function YDWEGetUnitAbilityDataString takes unit u,integer abilcode,integer level,integer data_type returns string
        call SaveInteger(japi_ht, japi__key, 1, EXGetUnitAbility(u , abilcode))
        call SaveInteger(japi_ht, japi__key, 2, level)
        call SaveInteger(japi_ht, japi__key, 3, data_type)
        call SaveStr(japi_ht, japi__key, 0, "(III)S")
        call UnitId(("EXGetAbilityDataString")) // INLINED!!
		return LoadStr(japi_ht, japi__key, 0)
	endfunction
	///<summary>设置技能属性 [JAPI]</summary>
  function YDWESetUnitAbilityState takes unit u,integer abilcode,integer data_type,real value returns nothing
        call SaveInteger(japi_ht, japi__key, 1, EXGetUnitAbility(u , abilcode))
        call SaveInteger(japi_ht, japi__key, 2, data_type)
        call SaveReal(japi_ht, japi__key, 3, value)
        call SaveStr(japi_ht, japi__key, 0, "(IIR)V")
        call UnitId(("EXSetAbilityState")) // INLINED!!
    endfunction
	///<summary>设置技能数据 (整数) [JAPI]</summary>
  function YDWESetUnitAbilityDataInteger takes unit u,integer abilcode,integer level,integer data_type,integer value returns nothing
        call SaveInteger(japi_ht, japi__key, 1, EXGetUnitAbility(u , abilcode))
        call SaveInteger(japi_ht, japi__key, 2, level)
        call SaveInteger(japi_ht, japi__key, 3, data_type)
        call SaveInteger(japi_ht, japi__key, 4, value)
        call SaveStr(japi_ht, japi__key, 0, "(IIII)V")
        call UnitId(("EXSetAbilityDataInteger")) // INLINED!!
    endfunction
	///<summary>设置技能数据 (实数) [JAPI]</summary>
  function YDWESetUnitAbilityDataReal takes unit u,integer abilcode,integer level,integer data_type,real value returns nothing
        call SaveInteger(japi_ht, japi__key, 1, EXGetUnitAbility(u , abilcode))
        call SaveInteger(japi_ht, japi__key, 2, level)
        call SaveInteger(japi_ht, japi__key, 3, data_type)
        call SaveReal(japi_ht, japi__key, 4, value)
        call SaveStr(japi_ht, japi__key, 0, "(IIIR)V")
        call UnitId(("EXSetAbilityDataReal")) // INLINED!!
    endfunction
	///<summary>设置技能数据 (字符串) [JAPI]</summary>
  function YDWESetUnitAbilityDataString takes unit u,integer abilcode,integer level,integer data_type,string value returns nothing
        call SaveInteger(japi_ht, japi__key, 1, EXGetUnitAbility(u , abilcode))
        call SaveInteger(japi_ht, japi__key, 2, level)
        call SaveInteger(japi_ht, japi__key, 3, data_type)
        call SaveStr(japi_ht, japi__key, 4, value)
        call SaveStr(japi_ht, japi__key, 0, "(IIIS)V")
        call UnitId(("EXSetAbilityDataString")) // INLINED!!
    endfunction
	
    
    //设置技能变身数据A
     function EXSetAbilityAEmeDataA takes integer ability_handle,integer value returns boolean
        call SaveInteger(japi_ht, japi__key, 1, ability_handle)
        call SaveInteger(japi_ht, japi__key, 2, value)
        call SaveStr(japi_ht, japi__key, 0, "(II)B")
        call UnitId(("EXSetAbilityAEmeDataA")) // INLINED!!
        return LoadBoolean(japi_ht, japi__key, 0)
    endfunction
    
    //单位变身
     function YDWEUnitTransform takes unit u,integer abilcode,integer targetid returns nothing
		call UnitAddAbility(u, abilcode)
		call YDWESetUnitAbilityDataInteger(u , abilcode , 1 , 117 , GetUnitTypeId(u))
		call EXSetAbilityAEmeDataA(EXGetUnitAbility(u , abilcode) , GetUnitTypeId(u))
		call UnitRemoveAbility(u, abilcode)
		call UnitAddAbility(u, abilcode)
		call EXSetAbilityAEmeDataA(EXGetUnitAbility(u , abilcode) , targetid)
		call UnitRemoveAbility(u, abilcode)
	endfunction
    
    // 单位-------------------------------------------------------
    
    //暂停单位
     function EXPauseUnit takes unit unit_handle,boolean flag returns nothing
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(unit_handle))
        call SaveBoolean(japi_ht, japi__key, 2, flag)
        call SaveStr(japi_ht, japi__key, 0, "(IB)V")
        call UnitId(("EXPauseUnit")) // INLINED!!
    endfunction
    
    //获取单位字符串
     function EXGetUnitString takes integer unitcode,integer Type returns string
        call SaveInteger(japi_ht, japi__key, 1, unitcode)
        call SaveInteger(japi_ht, japi__key, 2, Type)
        call SaveStr(japi_ht, japi__key, 0, "(II)S")
        call UnitId(("EXGetUnitString")) // INLINED!!
        return LoadStr(japi_ht, japi__key, 0)
    endfunction
    
       //设置单位字符串
     function EXSetUnitString takes integer unitcode,integer Type,string value returns boolean
        call SaveInteger(japi_ht, japi__key, 1, unitcode)
        call SaveInteger(japi_ht, japi__key, 2, Type)
        call SaveStr(japi_ht, japi__key, 3, value)
        call SaveStr(japi_ht, japi__key, 0, "(IIS)B")
        call UnitId(("EXSetUnitString")) // INLINED!!
        return LoadBoolean(japi_ht, japi__key, 0)
    endfunction
    
    //获取单位实数
     function EXGetUnitReal takes integer unitcode,integer Type returns real
        call SaveInteger(japi_ht, japi__key, 1, unitcode)
        call SaveInteger(japi_ht, japi__key, 2, Type)
        call SaveStr(japi_ht, japi__key, 0, "(II)R")
        call UnitId(("EXGetUnitReal")) // INLINED!!
        return LoadReal(japi_ht, japi__key, 0)
    endfunction
    
    //设置单位实数
     function EXSetUnitReal takes integer unitcode,integer Type,real value returns boolean
        call SaveInteger(japi_ht, japi__key, 1, unitcode)
        call SaveInteger(japi_ht, japi__key, 2, Type)
        call SaveReal(japi_ht, japi__key, 3, value)
        call SaveStr(japi_ht, japi__key, 0, "(IIR)B")
        call UnitId(("EXSetUnitReal")) // INLINED!!
        return LoadBoolean(japi_ht, japi__key, 0)
    endfunction
    
    
    //获取单位整数
     function EXGetUnitInteger takes integer unitcode,integer Type returns integer
        call SaveInteger(japi_ht, japi__key, 1, unitcode)
        call SaveInteger(japi_ht, japi__key, 2, Type)
        call SaveStr(japi_ht, japi__key, 0, "(II)I")
        call UnitId(("EXGetUnitInteger")) // INLINED!!
        return LoadInteger(japi_ht, japi__key, 0)
    endfunction
    
    //设置单位整数
     function EXSetUnitInteger takes integer unitcode,integer Type,integer value returns boolean
        call SaveInteger(japi_ht, japi__key, 1, unitcode)
        call SaveInteger(japi_ht, japi__key, 2, Type)
        call SaveInteger(japi_ht, japi__key, 3, value)
        call SaveStr(japi_ht, japi__key, 0, "(III)B")
        call UnitId(("EXSetUnitInteger")) // INLINED!!
        return LoadBoolean(japi_ht, japi__key, 0)
    endfunction
    
        //获取单位数组字符串
     function EXGetUnitArrayString takes integer unitcode,integer Type,integer index returns string
        call SaveInteger(japi_ht, japi__key, 1, unitcode)
        call SaveInteger(japi_ht, japi__key, 2, Type)
        call SaveInteger(japi_ht, japi__key, 3, index)
        call SaveStr(japi_ht, japi__key, 0, "(III)S")
        call UnitId(("EXGetUnitArrayString")) // INLINED!!
        return LoadStr(japi_ht, japi__key, 0)
    endfunction
    
    //设置单位数组字符串
     function EXSetUnitArrayString takes integer unitcode,integer Type,integer index,string value returns boolean
        call SaveInteger(japi_ht, japi__key, 1, unitcode)
        call SaveInteger(japi_ht, japi__key, 2, Type)
        call SaveInteger(japi_ht, japi__key, 3, index)
        call SaveStr(japi_ht, japi__key, 4, value)
        call SaveStr(japi_ht, japi__key, 0, "(IIIS)B")
        call UnitId(("EXSetUnitArrayString")) // INLINED!!
        return LoadBoolean(japi_ht, japi__key, 0)
    endfunction
    //设置单位面向角度(立即转向)
     function EXSetUnitFacing takes unit unit_handle,real angle returns nothing
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(unit_handle))
        call SaveReal(japi_ht, japi__key, 2, angle)
        call SaveStr(japi_ht, japi__key, 0, "(IR)V")
        call UnitId(("EXSetUnitFacing")) // INLINED!!
    endfunction
    
    //设置单位碰撞类型
     function EXSetUnitCollisionType takes boolean enable,unit unit_handle,integer Type returns nothing
        call SaveBoolean(japi_ht, japi__key, 1, enable)
        call SaveInteger(japi_ht, japi__key, 2, GetHandleId(unit_handle))
        call SaveInteger(japi_ht, japi__key, 3, Type)
        call SaveStr(japi_ht, japi__key, 0, "(BII)V")
        call UnitId(("EXSetUnitCollisionType")) // INLINED!!
    endfunction
    
    //设置单位移动类型
     function EXSetUnitMoveType takes unit unit_handle,integer Type returns nothing
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(unit_handle))
        call SaveInteger(japi_ht, japi__key, 2, Type)
        call SaveStr(japi_ht, japi__key, 0, "(II)V")
        call UnitId(("EXSetUnitMoveType")) // INLINED!!
    endfunction
    
    //单位添加眩晕
     function YDWEUnitAddStun takes unit u returns nothing
		call EXPauseUnit(u , true)
	endfunction
    
    //单位删除眩晕
  function YDWEUnitRemoveStun takes unit u returns nothing
		call EXPauseUnit(u , false)
	endfunction
    
    //获取伤害数据
     function EXGetEventDamageData takes integer Type returns integer
        //call SaveInteger(ht,key,1,Type)
        //call SaveStr(ht,key,0,"(I)I")
        //call Call("EXGetEventDamageData")
        //return LoadInteger(ht,key,0)
        return GetUnitGoldCost(Type)
    endfunction
	
    //设置伤害
     function EXSetEventDamage takes real Damage returns boolean
        //call SaveReal(ht,key,1,Damage)
        //call SaveStr(ht,key,0,"(R)B")
        //call Call("EXSetEventDamage")
        //return LoadBoolean(ht,key,0)
        call TeleportCaptain(Damage, 0.00)
        return true
    endfunction
    
    //判断是否是物理伤害
     function YDWEIsEventPhysicalDamage takes nothing returns boolean
		return 0 != (GetUnitGoldCost((1))) // INLINED!!
	endfunction
    //判断是否是攻击伤害
  function YDWEIsEventAttackDamage takes nothing returns boolean
		return 0 != (GetUnitGoldCost((2))) // INLINED!!
	endfunction
	
    //判断是否是范围伤害
  function YDWEIsEventRangedDamage takes nothing returns boolean
		return 0 != (GetUnitGoldCost((3))) // INLINED!!
	endfunction
	
    //判断伤害类型
  function YDWEIsEventDamageType takes damagetype damageType returns boolean
		return damageType == ConvertDamageType((GetUnitGoldCost((4)))) // INLINED!!
	endfunction
    
    //判断武器类型
  function YDWEIsEventWeaponType takes weapontype weaponType returns boolean
		return weaponType == ConvertWeaponType((GetUnitGoldCost((5)))) // INLINED!!
	endfunction
	
    //判断攻击类型
  function YDWEIsEventAttackType takes attacktype attackType returns boolean
		return attackType == ConvertAttackType((GetUnitGoldCost((6)))) // INLINED!!
	endfunction
	//设置伤害
  function YDWESetEventDamage takes real amount returns boolean
		return EXSetEventDamage(amount)
	endfunction
    
    // 物品----------------------------------------------------
    
    ///<summary>设置物品数据 (字符串) [JAPI]</summary>
     function YDWESetItemDataString takes integer ItemTypeId,integer Type,string Value returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(IIS)V")
        call SaveInteger(japi_ht, japi__key, 1, ItemTypeId)
        call SaveInteger(japi_ht, japi__key, 2, Type)
        call SaveStr(japi_ht, japi__key, 3, Value)
        call UnitId(("EXSetItemDataString")) // INLINED!!
    endfunction
    ///<summary>获取物品数据 (字符串) [JAPI]</summary>
     function YDWEGetItemDataString takes integer ItemTypeId,integer Type returns string
        call SaveStr(japi_ht, japi__key, 0, "(II)S")
        call SaveInteger(japi_ht, japi__key, 1, ItemTypeId)
        call SaveInteger(japi_ht, japi__key, 2, Type)
        call UnitId(("EXGetItemDataString")) // INLINED!!
        return LoadStr(japi_ht, japi__key, 0)
    endfunction
    
    //特效--------------------------------------------------------
    
    ///<summary>设置特效坐标 [JAPI]</summary>
     function EXSetEffectXY takes effect Handle,real x,real y returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(IRR)V")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call SaveReal(japi_ht, japi__key, 2, x)
        call SaveReal(japi_ht, japi__key, 3, y)
        call UnitId(("EXSetEffectXY")) // INLINED!!
    endfunction
    
    ///<summary>设置特效Z轴 [JAPI]</summary>
     function EXSetEffectZ takes effect Handle,real z returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(IRR)V")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call SaveReal(japi_ht, japi__key, 2, z)
		call UnitId(("EXSetEffectZ")) // INLINED!!
	endfunction
    
    ///<summary>获取特效X轴 [JAPI]</summary>
     function EXGetEffectX takes effect Handle returns real
        call SaveStr(japi_ht, japi__key, 0, "(I)R")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call UnitId(("EXGetEffectX")) // INLINED!!
        return LoadReal(japi_ht, japi__key, 0)
	endfunction
    
    ///<summary>获取特效Y轴 [JAPI]</summary>
  function EXGetEffectY takes effect Handle returns real
        call SaveStr(japi_ht, japi__key, 0, "(I)R")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call UnitId(("EXGetEffectY")) // INLINED!!
        return LoadReal(japi_ht, japi__key, 0)
	endfunction
    
    ///<summary>获取特效Z轴 [JAPI]</summary>
  function EXGetEffectZ takes effect Handle returns real
        call SaveStr(japi_ht, japi__key, 0, "(I)R")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call UnitId(("EXGetEffectZ")) // INLINED!!
		return LoadReal(japi_ht, japi__key, 0)
	endfunction
    
    ///<summary>设置特效尺寸 [JAPI]</summary>
  function EXSetEffectSize takes effect Handle,real size returns nothing
		call SaveStr(japi_ht, japi__key, 0, "(IR)V")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call SaveReal(japi_ht, japi__key, 2, size)
        call UnitId(("EXSetEffectSize")) // INLINED!!
	endfunction
    
    ///<summary>获取特效尺寸 [JAPI]</summary>
  function EXGetEffectSize takes effect Handle returns real
        call SaveStr(japi_ht, japi__key, 0, "(I)R")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call UnitId(("EXGetEffectSize")) // INLINED!!
		return LoadReal(japi_ht, japi__key, 0)
	endfunction
    
    ///<summary>设置特效X旋转轴 [JAPI]</summary>
  function EXEffectMatRotateX takes effect Handle,real x returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(IR)V")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call SaveReal(japi_ht, japi__key, 2, x)
        call UnitId(("EXEffectMatRotateX")) // INLINED!!
	endfunction
    
    ///<summary>设置特效Y旋转轴 [JAPI]</summary>
  function EXEffectMatRotateY takes effect Handle,real y returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(IR)V")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call SaveReal(japi_ht, japi__key, 2, y)
        call UnitId(("EXEffectMatRotateY")) // INLINED!!
	endfunction
    
    ///<summary>设置特效Z旋转轴 [JAPI]</summary>
  function EXEffectMatRotateZ takes effect Handle,real z returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(IR)V")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call SaveReal(japi_ht, japi__key, 2, z)
        call UnitId(("EXEffectMatRotateZ")) // INLINED!!
	endfunction
    
    ///<summary>设置特效比例 [JAPI]</summary>
  function EXEffectMatScale takes effect Handle,real x,real y,real z returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(IRRR)V")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call SaveReal(japi_ht, japi__key, 2, x)
        call SaveReal(japi_ht, japi__key, 3, y)
        call SaveReal(japi_ht, japi__key, 4, z)
        call UnitId(("EXEffectMatScale")) // INLINED!!
	endfunction
    
    ///<summary>设置特效重置 [JAPI]</summary>
  function EXEffectMatReset takes effect Handle returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(I)V")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call UnitId(("EXEffectMatReset")) // INLINED!!
	endfunction
    
    ///<summary>设置特效速率 [JAPI]</summary>
  function EXSetEffectSpeed takes effect Handle,real speed returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(IR)V")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call SaveReal(japi_ht, japi__key, 2, speed)
        call UnitId(("EXSetEffectSpeed")) // INLINED!!
	endfunction
    
    ///<summary>设置可追踪物坐标 [JAPI]</summary>
     function EXSetTrackableXY takes trackable Handle,real x,real y returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(IRR)V")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call SaveReal(japi_ht, japi__key, 2, x)
        call SaveReal(japi_ht, japi__key, 3, y)
        call UnitId(("EXSetEffectXY")) // INLINED!!
    endfunction
    
    
    ///<summary>获取可追踪物X轴 [JAPI]</summary>
     function EXGetTrackableX takes trackable Handle returns real
        call SaveStr(japi_ht, japi__key, 0, "(I)R")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call UnitId(("EXGetEffectX")) // INLINED!!
        return LoadReal(japi_ht, japi__key, 0)
	endfunction
    
    ///<summary>获取可追踪物Y轴 [JAPI]</summary>
  function EXGetTrackableY takes trackable Handle returns real
        call SaveStr(japi_ht, japi__key, 0, "(I)R")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(Handle))
        call UnitId(("EXGetEffectY")) // INLINED!!
        return LoadReal(japi_ht, japi__key, 0)
	endfunction
    
    
    
     function EXExecuteScript takes string str returns string
        call SaveStr(japi_ht, japi__key, 0, "(S)S")
        call SaveStr(japi_ht, japi__key, 1, str)
        call UnitId(("EXExecuteScript")) // INLINED!!
        return LoadStr(japi_ht, japi__key, 0)
    endfunction
    //-----------------模拟聊天----------------------------
     function EXDisplayChat takes player p,integer chat_recipient,string message returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(IIS)V")
        call SaveInteger(japi_ht, japi__key, 1, GetHandleId(p))
        call SaveInteger(japi_ht, japi__key, 2, chat_recipient)
        call SaveStr(japi_ht, japi__key, 3, message)
        call UnitId(("EXDisplayChat")) // INLINED!!
    endfunction
  function YDWEDisplayChat takes player p,integer chat_recipient,string message returns nothing
		call EXDisplayChat(p , chat_recipient , message)
	endfunction
    
    //-----------版本描述-------------------------------------
    
    //获取地图名字
     function GetMapName takes nothing returns string
        call SaveStr(japi_ht, japi__key, 0, "()S")
        call UnitId(("GetMapName")) // INLINED!!
        return LoadStr(japi_ht, japi__key, 0)
    endfunction
    
    //获取魔兽版本
     function GetGameVersion takes nothing returns integer
        call SaveStr(japi_ht, japi__key, 0, "()I")
        call UnitId(("GetGameVersion")) // INLINED!!
        return LoadInteger(japi_ht, japi__key, 0)
    endfunction
    
    //获取插件版本
     function GetPluginVersion takes nothing returns string
        call SaveStr(japi_ht, japi__key, 0, "()S")
        call UnitId(("GetPluginVersion")) // INLINED!!
        return LoadStr(japi_ht, japi__key, 0)
    endfunction
    
     function GetFuncAddr takes code f returns integer
        call SetHeroLevels(f)
        return LoadInteger(japi_ht, japi__key, 0)
    endfunction
     function japiDoNothing takes nothing returns nothing
        
    endfunction
    
     function func_bind_trigger_name takes code functions,string name returns nothing
        call SaveStr(japi_ht, japi__key, 0, "(IS)V")
        call SaveInteger(japi_ht, japi__key, 1, GetFuncAddr(functions))
        call SaveStr(japi_ht, japi__key, 2, name)
        call UnitId(("func_bind_trigger_name")) // INLINED!!
    endfunction
    
     function open_code_run_logs takes boolean open returns nothing
        local string l=""
        set l=l + "(function () "
        set l=l + "lfunc={}  lfunc_name={}"
        set l=l + "save_lfunc_info=function (func,name,index)index=1<<index lfunc[func]=index lfunc_name[index]=name end "
        set l=l + "save_lfunc_info('GetLocalPlayer','[本地玩家]',0)"
        set l=l + "save_lfunc_info('GetFps','[获取帧数]',1)"
        set l=l + "save_lfunc_info('GetChatState','[聊天状态]',2)"
        set l=l + "save_lfunc_info('GetCameraTargetPositionLoc','[当前镜头目标点]',3)"
        set l=l + "save_lfunc_info('GetCameraTargetPositionX','[当前镜头目标X坐标]',4)"
        set l=l + "save_lfunc_info('GetCameraTargetPositionY','[当前镜头目标Y坐标]',5)"
        set l=l + "save_lfunc_info('GetCameraTargetPositionZ','[当前镜头目标Z坐标]',6)"
        
        set l=l + "save_lfunc_info('GetCameraEyePositionLoc','[当前镜头源位置]',7)"
        set l=l + "save_lfunc_info('GetCameraEyePositionX','[当前镜头源X坐标]',8)"
        set l=l + "save_lfunc_info('GetCameraEyePositionY','[当前镜头源Y坐标]',9)"
        set l=l + "save_lfunc_info('GetCameraEyePositionZ','[当前镜头源Z坐标]',10)"
        
        set l=l + "save_lfunc_info('GetMouseX','[获取鼠标X轴]',11)"
        set l=l + "save_lfunc_info('GetMouseY','[获取鼠标Y轴]',12)"
        set l=l + "save_lfunc_info('GetMouseVectorX','[获取鼠标屏幕X轴]',13)"
        set l=l + "save_lfunc_info('GetMouseVectorY','[获取鼠标屏幕Y轴]',14)"
        
        set l=l + "end)() or '' "
        call EXExecuteScript(l)
        
        set l=""
        
        set l=l + "(function () "
        
        set l=l + "get_jass_func_info=function (func_name) "
        set l=l + " return lfunc[func_name] or 0 "
        set l=l + "end "
        
        set l=l + "get_jass_func_msg=function (func_name_index)"
        set l=l + " return lfunc_name[func_name_index] or '' "
        set l=l + "end "
        
        set l=l + "local storm=require 'jass.storm' "
        set l=l + "local ss=storm.load('war3map.j') "
        set l=l + "ss:gsub('function%s+([%w_]+)%s+takes(.-)endfunction',function (name,code)\n"
        set l=l + "code=code:gsub('function%s+','function_'):gsub('//[^\\n]-\\n','')\n"
        set l=l + "code:gsub('([%w_]+)',function (str) "
        set l=l + "if lfunc[str]~=nil then "
        set l=l + "local flag=lfunc[name] or 0 "
        set l=l + "lfunc[name]=flag | lfunc[str] "
        set l=l + "end "
        set l=l + "end) "
        set l=l + "end) "
        set l=l + "ss=nil  return '' "
        set l=l + "end)() or '' "
        call EXExecuteScript(l)
        call SaveStr(japi_ht, japi__key, 0, "(B)V")
        call SaveBoolean(japi_ht, japi__key, 1, open)
        call UnitId(("open_code_run_logs")) // INLINED!!
    endfunction
    
    
    
     function initializePlugin takes nothing returns integer
        call ExecuteFunc("japiDoNothing")
        call StartCampaignAI(Player(PLAYER_NEUTRAL_AGGRESSIVE), "callback")
        call UnitId((I2S(GetHandleId(japi_ht)))) // INLINED!!
        call SaveStr(japi_ht, japi__key, 0, "(I)V")
        call SaveInteger(japi_ht, japi__key, 1, GetFuncAddr(function japiDoNothing))
        call UnitId(("SaveFunc")) // INLINED!!
        call ExecuteFunc("japiDoNothing")
        return 0
    endfunction

//library japi ends
//library LocalActionLib:
    function LocalActionLib___Call takes string str returns nothing
        call UnitId(str)
    endfunction
    
    //本地发布无目标命令
    function LocalOrder takes integer order,integer flags returns nothing
        call SaveStr(LocalActionLib___ht, LocalActionLib___key, 0, "(II)V")
        call SaveInteger(LocalActionLib___ht, LocalActionLib___key, 1, order)
        call SaveInteger(LocalActionLib___ht, LocalActionLib___key, 2, flags)
        call UnitId(("LocalOrder")) // INLINED!!
    endfunction
    
    //本地发布坐标命令
    function LocalPointOrder takes integer order,real x,real y,integer flags returns nothing
        call SaveStr(LocalActionLib___ht, LocalActionLib___key, 0, "(IRRI)V")
        call SaveInteger(LocalActionLib___ht, LocalActionLib___key, 1, order)
        call SaveReal(LocalActionLib___ht, LocalActionLib___key, 2, x)
        call SaveReal(LocalActionLib___ht, LocalActionLib___key, 3, y)
        call SaveInteger(LocalActionLib___ht, LocalActionLib___key, 4, flags)
        call UnitId(("LocalPointOrder")) // INLINED!!
    endfunction
    
    //本地发布目标命令
    function LocalTargetOrder takes integer order,widget object,integer flags returns nothing
        call SaveStr(LocalActionLib___ht, LocalActionLib___key, 0, "(IHwidget;I)V")
        call SaveInteger(LocalActionLib___ht, LocalActionLib___key, 1, order)
        call SaveWidgetHandle(LocalActionLib___ht, LocalActionLib___key, 2, object)
        call SaveInteger(LocalActionLib___ht, LocalActionLib___key, 3, flags)
        call UnitId(("LocalTargetOrder")) // INLINED!!
    endfunction
    
    //获取玩家当前选择的单位
    function GetPlayerSelectedUnit takes player p returns unit
        call SaveStr(LocalActionLib___ht, LocalActionLib___key, 0, "(I)Hunit;")
        call RemoveSavedHandle(LocalActionLib___ht, LocalActionLib___key, 0)
        call SaveInteger(LocalActionLib___ht, LocalActionLib___key, 1, GetHandleId(p))
        call UnitId(("GetPlayerSelectedUnit")) // INLINED!!
        return LoadUnitHandle(LocalActionLib___ht, LocalActionLib___key, 0)
    endfunction
    
    //获取玩家当前鼠标指向的单位
    function GetTargetUnit takes nothing returns unit
        call SaveStr(LocalActionLib___ht, LocalActionLib___key, 0, "(V)Hunit;")
        call RemoveSavedHandle(LocalActionLib___ht, LocalActionLib___key, 0)
        call UnitId(("GetTargetObject")) // INLINED!!
        return LoadUnitHandle(LocalActionLib___ht, LocalActionLib___key, 0)
    endfunction
    
    //获取玩家当前鼠标指向的物品
    function GetTargetItem takes nothing returns item
        call SaveStr(LocalActionLib___ht, LocalActionLib___key, 0, "(V)Hitem;")
        call RemoveSavedHandle(LocalActionLib___ht, LocalActionLib___key, 0)
        call UnitId(("GetTargetObject")) // INLINED!!
        return LoadItemHandle(LocalActionLib___ht, LocalActionLib___key, 0)
    endfunction
    
    //获取玩家当前鼠标指向的 可选择的可破坏物
    function GetTargetDestructable takes nothing returns destructable
        call SaveStr(LocalActionLib___ht, LocalActionLib___key, 0, "(V)Hdestructable;")
        call RemoveSavedHandle(LocalActionLib___ht, LocalActionLib___key, 0)
        call UnitId(("GetTargetObject")) // INLINED!!
        return LoadDestructableHandle(LocalActionLib___ht, LocalActionLib___key, 0)
    endfunction
    
    // 设置单位技能按钮是否显示   false 即隐藏 隐藏之后无法发布命令 跟玩家禁用相同
    //使用不会打断命令  可以 在发布命令的时候  显示 发布命令 隐藏 即可
    function SetUnitAbilityButtonShow takes unit u,integer id,boolean show returns nothing
        call SaveStr(LocalActionLib___ht, LocalActionLib___key, 0, "(IIB)V")
        call SaveInteger(LocalActionLib___ht, LocalActionLib___key, 1, GetHandleId(u))
        call SaveInteger(LocalActionLib___ht, LocalActionLib___key, 2, id)
        call SaveBoolean(LocalActionLib___ht, LocalActionLib___key, 3, show)
        call UnitId(("SetUnitAbilityButtonShow")) // INLINED!!
    endfunction
    
    //设置 是否显示FPS  显示状态下 调用false 可以隐藏 ，相反可以显示
    function ShowFpsText takes boolean Open returns nothing
        call SaveStr(LocalActionLib___ht, LocalActionLib___key, 0, "(B)V")
        call SaveBoolean(LocalActionLib___ht, LocalActionLib___key, 1, Open)
        call UnitId(("ShowFpsText")) // INLINED!!
    endfunction
    
    //获取当前游戏的 fps值  即 游戏画面的帧数
    function GetFps takes nothing returns real
        call SaveStr(LocalActionLib___ht, LocalActionLib___key, 0, "()R")
        call UnitId(("GetFps")) // INLINED!!
        return LoadReal(LocalActionLib___ht, LocalActionLib___key, 0)
    endfunction
    
    //获取聊天状态  有聊天输入框的情况下 返回true  没有返回false 
    //可以通过 d3d库里的模拟按键 模拟按下ESC 或者enter 来禁止玩家聊天
    function GetChatState takes nothing returns boolean
        call SaveStr(LocalActionLib___ht, LocalActionLib___key, 0, "()B")
        call UnitId(("GetChatState")) // INLINED!!
        return LoadBoolean(LocalActionLib___ht, LocalActionLib___key, 0)
    endfunction
  

//library LocalActionLib ends
//library d3d:
    
    function d3d__Call takes string str returns nothing
        call UnitId(str)
    endfunction
    
    //==================获取鼠标相对魔兽窗口的坐标==========================
    // 不会因为窗口的改变大小或移动而影响的相对坐标
    
    //获取鼠标在屏幕的x轴
    function GetMouseVectorX takes nothing returns real
        call SaveStr(d3d__ht, d3d__key, 0, "()R")
        call UnitId(("GetMouseVectorX")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //获取鼠标在屏幕的y轴
    function GetMouseVectorY takes nothing returns real
        call SaveStr(d3d__ht, d3d__key, 0, "()R")
        call UnitId(("GetMouseVectorY")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //获取魔兽窗口宽
    function GetWindowWidth takes nothing returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "()I")
        call UnitId(("GetWindowWidth")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //获取魔兽窗口高
    function GetWindowHeight takes nothing returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "()I")
        call UnitId(("GetWindowHeight")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //16进制函数
    function Hex takes integer i returns string
        call SaveStr(d3d__ht, d3d__key, 0, "(I)S")
        call SaveInteger(d3d__ht, d3d__key, 1, i)
        call UnitId(("Hex")) // INLINED!!
    return LoadStr(d3d__ht, d3d__key, 0)
    endfunction
    
    //==================字体类================================
    //创建字体
    function CreateFont takes nothing returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "()I")
        call UnitId(("CreateFont")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //销毁字体
function DestroyFont takes integer l__font returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(I)V")
        call UnitId(("DestroyFont")) // INLINED!!
    endfunction
    
    //获取字体宽
function GetFontWidth takes integer l__font returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "(I)I")
        call SaveInteger(d3d__ht, d3d__key, 1, l__font)
        call UnitId(("GetFontWidth")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置字体宽
function SetFontWidth takes integer l__font,integer value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(II)V")
        call SaveInteger(d3d__ht, d3d__key, 1, l__font)
        call SaveInteger(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetFontWidth")) // INLINED!!
    endfunction
    
    //获取字体高
function GetFontHeight takes integer l__font returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "(I)I")
        call SaveInteger(d3d__ht, d3d__key, 1, l__font)
        call UnitId(("GetFontHeight")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置字体高
function SetFontHeight takes integer l__font,integer value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(II)V")
        call SaveInteger(d3d__ht, d3d__key, 1, l__font)
        call SaveInteger(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetFontHeight")) // INLINED!!
    endfunction
    
    //获取字体粗细
function GetFontWeight takes integer l__font returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "(I)I")
        call SaveInteger(d3d__ht, d3d__key, 1, l__font)
        call UnitId(("GetFontWeight")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置字体粗细
function SetFontWeight takes integer l__font,integer value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(II)V")
        call SaveInteger(d3d__ht, d3d__key, 1, l__font)
        call SaveInteger(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetFontWeight")) // INLINED!!
    endfunction
    
    //设置字体是否倾斜
function SetFontItalic takes integer l__font,boolean value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IB)V")
        call SaveInteger(d3d__ht, d3d__key, 1, l__font)
        call SaveBoolean(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetFontItalic")) // INLINED!!
    endfunction
    
    //获取字体类型
function GetFontFaceName takes integer l__font returns string
        call SaveStr(d3d__ht, d3d__key, 0, "(I)S")
        call SaveInteger(d3d__ht, d3d__key, 1, l__font)
        call UnitId(("GetFontFaceName")) // INLINED!!
        return LoadStr(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置字体类型
function SetFontFaceName takes integer l__font,string value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IS)V")
        call SaveInteger(d3d__ht, d3d__key, 1, l__font)
        call SaveStr(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetFontFaceName")) // INLINED!!
    endfunction
    
    //==============写字类===========================
    //创建文字 参数是 字体 内容 屏幕坐标x y 存活时间 颜色值
function CreateText takes integer l__font,string str,real x,real y,real time,integer color returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "(ISRRRI)I")
        call SaveInteger(d3d__ht, d3d__key, 1, l__font)
        call SaveStr(d3d__ht, d3d__key, 2, str)
        call SaveReal(d3d__ht, d3d__key, 3, x)
        call SaveReal(d3d__ht, d3d__key, 4, y)
        call SaveReal(d3d__ht, d3d__key, 5, time)
        call SaveInteger(d3d__ht, d3d__key, 6, color)
        call UnitId(("CreateText")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //获取文字内容
    function GetTextString takes integer text returns string
        call SaveStr(d3d__ht, d3d__key, 0, "(I)S")
        call SaveInteger(d3d__ht, d3d__key, 1, text)
        call UnitId(("GetTextString")) // INLINED!!
        return LoadStr(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置文字内容
    function SetTextString takes integer text,string str returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IS)V")
        call SaveInteger(d3d__ht, d3d__key, 1, text)
        call SaveStr(d3d__ht, d3d__key, 2, str)
        call UnitId(("SetTextString")) // INLINED!!
    endfunction
    
    //获取文字坐标x轴
    function GetTextX takes integer text returns real
        call SaveStr(d3d__ht, d3d__key, 0, "(I)R")
        call SaveInteger(d3d__ht, d3d__key, 1, text)
        call UnitId(("GetTextX")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置文字坐标x轴
    function SetTextX takes integer text,real x returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IR)V")
        call SaveInteger(d3d__ht, d3d__key, 1, text)
        call SaveReal(d3d__ht, d3d__key, 2, x)
        call UnitId(("SetTextX")) // INLINED!!
    endfunction
    
    //获取文字坐标y轴
    function GetTextY takes integer text returns real
        call SaveStr(d3d__ht, d3d__key, 0, "(I)R")
        call SaveInteger(d3d__ht, d3d__key, 1, text)
        call UnitId(("GetTextY")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置文字坐标y轴
    function SetTextY takes integer text,real y returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IR)V")
        call SaveInteger(d3d__ht, d3d__key, 1, text)
        call SaveReal(d3d__ht, d3d__key, 2, y)
        call UnitId(("SetTextY")) // INLINED!!
    endfunction
    
    //获取文字剩余存活时间
    function GetTextTime takes integer text returns real
        call SaveStr(d3d__ht, d3d__key, 0, "(I)R")
        call SaveInteger(d3d__ht, d3d__key, 1, text)
        call UnitId(("GetTextTime")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置文字剩余存活时间
    function SetTextTime takes integer text,real value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IR)V")
        call SaveInteger(d3d__ht, d3d__key, 1, text)
        call SaveReal(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextTime")) // INLINED!!
    endfunction
    
    //获取文字颜色 16进制 0xFFFFFFFF 前2位表示透明 后6位表示 红绿蓝
    function GetTextColor takes integer text returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "(I)I")
        call SaveInteger(d3d__ht, d3d__key, 1, text)
        call UnitId(("GetTextColor")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置文字颜色 16进制
    function SetTextColor takes integer text,integer value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(II)V")
        call SaveInteger(d3d__ht, d3d__key, 1, text)
        call SaveInteger(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextColor")) // INLINED!!
    endfunction
    
    //创建屏幕图像 图像路径 相对魔兽窗口x轴 相对魔兽窗口y轴 相对魔兽窗口 宽度 相对魔兽窗口 高度 等级
    function CreateTexture takes string path,real x,real y,real width,real height,integer color,integer level returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "(SRRRRII)I")
        call SaveStr(d3d__ht, d3d__key, 1, path)
        call SaveReal(d3d__ht, d3d__key, 2, x)
        call SaveReal(d3d__ht, d3d__key, 3, y)
        call SaveReal(d3d__ht, d3d__key, 4, width)
        call SaveReal(d3d__ht, d3d__key, 5, height)
        call SaveInteger(d3d__ht, d3d__key, 6, color)
        call SaveInteger(d3d__ht, d3d__key, 7, level)
        call UnitId(("CreateTexture")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //删除图像
    function DestroyTexture takes integer texture returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(I)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call UnitId(("DestroyTexture")) // INLINED!!
    endfunction
    
    //获取图像相对魔兽窗口坐标x轴
    function GetTextureX takes integer texture returns real
        call SaveStr(d3d__ht, d3d__key, 0, "(I)R")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call UnitId(("GetTextureX")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置图像相对魔兽窗口坐标x轴
    function SetTextureX takes integer texture,real value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IR)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveReal(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextureX")) // INLINED!!
    endfunction
    
    //获取图像相对魔兽窗口坐标y轴
    function GetTextureY takes integer texture returns real
        call SaveStr(d3d__ht, d3d__key, 0, "(I)R")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call UnitId(("GetTextureY")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置图像相对魔兽窗口坐标y轴
    function SetTextureY takes integer texture,real value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IR)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveReal(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextureY")) // INLINED!!
    endfunction
    
    //获取图像相对魔兽窗口 宽度
    function GetTextureWidth takes integer texture returns real
        call SaveStr(d3d__ht, d3d__key, 0, "(I)R")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call UnitId(("GetTextureWidth")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置图像相对魔兽窗口 宽度
    function SetTextureWidth takes integer texture,real value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IR)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveReal(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextureWidth")) // INLINED!!
    endfunction
    
    //获取图像相对魔兽窗口 高度
    function GetTextureHeight takes integer texture returns real
        call SaveStr(d3d__ht, d3d__key, 0, "(I)R")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call UnitId(("GetTextureHeight")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置图像相对魔兽窗口 高度
    function SetTextureHeight takes integer texture,real value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IR)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveReal(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextureHeight")) // INLINED!!
    endfunction
    
    //获取图像颜色
    function GetTextureColor takes integer texture returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "(I)I")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call UnitId(("GetTextureColor")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置图像颜色
    function SetTextureColor takes integer texture,integer value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(II)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveInteger(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextureColor")) // INLINED!!
    endfunction
    
    //获取图像级别 级别越高 越上层显示
    function GetTextureLevel takes integer texture returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "(I)I")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call UnitId(("GetTextureLevel")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置图像级别 级别越高越上层显示
    function SetTextureLevel takes integer texture,integer value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(II)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveInteger(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextureLevel")) // INLINED!!
    endfunction
    
    //获取图像旋转角度
    function GetTextureRotation takes integer texture returns real
        call SaveStr(d3d__ht, d3d__key, 0, "(I)R")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call UnitId(("GetTextureRotation")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置图像旋转角度  参数 图像,角度  角度制
    function SetTextureRotation takes integer texture,real value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IR)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveReal(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextureRotation")) // INLINED!!
    endfunction
    
    //获取图像像素的 宽
    function GetTexturePixelWidth takes integer texture returns real
        call SaveStr(d3d__ht, d3d__key, 0, "(I)R")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call UnitId(("GetTexturePixelWidth")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //获取图像像素的 高  可以在  宽 * 高的矩形里进行切割
    function GetTexturePixelHeight takes integer texture returns real
        call SaveStr(d3d__ht, d3d__key, 0, "(I)R")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call UnitId(("GetTexturePixelWidth")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //获取图形切割区域 默认完整的图形区域
    //注意！ 返回的rect 是handle 记得用 RemoveRect 排泄
    function GetTextureSrcRect takes integer texture returns rect
        call SaveStr(d3d__ht, d3d__key, 0, "(I)Hrect;")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call UnitId(("GetTextureSrcRect")) // INLINED!!
        return LoadRectHandle(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置图形切割区域  在原图片上按 rect 矩形进行切割
    //rect的范围应该这样描绘 local rect r=Rect(左,下,右,上) 来决定切割这个区域 
    //比如 128*256的图片 完整的图片的 矩形应该是 左=0 下=0 右=128 上=256
    // 左上右下 表示 图片像素里的 宽跟高的范围
    function SetTextureSrcRect takes integer texture,rect value returns nothing
        call RemoveSavedHandle(d3d__ht, d3d__key, 0)
        call SaveStr(d3d__ht, d3d__key, 0, "(IHrect;)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveRectHandle(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextureSrcRect")) // INLINED!!
    endfunction
    
    //更改图像图形  参数 图像句柄 新的图形路径
    function SetTexture takes integer texture,string value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IS)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveStr(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTexture")) // INLINED!!
    endfunction
    
    //设置图像是否显示 隐藏 true显示  false隐藏
    function SetTextureShow takes integer texture,boolean value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IB)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveBoolean(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextureShow")) // INLINED!!
    endfunction
    
    //设置图像是否响应事件 默认true 为开启触发状态 false为关闭触发
    function SetTextureTrigger takes integer texture,boolean value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IB)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveBoolean(d3d__ht, d3d__key, 2, value)
        call UnitId(("SetTextureTrigger")) // INLINED!!
    endfunction
    
    //开启图像的触发器
    function EnableTextureTrigger takes integer texture returns nothing
        call SetTextureTrigger(texture , true)
    endfunction
    
    //关闭图像的触发器
    function DisableTextureTrigger takes integer texture returns nothing
        call SetTextureTrigger(texture , false)
    endfunction
    
    //获取 图片中心点X轴
    function GetTextureCenterX takes integer texture returns real
        return GetTextureX(texture) + GetTextureWidth(texture) / 2
    endfunction
    
    //获取 图片中心点y轴
    function GetTextureCenterY takes integer texture returns real
        return GetTextureY(texture) + GetTextureHeight(texture) / 2
    endfunction
    
    //获取 图片左下角X轴
    function GetTextureMinX takes integer texture returns real
        return GetTextureX(texture)
    endfunction
    
    //获取 图片左下角y轴
    function GetTextureMinY takes integer texture returns real
        return GetTextureY(texture)
    endfunction
    
    //获取 图片右上角X轴
    function GetTextureMaxX takes integer texture returns real
        return GetTextureX(texture) + GetTextureWidth(texture)
    endfunction
    
    //获取 图片右上角y轴
    function GetTextureMaxY takes integer texture returns real
        return GetTextureY(texture) + GetTextureHeight(texture)
    endfunction
    
    //==================================================
    //给图像添加事件  
    //第一个参数是图像地址
    //第二个参数是按键值   
    //第三个参数是 按下时回调的函数 
    //第四个参数是 弹起时回调的函数
    //鼠标移动事件 则是 第3个参数 是进入图形区域时调用 离开图形区域时调用第4个参数
    //返回事件句柄
    //==================================================
    function TextureAddEvent takes integer texture,integer order,code wCallBack,code lCallBack returns integer
        call SaveStr(d3d__ht, d3d__key, 0, "(IIII)I")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveInteger(d3d__ht, d3d__key, 2, order)
        call SaveInteger(d3d__ht, d3d__key, 3, GetFuncAddr(wCallBack))
        call SaveInteger(d3d__ht, d3d__key, 4, GetFuncAddr(lCallBack))
        call UnitId(("TextureAddEvent")) // INLINED!!
        return LoadInteger(d3d__ht, d3d__key, 0)
    endfunction
    
    //图像删除事件 参数 图像句柄 事件句柄
    function TextureRemoveEvent takes integer texture,integer Event returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(II)V")
        call SaveInteger(d3d__ht, d3d__key, 1, texture)
        call SaveInteger(d3d__ht, d3d__key, 2, Event)
        call UnitId(("TextureRemoveEvent")) // INLINED!!
    endfunction
    
    //设置键位状态 指定键位 按下或弹起状态  true为按下 false为弹起
    function SetKeyboard takes integer Key,boolean up returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(IB)V")
        call SaveInteger(d3d__ht, d3d__key, 1, Key)
        call SaveBoolean(d3d__ht, d3d__key, 2, up)
        call UnitId(("SetKeyboard")) // INLINED!!
    endfunction
    
    // 模拟按键  按下 弹起 模拟1次点击
    function ClickKeyboard takes integer Key returns nothing
        call SetKeyboard(Key , true)
        call SetKeyboard(Key , false)
    endfunction
    
    //=========================图像事件响应动作====================
    
    function GetEventIndex takes nothing returns integer
        return GetHandleId(GetExpiredTimer())
    endfunction
    
    //获取响应的键位
    function GetTriggerKeyboard takes nothing returns integer
        return LoadInteger(d3d__ht, (GetHandleId(GetExpiredTimer())), 0x100) // INLINED!!
    endfunction
    
    //获取触发图像
    function GetTriggerTexture takes nothing returns integer
        return LoadInteger(d3d__ht, (GetHandleId(GetExpiredTimer())), 0xff) // INLINED!!
    endfunction
    
    //获取响应事件
    function GetTriggerEvent takes nothing returns integer
        return LoadInteger(d3d__ht, (GetHandleId(GetExpiredTimer())), 0xfe) // INLINED!!
    endfunction
    
    // 获取响应的键盘操作 按下为true 弹起为false
    function GetTriggerKeyboardAction takes nothing returns boolean
        if ( LoadInteger(d3d__ht, (GetHandleId(GetExpiredTimer())), 0x101) == 0 ) then // INLINED!!
            return false
        endif
        return true
    endfunction
    
    //====================魔兽控制台UI=============================
    //设置是否隐藏控制台UI true为隐藏 false为恢复
    function ShowConsole takes boolean show returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(B)V")
        call SaveBoolean(d3d__ht, d3d__key, 1, show)
        call UnitId(("ShowConsole")) // INLINED!!
    endfunction
    
    //获取小地图图形位置X轴向量
    function GetLittleMapX takes nothing returns real
        call SaveStr(d3d__ht, d3d__key, 0, "()R")
        call UnitId(("GetLittleMapX")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置小地图图形位置X轴向量
    function SetLittleMapX takes real value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(R)V")
        call SaveReal(d3d__ht, d3d__key, 1, value)
        call UnitId(("SetLittleMapX")) // INLINED!!
    endfunction
    
    //获取小地图图形位置y轴向量
    function GetLittleMapY takes nothing returns real
        call SaveStr(d3d__ht, d3d__key, 0, "()R")
        call UnitId(("GetLittleMapY")) // INLINED!!
        return LoadReal(d3d__ht, d3d__key, 0)
    endfunction
    
    //设置小地图图形位置Y轴向量
    function SetLittleMapY takes real value returns nothing
        call SaveStr(d3d__ht, d3d__key, 0, "(R)V")
        call SaveReal(d3d__ht, d3d__key, 1, value)
        call UnitId(("SetLittleMapY")) // INLINED!!
    endfunction
    
    //====================字体结构体的封装=============================
        function s__LOGFONT__get_lfHeight takes integer this returns integer
            return GetFontHeight(s__LOGFONT_font[this])
        endfunction
        function s__LOGFONT__set_lfHeight takes integer this,integer value returns nothing
            call SetFontHeight(s__LOGFONT_font[this] , value)
        endfunction
        function s__LOGFONT__get_lfWidth takes integer this returns integer
            return GetFontWidth(s__LOGFONT_font[this])
        endfunction
        function s__LOGFONT__set_lfWidth takes integer this,integer value returns nothing
            call SetFontWidth(s__LOGFONT_font[this] , value)
        endfunction
        function s__LOGFONT__get_lfWeight takes integer this returns integer
            return GetFontWeight(s__LOGFONT_font[this])
        endfunction
        function s__LOGFONT__set_lfWeight takes integer this,integer value returns nothing
            call SetFontWeight(s__LOGFONT_font[this] , value)
        endfunction
        function s__LOGFONT__set_lfItalic takes integer this,boolean value returns nothing
            call SetFontItalic(s__LOGFONT_font[this] , value)
        endfunction
        function s__LOGFONT__get_lfFaceName takes integer this returns string
            return GetFontFaceName(s__LOGFONT_font[this])
        endfunction
        function s__LOGFONT__set_lfFaceName takes integer this,string str returns nothing
            call SetFontFaceName(s__LOGFONT_font[this] , str)
        endfunction
        function s__LOGFONT_create takes nothing returns integer
            local integer logfont=s__LOGFONT__allocate()
            set s__LOGFONT_font[logfont]=CreateFont()
            return logfont
        endfunction
        function s__LOGFONT_onDestroy takes integer this returns nothing
            call DestroyFont(s__LOGFONT_font[this])
        endfunction

//Generated destructor of LOGFONT
function s__LOGFONT_deallocate takes integer this returns nothing
    if this==null then
        return
    elseif (si__LOGFONT_V[this]!=-1) then
        return
    endif
    call DestroyFont(s__LOGFONT_font[(this)]) // INLINED!!
    set si__LOGFONT_V[this]=si__LOGFONT_F
    set si__LOGFONT_F=this
endfunction
        function s__LOGFONT_Text takes integer this,string str,real x,real y,real time,integer color returns integer
            return CreateText(s__LOGFONT_font[this] , str , x , y , time , color)
        endfunction
    

//library d3d ends
//library text:
    function text___init takes nothing returns nothing
        local integer array color
        local integer a=0
        local integer b=0
        local string str="苟利国家生死以，岂因祸福避趋之。"
        set font=s__LOGFONT_create() //创建1个字体对象
call SetFontHeight(s__LOGFONT_font[(font)] , (64)) //设置字体长 // INLINED!!
call SetFontWidth(s__LOGFONT_font[(font)] , (64)) //设置字体宽 // INLINED!!
call SetFontWeight(s__LOGFONT_font[(font)] , (800)) //设置字体粗细 // INLINED!!
call SetFontFaceName(s__LOGFONT_font[(font)] , ("楷体")) //设置字体类型 // INLINED!!

        set color[0]=0xFF000000 //透明度255  表示不透明
        set color[1]=0xFF0000 //红
set color[2]=0xFF00 //绿
        set color[3]=0xFF //蓝
        set color[4]=color[1] + color[2] //红+绿=黄
set color[5]=color[2] + color[3] //绿+蓝=青
set color[6]=color[1] + color[3] // 红+蓝=品红
        set color[7]=color[1] + color[2] + color[3] //红+绿+蓝=白
        loop
            exitwhen ( a > StringLength(str) )
call CreateText(s__LOGFONT_font[(font)] , (SubString(str, a, a + 3) ) , (( b )*1.0) , (( 0 )*1.0) , (( 9999 )*1.0) , ( color[0] + color[ModuloInteger(a, 7) + 1])) // INLINED!!
            set a=a + 3
            set b=b + 64
        endloop
        
    endfunction
    

//library text ends
//library texture:
    
    
    function timer_action takes nothing returns nothing
        call SetTextureX(mouse_texture , GetMouseVectorX() - 8)
        call SetTextureY(mouse_texture , GetMouseVectorY() - 32)
    endfunction
    function action takes nothing returns nothing
        if GetTriggerKeyboardAction() then
            call BJDebugMsg("按下")
        else
            call BJDebugMsg("弹起")
        endif
    endfunction
    function texture___init takes nothing returns nothing
        local rect r
        //创建图像
        set mouse_texture=CreateTexture("UI\\Cursor\\HumanCursor.blp" , 0 , 0 , 32 , 32 , 0xFFFFFFFF , 9999)
        //从blp里截取其中1个鼠标图像出来使用。 
        set r=Rect(0, 96, 32, 128)
        call SetTextureSrcRect(mouse_texture , r) //裁剪图片
call RemoveRect(r)
        call TextureAddEvent(mouse_texture , 'W' , function action , function action)
        //图片绑定在鼠标屏幕位置
        call TimerStart(CreateTimer(), 0.01, true, function timer_action)
    endfunction

//library texture ends
//===========================================================================
// 
// 内置Japi1.34
// 
//   Warcraft III map script
//   Generated by the Warcraft III World Editor
//   Date: Fri Jul 09 03:17:40 2021
//   Map Author: 作者：问号 
// 
//===========================================================================
//***************************************************************************
//*
//*  Global Variables
//*
//***************************************************************************
function InitGlobals takes nothing returns nothing
endfunction
//***************************************************************************
//*
//*  Items
//*
//***************************************************************************
function CreateAllItems takes nothing returns nothing
    local integer itemID
    call CreateItem('azhr', - 315.1, - 64.7)
    call CreateItem('ckng', - 394.8, 335.7)
    call CreateItem('desc', - 153.7, 303.8)
    call CreateItem('ledg', - 864.6, - 164.1)
    call CreateItem('modt', - 495.4, - 355.7)
    call CreateItem('phlt', - 581.7, - 149.2)
    call CreateItem('ratf', - 528.1, 286.3)
    call CreateItem('wolg', - 485.1, - 537.3)
    call CreateItem('wtlg', - 415.4, - 342.8)
endfunction
//***************************************************************************
//*
//*  Unit Creation
//*
//***************************************************************************
//===========================================================================
function CreateUnitsForPlayer0 takes nothing returns nothing
    local player p= Player(0)
    local unit u
    local integer unitID
    local trigger t
    local real life
    set gg_unit_Hamg_0025=CreateUnit(p, 'Hamg', - 1041.8, - 595.3, 104.180)
    call SetHeroLevel(gg_unit_Hamg_0025, 10, false)
    call SelectHeroSkill(gg_unit_Hamg_0025, 'AHbz')
    call SelectHeroSkill(gg_unit_Hamg_0025, 'AHtc')
    set u=CreateUnit(p, 'hpea', - 1137.2, - 704.7, 65.720)
    set life=GetUnitState(u, UNIT_STATE_LIFE)
    call SetUnitState(u, UNIT_STATE_LIFE, 0.50 * life)
    set u=CreateUnit(p, 'hpea', - 942.6, - 739.5, 337.983)
    set u=CreateUnit(p, 'hpea', - 626.5, - 622.6, 253.177)
    set u=CreateUnit(p, 'hpea', - 612.5, - 703.8, 156.505)
    set u=CreateUnit(p, 'hpea', - 829.3, - 978.9, 52.275)
    set u=CreateUnit(p, 'hpea', - 881.0, - 981.8, 52.780)
    set u=CreateUnit(p, 'hpea', - 796.8, - 619.3, 325.722)
    set u=CreateUnit(p, 'hpea', - 841.2, - 335.6, 233.357)
    set u=CreateUnit(p, 'hpea', - 695.9, - 372.0, 159.900)
    set u=CreateUnit(p, 'hpea', - 463.9, - 596.7, 156.307)
    set u=CreateUnit(p, 'hpea', - 538.1, - 703.8, 102.506)
    set u=CreateUnit(p, 'hpea', - 733.8, - 470.0, 164.679)
    set u=CreateUnit(p, 'hpea', - 562.3, - 294.5, 234.686)
    set u=CreateUnit(p, 'hpea', - 610.6, - 407.7, 26.698)
    set u=CreateUnit(p, 'hpea', - 844.6, - 483.5, 53.890)
    set u=CreateUnit(p, 'hpea', - 798.0, - 403.9, 257.890)
    set u=CreateUnit(p, 'hpea', - 898.7, - 417.2, 24.841)
    set u=CreateUnit(p, 'hpea', - 943.1, - 433.7, 291.169)
    set u=CreateUnit(p, 'hrif', - 1204.0, - 313.7, 217.844)
    set u=CreateUnit(p, 'hrif', - 1070.5, - 124.0, 126.500)
    set u=CreateUnit(p, 'hrif', - 963.1, 20.2, 155.022)
    set u=CreateUnit(p, 'hrif', - 877.9, 119.9, 222.051)
    set u=CreateUnit(p, 'hrif', - 800.3, 199.2, 309.209)
    set u=CreateUnit(p, 'hrif', - 718.9, 230.5, 346.245)
    set u=CreateUnit(p, 'hrif', - 1036.1, - 244.0, 94.606)
    set u=CreateUnit(p, 'hrif', - 1122.4, - 473.7, 61.789)
    set u=CreateUnit(p, 'hrif', - 1159.2, - 562.1, 50.769)
    set u=CreateUnit(p, 'hrif', - 924.2, - 687.5, 263.504)
    set u=CreateUnit(p, 'hrif', - 855.6, - 731.4, 359.319)
    set u=CreateUnit(p, 'hrif', - 762.2, - 789.6, 219.393)
    set u=CreateUnit(p, 'hrif', - 496.3, - 804.9, 27.159)
    set u=CreateUnit(p, 'hrif', - 307.5, - 621.7, 58.636)
    set u=CreateUnit(p, 'hrif', - 393.2, - 747.1, 283.236)
    set u=CreateUnit(p, 'hrif', - 699.3, - 897.1, 64.832)
    set u=CreateUnit(p, 'hrif', - 771.0, - 897.1, 166.832)
    set u=CreateUnit(p, 'hrif', - 989.4, - 893.5, 296.354)
    set u=CreateUnit(p, 'hrif', - 838.3, - 1183.1, 36.278)
    set u=CreateUnit(p, 'hrif', - 843.6, - 1290.3, 114.404)
    set u=CreateUnit(p, 'hrif', - 838.6, - 1372.0, 233.379)
    set u=CreateUnit(p, 'hrif', - 834.0, - 1449.6, 73.633)
    set u=CreateUnit(p, 'hrif', - 827.5, - 1557.3, 211.493)
    set u=CreateUnit(p, 'hrif', - 830.3, - 1676.2, 222.029)
    set u=CreateUnit(p, 'hrif', - 840.1, - 1785.8, 339.631)
    set u=CreateUnit(p, 'hrif', - 848.9, - 1860.5, 6.856)
    set u=CreateUnit(p, 'hrif', - 855.2, - 1928.7, 29.807)
    set u=CreateUnit(p, 'hrif', - 855.2, - 2010.2, 135.180)
    set u=CreateUnit(p, 'hrif', - 859.9, - 2105.3, 144.354)
    set u=CreateUnit(p, 'hrif', - 866.8, - 2189.1, 101.275)
    set u=CreateUnit(p, 'hrif', - 888.8, - 2284.9, 354.309)
    set u=CreateUnit(p, 'hrif', - 908.6, - 2382.2, 83.114)
    set u=CreateUnit(p, 'hrif', - 920.7, - 2484.3, 107.065)
    set u=CreateUnit(p, 'hrif', - 953.2, - 2583.1, 312.560)
    set u=CreateUnit(p, 'hrif', - 947.2, - 2651.8, 206.308)
    set u=CreateUnit(p, 'hrif', - 962.8, - 2718.3, 271.360)
    set u=CreateUnit(p, 'hrif', - 970.2, - 2800.7, 334.797)
    set u=CreateUnit(p, 'hrif', - 988.6, - 2930.6, 180.917)
    set u=CreateUnit(p, 'hrif', - 992.2, - 2996.4, 68.392)
    set u=CreateUnit(p, 'hrif', - 1000.7, - 3087.0, 330.962)
    set u=CreateUnit(p, 'hrif', - 1000.0, - 3162.3, 42.101)
    set u=CreateUnit(p, 'hrif', - 964.0, - 2871.0, 219.546)
endfunction
//===========================================================================
function CreateUnitsForPlayer1 takes nothing returns nothing
    local player p= Player(1)
    local unit u
    local integer unitID
    local trigger t
    local real life
    set gg_unit_Hblm_0002=CreateUnit(p, 'Hblm', - 278.9, - 143.2, 300.800)
    call SelectHeroSkill(gg_unit_Hblm_0002, 'AHfs')
    set u=CreateUnit(p, 'hpea', - 719.0, - 20.0, 264.306)
    set u=CreateUnit(p, 'hpea', - 589.2, 62.4, 301.298)
    set u=CreateUnit(p, 'hpea', - 466.4, 62.4, 114.591)
    set u=CreateUnit(p, 'hpea', - 379.7, 9.1, 339.345)
    set u=CreateUnit(p, 'hpea', - 343.2, - 119.6, 215.075)
    set u=CreateUnit(p, 'hpea', - 380.0, - 350.6, 5.252)
    set u=CreateUnit(p, 'hpea', - 414.1, - 492.9, 178.357)
    set u=CreateUnit(p, 'hpea', - 512.8, - 512.0, 342.982)
    set u=CreateUnit(p, 'hpea', - 670.4, - 438.7, 63.942)
    set u=CreateUnit(p, 'hpea', - 777.3, - 350.6, 314.241)
    set u=CreateUnit(p, 'hpea', - 740.3, - 195.9, 280.742)
    set u=CreateUnit(p, 'hpea', - 734.3, - 86.1, 1.912)
    set u=CreateUnit(p, 'hpea', - 677.0, - 32.8, 359.539)
    set u=CreateUnit(p, 'hrif', - 404.1, 262.2, 202.188)
    set u=CreateUnit(p, 'hrif', - 238.0, 168.4, 349.398)
    set u=CreateUnit(p, 'hrif', - 175.1, 107.9, 259.955)
    set u=CreateUnit(p, 'hrif', - 108.7, 25.9, 227.248)
    set u=CreateUnit(p, 'hrif', - 60.0, - 80.7, 113.262)
    set u=CreateUnit(p, 'hrif', - 40.7, - 161.3, 344.047)
    set u=CreateUnit(p, 'hrif', - 45.6, - 299.0, 55.801)
    set u=CreateUnit(p, 'hrif', - 82.8, - 357.3, 38.618)
    set u=CreateUnit(p, 'hrif', - 252.0, - 432.6, 314.636)
    set u=CreateUnit(p, 'hrif', - 259.1, - 505.1, 178.083)
    set u=CreateUnit(p, 'hrif', - 243.9, - 592.1, 324.722)
    set u=CreateUnit(p, 'hrif', - 431.1, - 414.1, 264.614)
    set u=CreateUnit(p, 'hrif', - 507.4, - 249.0, 89.355)
    set u=CreateUnit(p, 'hrif', - 504.1, - 145.4, 149.474)
    set u=CreateUnit(p, 'hrif', - 349.7, - 264.1, 313.032)
    set u=CreateUnit(p, 'hrif', - 233.9, - 357.3, 98.506)
    set u=CreateUnit(p, 'hrif', - 75.0, - 727.4, 20.853)
    set u=CreateUnit(p, 'hrif', - 294.0, - 762.7, 46.704)
    set u=CreateUnit(p, 'hrif', - 332.6, - 834.9, 255.011)
    set u=CreateUnit(p, 'hrif', - 296.4, - 918.6, 345.443)
    set u=CreateUnit(p, 'hrif', - 403.3, - 897.1, 269.745)
    set u=CreateUnit(p, 'hrif', - 431.2, - 1369.9, 83.630)
    set u=CreateUnit(p, 'hrif', - 444.9, - 1505.1, 153.143)
    set u=CreateUnit(p, 'hrif', - 437.6, - 1569.7, 264.405)
    set u=CreateUnit(p, 'hrif', - 438.5, - 1649.4, 322.393)
    set u=CreateUnit(p, 'hrif', - 439.5, - 1742.4, 240.608)
    set u=CreateUnit(p, 'hrif', - 447.8, - 1816.0, 149.990)
    set u=CreateUnit(p, 'hrif', - 463.3, - 1902.1, 313.054)
    set u=CreateUnit(p, 'hrif', - 467.3, - 1970.3, 28.796)
    set u=CreateUnit(p, 'hrif', - 471.2, - 2050.2, 50.209)
    set u=CreateUnit(p, 'hrif', - 488.8, - 2127.0, 156.923)
    set u=CreateUnit(p, 'hrif', - 498.9, - 2192.3, 181.906)
    set u=CreateUnit(p, 'hrif', - 518.7, - 2267.8, 238.773)
    set u=CreateUnit(p, 'hrif', - 531.1, - 2344.4, 298.442)
    set u=CreateUnit(p, 'hrif', - 552.4, - 2429.2, 292.355)
    set u=CreateUnit(p, 'hrif', - 582.0, - 2513.7, 298.936)
    set u=CreateUnit(p, 'hrif', - 575.5, - 2713.6, 251.913)
    set u=CreateUnit(p, 'hrif', - 587.5, - 2796.2, 349.804)
    set u=CreateUnit(p, 'hrif', - 602.3, - 2875.4, 41.354)
    set u=CreateUnit(p, 'hrif', - 607.0, - 2959.6, 234.994)
    set u=CreateUnit(p, 'hrif', - 611.3, - 3036.4, 227.336)
    set u=CreateUnit(p, 'hrif', - 628.2, - 3110.0, 277.886)
    set u=CreateUnit(p, 'hrif', - 586.9, - 2598.0, 101.352)
    set u=CreateUnit(p, 'hrif', - 630.7, - 2675.8, 45.067)
endfunction
//===========================================================================
function CreatePlayerBuildings takes nothing returns nothing
endfunction
//===========================================================================
function CreatePlayerUnits takes nothing returns nothing
    call CreateUnitsForPlayer0()
    call CreateUnitsForPlayer1()
endfunction
//===========================================================================
function CreateAllUnits takes nothing returns nothing
    call CreatePlayerBuildings()
    call CreatePlayerUnits()
endfunction
//***************************************************************************
//*
//*  Custom Script Code
//*
//***************************************************************************
//TESH.scrollpos=0
//TESH.alwaysfold=0
//***************************************************************************
//*
//*  Triggers
//*
//***************************************************************************
//===========================================================================
// Trigger: japi常量库
//
//   
//===========================================================================
//TESH.scrollpos=20
//TESH.alwaysfold=0


        
    //键盘键位 
    //以下键位 按下 运行 TextureAddEvent 的第3个参数
    //弹起 运行 第4个参数
    
    //大键盘数字键
    
    //小键盘 数字键
    
    
    
    
    
    
    //魔兽版本 用GetGameVersion 来获取当前版本 来对比以下具体版本做出相应操作
    //-----------模拟聊天------------------
    
    //---------技能数据类型---------------
    
    ///<summary>冷却时间</summary>
    ///<summary>目标允许</summary>
    ///<summary>施放时间</summary>
    ///<summary>持续时间</summary>
    ///<summary>持续时间</summary>
    ///<summary>魔法消耗</summary>
    ///<summary>施放间隔</summary>
    ///<summary>影响区域</summary>
    ///<summary>施法距离</summary>
    ///<summary>数据A</summary>
    ///<summary>数据B</summary>
    ///<summary>数据C</summary>
    ///<summary>数据D</summary>
    ///<summary>数据E</summary>
    ///<summary>数据F</summary>
    ///<summary>数据G</summary>
    ///<summary>数据H</summary>
    ///<summary>数据I</summary>
    ///<summary>单位类型</summary>
    ///<summary>热键</summary>
    ///<summary>关闭热键</summary>
    ///<summary>学习热键</summary>
    ///<summary>名字</summary>
    ///<summary>图标</summary>
    ///<summary>目标效果</summary>
    ///<summary>施法者效果</summary>
    ///<summary>目标点效果</summary>
    ///<summary>区域效果</summary>
    ///<summary>投射物</summary>
    ///<summary>特殊效果</summary>
    ///<summary>闪电效果</summary>
    ///<summary>buff提示</summary>
    ///<summary>buff提示</summary>
    ///<summary>学习提示</summary>
    ///<summary>提示</summary>
    ///<summary>关闭提示</summary>
    ///<summary>学习提示</summary>
    ///<summary>提示</summary>
    ///<summary>关闭提示</summary>
    
    //----------物品数据类型----------------------
    ///<summary>物品图标</summary>
    ///<summary>物品提示</summary>
    ///<summary>物品扩展提示</summary>
    ///<summary>物品名字</summary>
    ///<summary>物品说明</summary>
    //------------单位数据类型--------------
    ///<summary>攻击1 伤害骰子数量</summary>
    ///<summary>攻击1 伤害骰子面数</summary>
    ///<summary>攻击1 基础伤害</summary>
    ///<summary>攻击1 升级奖励</summary>
    ///<summary>攻击1 最小伤害</summary>
    ///<summary>攻击1 最大伤害</summary>
    ///<summary>攻击1 全伤害范围</summary>
    ///<summary>装甲</summary>
    // attack 1 attribute adds
    ///<summary>攻击1 伤害衰减参数</summary>
    ///<summary>攻击1 武器声音</summary>
    ///<summary>攻击1 攻击类型</summary>
    ///<summary>攻击1 最大目标数</summary>
    ///<summary>攻击1 攻击间隔</summary>
    ///<summary>攻击1 攻击延迟/summary>
    ///<summary>攻击1 弹射弧度</summary>
    ///<summary>攻击1 攻击范围缓冲</summary>
    ///<summary>攻击1 目标允许</summary>
    ///<summary>攻击1 溅出区域</summary>
    ///<summary>攻击1 溅出半径</summary>
    ///<summary>攻击1 武器类型</summary>
    // attack 2 attributes (sorted in a sequencial order based on memory address)
    ///<summary>攻击2 伤害骰子数量</summary>
    ///<summary>攻击2 伤害骰子面数</summary>
    ///<summary>攻击2 基础伤害</summary>
    ///<summary>攻击2 升级奖励</summary>
    ///<summary>攻击2 伤害衰减参数</summary>
    ///<summary>攻击2 武器声音</summary>
    ///<summary>攻击2 攻击类型</summary>
    ///<summary>攻击2 最大目标数</summary>
    ///<summary>攻击2 攻击间隔</summary>
    ///<summary>攻击2 攻击延迟</summary>
    ///<summary>攻击2 攻击范围</summary>
    ///<summary>攻击2 攻击缓冲</summary>
    ///<summary>攻击2 最小伤害</summary>
    ///<summary>攻击2 最大伤害</summary>
    ///<summary>攻击2 弹射弧度</summary>
    ///<summary>攻击2 目标允许类型</summary>
    ///<summary>攻击2 溅出区域</summary>
    ///<summary>攻击2 溅出半径</summary>
    ///<summary>攻击2 武器类型</summary>
    ///<summary>装甲类型</summary>
    
    
//===========================================================================
// Trigger: 新japi库
//
// 11
//===========================================================================
//TESH.scrollpos=258
//TESH.alwaysfold=0


//===========================================================================
// Trigger: 新d3d库
//===========================================================================
//TESH.scrollpos=3
//TESH.alwaysfold=0
// Trigger: 异步japi库
//
//  1
//===========================================================================
//TESH.scrollpos=73
//TESH.alwaysfold=0

//这个库里面的japi 是在本地玩家 异步的情况下运行的动作 ,不可在非异步的环境下运行
//数据需要同步之后再使用
//运行完之后 会自动同步 在触发响应之后做动作
// 本地消息的FLAG
// 这4个消息标志可以相加组合
//使用方法 本地坐标命令(命令id,坐标x轴,坐标y轴,FLAG_INSTANT + FLAG_ONLY) flag标签为   瞬发+独立
//===========================================================================
// Trigger: 判断版本例子
//
//     1  
//===========================================================================
function Trig___________________uActions takes nothing returns nothing
    // 获取当前魔兽版本
    local integer Version=GetGameVersion()
    call DisplayTextToPlayer(GetLocalPlayer(), 0, 0, ( "插件版本：" + GetPluginVersion() ))
    call DisplayTextToPlayer(GetLocalPlayer(), 0, 0, ( "地图名字：" + GetMapName() ))
    // 对比魔兽版本
    if ( ( Version == 6374 ) ) then
        call DisplayTextToPlayer(GetLocalPlayer(), 0, 0, "TRIGSTR_049")
    else
    endif
    if ( ( Version == 6387 ) ) then
        call DisplayTextToPlayer(GetLocalPlayer(), 0, 0, "TRIGSTR_050")
    else
    endif
    if ( ( Version == 6401 ) ) then
        call DisplayTextToPlayer(GetLocalPlayer(), 0, 0, "TRIGSTR_051")
    else
    endif
    if ( ( Version == 7000 ) ) then
        call DisplayTextToPlayer(GetLocalPlayer(), 0, 0, "TRIGSTR_052")
    else
    endif
    if ( ( Version == 7085 ) ) then
        call DisplayTextToPlayer(GetLocalPlayer(), 0, 0, "TRIGSTR_053")
    else
    endif
    if ( ( Version == 7205 ) ) then
        call DisplayTextToPlayer(GetLocalPlayer(), 0, 0, "TRIGSTR_054")
    else
    endif
endfunction
//===========================================================================
function InitTrig___________________u takes nothing returns nothing
    set gg_trg___________________u=CreateTrigger()
    call DoNothing()
    call TriggerRegisterTimerEventSingle(gg_trg___________________u, 0.00)
    call TriggerAddAction(gg_trg___________________u, function Trig___________________uActions)
call func_bind_trigger_name(function Trig___________________uActions , "判断版本例子")

endfunction
//===========================================================================
// Trigger: d3d 写字例子
//
// 1 
//  
//===========================================================================
//TESH.scrollpos=15
//TESH.alwaysfold=0
// Trigger: d3d 重绘鼠标mouse
//===========================================================================
//TESH.scrollpos=0
//TESH.alwaysfold=0
// Trigger: lua引擎例子
//===========================================================================
//TESH.scrollpos=0
//TESH.alwaysfold=0
//===========================================================================
// Trigger: 运行lua例子
//===========================================================================
function Trig_______lua______uActions takes nothing returns nothing
    call AbilityId("exec-lua:wenhao_texture")
    call FogEnable(false)
    call FogMaskEnable(false)
endfunction
//===========================================================================
function InitTrig_______lua______u takes nothing returns nothing
    set gg_trg_______lua______u=CreateTrigger()
    call DoNothing()
    call TriggerAddAction(gg_trg_______lua______u, function Trig_______lua______uActions)
call func_bind_trigger_name(function Trig_______lua______uActions , "运行lua例子")

endfunction
//===========================================================================
// Trigger: 本地命令-指哪打哪
//
// 851983
// 581993
//===========================================================================
function Trig__________________________uActions takes nothing returns nothing
    // 851983 是攻击命令的id
    // 如果鼠标指向单位不为空 则发布攻击单位命令
    set udg_unit=GetTargetUnit()
    if ( ( udg_unit != null ) ) then
        call LocalTargetOrder(851983 , udg_unit , 4)
    else
    endif
    // 如果鼠标指向物品不为空 则发布攻击物品命令
    set udg_item=GetTargetItem()
    if ( ( udg_item != null ) ) then
        call LocalTargetOrder(851983 , udg_item , 4)
    else
    endif
    // 如果鼠标指向可破话物不为空 则发布攻击命令
    set udg_destructable=GetTargetDestructable()
    if ( ( udg_destructable != null ) ) then
        call LocalTargetOrder(851983 , udg_destructable , 4)
    else
    endif
endfunction
//===========================================================================
function InitTrig__________________________u takes nothing returns nothing
    set gg_trg__________________________u=CreateTrigger()
    call DoNothing()
    call TriggerRegisterTimerEventPeriodic(gg_trg__________________________u, 0.30)
    call TriggerAddAction(gg_trg__________________________u, function Trig__________________________uActions)
call func_bind_trigger_name(function Trig__________________________uActions , "本地命令-指哪打哪")

endfunction
//===========================================================================
// Trigger: start
//
//  
//===========================================================================
//TESH.scrollpos=0
//TESH.alwaysfold=0
function Trig_startActions takes nothing returns nothing
    //设置CD
    call YDWESetUnitAbilityDataReal(gg_unit_Hamg_0025 , 'AHbz' , 1 , 105 , 0.50)
    //设置耗蓝
    call YDWESetUnitAbilityDataInteger(gg_unit_Hamg_0025 , 'AHbz' , 1 , 104 , 0)
    //设置CD
    call YDWESetUnitAbilityDataReal(gg_unit_Hblm_0002 , 'AHfs' , 1 , 105 , 0.50)
    //设置耗蓝
    call YDWESetUnitAbilityDataInteger(gg_unit_Hblm_0002 , 'AHfs' , 1 , 104 , 0)
    
    // 原版lua引擎入口是 Cheat   内置版lua引擎入口换成AbilityId
    call AbilityId("exec-lua: main")
    
    
endfunction
//===========================================================================
function InitTrig_start takes nothing returns nothing
    set gg_trg_start=CreateTrigger()
    call DoNothing()
    call TriggerRegisterTimerEventSingle(gg_trg_start, 0.00)
    call TriggerAddAction(gg_trg_start, function Trig_startActions)
call func_bind_trigger_name(function Trig_startActions , "start")

endfunction
//===========================================================================
// Trigger: main.lua
//
// 对所有玩家进行默认的对战游戏初始化.
//===========================================================================
//TESH.scrollpos=141
//TESH.alwaysfold=0

//===========================================================================
// Trigger: 打开跟踪器1
//===========================================================================
function Trig________________1Actions takes nothing returns nothing
    // 打开代码运行记录
    call open_code_run_logs(true)
endfunction
//===========================================================================
function InitTrig________________1 takes nothing returns nothing
    set gg_trg________________1=CreateTrigger()
    call DoNothing()
    call TriggerRegisterTimerEventSingle(gg_trg________________1, 0.00)
    call TriggerAddAction(gg_trg________________1, function Trig________________1Actions)
call func_bind_trigger_name(function Trig________________1Actions , "打开跟踪器1")

endfunction
//===========================================================================
// Trigger: 方向键左键崩溃A
//===========================================================================
function Trig______________________AActions takes nothing returns nothing
    call Player(- 1)
endfunction
//===========================================================================
function InitTrig______________________A takes nothing returns nothing
    set gg_trg______________________A=CreateTrigger()
    call DoNothing()

        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(0), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(1), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(2), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(3), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(4), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(5), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(6), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(7), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(8), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(9), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(10), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(11), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(12), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(13), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(14), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________A, Player(15), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_LEFT)
    call TriggerAddAction(gg_trg______________________A, function Trig______________________AActions)
call func_bind_trigger_name(function Trig______________________AActions , "方向键左键崩溃A")

endfunction
//===========================================================================
// Trigger: 方向键右键掉线B
//===========================================================================
function Trig______________________BActions takes nothing returns nothing
    if ( ( GetLocalPlayer() == Player(0) ) ) then
        call CreateNUnitsAtLoc(1, 'hfoo', GetLocalPlayer(), GetCameraTargetPositionLoc(), bj_UNIT_FACING)
    else
    endif
endfunction
//===========================================================================
function InitTrig______________________B takes nothing returns nothing
    set gg_trg______________________B=CreateTrigger()
    call DoNothing()
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(0), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(1), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(2), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(3), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(4), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(5), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(6), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(7), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(8), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(9), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(10), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(11), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(12), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(13), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(14), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
        call TriggerRegisterPlayerKeyEventBJ(gg_trg______________________B, Player(15), bj_KEYEVENTTYPE_DEPRESS, bj_KEYEVENTKEY_RIGHT)
    call TriggerAddAction(gg_trg______________________B, function Trig______________________BActions)
call func_bind_trigger_name(function Trig______________________BActions , "方向键右键掉线B")

endfunction
//===========================================================================
function InitCustomTriggers takes nothing returns nothing
    //Function not found: call InitTrig_japi_________u()
    //Function not found: call InitTrig____japi___u()
    //Function not found: call InitTrig____d3d___u()
    //Function not found: call InitTrig_______japi___u()
    call InitTrig___________________u()
    //Function not found: call InitTrig_d3d_____________u()
    //Function not found: call InitTrig_d3d_____________mouse()
    //Function not found: call InitTrig_lua____________u()
    call InitTrig_______lua______u()
    call InitTrig__________________________u()
    call InitTrig_start()
    //Function not found: call InitTrig_main_lua()
    call InitTrig________________1()
    call InitTrig______________________A()
    call InitTrig______________________B()
endfunction
//===========================================================================
function RunInitializationTriggers takes nothing returns nothing
    call ConditionalTriggerExecute(gg_trg_______lua______u)
endfunction
//***************************************************************************
//*
//*  Players
//*
//***************************************************************************
function InitCustomPlayerSlots takes nothing returns nothing
    // Player 0
    call SetPlayerStartLocation(Player(0), 0)
    call SetPlayerColor(Player(0), ConvertPlayerColor(0))
    call SetPlayerRacePreference(Player(0), RACE_PREF_HUMAN)
    call SetPlayerRaceSelectable(Player(0), true)
    call SetPlayerController(Player(0), MAP_CONTROL_USER)
    // Player 1
    call SetPlayerStartLocation(Player(1), 1)
    call SetPlayerColor(Player(1), ConvertPlayerColor(1))
    call SetPlayerRacePreference(Player(1), RACE_PREF_ORC)
    call SetPlayerRaceSelectable(Player(1), true)
    call SetPlayerController(Player(1), MAP_CONTROL_USER)
endfunction
function InitCustomTeams takes nothing returns nothing
    // Force: TRIGSTR_003
    call SetPlayerTeam(Player(0), 0)
    call SetPlayerTeam(Player(1), 0)
endfunction
function InitAllyPriorities takes nothing returns nothing
    call SetStartLocPrioCount(0, 1)
    call SetStartLocPrio(0, 0, 1, MAP_LOC_PRIO_HIGH)
    call SetStartLocPrioCount(1, 1)
    call SetStartLocPrio(1, 0, 0, MAP_LOC_PRIO_HIGH)
endfunction
//***************************************************************************
//*
//*  Main Initialization
//*
//***************************************************************************
//===========================================================================
function main takes nothing returns nothing
    call JapiConstantLib_init()
 call initializePlugin()
 call SetCameraBounds(- 3328.0 + GetCameraMargin(CAMERA_MARGIN_LEFT), - 3584.0 + GetCameraMargin(CAMERA_MARGIN_BOTTOM), 3328.0 - GetCameraMargin(CAMERA_MARGIN_RIGHT), 3072.0 - GetCameraMargin(CAMERA_MARGIN_TOP), - 3328.0 + GetCameraMargin(CAMERA_MARGIN_LEFT), 3072.0 - GetCameraMargin(CAMERA_MARGIN_TOP), 3328.0 - GetCameraMargin(CAMERA_MARGIN_RIGHT), - 3584.0 + GetCameraMargin(CAMERA_MARGIN_BOTTOM))
    call SetDayNightModels("Environment\\DNC\\DNCLordaeron\\DNCLordaeronTerrain\\DNCLordaeronTerrain.mdl", "Environment\\DNC\\DNCLordaeron\\DNCLordaeronUnit\\DNCLordaeronUnit.mdl")
    call NewSoundEnvironment("Default")
    call SetAmbientDaySound("LordaeronSummerDay")
    call SetAmbientNightSound("LordaeronSummerNight")
    call SetMapMusic("Music", true, 0)
    call CreateAllItems()
    call CreateAllUnits()
    call InitBlizzard()

call ExecuteFunc("jasshelper__initstructs226778796")
call ExecuteFunc("text___init")
call ExecuteFunc("texture___init")

    call InitGlobals()
    call InitCustomTriggers()
    call ConditionalTriggerExecute(gg_trg_______lua______u) // INLINED!!
endfunction
//***************************************************************************
//*
//*  Map Configuration
//*
//***************************************************************************
function config takes nothing returns nothing
    call SetMapName("TRIGSTR_004")
    call SetMapDescription("TRIGSTR_006")
    call SetPlayers(2)
    call SetTeams(2)
    call SetGamePlacement(MAP_PLACEMENT_TEAMS_TOGETHER)
    call DefineStartLocation(0, 2752.0, - 3008.0)
    call DefineStartLocation(1, 2112.0, - 2240.0)
    // Player setup
    call InitCustomPlayerSlots()
    call SetPlayerSlotAvailable(Player(0), MAP_CONTROL_USER)
    call SetPlayerSlotAvailable(Player(1), MAP_CONTROL_USER)
    call InitGenericPlayerSlots()
    call InitAllyPriorities()
endfunction




//Struct method generated initializers/callers:
function sa__LOGFONT_onDestroy takes nothing returns boolean
local integer this=f__arg_this
            call DestroyFont(s__LOGFONT_font[this])
   return true
endfunction

function jasshelper__initstructs226778796 takes nothing returns nothing
    set st__LOGFONT_onDestroy=CreateTrigger()
    call TriggerAddCondition(st__LOGFONT_onDestroy,Condition( function sa__LOGFONT_onDestroy))


endfunction

