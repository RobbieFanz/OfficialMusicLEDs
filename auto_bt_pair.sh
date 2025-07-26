#!/bin/bash

# Start bluetoothctl and set agent, default, and discoverable
echo "Setting up bluetoothctl..."
bluetoothctl << EOF
agent on
default-agent
discoverable on
EOF

echo "Waiting for a device to connect..."