import serial
import time

PORT = 'COM5'
BAUD = 9600

ser = serial.Serial(PORT, BAUD, timeout=2)
time.sleep(2)

def compute_checksum(type_byte, length, data):
    checksum = type_byte ^ length
    for b in data:
        checksum ^= b
    return checksum

def poll(sensor_type, label):
    ser.write(bytes([sensor_type]))
    
    start = ser.read(1)
    if len(start) == 0 or start[0] != 0xAA:
        print(f"{label}: FAILED (no start byte, got {start.hex()})")
        return
    
    type_byte = ser.read(1)[0]
    length = ser.read(1)[0]
    data = ser.read(length)
    checksum = ser.read(1)[0]
    
    expected_checksum = compute_checksum(type_byte, length, data)
    valid = (checksum == expected_checksum)
    
    print(f"{label}: type={hex(type_byte)} len={length} data={data.hex()} "
          f"checksum={hex(checksum)} expected={hex(expected_checksum)} "
          f"{'VALID' if valid else 'INVALID'}")
    
    return data

poll(0x01, "Ultrasonic")
time.sleep(0.2)
poll(0x02, "PIR")
time.sleep(0.2)
poll(0x03, "Photoresistor")

ser.close()