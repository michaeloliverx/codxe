param()
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..'))
$buildRoot = Join-Path $repoRoot 'build\iw3-native-bots'
$packageRoot = [IO.Path]::GetFullPath((Join-Path $buildRoot 'package'))
$modRoot = Join-Path $buildRoot 'mod'
$pluginPath = Join-Path $repoRoot 'build\Release\bin\codxe.xex'
$archivePath = Join-Path $buildRoot 'iw3-native-bots-experiment.zip'
if (!$packageRoot.StartsWith($repoRoot + '\', [StringComparison]::OrdinalIgnoreCase) -or
    $packageRoot -ne (Join-Path $buildRoot 'package')) {
    throw 'Package cleanup target is outside the expected workspace directory'
}
if (!(Test-Path -LiteralPath $pluginPath) -or !(Test-Path -LiteralPath (Join-Path $modRoot 'maps\mp\bots\_bot_internal.gsc'))) {
    throw 'Build the Release plugin and run prepare-mod.cjs first'
}
$nativeSources = @(
    (Join-Path $repoRoot 'src\game\iw3\mp\components\experimental\bot_native.cpp'),
    (Join-Path $repoRoot 'src\game\iw3\mp\components\experimental\bot_native_core.h'),
    (Join-Path $repoRoot 'src\game\iw3\mp\components\experimental\bot_native_controller.h'),
    (Join-Path $repoRoot 'src\game\iw3\mp\components\experimental\bot_native_controller.inl'),
    (Join-Path $repoRoot 'src\game\iw3\mp\components\experimental\bot_native.h'),
    (Join-Path $repoRoot 'src\detour.h'),
    (Join-Path $repoRoot 'src\game\iw3\mp\components\sv_bots.cpp'),
    (Join-Path $repoRoot 'src\game\iw3\mp\components\gsc.cpp'),
    (Join-Path $repoRoot 'src\game\iw3\mp\symbols.h')
)
foreach ($sourcePath in $nativeSources) {
    if ((Get-Item -LiteralPath $sourcePath).LastWriteTimeUtc -gt (Get-Item -LiteralPath $pluginPath).LastWriteTimeUtc) {
        throw "Release plugin is older than $sourcePath; rebuild before packaging"
    }
}
if (Test-Path -LiteralPath $packageRoot) { Remove-Item -LiteralPath $packageRoot -Recurse -Force }
$gameRoot = Join-Path $packageRoot '_codxe\games\iw3\mods'
New-Item -ItemType Directory -Force -Path $gameRoot | Out-Null
Copy-Item -LiteralPath $modRoot -Destination (Join-Path $gameRoot 'bot_warfare_native') -Recurse
Copy-Item -LiteralPath $pluginPath -Destination (Join-Path $packageRoot '_codxe\codxe.xex')
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'README.md') -Destination (Join-Path $packageRoot 'README.md')
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'T5-COMPARISON.md') -Destination (Join-Path $packageRoot 'T5-COMPARISON.md')
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'CAPTURE-ANALYSIS.md') -Destination (Join-Path $packageRoot 'CAPTURE-ANALYSIS.md')
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'NATIVE-CONTROLLER.md') -Destination (Join-Path $packageRoot 'NATIVE-CONTROLLER.md')
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'CRASH-91D5AF64.md') -Destination (Join-Path $packageRoot 'CRASH-91D5AF64.md')
Copy-Item -LiteralPath (Join-Path $buildRoot 'source-manifest.json') -Destination (Join-Path $packageRoot 'source-manifest.json')

Add-Type -AssemblyName System.IO.Compression.FileSystem
if (Test-Path -LiteralPath $archivePath) { Remove-Item -LiteralPath $archivePath -Force }
[IO.Compression.ZipFile]::CreateFromDirectory($packageRoot, $archivePath, [IO.Compression.CompressionLevel]::Optimal, $false)
$zip = [IO.Compression.ZipFile]::OpenRead($archivePath)
$entries = 0
try {
    foreach ($entry in $zip.Entries) {
        if (!$entry.Name) { continue }
        $filePath = Join-Path $packageRoot $entry.FullName.Replace('/', '\')
        $stream = $entry.Open()
        $sha = [Security.Cryptography.SHA256]::Create()
        try { $zipHash = [BitConverter]::ToString($sha.ComputeHash($stream)).Replace('-', '') }
        finally { $stream.Dispose(); $sha.Dispose() }
        if ($zipHash -ne (Get-FileHash -LiteralPath $filePath -Algorithm SHA256).Hash) {
            throw "Archive content mismatch: $($entry.FullName)"
        }
        ++$entries
    }
} finally { $zip.Dispose() }
if ($entries -ne @(Get-ChildItem -LiteralPath $packageRoot -File -Recurse).Count) {
    throw 'Archive file count mismatch'
}
$result = [ordered]@{
    archive = $archivePath
    entries = $entries
    bytes = (Get-Item -LiteralPath $archivePath).Length
    archive_sha256 = (Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash
    plugin_sha256 = (Get-FileHash -LiteralPath $pluginPath -Algorithm SHA256).Hash
}
$result | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $buildRoot 'package-manifest.json')
$result | ConvertTo-Json
