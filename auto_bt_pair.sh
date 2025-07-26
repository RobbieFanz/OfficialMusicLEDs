#!/bin/bash

# Start bluetoothctl and set agent, default, and discoverable
echo "Setting up bluetoothctl..."
bluetoothctl << EOF
agent on
default-agent
discoverable on
EOF

echo "Waiting for a device to connect..."

# Monitor bluetoothctl output to detect the first device that attempts to pair
# Loop until we find a MAC address
while true; do
    output=$(timeout 5 bluetoothctl devices | grep Device)
    
    if [ ! -z "$output" ]; then
        # Get the first MAC address
        mac=$(echo "$output" | head -n 1 | awk '{print $2}')
        echo "Found device: $mac"

        # Pair, trust, and connect to the device
        bluetoothctl << EOF
pair $mac
trust $mac
connect $mac
EOF

        echo "Device $mac paired, trusted, and connected."
        break
    fi

    sleep 2
done
