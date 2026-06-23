# tools/pack.ps1
# Pack map: bundle Lua sources + inject DLL + patch war3map.j + repack into .w3x
#
# Usage:
#   .\tools\pack.ps1                                    # all defaults
#   .\tools\pack.ps1 -DllPath "path\to\yd_japi.dll"    # custom DLL
#   .\tools\pack.ps1 -BaseMap "other.w3m"               # custom base map
#   .\tools\pack.ps1 -OutFile "out\test.w3x"            # custom output
param(
    [string]$DllPath  = "",
    [string]$BaseMap  = "",
    [string]$OutFile  = "",
    [string]$MapDir   = ""
)

$Root = Split-Path -Parent $PSScriptRoot
$W3x2Lni = "$Root\tools\w3x2lni"
$Lua     = "$W3x2Lni\bin\w3x2lni-lua.exe"
$Script  = "$W3x2Lni\script"

# Defaults
if (-not $DllPath)  { $DllPath  = "$Root\src\build\bin\Release\yd_japi.dll" }
if (-not $BaseMap)  { throw "BaseMap is required. Provide a base .w3m/.w3x map via -BaseMap parameter." }
if (-not $OutFile)  { $OutFile  = "$Root\build\output.w3x" }
if (-not $MapDir)   { $MapDir   = "$Root\map" }
$CbSrc   = "$Root\tools\callback"
$WorkDir = "$Root\build\work"

# Resolve to absolute paths
function Resolve-Abs($p) {
    if ([System.IO.Path]::IsPathRooted($p)) { return $p }
    return [System.IO.Path]::GetFullPath([System.IO.Path]::Combine($PWD.Path, $p))
}
$DllPath = Resolve-Abs $DllPath
$BaseMap = Resolve-Abs $BaseMap
$OutFile = Resolve-Abs $OutFile
$MapDir  = Resolve-Abs $MapDir

# Validate
if (-not (Test-Path $BaseMap)) { throw "Base map not found: $BaseMap" }
if (-not (Test-Path $DllPath)) { throw "DLL not found: $DllPath`n  Build first: src\build.bat" }
if (-not (Test-Path $Lua))     { throw "w3x2lni not found: $Lua" }
if (-not (Test-Path $CbSrc))   { throw "callback not found: $CbSrc" }
if (-not (Test-Path $MapDir))  { throw "Map dir not found: $MapDir" }

$MainLua = "$MapDir\war3map.lua"
if (-not (Test-Path $MainLua)) { throw "war3map.lua not found: $MainLua" }

Write-Host "  Base map: $BaseMap"
Write-Host "  Map dir:  $MapDir"
Write-Host "  DLL:      $DllPath"
Write-Host "  Output:   $OutFile"

# ── w3x2lni helper ──────────────────────────────────────────────
function Use-W3x2Lni {
    param($Mode, $Source, $Target)
    $cpath = Join-Path $W3x2Lni 'bin\?.dll'
    $ppath = (Join-Path $Script '?.lua') + ';' + (Join-Path $Script '?\init.lua')
    Push-Location $Script
    try {
        $env:PATH = (Join-Path $W3x2Lni 'bin') + ';' + $env:PATH
        & $Lua -E -e "_W2L_MODE='CLI';package.cpath=[[$cpath]];package.path=[[$ppath]]" main.lua $Mode $Source $Target 2>&1
        if ($LASTEXITCODE -ne 0) { throw "w3x2lni $Mode failed" }
    } finally { Pop-Location }
}

# ── Lua bundler ─────────────────────────────────────────────────
# Collects all .lua files under $MapDir, converts paths to module names,
# and inlines require() calls into a single war3map.lua.

function Get-ModuleName {
    param([string]$FilePath, [string]$BaseDir)
    # map/lib/util.lua → lib.util
    # map/tests/test_suite.lua → tests.test_suite
    # map/war3map.lua → war3map (entry point, not a module)
    $rel = $FilePath.Substring($BaseDir.Length).TrimStart('\', '/')
    $noext = $rel -replace '\\', '/' -replace '\.lua$', ''
    return $noext -replace '/', '.'
}

function Bundle-Lua {
    param([string]$MapDir, [string]$OutPath)

    # Collect all .lua files
    $allFiles = Get-ChildItem -Path $MapDir -Filter "*.lua" -Recurse -File

    # Build module map: name → content
    $moduleMap = @{}
    foreach ($f in $allFiles) {
        $name = Get-ModuleName $f.FullName $MapDir
        $content = Get-Content $f.FullName -Raw -Encoding UTF8
        $moduleMap[$name] = $content
    }

    # Entry point
    $entry = $moduleMap["war3map"]
    if (-not $entry) { throw "war3map.lua not found in $MapDir" }

    # Inline require() calls recursively
    # Pattern: local xxx = require "module.name"  or  require "module.name"
    $requirePattern = '(?m)^(local\s+\w+\s*=\s*)?require\s+"([^"]+)"'

    function Inline-Requires {
        param([string]$code, [hashtable]$modules, [System.Collections.Generic.HashSet[string]]$visited)
        $result = $code
        $matches = [regex]::Matches($code, $requirePattern)
        foreach ($m in $matches) {
            $modName = $m.Groups[2].Value
            if (-not $modules.ContainsKey($modName)) { continue }
            $requirePart = "require `"$modName`""
            if ($visited.Contains($modName)) {
                # Already inlined — replace require with a no-op (module already loaded)
                $result = $result.Replace($requirePart, "nil --[[already loaded: $modName]]")
                continue
            }
            $visited.Add($modName) | Out-Null
            $modCode = Inline-Requires $modules[$modName] $modules $visited
            # Only replace the require "..." part, keep the "local xxx = " prefix
            $replacement = "(function()`r`n$modCode`r`nend)()"
            $result = $result.Replace($requirePart, $replacement)
        }
        return $result
    }

    $visited = [System.Collections.Generic.HashSet[string]]::new()
    $visited.Add("war3map") | Out-Null
    $bundled = Inline-Requires $entry $moduleMap $visited

    Set-Content $OutPath $bundled -Encoding Ascii
    $count = $moduleMap.Count
    Write-Host "  Bundled $count Lua files into war3map.lua"
}

# ── Main pipeline ───────────────────────────────────────────────

# 1. Clean and unpack base map
if (Test-Path $WorkDir) { Remove-Item $WorkDir -Recurse -Force -ErrorAction SilentlyContinue }
New-Item -ItemType Directory $WorkDir -Force | Out-Null
Write-Host '  [1/5] Unpack base map'
Use-W3x2Lni 'unpack' $BaseMap $WorkDir

$JFile = "$WorkDir\war3map.j"
if (-not (Test-Path $JFile)) {
    Set-Content $JFile "// empty war3map.j`r`n" -Encoding Ascii
    Write-Host '  No war3map.j found, created empty'
}

# 2. Bundle Lua sources
Write-Host '  [2/5] Bundle Lua'
Bundle-Lua $MapDir "$WorkDir\war3map.lua"

# 3. war3map.j — no patch needed
Write-Host '  [3/5] war3map.j ready'

# 4. Inject DLL and callback
Write-Host '  [4/5] Inject DLL + callback'
Copy-Item $DllPath "$WorkDir\japi.tga" -Force
Copy-Item $CbSrc  "$WorkDir\callback" -Force

# Update MPQ listfile so w3x2lni includes the new files
$listfile = "$WorkDir\(listfile)"
$needed = @("callback", "japi.tga")
$existing = if (Test-Path $listfile) { Get-Content $listfile -Raw } else { "" }
foreach ($f in $needed) {
    if ($existing -notmatch [regex]::Escape($f)) {
        Add-Content $listfile "`n$f" -Encoding ASCII
    }
}

# DLL loads war3map.lua from script\war3map.lua in the MPQ
# Create script subdirectory and copy the bundled lua there
$scriptDir = "$WorkDir\script"
if (-not (Test-Path $scriptDir)) { New-Item -ItemType Directory $scriptDir -Force | Out-Null }
Copy-Item "$WorkDir\war3map.lua" "$scriptDir\war3map.lua" -Force
if ($existing -notmatch "script\\\\war3map\.lua") {
    Add-Content $listfile "`nscript\war3map.lua" -Encoding ASCII
}

# 5. Repack
Write-Host '  [5/5] Repack'
$outDir = Split-Path -Parent $OutFile
if (-not (Test-Path $outDir)) { New-Item -ItemType Directory $outDir -Force | Out-Null }
Use-W3x2Lni 'pack' $WorkDir $OutFile

Write-Host "  Done -> $OutFile"
