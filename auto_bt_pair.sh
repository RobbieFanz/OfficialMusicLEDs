#!/bin/bash

echo "Starting Bluetooth pairing agent..."

# Start bluetoothctl in the background and configure basic settings
bluetoothctl << EOF
agent on
default-agent
discoverable on
pairable on
power on
EOF

echo "Waiting for pairing requests..."

# Run expect to handle pairing interaction
expect << 'EOD'
spawn bluetoothctl
expect "#"

send "agent on\r"
expect "#"

send "default-agent\r"
expect "#"

send "discoverable on\r"
expect "#"

send "pairable on\r"
expect "#"

send "scan on\r"
expect {
    -re ".*Device ([0-9A-F:]+) .*" {
        set mac $expect_out(1,string)
        send_user "\nFound device: $mac\n"

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
                send_user "\nDevice $mac paired, trusted, and connected.\n"
                exit
            }
            timeout {
                send_user "Pairing timed out\n"
                exit 1
            }
        }
    }
}
EOD
