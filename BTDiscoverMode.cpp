#include <cstdlib>
#include <iostream>

void makePiDiscoverable() {
    int result;

    std::cout << "Powering on Bluetooth...\n";
    result = system("bluetoothctl power on");
    if (result != 0) std::cerr << "Failed to power on Bluetooth\n";

    std::cout << "Enabling agent...\n";
    result = system("bluetoothctl agent NoInputNoOutput");
    if (result != 0) std::cerr << "Failed to enable agent\n";

    std::cout << "Setting default agent...\n";
    result = system("bluetoothctl default-agent");
    if (result != 0) std::cerr << "Failed to set default agent\n";

    std::cout << "Making discoverable...\n";
    result = system("bluetoothctl discoverable on");
    if (result != 0) std::cerr << "Failed to make discoverable\n";

    std::cout << "Making pairable...\n";
    result = system("bluetoothctl pairable on");
    if (result != 0) std::cerr << "Failed to make pairable\n";

    std::cout << "Done. Pi should now be discoverable!\n";
}
