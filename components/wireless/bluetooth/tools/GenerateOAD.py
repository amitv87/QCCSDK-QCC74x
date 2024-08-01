import hashlib
import shutil
import sys

def calculate_sha256(file_path):
    """Calculate the SHA-256 hash of a file and return it as a byte array."""
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
    return sha256_hash.digest()

def create_sha256_file(file_path, output):
    """Copy the file to a new location and append its SHA-256 hash in binary format."""
    try:
        # Copy the original file to the new file
        shutil.copyfile(file_path, output)
        print(f"File copied from {file_path} to {output}")
    except FileNotFoundError:
        print(f"File not found: {file_path}")
        return
    except PermissionError:
        print(f"Permission denied: {file_path} or {output}")
        return
    except Exception as e:
        print(f"Error copying file: {e}")
        return

    # Append the SHA-256 hash to the new file
    try:
        sha256_hash = calculate_sha256(file_path)
        with open(output, "ab") as f:  # Open in append binary mode
            f.write(sha256_hash)
        print(f"SHA-256 hash appended to {output}")
    except Exception as e:
        print(f"Error writing SHA-256 hash to file: {e}")

def main(file_path, output):
    create_sha256_file(file_path, output)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <file_path> <output_path>")
    else:
        main(sys.argv[1], sys.argv[2])
