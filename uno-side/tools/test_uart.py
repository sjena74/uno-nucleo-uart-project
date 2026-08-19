import serial
import time

PORT = 'COM5'
BAUD = 9600

ser = serial.Serial(PORT, BAUD, timeout=2)
time.sleep(2)

def send_command(type_byte, data_bytes):
    packet = bytearray()
    packet.append(0xAA)              # START
    packet.append(type_byte)         # TYPE
    packet.append(len(data_bytes))   # LEN
    packet.extend(data_bytes)        # DATA
    
    checksum = type_byte ^ len(data_bytes)
    for b in data_bytes:
        checksum ^= b
    packet.append(checksum)          # CHECKSUM
    
    ser.write(packet)
    print(f"Sent: {packet.hex()}")

# Turn LED to full brightness
send_command(0x11, [255])
time.sleep(2)

# Turn LED off
send_command(0x11, [0])

ser.close()