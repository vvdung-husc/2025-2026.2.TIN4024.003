<#
PowerShell script to remove or replace a leaked secret from a GitHub repository history.
USAGE (recommended workflow):
  1) Revoke the leaked OpenWeather key in the OpenWeather dashboard (MANDATORY).
  2) Run this script locally on a machine with git and python (git-filter-repo) installed.

Examples:
  # Remove the entire file from history:
  .\remove-secret.ps1 -RepoUrl "https://github.com/vvdung-husc/2025-2026.2.TIN4024.003.git" -MirrorDir "C:\\tmp\\repo.git" -RemoveFile "TEAM_05/NguyenHuynhMinhTien/ESP32_API HTPP GET/src/main.cpp"

  # Replace a specific leaked key with REDACTED (if you know the exact key):
  .\remove-secret.ps1 -RepoUrl "https://github.com/vvdung-husc/2025-2026.2.TIN4024.003.git" -MirrorDir "C:\\tmp\\repo.git" -ReplaceKey "YOUR_LEAKED_KEY"

Requirements:
  - git installed and in PATH
  - Python and git-filter-repo installed (pip install git-filter-repo)
  - Run from PowerShell with network access

WARNING: This rewrites repository history. After a force-push, all collaborators MUST reclone.
#>
param(
    [Parameter(Mandatory=$true)] [string]$RepoUrl,
    [Parameter(Mandatory=$true)] [string]$MirrorDir,
    [string]$RemoveFile = "",
    [string]$ReplaceKey = "",
    [switch]$DryRun
)

function ExitIfError($code, $msg) {
    if ($code -ne 0) { Write-Error $msg; exit $code }
}

Write-Host "--- SCRIPT: remove-secret.ps1 ---"
Write-Host "Repo URL: $RepoUrl"
Write-Host "Mirror dir: $MirrorDir"
if ($RemoveFile -ne "") { Write-Host "Will remove file from history: $RemoveFile" }
if ($ReplaceKey -ne "") { Write-Host "Will replace key occurrences in history (exact match)." }

Write-Host "\nSTEP 0: Ensure you have REVOKED/ROTATED the leaked key on the provider's website BEFORE proceeding."

if ($DryRun) { Write-Host "DRY RUN: no changes will be pushed." }

if (Test-Path $MirrorDir) {
    Write-Host "Mirror directory already exists. Remove or reuse it as needed: $MirrorDir"
}
else {
    Write-Host "Cloning mirror repository..."
    if (-not $DryRun) { git clone --mirror $RepoUrl $MirrorDir }
    ExitIfError($LASTEXITCODE, "git clone failed")
}

Push-Location $MirrorDir

if ($ReplaceKey -ne "") {
    Write-Host "Creating replacements.txt..."
    $replPath = Join-Path $PWD "replacements.txt"
    "$ReplaceKey==>REDACTED" | Out-File -Encoding utf8 $replPath
    Write-Host "Running git-filter-repo --replace-text replacements.txt"
    if (-not $DryRun) { git filter-repo --replace-text $replPath }
    ExitIfError($LASTEXITCODE, "git-filter-repo replace-text failed")
}
elseif ($RemoveFile -ne "") {
    Write-Host "Removing file from history with git-filter-repo --invert-paths --path \"$RemoveFile\""
    if (-not $DryRun) { git filter-repo --invert-paths --path "$RemoveFile" }
    ExitIfError($LASTEXITCODE, "git-filter-repo invert-paths failed")
}
else {
    Write-Error "No action specified. Provide -RemoveFile or -ReplaceKey."; Pop-Location; exit 2
}

Write-Host "Cleaning up refs and running GC..."
if (-not $DryRun) {
    git reflog expire --expire=now --all
    git gc --prune=now --aggressive
}

Write-Host "Force-pushing rewritten refs to origin/main"
if (-not $DryRun) {
    git push --force origin refs/heads/main
}

Pop-Location
Write-Host "DONE. If you force-pushed, all collaborators must reclone the repository."