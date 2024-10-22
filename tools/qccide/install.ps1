
# install Chocolatey
if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
    Write-Host "Chocolatey is not installed. Installing Chocolatey..."
    Set-ExecutionPolicy Bypass -Scope Process -Force; Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Chocolatey installation failed. Exiting."
        exit 1
    }
}else {
    Write-Host "Chocolatey is already installed."
}

# install Git
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Host "Git is not installed. Installing Git..."

    choco install git -y
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Git installation failed. Exiting."
        exit 1
    }

    Write-Host "Git has been successfully installed."
} else {
    Write-Host "Git is already installed."
}

# install Python 3
$pythonInstalled = Get-Command python -ErrorAction SilentlyContinue

if ($pythonInstalled) {
    $pythonVersion = (& $pythonInstalled --version 2>&1)
    if ($pythonVersion -notlike "*Python was not found*") {
        Write-Host "Python is installed. Version: $pythonVersion"
    } else {
        Write-Host "Failed to retrieve Python version. Error: $pythonVersion"

        choco install python --version=3.11.5 -y
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Python 3 installation failed. Exiting."
            exit 1
        }

        Write-Host "Python 3 has been successfully installed."
    }
} else {
    Write-Host "Python is not installed."
}


# install Visual Studio Code
if (-not (Get-Command code -ErrorAction SilentlyContinue)) {
    Write-Host "Visual Studio Code is not installed. Installing Visual Studio Code..."

    choco install vscode -y
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Visual Studio Code installation failed. Exiting."
        exit 1
    }

    Write-Host "Visual Studio Code has been successfully installed."
} else {
    Write-Host "Visual Studio Code is already installed."
}

# Add the installation path to the system environment variable (Path)
Import-Module $env:ChocolateyInstall\helpers\chocolateyProfile.psm1
refreshenv

# get all .vsix 
$vsixFiles = Get-ChildItem -Filter *.vsix

if ($vsixFiles.Count -eq 0) {
    Write-Host "No .vsix files found in the current directory."
} else {
    Write-Host "Found the following .vsix files in the current directory:"
    foreach ($file in $vsixFiles) {
        Write-Host $file.Name
    }
    # get last .vsix 
    $lastVsixFile = $vsixFiles[-1].Name
    Write-Host "The last .vsix file: $lastVsixFile"

    code --install-extension $lastVsixFile
}

Read-Host "Press Enter to exit..."


