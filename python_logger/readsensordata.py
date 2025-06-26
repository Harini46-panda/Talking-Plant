import json
from mcap.reader import make_reader

# === CONFIGURATION ===
MCAP_FILE = "sensor_data.mcap" # The name of the MCAP file to read

# === READ DATA FROM MCAP FILE ===
print(f"📖 Attempting to read data from MCAP file: {MCAP_FILE}\n")

try:
    # Open the MCAP file in binary read mode ('rb')
    with open(MCAP_FILE, "rb") as f:
        # Create an MCAP reader from the file object.
        # make_reader automatically detects the MCAP format and provides an interface
        # to iterate through records and messages.
        reader = make_reader(f)

        # Iterate through all messages in the MCAP file.
        # The reader yields message objects, which contain channel information,
        # message data, and timestamps.
        message_count = 0
        for schema, channel, message in reader.iter_messages():
            message_count += 1
            print(f"--- Message {message_count} ---")
            print(f"  Topic: {channel.topic}")
            print(f"  Schema Name: {schema.name}")
            print(f"  Message Encoding: {channel.message_encoding}")
            print(f"  Log Time (ns): {message.log_time}")
            print(f"  Publish Time (ns): {message.publish_time}")

            # Assuming the messages are JSON encoded, decode the data.
            if channel.message_encoding == "json":
                try:
                    # Decode the byte data back into a UTF-8 string, then parse it as JSON.
                    json_data = json.loads(message.data.decode("utf-8"))
                    print(f"  Data (JSON): {json.dumps(json_data, indent=2)}")
                except json.JSONDecodeError:
                    print(f"  Data: (Error decoding JSON) {message.data}")
                except UnicodeDecodeError:
                    print(f"  Data: (Error decoding UTF-8) {message.data}")
            else:
                # If it's not JSON, print the raw bytes and encoding info.
                print(f"  Data (Raw Bytes): {message.data}")
            print("-" * 20)

        if message_count == 0:
            print("No messages found in the MCAP file.")
        else:
            print(f"\n✅ Successfully read {message_count} messages from {MCAP_FILE}.")

except FileNotFoundError:
    print(f"❌ Error: The file '{MCAP_FILE}' was not found. Please ensure it exists and the path is correct.")
except Exception as e:
    print(f"❌ An unexpected error occurred while reading the MCAP file: {e}")

