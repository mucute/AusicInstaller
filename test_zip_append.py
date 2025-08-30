#!/usr/bin/env python3
"""
测试脚本：验证新的ZIP附加和读取功能
"""

import os
import sys
import struct
import tempfile
import shutil

# 添加当前目录到路径，以便导入append_zip模块
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from append_zip import append_zip_to_exe, MAGIC_SIGNATURE

def create_test_zip(zip_path):
    """创建一个测试用的ZIP文件"""
    import zipfile
    
    with zipfile.ZipFile(zip_path, 'w') as zf:
        zf.writestr('test.txt', 'This is a test file for ZIP append functionality.')
        zf.writestr('folder/nested.txt', 'This is a nested file in the ZIP.')
    
    return os.path.exists(zip_path)

def create_test_exe(exe_path):
    """创建一个测试用的EXE文件（实际上是一个简单的二进制文件）"""
    test_exe_content = b'\x4D\x5A\x90\x00'  # PE文件头的开始
    test_exe_content += b'\x00' * 1000  # 填充一些数据
    test_exe_content += b'This is a test executable file for testing ZIP append functionality.'
    
    with open(exe_path, 'wb') as f:
        f.write(test_exe_content)
    
    return os.path.exists(exe_path)

def read_zip_metadata(exe_path):
    """从EXE文件中读取ZIP元数据（模拟C++代码的逻辑）"""
    if not os.path.exists(exe_path):
        return None
    
    file_size = os.path.getsize(exe_path)
    metadata_size = 14 + 8 + 8 + 4  # magic + offset + size + metadata_size
    
    if file_size < metadata_size:
        return None
    
    with open(exe_path, 'rb') as f:
        # 读取文件末尾的元数据
        f.seek(file_size - metadata_size)
        metadata_block = f.read(metadata_size)
        
        if len(metadata_block) != metadata_size:
            return None
        
        # 检查魔术签名
        magic = metadata_block[:14]
        if magic == MAGIC_SIGNATURE:
            # 解析元数据
            metadata = metadata_block[14:]
            zip_offset, zip_size, meta_size = struct.unpack('<QQI', metadata)
            
            return {
                'zip_offset': zip_offset,
                'zip_size': zip_size,
                'metadata_size': meta_size,
                'magic_found': True
            }
    
    return {'magic_found': False}

def extract_zip_from_exe(exe_path, output_zip_path):
    """从EXE文件中提取ZIP文件（模拟C++代码的逻辑）"""
    metadata = read_zip_metadata(exe_path)
    
    if not metadata or not metadata.get('magic_found'):
        print("Error: No valid ZIP metadata found in executable")
        return False
    
    zip_offset = metadata['zip_offset']
    zip_size = metadata['zip_size']
    
    with open(exe_path, 'rb') as exe_file:
        exe_file.seek(zip_offset)
        zip_data = exe_file.read(zip_size)
        
        if len(zip_data) != zip_size:
            print(f"Error: Expected {zip_size} bytes, got {len(zip_data)} bytes")
            return False
        
        # 验证ZIP文件头
        if len(zip_data) < 4 or zip_data[:4] != b'PK\x03\x04':
            print("Error: Invalid ZIP signature in extracted data")
            return False
        
        with open(output_zip_path, 'wb') as zip_file:
            zip_file.write(zip_data)
    
    return True

def test_zip_append_functionality():
    """测试完整的ZIP附加和读取功能"""
    print("=== 测试ZIP附加和读取功能 ===")
    
    # 创建临时目录
    temp_dir = tempfile.mkdtemp()
    
    try:
        # 文件路径
        test_exe = os.path.join(temp_dir, 'test.exe')
        test_zip = os.path.join(temp_dir, 'test.zip')
        extracted_zip = os.path.join(temp_dir, 'extracted.zip')
        
        # 步骤1: 创建测试文件
        print("1. 创建测试文件...")
        if not create_test_exe(test_exe):
            print("   ❌ 创建测试EXE失败")
            return False
        
        if not create_test_zip(test_zip):
            print("   ❌ 创建测试ZIP失败")
            return False
        
        print(f"   ✅ 测试文件创建成功")
        print(f"   - EXE文件大小: {os.path.getsize(test_exe)} bytes")
        print(f"   - ZIP文件大小: {os.path.getsize(test_zip)} bytes")
        
        # 步骤2: 附加ZIP到EXE
        print("\n2. 附加ZIP到EXE...")
        if not append_zip_to_exe(test_exe, test_zip):
            print("   ❌ ZIP附加失败")
            return False
        
        print(f"   ✅ ZIP附加成功")
        print(f"   - 附加后EXE大小: {os.path.getsize(test_exe)} bytes")
        
        # 步骤3: 读取元数据
        print("\n3. 读取ZIP元数据...")
        metadata = read_zip_metadata(test_exe)
        
        if not metadata or not metadata.get('magic_found'):
            print("   ❌ 元数据读取失败")
            return False
        
        print("   ✅ 元数据读取成功")
        print(f"   - ZIP偏移: {metadata['zip_offset']}")
        print(f"   - ZIP大小: {metadata['zip_size']}")
        print(f"   - 元数据大小: {metadata['metadata_size']}")
        
        # 步骤4: 提取ZIP文件
        print("\n4. 从EXE提取ZIP文件...")
        if not extract_zip_from_exe(test_exe, extracted_zip):
            print("   ❌ ZIP提取失败")
            return False
        
        print("   ✅ ZIP提取成功")
        print(f"   - 提取的ZIP大小: {os.path.getsize(extracted_zip)} bytes")
        
        # 步骤5: 验证提取的ZIP文件
        print("\n5. 验证提取的ZIP文件...")
        import zipfile
        
        try:
            with zipfile.ZipFile(extracted_zip, 'r') as zf:
                files = zf.namelist()
                print(f"   ✅ ZIP文件有效，包含 {len(files)} 个文件:")
                for file in files:
                    print(f"     - {file}")
        except Exception as e:
            print(f"   ❌ ZIP文件验证失败: {e}")
            return False
        
        print("\n🎉 所有测试通过！新的ZIP附加和读取功能工作正常。")
        return True
        
    finally:
        # 清理临时文件
        shutil.rmtree(temp_dir, ignore_errors=True)

if __name__ == "__main__":
    success = test_zip_append_functionality()
    sys.exit(0 if success else 1)