#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import zipfile
import os
import struct

def analyze_zip_file(zip_path):
    """分析ZIP文件的详细信息"""
    print(f"\n=== 分析文件: {zip_path} ===")
    
    if not os.path.exists(zip_path):
        print(f"错误: 文件不存在 - {zip_path}")
        return None
    
    file_size = os.path.getsize(zip_path)
    print(f"文件大小: {file_size} 字节")
    
    try:
        # 使用zipfile模块分析
        with zipfile.ZipFile(zip_path, 'r') as zf:
            print(f"ZIP文件可读: 是")
            print(f"文件数量: {len(zf.filelist)}")
            
            # 分析每个文件的信息
            for info in zf.filelist:
                print(f"  文件: {info.filename}")
                print(f"    压缩大小: {info.compress_size}")
                print(f"    原始大小: {info.file_size}")
                print(f"    压缩方法: {info.compress_type}")
                print(f"    CRC32: {hex(info.CRC)}")
                print(f"    创建系统: {info.create_system}")
                print(f"    提取版本: {info.extract_version}")
                print(f"    标志位: {info.flag_bits}")
    except Exception as e:
        print(f"ZIP文件读取错误: {e}")
        return None
    
    # 分析ZIP文件的二进制结构
    try:
        with open(zip_path, 'rb') as f:
            # 检查文件头
            header = f.read(4)
            print(f"文件头: {header.hex()} ({header})")
            
            # 查找ZIP结束记录
            f.seek(-22, 2)  # 从文件末尾向前22字节
            end_record = f.read(22)
            
            if end_record[:4] == b'PK\x05\x06':
                print("找到ZIP结束记录")
                # 解析结束记录
                signature, disk_num, start_disk, entries_disk, total_entries, \
                central_size, central_offset, comment_len = struct.unpack('<IHHHHIIH', end_record)
                
                print(f"  中央目录条目数: {total_entries}")
                print(f"  中央目录大小: {central_size}")
                print(f"  中央目录偏移: {central_offset}")
                print(f"  注释长度: {comment_len}")
                
                # 检查中央目录
                f.seek(central_offset)
                central_header = f.read(4)
                print(f"  中央目录头: {central_header.hex()} ({central_header})")
            else:
                print("未找到标准ZIP结束记录")
                # 尝试查找扩展的结束记录
                f.seek(-22-comment_len if 'comment_len' in locals() else -1024, 2)
                data = f.read()
                end_pos = data.rfind(b'PK\x05\x06')
                if end_pos >= 0:
                    print(f"在偏移 {len(data) - end_pos} 处找到ZIP结束记录")
    
    except Exception as e:
        print(f"二进制分析错误: {e}")
    
    return True

def compare_zip_files(zip1_path, zip2_path):
    """比较两个ZIP文件的差异"""
    print("\n" + "="*60)
    print("ZIP文件对比分析")
    print("="*60)
    
    print(f"正常文件: {zip1_path}")
    print(f"异常文件: {zip2_path}")
    
    # 分析两个文件
    result1 = analyze_zip_file(zip1_path)
    result2 = analyze_zip_file(zip2_path)
    
    if result1 and result2:
        print("\n=== 对比总结 ===")
        
        # 比较文件大小
        size1 = os.path.getsize(zip1_path)
        size2 = os.path.getsize(zip2_path)
        print(f"文件大小差异: {size1} vs {size2} (差值: {abs(size1-size2)})")
        
        # 比较ZIP内容
        try:
            with zipfile.ZipFile(zip1_path, 'r') as zf1, zipfile.ZipFile(zip2_path, 'r') as zf2:
                files1 = set(zf1.namelist())
                files2 = set(zf2.namelist())
                
                print(f"文件列表差异:")
                print(f"  仅在正常文件中: {files1 - files2}")
                print(f"  仅在异常文件中: {files2 - files1}")
                print(f"  共同文件: {files1 & files2}")
                
                # 比较共同文件的属性
                for filename in files1 & files2:
                    info1 = zf1.getinfo(filename)
                    info2 = zf2.getinfo(filename)
                    
                    if (info1.compress_type != info2.compress_type or 
                        info1.file_size != info2.file_size or
                        info1.compress_size != info2.compress_size):
                        print(f"  文件 {filename} 属性差异:")
                        print(f"    压缩方法: {info1.compress_type} vs {info2.compress_type}")
                        print(f"    原始大小: {info1.file_size} vs {info2.file_size}")
                        print(f"    压缩大小: {info1.compress_size} vs {info2.compress_size}")
        
        except Exception as e:
            print(f"内容比较错误: {e}")

if __name__ == "__main__":
    # 分析用户提供的两个ZIP文件
    normal_zip = r"C:\Users\mucute\Desktop\Ausic.zip"
    abnormal_zip = r"C:\Users\mucute\ausic-workspace\Ausic-app\composeApp\build\compose\binaries\main-release\app\Ausic.zip"
    
    compare_zip_files(normal_zip, abnormal_zip)