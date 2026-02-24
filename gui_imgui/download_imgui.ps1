# Download Dear ImGui
Write-Host "Downloading Dear ImGui..." -ForegroundColor Cyan

$imguiVersion = "v1.90.4"
$downloadUrl = "https://github.com/ocornut/imgui/archive/refs/tags/$imguiVersion.zip"
$outputFile = "imgui.zip"

# Download
Invoke-WebRequest -Uri $downloadUrl -OutFile $outputFile

# Extract
Write-Host "Extracting..." -ForegroundColor Cyan
Expand-Archive -Path $outputFile -DestinationPath "." -Force

# Rename folder
$extractedFolder = "imgui-$($imguiVersion.TrimStart('v'))"
if (Test-Path "imgui") {
    Remove-Item -Path "imgui" -Recurse -Force
}
Rename-Item -Path $extractedFolder -NewName "imgui"

# Clean up
Remove-Item $outputFile

Write-Host "Dear ImGui downloaded successfully!" -ForegroundColor Green
Write-Host "Location: .\imgui\" -ForegroundColor Yellow
