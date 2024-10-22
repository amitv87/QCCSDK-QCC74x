import os
import subprocess
import shutil
import sys

# Check if the correct number of arguments are provided
if len(sys.argv) != 2:
    print("Usage: python checkout_matter_code.py <directory_path>")
    sys.exit(1)

# Get the directory path from the command line
directory_path = sys.argv[1]

# Get the present working directory
pwd = os.getcwd()
print(f"Present working directory: {pwd}")

# Change directory to the specified path
os.chdir(directory_path)
print(f"Changed directory to: {os.getcwd()}")

# Clone the git repository with branch v1.3-branch
subprocess.run(["git", "clone", "-b", "v1.3-branch", "https://github.com/project-chip/connectedhomeip.git"])

# Change directory to connectedhomeip
os.chdir("connectedhomeip")
print(f"Changed directory to: {os.getcwd()}")

# Checkout the commit 9d1963d8dbfc0024d21b8853790388d96e705f5a
subprocess.run(["git", "checkout", "9d1963d8dbfc0024d21b8853790388d96e705f5a"])

# Copy the patch file from the ../patches directory to the current directory
shutil.copy(os.path.join(pwd, "../patches", "0001-Matter-support-over-Wi-Fi-and-Thread.patch"), ".")

# Apply the patch
subprocess.run(["git", "am", "0001-Matter-support-over-Wi-Fi-and-Thread.patch"])

# Update git submodules recursively
subprocess.run(["git", "submodule", "update", "--init", "--recursive"])

# Check the git status
subprocess.run(["git", "status"])

# Change directory back to the present working directory
os.chdir(pwd)
print(f"Changed directory back to: {os.getcwd()}")
