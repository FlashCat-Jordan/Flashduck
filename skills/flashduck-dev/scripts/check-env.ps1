# Flashduck environment check for Windows PowerShell.
# Usage: powershell -ExecutionPolicy Bypass -File check-env.ps1
$ErrorActionPreference = "Continue"
$fail = 0
function Ok($m)   { Write-Host "  [OK]   $m" }
function Miss($m) { Write-Host "  [MISS] $m"; $script:fail = 1 }
function Warn($m) { Write-Host "  [WARN] $m" }

$repo = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..")).Path
Write-Host "Flashduck environment check"
Write-Host "Repository: $repo`n"

Write-Host "Basic tools"
foreach ($cmd in @("git", "python", "cmake", "ninja")) {
  if (Get-Command $cmd -ErrorAction SilentlyContinue) { Ok "$cmd found" } else { Miss "$cmd not found" }
}

Write-Host "`nESP-IDF"
if ($env:IDF_PATH -and (Test-Path (Join-Path $env:IDF_PATH "export.ps1"))) {
  Ok "IDF_PATH=$env:IDF_PATH"
  try {
    $ver = (& idf.py --version 2>$null) -join " "
    Write-Host "  [INFO] $ver"
  } catch { Miss "idf.py not runnable" }
} else {
  Miss "IDF_PATH not set. Use the ESP-IDF PowerShell shortcut or run export.ps1."
}

Write-Host "`nProject paths"
foreach ($p in @("firmware\voice\CMakeLists.txt", "firmware\duckclaw\CMakeLists.txt", "skills\flashduck-dev\SKILL.md")) {
  if (Test-Path (Join-Path $repo $p)) { Ok $p } else { Miss $p }
}

Write-Host "`nPath check"
if ($repo -match '[^\x00-\x7F]') {
  Warn "repository path contains non-ASCII characters; build from an ASCII path (e.g. C:\flashduck-build)"
} else { Ok "repository path is ASCII" }

Write-Host "`nSerial ports"
$ports = Get-CimInstance Win32_SerialPort -ErrorAction SilentlyContinue
if ($ports) { $ports | ForEach-Object { Write-Host "  [INFO] $($_.DeviceID) $($_.Description)" } }
else { Warn "no COM port found; connect the board with a data cable" }

if ($fail -gt 0) {
  Write-Host "`nResult: MISSING dependencies. See references/environment.md."
  exit 1
}
Write-Host "`nResult: READY (or ready with warnings)."
exit 0
