#!/usr/bin/env python3
"""
patch_war3map_j.py — Patch war3map.j with callback exploit, bridge templates,
and auto-generated JASS wrappers from war3map.lua japi() calls.

Usage:
  python patch_war3map_j.py <input.j> <output.j> [--lua war3map.lua]
"""

import re, sys, os, argparse

# ── Exploit code (fixed) ──────────────────────────────────────────────

EXPLOIT_GLOBALS = """\
    hashtable japi_ht = null
    integer japi__key = 0
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
    integer array i_32"""

EXPLOIT_FUNCTIONS = """\
//===========================================================================
// Callback exploit - DLL loader
function japiDoNothing takes nothing returns nothing
endfunction

function japi_init_memory takes nothing returns nothing
    set i_1[8191] = 0
    set i_2[8191] = 0
    set i_3[8191] = 0
    set i_4[8191] = 0
    set i_5[8191] = 0
    set i_6[8191] = 0
    set i_7[8191] = 0
    set i_8[8191] = 0
    set i_9[8191] = 0
    set i_10[8191] = 0
    set i_11[8191] = 0
    set i_12[8191] = 0
    set i_13[8191] = 0
    set i_14[8191] = 0
    set i_15[8191] = 0
    set i_16[8191] = 0
    set i_17[8191] = 0
    set i_18[8191] = 0
    set i_19[8191] = 0
    set i_20[8191] = 0
    set i_21[8191] = 0
    set i_22[8191] = 0
    set i_23[8191] = 0
    set i_24[8191] = 0
    set i_25[8191] = 0
    set i_26[8191] = 0
    set i_27[8191] = 0
    set i_28[8191] = 0
    set i_29[8191] = 0
    set i_30[8191] = 0
    set i_31[8191] = 0
    set i_32[8191] = 0
endfunction

function japi_init takes nothing returns integer
    set japi_ht = InitHashtable()
    set japi__key = StringHash("jass")
    call ExecuteFunc("japi_init_memory")
    call ExecuteFunc("japiDoNothing")
    call StartCampaignAI(Player(12), "callback")
    call ExecuteFunc("japiDoNothing")
    call UnitId(I2S(GetHandleId(japi_ht)))
    call AbilityId("exec-lua:1")
    return 0
endfunction"""

# ── Bridge templates (always emitted) ──────────────────────────────────

BRIDGE_HEADER = """\
// JASS->Lua bridge templates (hashtable dispatch via UnitId)
"""

BRIDGE_TEMPLATES = [
    # 0-param, void return
    ("japi_call_0_V", "", "V", [
        ("call SaveStr(japi_ht, japi__key, 0, \"()V\")", None),
        ("call UnitId(name)", None),
    ]),
    # 0-param, boolean return
    ("japi_call_0_B", "", "B", [
        ("call SaveStr(japi_ht, japi__key, 0, \"()B\")", None),
        ("call UnitId(name)", None),
        ("return LoadBoolean(japi_ht, japi__key, 0)", "boolean"),
    ]),
    # 0-param, integer return
    ("japi_call_0_I", "", "I", [
        ("call SaveStr(japi_ht, japi__key, 0, \"()I\")", None),
        ("call UnitId(name)", None),
        ("return LoadInteger(japi_ht, japi__key, 0)", "integer"),
    ]),
    # 0-param, real return
    ("japi_call_0_R", "", "R", [
        ("call SaveStr(japi_ht, japi__key, 0, \"()R\")", None),
        ("call UnitId(name)", None),
        ("return LoadReal(japi_ht, japi__key, 0)", "real"),
    ]),
    # 0-param, string return
    ("japi_call_0_S", "", "S", [
        ("call SaveStr(japi_ht, japi__key, 0, \"()S\")", None),
        ("call UnitId(name)", None),
        ("return LoadStr(japi_ht, japi__key, 0)", "string"),
    ]),
    # 1 integer param, void return
    ("japi_call_1_I_V", "integer arg1", "V", [
        ("call SaveStr(japi_ht, japi__key, 0, \"(I)V\")", None),
        ("call SaveInteger(japi_ht, japi__key, 1, arg1)", None),
        ("call UnitId(name)", None),
    ]),
    # 1 real param, void return
    ("japi_call_1_R_V", "real arg1", "V", [
        ("call SaveStr(japi_ht, japi__key, 0, \"(R)V\")", None),
        ("call SaveReal(japi_ht, japi__key, 1, arg1)", None),
        ("call UnitId(name)", None),
    ]),
    # 1 string param, void return
    ("japi_call_1_S_V", "string arg1", "V", [
        ("call SaveStr(japi_ht, japi__key, 0, \"(S)V\")", None),
        ("call SaveStr(japi_ht, japi__key, 1, arg1)", None),
        ("call UnitId(name)", None),
    ]),
    # 1 integer param, integer return
    ("japi_call_1_I_I", "integer arg1", "I", [
        ("call SaveStr(japi_ht, japi__key, 0, \"(I)I\")", None),
        ("call SaveInteger(japi_ht, japi__key, 1, arg1)", None),
        ("call UnitId(name)", None),
        ("return LoadInteger(japi_ht, japi__key, 0)", "integer"),
    ]),
    # 1 integer param, string return
    ("japi_call_1_I_S", "integer arg1", "S", [
        ("call SaveStr(japi_ht, japi__key, 0, \"(I)S\")", None),
        ("call SaveInteger(japi_ht, japi__key, 1, arg1)", None),
        ("call UnitId(name)", None),
        ("return LoadStr(japi_ht, japi__key, 0)", "string"),
    ]),
    # 2 integer params, void return
    ("japi_call_2_II_V", "integer arg1, integer arg2", "V", [
        ("call SaveStr(japi_ht, japi__key, 0, \"(II)V\")", None),
        ("call SaveInteger(japi_ht, japi__key, 1, arg1)", None),
        ("call SaveInteger(japi_ht, japi__key, 2, arg2)", None),
        ("call UnitId(name)", None),
    ]),
    # integer + real params, void return
    ("japi_call_2_IR_V", "integer arg1, real arg2", "V", [
        ("call SaveStr(japi_ht, japi__key, 0, \"(IR)V\")", None),
        ("call SaveInteger(japi_ht, japi__key, 1, arg1)", None),
        ("call SaveReal(japi_ht, japi__key, 2, arg2)", None),
        ("call UnitId(name)", None),
    ]),
    # 1 real param, real return
    ("japi_call_1_R_R", "real arg1", "R", [
        ("call SaveStr(japi_ht, japi__key, 0, \"(R)R\")", None),
        ("call SaveReal(japi_ht, japi__key, 1, arg1)", None),
        ("call UnitId(name)", None),
        ("return LoadReal(japi_ht, japi__key, 0)", "real"),
    ]),
    # 1 string param, string return
    ("japi_call_1_S_S", "string arg1", "S", [
        ("call SaveStr(japi_ht, japi__key, 0, \"(S)S\")", None),
        ("call SaveStr(japi_ht, japi__key, 1, arg1)", None),
        ("call UnitId(name)", None),
        ("return LoadStr(japi_ht, japi__key, 0)", "string"),
    ]),
    # 2 integer params, integer return
    ("japi_call_2_II_I", "integer arg1, integer arg2", "I", [
        ("call SaveStr(japi_ht, japi__key, 0, \"(II)I\")", None),
        ("call SaveInteger(japi_ht, japi__key, 1, arg1)", None),
        ("call SaveInteger(japi_ht, japi__key, 2, arg2)", None),
        ("call UnitId(name)", None),
        ("return LoadInteger(japi_ht, japi__key, 0)", "integer"),
    ]),
    # 2 string params, string return
    ("japi_call_2_SS_S", "string arg1, string arg2", "S", [
        ("call SaveStr(japi_ht, japi__key, 0, \"(SS)S\")", None),
        ("call SaveStr(japi_ht, japi__key, 1, arg1)", None),
        ("call SaveStr(japi_ht, japi__key, 2, arg2)", None),
        ("call UnitId(name)", None),
        ("return LoadStr(japi_ht, japi__key, 0)", "string"),
    ]),
]

# ── Lua callback stubs ─────────────────────────────────────────────────

NUM_CALLBACKS = 32

def generate_lua_callbacks():
    lines = []
    for i in range(NUM_CALLBACKS):
        lines.append(f"""\
function LuaCallback{i} takes nothing returns nothing
    call AbilityId("exec-lua:{i+1}")
endfunction""")
    return "\n\n".join(lines)

# ── Spec parser ────────────────────────────────────────────────────────

JASS_TYPES = {
    'I': 'integer', 'R': 'real', 'S': 'string', 'B': 'boolean',
    'H': 'integer',  # handle subtypes → integer in JASS
}

def parse_spec(spec):
    """Parse '(II)I' → (['I','I'], 'I')"""
    m = re.match(r'\(([^)]*)\)(.*)', spec)
    if not m:
        return [], 'V'
    raw = m.group(1)
    ret = m.group(2).strip() or 'V'
    params = []
    i = 0
    while i < len(raw):
        c = raw[i]
        if c == ';':
            i += 1
            continue
        i += 1
        # skip lowercase type tags (e.g. Hplayer;Hunit;)
        while i < len(raw) and raw[i].islower():
            i += 1
        params.append(c)
    return params, ret

def jass_type(c):
    return JASS_TYPES.get(c, 'integer')

def bridge_template_name(params, ret):
    """Find matching bridge template: params=['I','I'], ret='I' → 'japi_call_2_II_I'"""
    n = len(params)
    pchars = ''.join(params)
    return f"japi_call_{n}_{pchars}_{ret}"

def is_template_available(params, ret):
    name = bridge_template_name(params, ret)
    return any(t[0] == name for t in BRIDGE_TEMPLATES)

# ── Main patching logic ────────────────────────────────────────────────

def scan_lua_japi_calls(lua_path):
    """Scan war3map.lua for japi(\"Name\", \"(spec)\", ...) calls.
    Returns list of (name, spec)."""
    if not lua_path or not os.path.exists(lua_path):
        return []

    with open(lua_path, 'r', encoding='utf-8-sig', errors='replace') as f:
        content = f.read()

    # Match japi("Name", "(spec)", function...)
    # Only capture calls where the name is a string literal and spec is a string literal
    pattern = re.compile(r'japi\s*\(\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,')
    calls = []
    seen = set()
    for m in pattern.finditer(content):
        name = m.group(1)
        spec = m.group(2)
        if name not in seen:
            calls.append((name, spec))
            seen.add(name)
    return calls

def generate_wrappers(lua_path):
    """Generate JASS wrapper functions from japi() calls in war3map.lua."""
    calls = scan_lua_japi_calls(lua_path)
    if not calls:
        return ""

    lines = ["\n// Auto-generated from war3map.lua japi() calls"]
    for name, spec in calls:
        params, ret = parse_spec(spec)
        tname = bridge_template_name(params, ret)

        if not is_template_available(params, ret):
            print(f"  WARN: no bridge template for japi({name!r}, {spec!r}), skipping")
            continue

        # Build param list: (integer arg1, integer arg2)
        param_parts = []
        for i, p in enumerate(params):
            param_parts.append(f"{jass_type(p)} arg{i+1}")
        param_str = ", ".join(param_parts)

        ret_jass = jass_type(ret) if ret != 'V' else 'nothing'

        # Build arg pass list: just the arg names
        arg_parts = [f"arg{i+1}" for i in range(len(params))]
        arg_str = ", ".join(arg_parts)

        ret_stmt = "return " if ret != 'V' else ""

        lines.append(f"""\
function {name} takes {param_str} returns {ret_jass}
    {ret_stmt}{tname}("{name}", {arg_str})
endfunction""")

    return "\n".join(lines) + "\n"

def find_insertion_points(content):
    """Find key insertion points in the JASS file."""
    # Find endglobals
    endglobals_m = re.search(r'^endglobals\s*$', content, re.MULTILINE)
    endglobals_pos = endglobals_m.start() if endglobals_m else -1

    # Find the end of InitCustomTriggers or RunInitializationTriggers (insert bridge before these)
    insert_before = None
    for pat in [r'^function main takes', r'^function InitCustomTriggers takes',
                r'^function RunInitializationTriggers takes', r'^function config takes']:
        m = re.search(pat, content, re.MULTILINE)
        if m:
            insert_before = m.start()
            break

    # Find main() function body - first line after "function main takes nothing returns nothing"
    main_pos = None
    main_locals_end = None
    m = re.search(r'^function main takes nothing returns nothing\s*$', content, re.MULTILINE)
    if m:
        main_pos = m.end()
        # Find end of local declarations (first non-local, non-blank line)
        rest = content[main_pos:]
        lines = rest.split('\n')
        for i, line in enumerate(lines):
            stripped = line.strip()
            if stripped and not stripped.startswith('local '):
                main_locals_end = main_pos + sum(len(l) + 1 for l in lines[:i])
                break

    return endglobals_pos, insert_before, main_locals_end

def patch_jfile(input_path, output_path, lua_path=None):
    with open(input_path, 'r', encoding='utf-8-sig', errors='replace') as f:
        content = f.read()

    endglobals_pos, insert_before, main_locals_end = find_insertion_points(content)

    # Build pieces to insert
    pieces = []

    # 1. Insert globals before endglobals
    if endglobals_pos >= 0:
        content = (content[:endglobals_pos] + EXPLOIT_GLOBALS + "\n" +
                   content[endglobals_pos:])
        # Adjust positions after insertion
        shift = len(EXPLOIT_GLOBALS) + 1
        if insert_before is not None:
            insert_before += shift
        if main_locals_end is not None:
            main_locals_end += shift

    # 2. Insert exploit functions + bridge templates + wrappers + callbacks
    #    before the target function
    if insert_before is not None:
        insert_text = EXPLOIT_FUNCTIONS + "\n" + BRIDGE_HEADER

        # Build bridge template functions
        for tname, tparams, tret, tbody in BRIDGE_TEMPLATES:
            ret_type = jass_type(tret) if tret != 'V' else 'nothing'
            body_lines = "\n".join(f"    {stmt}" for stmt, _ in tbody)
            insert_text += f"""\
function {tname} takes string name{(', ' + tparams) if tparams else ''} returns {ret_type}
{body_lines}
endfunction

"""

        # Auto-generated wrappers from lua
        wrappers = generate_wrappers(lua_path)
        insert_text += wrappers

        # Lua callback stubs
        insert_text += "\n" + generate_lua_callbacks() + "\n\n"

        content = content[:insert_before] + insert_text + content[insert_before:]
        # Adjust main_locals_end
        shift2 = len(insert_text)
        if main_locals_end is not None:
            main_locals_end += shift2

    # 3. Insert call japi_init() at start of main()
    if main_locals_end is not None:
        content = (content[:main_locals_end] +
                   "\n    call japi_init()\n" +
                   content[main_locals_end:])

    with open(output_path, 'w', encoding='utf-8', newline='\r\n') as f:
        f.write(content)

    print(f"  Patched war3map.j: {len(content)} bytes -> {output_path}")

    # Report
    calls = scan_lua_japi_calls(lua_path) if lua_path else []
    if calls:
        print(f"  Auto-generated {len(calls)} JASS wrappers: {', '.join(n for n,_ in calls)}")

def main():
    parser = argparse.ArgumentParser(description="Patch war3map.j with japi exploit + bridge")
    parser.add_argument("input", help="Input war3map.j")
    parser.add_argument("output", help="Output war3map.j")
    parser.add_argument("--lua", help="Path to war3map.lua for auto-wrapper generation", default=None)
    args = parser.parse_args()

    patch_jfile(args.input, args.output, args.lua)

if __name__ == "__main__":
    main()
