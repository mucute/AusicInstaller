#!/usr/bin/env python3
import sys
import os

def append_zip_to_exe(exe_path, zip_path):
    """Append zip file content to exe file"""
    
    # Check if files exist
    if not os.path.exists(exe_path):
        print(f"Error: Executable file not found: {exe_path}")
        return False
        
    if not os.path.exists(zip_path):
        print(f"Error: Zip file not found: {zip_path}")
        return False
    
    try:
        # Read zip file content
        with open(zip_path, 'rb') as zip_file:
            zip_data = zip_file.read()
        
        # Verify it's a valid ZIP file by checking signature
        if len(zip_data) < 4 or zip_data[:4] != b'PK\x03\x04':
            print(f"Error: {zip_path} is not a valid ZIP file")
            return False
        
        # Get original exe size before appending
        original_size = os.path.getsize(exe_path)
        
        # Append zip data to exe file
        with open(exe_path, 'ab') as exe_file:
            exe_file.write(zip_data)
        
        print(f"Successfully appended {len(zip_data)} bytes from {zip_path} to {exe_path}")
        print(f"Original exe size: {original_size}, ZIP starts at offset: {original_size}")
        return True
        
    except Exception as e:
        print(f"Error: Failed to append zip file: {str(e)}")
        return False

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python append_zip.py <exe_path> <zip_path>")
        sys.exit(1)
    
    exe_path = sys.argv[1]
    zip_path = sys.argv[2]
    
    success = append_zip_to_exe(exe_path, zip_path)
    sys.exit(0 if success else 1)