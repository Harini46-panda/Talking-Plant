import serial
import time
import json
from mcap.writer import Writer
import os # Import os for file system sync and path handling
# === CONFIGURATION ===
SERIAL_PORT = "COM5"   
BAUD_RATE = 115200
MCAP_FILE = "sensor_data.mcap"
MCAP_MAGIC_BYTES = b'\x89MCAP0\r\n' # Define the expected magic bytes for verification
# === CONNECT TO SERIAL ===
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
    print(f" Connected to {SERIAL_PORT}")
except serial.SerialException as e:
    # Handle serial port opening errors
    print(f" Could not open serial port '{SERIAL_PORT}': {e}")
    print("Please ensure the device is connected and the port is correct.")
    exit(1) # Exit if we can't connect to the serial port
# === PRE-WRITE MCAP MAGIC BYTES ===
print(f"LET'S BEGIN CAPTURING THE DATA TO: {MCAP_FILE}...")
try:
    with open(MCAP_FILE, "wb") as f_pre:
        f_pre.write(MCAP_MAGIC_BYTES)
except Exception as e:
    print(f" Error pre-writing magic bytes: {e}")
    exit(1)
with open(MCAP_FILE, "ab") as f:
    writer = Writer(f)
    schema_id = writer.register_schema(
        name="SensorJson",
        encoding="jsonschema",
        data=b"{}" # Empty schema data indicating a generic JSON structure
    )
    # Register the channel where messages will be published.
    # 'topic' is a logical name for the stream of messages within the MCAP file.
    # 'message_encoding' specifies how the 'data' field of the message is encoded.
    # 'schema_id' links this channel to the registered schema.
    channel_id = writer.register_channel(
        topic="sensor/readings",
        message_encoding="json",
        schema_id=schema_id
    )
    print(" Logging sensor data to MCAP (Press Ctrl+C to stop)...\n")
    try:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line or not line.startswith("{") or not line.endswith("}"):
                continue
            try:
                json_data = json.loads(line) 
                timestamp_ns = int(time.time() * 1e9)
                writer.add_message(
                    channel_id=channel_id,
                    log_time=timestamp_ns,
                    publish_time=timestamp_ns,
                    data=line.encode("utf-8")
                )
                print(f"Logged: {line}")
            except json.JSONDecodeError:
                print(f"Skipping invalid JSON: {line}")
            except Exception as e:
                print(f"Error processing or writing to MCAP: {e}")
    except KeyboardInterrupt:
        print("\n Logging stopped by user.")
    finally:
        writer.finish()
        print(f"MCAP file saved to {MCAP_FILE}")
ser.close()
print("Serial connection closed.")
try:
    with open(MCAP_FILE, "rb") as f_check:
        first_bytes_read = f_check.read(len(MCAP_MAGIC_BYTES))
        print(f"\nFinal verification of MCAP file: {MCAP_FILE}")
        print(f"   First {len(first_bytes_read)} bytes read: {first_bytes_read}")
        if first_bytes_read == MCAP_MAGIC_BYTES:
            print("   Status: MCAP magic bytes are present at the beginning of the file as expected.")
        else:
            print(f"   Status:  MCAP magic bytes are NOT present as expected.")
            print(f"   Expected: {MCAP_MAGIC_BYTES}")
            print(f"   Actual:   {first_bytes_read}")
            print("   This might indicate an issue with file writing or an unexpected file corruption.")
            print("   Even with explicit pre-writing, the file appears corrupted at the start.")
except FileNotFoundError:
    print(f"\n Verification failed: MCAP file '{MCAP_FILE}' not found. This should not happen if pre-write succeeded.")
except Exception as e:
    print(f"\n An error occurred during final magic bytes verification: {e}")
