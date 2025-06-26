import serial
import time
import json
from mcap.writer import Writer
import os # Import os for file system sync and path handling

# === CONFIGURATION ===
# IMPORTANT: Adjust SERIAL_PORT to match your system's serial port.
# On Windows, it might be 'COM1', 'COM2', etc.
# On Linux/macOS, it might be '/dev/ttyUSB0', '/dev/ttyACM0', '/dev/tty.usbmodemXXXX', etc.
SERIAL_PORT = "COM5"   # Change if needed
BAUD_RATE = 115200
MCAP_FILE = "sensor_data.mcap"
MCAP_MAGIC_BYTES = b'\x89MCAP0\r\n' # Define the expected magic bytes for verification

# === CONNECT TO SERIAL ===
try:
    # Attempt to open the serial port
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
    print(f"📡 Connected to {SERIAL_PORT}")
except serial.SerialException as e:
    # Handle serial port opening errors
    print(f"❌ Could not open serial port '{SERIAL_PORT}': {e}")
    print("Please ensure the device is connected and the port is correct.")
    # You might want to list available ports here for user convenience
    # import serial.tools.list_ports
    # ports = serial.tools.list_ports.comports()
    # print("Available ports:")
    # for port, desc, hwid in sorted(ports):
    #     print(f"  {port}: {desc} [{hwid}]")
    exit(1) # Exit if we can't connect to the serial port

# === PRE-WRITE MCAP MAGIC BYTES ===
# This is a workaround to ensure the magic bytes are explicitly present at the start.
# The file is opened in 'wb' mode, magic bytes are written, and the file is immediately closed.
print(f"LET'S BEGIN CAPTURING THE DATA TO: {MCAP_FILE}...")
try:
    with open(MCAP_FILE, "wb") as f_pre:
        f_pre.write(MCAP_MAGIC_BYTES)
except Exception as e:
    print(f"❌ Error pre-writing magic bytes: {e}")
    exit(1)

# === OPEN MCAP FILE IN APPEND MODE AND PREPARE WRITER ===
# The 'with' statement ensures the file is properly closed even if errors occur.
# Changed mode to 'ab' (append binary) to allow the mcap Writer to append to the
# file that already contains the magic bytes.
with open(MCAP_FILE, "ab") as f:
    # Initialize the MCAP Writer.
    # When initialized with a file opened in 'ab' mode, the Writer should
    # assume the magic bytes are already present and start appending records.
    writer = Writer(f)

    # Register a minimal JSON schema.
    # For JSON data, an empty JSON schema is often sufficient if strict validation
    # is not required within the MCAP file, as the message_encoding specifies JSON.
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

    print("🟢 Logging sensor data to MCAP (Press Ctrl+C to stop)...\n")

    try:
        # Loop indefinitely to continuously read from the serial port
        while True:
            # Read a line from the serial port, decode it, and strip whitespace.
            # 'errors="ignore"' handles characters that cannot be decoded, preventing crashes.
            line = ser.readline().decode("utf-8", errors="ignore").strip()

            # Skip empty lines or lines that do not appear to be valid JSON (must start and end with curly braces).
            if not line or not line.startswith("{") or not line.endswith("}"):
                continue

            try:
                # Attempt to parse the line as JSON.
                # This acts as a validation step to ensure we only log well-formed JSON.
                json_data = json.loads(line) # Store parsed data; can be used for further processing if needed

                # Get the current timestamp in nanoseconds since the Unix epoch.
                # MCAP typically uses nanoseconds for timestamps.
                timestamp_ns = int(time.time() * 1e9)

                # Add the message to the MCAP file.
                # 'channel_id': The ID of the channel this message belongs to.
                # 'log_time': The time the message was recorded by this script.
                # 'publish_time': The time the message was originally generated (e.g., by the sensor);
                #                 using the same timestamp as log_time for simplicity here.
                # 'data': The actual message payload, which must be bytes. Encode the JSON string.
                writer.add_message(
                    channel_id=channel_id,
                    log_time=timestamp_ns,
                    publish_time=timestamp_ns,
                    data=line.encode("utf-8")
                )

                print(f"✅ Logged: {line}")

            except json.JSONDecodeError:
                # Catch specific error if the line cannot be parsed as valid JSON
                print(f"❌ Skipping invalid JSON: {line}")
            except Exception as e:
                # Catch any other unexpected errors during message processing or writing to MCAP
                print(f"⚠️ Error processing or writing to MCAP: {e}")

    except KeyboardInterrupt:
        # Gracefully handle a user-initiated stop (e.g., Ctrl+C)
        print("\n🛑 Logging stopped by user.")
    finally:
        # IMPORTANT: Ensure the writer finishes writing all buffered data to the file.
        # This is crucial for a complete and valid MCAP file.
        writer.finish()
        print(f"✅ MCAP file saved to {MCAP_FILE}")

# Close the serial connection when the script exits or finishes
ser.close()
print("🔌 Serial connection closed.")

# === POST-WRITING VERIFICATION OF MAGIC BYTES (Final Check) ===
# After the MCAP file has been completely written and closed by the 'with' block,
# we'll open it again in binary read mode ('rb') to inspect its very first bytes.
try:
    with open(MCAP_FILE, "rb") as f_check:
        # Read exactly the number of bytes expected for the MCAP magic bytes.
        first_bytes_read = f_check.read(len(MCAP_MAGIC_BYTES))
        print(f"\n🔍 Final verification of MCAP file: {MCAP_FILE}")
        print(f"   First {len(first_bytes_read)} bytes read: {first_bytes_read}")
        if first_bytes_read == MCAP_MAGIC_BYTES:
            print("   Status: ✅ MCAP magic bytes are present at the beginning of the file as expected.")
        else:
            print(f"   Status: ❌ MCAP magic bytes are NOT present as expected.")
            print(f"   Expected: {MCAP_MAGIC_BYTES}")
            print(f"   Actual:   {first_bytes_read}")
            print("   This might indicate an issue with file writing or an unexpected file corruption.")
            print("   Even with explicit pre-writing, the file appears corrupted at the start.")

except FileNotFoundError:
    print(f"\n⚠️ Verification failed: MCAP file '{MCAP_FILE}' not found. This should not happen if pre-write succeeded.")
except Exception as e:
    print(f"\n⚠️ An error occurred during final magic bytes verification: {e}")
