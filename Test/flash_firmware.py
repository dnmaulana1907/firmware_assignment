import serial
import time
import os
import struct
import sys
import binascii
import serial.tools.list_ports


FILE_PATH = os.path.join("../Application/Debug", "PB_Application.hex") 

BAUD_RATE = 115200    
TIMEOUT = 2           
CHUNK_SIZE = 512 

# --- KONSTANTA PROTOKOL ---
HANDSHAKE_CMD = b'\xAA\xAA\xAA\xAA\xAA' 
ACK_BYTE      = b'\x09'                 

def calculate_standard_crc32(data):
    return binascii.crc32(data) & 0xFFFFFFFF

def auto_detect_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if 'usbserial'in port.device:
            print(f"Port: {port.device}")
            return port.device
        # print(f"Port: {port.device} - {port.description}")

def load_hex_as_bytes(filepath):
    if not os.path.exists(filepath):
        print(f"ERROR: File {filepath} not found!")
        sys.exit(1)
        
    with open(filepath, 'r') as f:
        hex_text = f.read()
        
    clean_hex = hex_text.replace(' ', '').replace('\n', '').replace('\r', '')
    try:
        return bytes.fromhex(clean_hex)
    except ValueError as e:
        print(f"ERROR: FHex format is not valid {e}")
        sys.exit(1)

def flash_firmware():
    serial_port =  auto_detect_port()
    print(f"Menggunakan Port: {serial_port}")

    firmware_data = load_hex_as_bytes(FILE_PATH)
    total_bytes = len(firmware_data)
    
    print(f"--- INFO FIRMWARE ---")
    print(f"File            : {FILE_PATH}")
    print(f"Total Size      : {total_bytes} bytes")
    print("---------------------")

    try:
        ser = serial.Serial(serial_port, BAUD_RATE, timeout=TIMEOUT)
        time.sleep(1) 

        print("\n[1/3] Send Handshake...")
        ser.write(HANDSHAKE_CMD)
        response = ser.read(1)
        
        if response == ACK_BYTE:
            print("      Access Accepted (ACK).")
        else:
            print(f"      Failed! Respons: {response.hex() if response else 'Timeout'}")
            ser.close()
            return
        print("\n[2/3] Transfer Data (Packet 520 Bytes)...")
        offset = 0
        chunk_count = 0
        start_time = time.time()
        
        while offset < total_bytes:
            raw_chunk = firmware_data[offset : offset + CHUNK_SIZE]
            
            if len(raw_chunk) < CHUNK_SIZE:
                raw_chunk += b'\xFF' * (CHUNK_SIZE - len(raw_chunk))

            chunk_count += 1

            packet = bytearray([0xFF] * 520)
            packet[0] = 0x01
            # Index 1-512: Data Firmware
            packet[1:513] = raw_chunk
            # Index 513-514: Padding (Tetap 0xFF)
            # Calculate Standard ßCRC-32  (Byte 0 s.d 514 = Total 515 bytes)
            crc_val = calculate_standard_crc32(packet[0:515])           
            # Index 515-518: CRC (Big Endian)
            packet[515:519] = struct.pack('>I', crc_val)   
            # Index 519: Footer 0x04
            packet[519] = 0x04   
            percent = (min(offset + CHUNK_SIZE, total_bytes) / total_bytes) * 100
            print(f"      -> Package {chunk_count:<3} [{percent:>5.1f}%] ...", end=" ", flush=True)
            
            ser.write(packet)
            ack = ser.read(1)
            if ack == ACK_BYTE:
                print("OK")
                offset += CHUNK_SIZE
            else:
                print(f"FAIL ({ack.hex() if ack else 'TO'})")
                break
        if offset >= total_bytes:
            duration = time.time() - start_time
            print(f"\n[3/3] FINISH! Duration: {duration:.2f} seconds")
            
        ser.close()

    except Exception as e:
        print(f"\n[ERROR] {e}")

if __name__ == "__main__":
    flash_firmware()