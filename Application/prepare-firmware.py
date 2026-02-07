from hashlib import sha256
import binascii
import sys
import os

# ---  PATH CONFIG---
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# ---- BIN FILE DIRECTORY ----
input_bin = os.path.join(BASE_DIR, "Debug", "Application.bin")

output_final = os.path.join(BASE_DIR, "Debug", "PB_Application.bin")
output_hex = os.path.join(BASE_DIR, "Debug", "PB_Application.hex")

# --- HEADER SIZE ---
BUFFER_FOR_VER_N_SHA = 0x200

def get_len_file(filepath):
    """Get file size in bytes."""
    with open(filepath, "rb") as f:
        return len(f.read())

def get_data_from_file(filepath, size, offset):
    """Read a specific size of data from a file at a given offset."""
    if not os.path.exists(filepath):
        print(f"ERROR: File {filepath} Not Found!")
        sys.exit(1)
    with open(filepath, "rb") as f:
        f.seek(offset)
        return f.read(size)

def convert_to_array(origin_buffer):
    """Convert bytes buffer to list of integers."""
    return [int(b) for b in origin_buffer]

def fill_buff_with_0xff(length):
    """Create a buffer filled with 0xFF bytes of given length."""
    return bytearray([0xFF] * length)

def convert_string_to_array(origin_buffer, base, length, count_shift):
    """Convert list of string hex values to list of integers with bit shift."""
    array_result = []
    for i in range(length):
        array_hex = int(origin_buffer[i], base)
        array_result.append(array_hex >> count_shift)
    return array_result

def get_crc_bytearray(crc_hex):
    """Convert CRC hex string to bytearray."""
    crc_padding = crc_hex.zfill(8)
    crc_hexstring = [crc_padding[i:i+2] for i in range(0, 8, 2)]
    return bytearray(convert_string_to_array(crc_hexstring, 16, 4, 0))

def get_file_size_bytes(size):
    """Convert integer size to byte array (4 bytes, little-endian)."""
    hex_size = format(size, '08x')
    size_string = [hex_size[i:i+2] for i in range(0, 8, 2)]
    return convert_string_to_array(size_string, 16, 4, 0)

def sha_loop_calculation(sha_buffer, count_loop):
    """Perform iterative SHA256 hashing."""
    current_buffer = sha_buffer
    for i in range(count_loop):
        sha2_hex = sha256(current_buffer).hexdigest()
        sha2_list = [sha2_hex[j:j+2] for j in range(0, 64, 2)]
        list_sha = convert_string_to_array(sha2_list, 16, 32, 0)
        current_buffer = bytearray(list_sha)
    return current_buffer

def generate_authentication(crc, version):
    """Generate authentication key based on CRC and version."""
    sha1_hex = sha256(crc).hexdigest()
    hash_hexstring = [sha1_hex[i:i+2] for i in range(0, 64, 2)]
    
    if len(version) > 0:
        count_shift = version[0] % 2
    else:
        count_shift = 0
        
    hash_with_shift = convert_string_to_array(hash_hexstring, 16, 32, count_shift)
    
    if len(version) > 1:
        count_loop = 1 + (version[1] % 3)
    else:
        count_loop = 1
        
    return sha_loop_calculation(bytearray(hash_with_shift), count_loop)


if __name__ == "__main__":
    print("--- STARTING FIRMWARE PREPARATION ---")
    print(f"Input File  : {input_bin}")
    print(f"Output Bin  : {output_final}")
    print(f"Output Hex  : {output_hex}")

    buffer_version = get_data_from_file(input_bin, 4, 0)
    version_in_array = convert_to_array(buffer_version)
    print(f"Detected Version: {version_in_array}")

    full_length = get_len_file(input_bin)
    
    if full_length <= BUFFER_FOR_VER_N_SHA:
        print("Error: File size is too small to process.")
        sys.exit(1)

    data_to_crc = get_data_from_file(input_bin, (full_length - BUFFER_FOR_VER_N_SHA), BUFFER_FOR_VER_N_SHA)
    
    calculate_crc = binascii.crc32(data_to_crc) & 0xFFFFFFFF
    
    crc_hex = format(calculate_crc, '08x')
    crc_byte_array = get_crc_bytearray(crc_hex)
    print(f"Application CRC32 (Standard): 0x{crc_hex.upper()}")

    app_size_val = full_length - BUFFER_FOR_VER_N_SHA
    app_size_kb = app_size_val / 1024
    apps_size_bytes = bytearray(get_file_size_bytes(app_size_val))
    print(f"Application Payload Size: {app_size_val} bytes ({app_size_kb:.2f} KB)") 

    auth_key = generate_authentication(crc_byte_array, version_in_array)
    print(f"Authentication Key: {auth_key.hex()}")
    
    header_used_len = len(buffer_version) + len(apps_size_bytes) + len(auth_key)
    fill_len = BUFFER_FOR_VER_N_SHA - header_used_len
    fill_buff = fill_buff_with_0xff(fill_len)

    final_payload = b''.join([buffer_version, apps_size_bytes, auth_key, fill_buff, data_to_crc])

    with open(output_final, "wb") as f:
        f.write(final_payload)
    print(f"[SUCCESS] Binary saved to: {output_final}")

    print(f"Generating Hex...")
    try:
        with open(output_final, "rb") as f_in, open(output_hex, "w") as f_out:
            data = f_in.read()
            for i in range(0, len(data), 16):
                chunk = data[i:i+16]
                hex_line = ' '.join(f"{b:02X}" for b in chunk)
                f_out.write(hex_line + '\n')
        print(f"[SUCCESS] Hex file saved to: {output_hex}")
    except Exception as e:
        print(f"[ERROR] Failed to generate hex: {e}")

    print("---------------------------------------")