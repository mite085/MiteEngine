param(
    [string]$BuildType = "Release"
)

Write-Host "=== MiteEngine Build Script ===" -ForegroundColor Green

# Function to load Visual Studio environment
function Load-VisualStudioEnvironment {
    Write-Host "Loading Visual Studio development environment..." -ForegroundColor Cyan
    
    # Possible Visual Studio installation paths
    $vsPaths = @(
        "C:\Program Files\Microsoft Visual Studio\2022\BuildTools",
        "C:\Program Files\Microsoft Visual Studio\2022\Community", 
        "C:\Program Files\Microsoft Visual Studio\2022\Professional",
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise"
    )
    
    # Find VsDevCmd.bat
    $devCmdPath = $null
    foreach ($path in $vsPaths) {
        $testPath = "$path\Common7\Tools\VsDevCmd.bat"
        if (Test-Path $testPath) {
            $devCmdPath = $testPath
            Write-Host "Found Visual Studio at: $path" -ForegroundColor Green
            break
        }
    }
    
    # If not found, warn and return false
    if (-not $devCmdPath) {
        Write-Host "Warning: Visual Studio development environment not found" -ForegroundColor Yellow
        Write-Host "Will try to use system PATH..." -ForegroundColor Yellow
        return $false
    }
    
    # Execute VsDevCmd.bat and capture environment variables
    Write-Host "Setting up MSVC environment..." -ForegroundColor Cyan
    cmd.exe /c "`"$devCmdPath`" -arch=x64 -host_arch=x64 >nul 2>&1 && set" | ForEach-Object {
        if ($_ -match "^(.*?)=(.*)$") {
            Set-Item -Path "env:\$($matches[1])" -Value $matches[2]
        }
    }
    
    return $true
}
# Load Visual Studio environment
$vsLoaded = Load-VisualStudioEnvironment
# Check if cl.exe is available
Write-Host "Checking compiler..." -ForegroundColor Cyan
if (Get-Command cl -ErrorAction SilentlyContinue) {
    $clVersion = cl 2>&1 | Select-String "Version"
    Write-Host "✓ C++ Compiler: $clVersion" -ForegroundColor Green
} else {
    Write-Host "✗ C++ compiler (cl.exe) not found in PATH" -ForegroundColor Red
    Write-Host "Please run this script from Visual Studio Developer Command Prompt" -ForegroundColor Yellow
    exit 1
}

# 1. Clean previous builds
if (Test-Path "build") {
    Write-Host "Cleaning old build..." -ForegroundColor Yellow
    Remove-Item -Path "build" -Recurse -Force -ErrorAction SilentlyContinue
}

# 2. Check required tools
$requiredTools = @("cmake", "ninja", "python", "git")
foreach ($tool in $requiredTools) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        Write-Host "Error: $tool not found" -ForegroundColor Red
        exit 1
    }
}

# 3. Verify Python version
try {
    $pythonVersion = python --version 2>&1
    Write-Host "Python version: $pythonVersion" -ForegroundColor Cyan
} catch {
    Write-Host "Warning: Could not get Python version" -ForegroundColor Yellow
}

# 4. Configure CMake
Write-Host "`n=== Configuring CMake ===" -ForegroundColor Green
New-Item -Path "build" -ItemType Directory -Force | Out-Null
Set-Location "build"

# 构建命令
$cmakeArgs = @(
    "-G", "Ninja",
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-DSHADERC_SKIP_TESTS=ON",
    "-DSHADERC_SKIP_EXAMPLES=ON",
    ".."
)

Write-Host "Running: cmake $($cmakeArgs -join ' ')" -ForegroundColor Gray
cmake @cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed" -ForegroundColor Red
    exit $LASTEXITCODE
}

# 5. Build project
Write-Host "`n=== Building project ===" -ForegroundColor Green
$cpuCores = [System.Environment]::ProcessorCount
Write-Host "Using $cpuCores CPU cores" -ForegroundColor Cyan

ninja -j $cpuCores

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed" -ForegroundColor Red
    exit $LASTEXITCODE
}

# 6. Run tests
Write-Host "`n=== Running tests ===" -ForegroundColor Green
$testFiles = Get-ChildItem -Path . -Recurse -Filter "*test*.exe" -File

if ($testFiles.Count -eq 0) {
    Write-Host "No test executables found" -ForegroundColor Yellow
} else {
    foreach ($test in $testFiles) {
        Write-Host "Test: $($test.Name)" -ForegroundColor Cyan
        & $test.FullName
        
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Test failed: $($test.Name)" -ForegroundColor Red
            exit $LASTEXITCODE
        }
    }
    Write-Host "`n✓ All tests passed" -ForegroundColor Green
}

# 7. Show build artifacts
Write-Host "`n=== Build artifacts ===" -ForegroundColor Green
Get-ChildItem -Path . -Recurse -Filter "*.exe" -File | ForEach-Object {
    $sizeKB = [math]::Round($_.Length / 1KB, 2)
    Write-Host "  $($_.Name) - $sizeKB KB"
}

# Return to project root
Set-Location ..

Write-Host "`n=== Build completed ===" -ForegroundColor Green
