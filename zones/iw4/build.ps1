param(
    [Parameter(Mandatory = $true)][string]$Linker,
    [Parameter(Mandatory = $true)][string]$GameDirectory,
    [Parameter(Mandatory = $true)][string]$StockPatch,
    [string]$Unlinker
)

$ErrorActionPreference = 'Stop'
$expectedPatch = 'FC2F2702996BF9A26C482BCA7FD5151F5F6EC2BB0344ADCE79CE6447BB2A11C7'
if ((Get-FileHash -LiteralPath $StockPatch -Algorithm SHA256).Hash -ne $expectedPatch) {
    throw 'Expected the original TU6 patch_mp.ff from TU_10LC20N_0000018000000.00000000001GA.'
}
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$output = Join-Path $repo 'build/iw4-zones'
$resources = Join-Path $repo 'resources/iw4/_codxe'
New-Item -ItemType Directory -Force -Path $output | Out-Null
# OAT follows loadMenu entries as source files. Generate untouched TU6 menus
# and its shared function definitions instead of keeping stock copies in Git.
if (!$Unlinker) {
    $Unlinker = Join-Path (Split-Path -Parent $Linker) 'Unlinker.exe'
}
$stockDump = Join-Path $output 'stock-menu-dump'
$stockMenus = Join-Path $output 'stock-menu-dependencies'
$generated = Join-Path $output 'generated'
& $Unlinker --include-assets menu,menulist --output-folder $stockDump $StockPatch
if ($LASTEXITCODE) { throw 'IW4 stock menu extraction failed' }
$menuList = [IO.File]::ReadAllText((Join-Path $stockDump 'ui_mp/patch_mp_menus.txt'))
# Expose only the list's unchanged dependencies; other patch menus stay native.
foreach ($entry in [regex]::Matches($menuList, 'loadMenu\s*\{\s*"([^"]+)"\s*\}')) {
    $menuPath = $entry.Groups[1].Value
    if (Test-Path -LiteralPath (Join-Path $PSScriptRoot "zone_raw/codxe_patch_mp/$menuPath") -PathType Leaf) {
        continue
    }
    $destination = Join-Path $stockMenus $menuPath
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destination) | Out-Null
    Copy-Item -LiteralPath (Join-Path $stockDump $menuPath) -Destination $destination -Force
}
$end = $menuList.LastIndexOf('}')
if ($end -lt 0) { throw 'IW4 stock patch menu list is missing its closing brace' }
$additionalMenus = @('ui_mp/settings_usermaps.menu', 'ui_mp/mods_menu.menu')
$loads = ($additionalMenus | ForEach-Object { '    loadMenu { "' + $_ + '" }' }) -join "`n"
New-Item -ItemType Directory -Force -Path (Join-Path $generated 'ui_mp') | Out-Null
[IO.File]::WriteAllText((Join-Path $generated 'ui_mp/patch_mp_menus.txt'), $menuList.Insert($end, $loads + "`n"))

$arguments = @(
    '--menu-permissive', '--menu-no-optimization',
    '--base-folder', $PSScriptRoot,
    '--asset-search-path', "?base?/zone_raw/?project?;?base?/raw;$generated;$stockMenus",
    '--output-folder', $output
)
& $Linker @arguments --load $StockPatch --load (Join-Path $GameDirectory 'common_mp.ff') --load (Join-Path $GameDirectory 'ui_mp.ff') codxe_patch_mp
if ($LASTEXITCODE) { throw 'IW4 patch link failed' }
New-Item -ItemType Directory -Force -Path (Join-Path $resources 'zone') | Out-Null
Copy-Item -LiteralPath (Join-Path $output 'patch_mp.ff') -Destination (Join-Path $resources 'zone/patch_mp.ff') -Force
