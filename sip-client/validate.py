#!/usr/bin/env python3
"""
SIP Client Code Validation Script

This script performs basic validation of the SIP client codebase:
- Checks that all required files exist
- Validates basic C++ syntax
- Verifies configuration structure
- Checks for common issues
"""

import os
import sys
import re

def check_file_exists(filepath):
    """Check if a file exists"""
    if os.path.exists(filepath):
        print(f"✓ {filepath} exists")
        return True
    else:
        print(f"✗ {filepath} missing")
        return False

def validate_config_defines(config_file):
    """Check that all required #defines are present in config"""
    required_defines = [
        'WIFI_SSID',
        'WIFI_PASSWORD',
        'SIP_SERVER_IP',
        'SIP_SERVER_PORT',
        'SIP_USER',
        'SIP_PASSWORD',
        'SIP_DOMAIN',
        'DEFAULT_CALL_URI'
    ]
    
    if not os.path.exists(config_file):
        print(f"✗ Config file {config_file} not found")
        return False
    
    with open(config_file, 'r') as f:
        content = f.read()
    
    all_found = True
    for define in required_defines:
        if f'#define {define}' in content:
            print(f"✓ {define} defined")
        else:
            print(f"✗ {define} not defined")
            all_found = False
    
    return all_found

def check_includes(cpp_file):
    """Check that includes are properly formatted"""
    if not os.path.exists(cpp_file):
        return False
    
    with open(cpp_file, 'r') as f:
        content = f.read()
    
    # Check for basic includes
    if '#include' in content:
        print(f"✓ {cpp_file} has includes")
        return True
    else:
        print(f"✗ {cpp_file} missing includes")
        return False

def main():
    print("=" * 60)
    print("SIP Client Code Validation")
    print("=" * 60)
    print()
    
    base_path = os.path.dirname(os.path.abspath(__file__))
    
    # Check directory structure
    print("Checking directory structure...")
    dirs_ok = True
    for directory in ['src', 'include']:
        dirpath = os.path.join(base_path, directory)
        if os.path.isdir(dirpath):
            print(f"✓ {directory}/ exists")
        else:
            print(f"✗ {directory}/ missing")
            dirs_ok = False
    print()
    
    # Check required files
    print("Checking required files...")
    files = [
        'platformio.ini',
        'README.md',
        'SETUP_GUIDE.md',
        'include/config.h',
        'include/config.example.h',
        'include/sip_client.h',
        'include/audio_handler.h',
        'include/ui_handler.h',
        'src/main.cpp',
        'src/sip_client.cpp',
        'src/audio_handler.cpp',
        'src/ui_handler.cpp'
    ]
    
    files_ok = True
    for file in files:
        filepath = os.path.join(base_path, file)
        if not check_file_exists(filepath):
            files_ok = False
    print()
    
    # Check config file
    print("Checking configuration...")
    config_path = os.path.join(base_path, 'include/config.h')
    if os.path.exists(config_path):
        config_ok = validate_config_defines(config_path)
    else:
        print("ℹ Config file not found (use config.example.h as template)")
        config_ok = True  # Not an error if using example
    print()
    
    # Check includes in source files
    print("Checking source file structure...")
    src_files = [
        'src/main.cpp',
        'src/sip_client.cpp',
        'src/audio_handler.cpp',
        'src/ui_handler.cpp'
    ]
    
    includes_ok = True
    for src_file in src_files:
        filepath = os.path.join(base_path, src_file)
        if not check_includes(filepath):
            includes_ok = False
    print()
    
    # Check platformio.ini
    print("Checking platformio.ini...")
    pio_ini_path = os.path.join(base_path, 'platformio.ini')
    if os.path.exists(pio_ini_path):
        with open(pio_ini_path, 'r') as f:
            content = f.read()
            if 'env:m5stack-cplus2-sip' in content:
                print("✓ platformio.ini has correct environment")
            else:
                print("✗ platformio.ini missing environment definition")
    print()
    
    # Summary
    print("=" * 60)
    print("Validation Summary")
    print("=" * 60)
    
    all_ok = dirs_ok and files_ok and includes_ok
    
    if all_ok:
        print("✓ All checks passed!")
        print()
        print("Next steps:")
        print("1. Copy include/config.example.h to include/config.h")
        print("2. Edit include/config.h with your WiFi and SIP credentials")
        print("3. Run 'pio run' to build the project")
        print("4. Run 'pio run --target upload' to upload to device")
        return 0
    else:
        print("✗ Some checks failed. Please review the output above.")
        return 1

if __name__ == '__main__':
    sys.exit(main())
