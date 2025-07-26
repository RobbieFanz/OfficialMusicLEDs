#!/bin/bash

# Start bluetoothctl and set agent, default, and discoverable
echo "Setting up bluetoothctl..."
bluetoothctl << EOF
agent on
default-agent
discoverable on
pairable on
power on
EOF

echo "Waiting for a device to connect..."

# Monitor bluetoothctl output to detect the first device that attempts to pair
while true; do
    output=$(timeout 5 bluetoothctl devices | grep Device)
    
    if [ ! -z "$output" ]; then
        # Get the first MAC address
        mac=$(echo "$output" | head -n 1 | awk '{print $2}')
        echo "Found device: $mac"

        # Use expect to handle pairing confirmation
        expect << EOD
spawn bluetoothctl
expect "#"
send "pair $mac\r"
expect {
    "Confirm passkey" {
        send "yes\r"
        exp_continue
    }
    "Pairing successful" {
        send "trust $mac\r"
        expect "#"
        send "connect $mac\r"
        expect "#"
    }
    timeout {
        send_user "Pairing timed out\n"
        exit 1
    }
}
EOD

        echo "Device $mac paired, trusted, and connected."
        break
    fi

    sleep 2
done
