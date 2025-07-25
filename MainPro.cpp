#include <iostream>
#include <complex>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <portaudio.h>

using namespace std;

#define MAX 2048
#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512
#define M_PI 3.14159265358979323846

// ---------------- FFT Functions ----------------
int log2int(int N) {
    int k = N, i = 0;
    while (k) { k >>= 1; i++; }
    return i - 1;
}

int reverse(int N, int n) {
    int j, p = 0;
    for (j = 1; j <= log2int(N); j++)
        if (n & (1 << (log2int(N) - j)))
            p |= 1 << (j - 1);
    return p;
}

void ordina(complex<double>* f1, int N) {
    complex<double> f2[MAX];
    for (int i = 0; i < N; i++)
        f2[i] = f1[reverse(N, i)];
    for (int j = 0; j < N; j++)
        f1[j] = f2[j];
}

void transform(complex<double>* f, int N) {
    ordina(f, N);
    complex<double>* W = new complex<double>[N / 2];
    W[1] = polar(1.0, -2.0 * M_PI / N);
    W[0] = 1;
    for (int i = 2; i < N / 2; i++)
        W[i] = pow(W[1], i);

    int n = 1;
    int a = N / 2;
    for (int j = 0; j < log2int(N); j++) {
        for (int i = 0; i < N; i++) {
            if (!(i & n)) {
                complex<double> temp = f[i];
                complex<double> Temp = W[(i * a) % (n * a)] * f[i + n];
                f[i] = temp + Temp;
                f[i + n] = temp - Temp;
            }
        }
        n *= 2;
        a /= 2;
    }
    delete[] W;
}

void FFT(complex<double>* f, int N, double d) {
    transform(f, N);
    for (int i = 0; i < N; i++)
        f[i] *= d;
}

// ------------- Bluetooth Discoverable -------------
void makeDiscoverable() {
    system("bluetoothctl << EOF\n"
           "power on\n"
           "agent on\n"
           "discoverable on\n"
           "pairable on\n"
           "EOF");
    std::cout << "[+] Pi is now discoverable and pairable.\n";
}

// ------------- Wait for iPhone A2DP Connection -------------
std::string waitForBluetoothAudioSource() {
    std::cout << "[*] Waiting for iPhone to connect and send audio...\n";

    while (true) {
        FILE* fp = popen("pactl list short sources", "r");
        if (!fp) {
            perror("popen");
            exit(1);
        }

        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "bluez_source") && strstr(line, "a2dp_source")) {
                std::string sourceLine(line);
                std::string sourceName = sourceLine.substr(0, sourceLine.find('\t'));
                std::cout << "[+] Found Bluetooth audio source: " << sourceLine;
                pclose(fp);
                return sourceLine.substr(sourceLine.find('\t') + 1, sourceLine.find('\t', sourceLine.find('\t') + 1) - sourceLine.find('\t') - 1);
            }
        }

        pclose(fp);
        sleep(1); // Check every second
    }
}

// ------------- Set Default PulseAudio Source -------------
void setDefaultPulseSource(const std::string& source) {
    std::string cmd = "pactl set-default-source " + source;
    system(cmd.c_str());
    std::cout << "[+] Set default PulseAudio input to: " << source << std::endl;
}

// ------------- PortAudio Callback with FFT -------------
static int paCallback(const void* inputBuffer, void*, unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*) {
    const float* in = static_cast<const float*>(inputBuffer);
    complex<double> vec[MAX];

    for (unsigned long i = 0; i < framesPerBuffer; ++i)
        vec[i] = in[i];

    FFT(vec, framesPerBuffer, SAMPLE_RATE);

    for (int i = 0; i < framesPerBuffer / 2; ++i) {
        double mag = sqrt(pow(vec[i].real(), 2) + pow(vec[i].imag(), 2));
        if (mag > 1e7) std::cout << "▲";
        else std::cout << ".";
    }
    std::cout << std::endl;

    return paContinue;
}

// ------------- Main Program -------------
int main() {
    makeDiscoverable();

    std::string btSource = waitForBluetoothAudioSource();
    setDefaultPulseSource(btSource);

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio init error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    PaStreamParameters inputParams;
    inputParams.device = Pa_GetDefaultInputDevice();
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = 0.050;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    PaStream* stream;
    err = Pa_OpenStream(&stream, &inputParams, nullptr, SAMPLE_RATE,
                        FRAMES_PER_BUFFER, paClipOff, paCallback, nullptr);
    if (err != paNoError) {
        std::cerr << "PortAudio stream error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    Pa_StartStream(stream);
    std::cout << "[*] Now processing Bluetooth audio input...\n";

    while (Pa_IsStreamActive(stream)) {
        Pa_Sleep(100);
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    return 0;
}
