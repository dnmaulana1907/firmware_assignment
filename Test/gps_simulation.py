import time
import random
import serial
import serial.tools.list_ports
from datetime import datetime,timezone
TIME_SEND_INTERVAL = 5  

def calculate_checksum(nmea_str):
    # Standard XOR checksum for NMEA 0183
    checksum = 0
    for char in nmea_str:
        checksum ^= ord(char)
    return hex(checksum)[2:].upper().zfill(2)

def format_coordinate(coord, is_lat=True):
    abs_coord = abs(coord)
    degrees = int(abs_coord)
    minutes = (abs_coord - degrees) * 60
    
    if is_lat:
        return f"{degrees:02d}{minutes:011.8f}"
    else:
        return f"{degrees:03d}{minutes:011.8f}"

def generate_rmc_sentences():
    # UTC time with 3 decimal precision for seconds
    now = datetime.now(timezone.utc)
    utc_time = now.strftime("%H%M%S.000") 
    utc_date = now.strftime("%d%m%y")

    # Jakarta area simulation
    lat_val = random.uniform(-6.3, -6.1)
    lon_val = random.uniform(106.7, 106.9)
    
    lat = format_coordinate(lat_val, is_lat=True)
    ns = "S" if lat_val < 0 else "N"
    lon = format_coordinate(lon_val, is_lat=False)
    ew = "W" if lon_val < 0 else "E"
    
    speed = f"{random.uniform(0, 10):.3f}" 
    cog = f"{random.uniform(0, 359):.2f}"   

    # LG290P RMC structure: UTC, Status, Lat, N/S, Lon, E/W, SOG, COG, Date, MagVar, MagDir, Mode, NavStatus
    # MagVar/MagDir not supported. NavStatus always 'V'.
    
    # GN Talker ID for multi-constellation 
    gn_body = f"GNRMC,{utc_time},A,{lat},{ns},{lon},{ew},{speed},{cog},{utc_date},,,A,V"
    gn_rmc = f"${gn_body}*{calculate_checksum(gn_body)}\r\n"
    
    # GP Talker ID for GPS only 
    gp_body = f"GPRMC,{utc_time},A,{lat},{ns},{lon},{ew},{speed},{cog},{utc_date},,,A,V"
    gp_rmc = f"${gp_body}*{calculate_checksum(gp_body)}\r\n"
    
    return gn_rmc, gp_rmc

def auto_detect_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if 'usbserial' in port.device.lower() or 'ttyusb' in port.device.lower():
            return port.device
    return None

def run_simulation():
    port_name = auto_detect_port()
    if not port_name:
        print("Serial port not found.")
        return

    try:
        ser = serial.Serial(port_name, baudrate=115200, timeout=1)
        print(f"Streaming GPS data on {port_name}...")
        
        while True:
            gn_rmc, gp_rmc = generate_rmc_sentences()

            # ser.write(gn_rmc.encode('ascii'))
            ser.write(gp_rmc.encode('ascii'))
            # print(gn_rmc.strip())
            print(gp_rmc.strip())
            
            time.sleep(TIME_SEND_INTERVAL)
            
    except KeyboardInterrupt:
        print("\nProcess terminated by user.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == "__main__":
    run_simulation()
