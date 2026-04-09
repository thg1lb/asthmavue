#include <arduinoFFT.h>

#define SAMPLES 256
#define SAMPLING_FREQUENCY 2000
#define MIC_PIN A0

double vReal[SAMPLES];
double vImag[SAMPLES];

ArduinoFFT<double> FFT(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

// Frequency range for wheeze detection
const double WHEEZE_MIN_HZ = 100.0;
const double WHEEZE_MAX_HZ = 1000.0;

// Adjusted thresholds for higher energy levels
double THRESHOLD = 7500.0;  // Base threshold
double noiseFloor = 5200.0;  // Changed from const to regular variable
const int CONSECUTIVE_DETECTIONS_REQUIRED = 3;

// Detection history
bool detectionHistory[3] = {false, false, false};
int detectionIndex = 0;

// Moving average for energy smoothing
const int ENERGY_HISTORY_SIZE = 5;
double energyHistory[ENERGY_HISTORY_SIZE] = {0};
int energyHistoryIndex = 0;

void setup() {
    Serial.begin(115200);
    pinMode(MIC_PIN, INPUT);
    
    Serial.println("Starting wheeze detection system...");
    Serial.println("Performing initial calibration...");
    calibrate();
}

void loop() {
    // Sample collection
    for (int i = 0; i < SAMPLES; i++) {
        vReal[i] = analogRead(MIC_PIN);
        vImag[i] = 0;
        delayMicroseconds(500);
    }

    // Perform FFT
    FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.complexToMagnitude(vReal, vImag, SAMPLES);

    // Calculate energy in wheeze frequency range
    int minBin = (WHEEZE_MIN_HZ * SAMPLES) / SAMPLING_FREQUENCY;
    int maxBin = (WHEEZE_MAX_HZ * SAMPLES) / SAMPLING_FREQUENCY;
    
    double energy = 0;
    double peakMagnitude = 0;
    int peakFrequency = 0;

    for (int i = minBin; i <= maxBin; i++) {
        energy += vReal[i];
        if (vReal[i] > peakMagnitude) {
            peakMagnitude = vReal[i];
            peakFrequency = i * SAMPLING_FREQUENCY / SAMPLES;
        }
    }

    // Update moving average
    energyHistory[energyHistoryIndex] = energy;
    energyHistoryIndex = (energyHistoryIndex + 1) % ENERGY_HISTORY_SIZE;
    
    double averageEnergy = 0;
    for (int i = 0; i < ENERGY_HISTORY_SIZE; i++) {
        averageEnergy += energyHistory[i];
    }
    averageEnergy /= ENERGY_HISTORY_SIZE;

    // Update detection history based on smoothed energy
    bool currentDetection = (averageEnergy > THRESHOLD && averageEnergy > noiseFloor);
    detectionHistory[detectionIndex] = currentDetection;
    detectionIndex = (detectionIndex + 1) % CONSECUTIVE_DETECTIONS_REQUIRED;

    // Check for consistent detection
    int detectCount = 0;
    for (int i = 0; i < CONSECUTIVE_DETECTIONS_REQUIRED; i++) {
        if (detectionHistory[i]) detectCount++;
    }

    // Print debug information
    Serial.print("Current Energy: ");
    Serial.print(energy);
    Serial.print(" | Avg Energy: ");
    Serial.print(averageEnergy);
    Serial.print(" | Threshold: ");
    Serial.print(THRESHOLD);
    Serial.print(" | Noise Floor: ");
    Serial.print(noiseFloor);
    Serial.print(" | Peak Freq: ");
    Serial.print(peakFrequency);
    Serial.print("Hz | Detection Count: ");
    Serial.println(detectCount);

    // Wheeze detection with improved criteria
    if (detectCount >= CONSECUTIVE_DETECTIONS_REQUIRED) {
        if (peakFrequency >= WHEEZE_MIN_HZ && peakFrequency <= WHEEZE_MAX_HZ) {
            Serial.println("========================");
            Serial.println("WHEEZE DETECTED!");
            Serial.print("Energy Level: ");
            Serial.println(averageEnergy);
            Serial.print("Peak Frequency: ");
            Serial.print(peakFrequency);
            Serial.println(" Hz");
            Serial.println("========================");
        }
    }

    delay(100);
}

void calibrate() {
    Serial.println("Calibrating...");
    double maxEnergy = 0;
    double minEnergy = 99999999;
    double sumEnergy = 0;
    const int calibrationSamples = 20;

    for (int i = 0; i < calibrationSamples; i++) {
        // Collect samples
        for (int j = 0; j < SAMPLES; j++) {
            vReal[j] = analogRead(MIC_PIN);
            vImag[j] = 0;
            delayMicroseconds(500);
        }

        // Process FFT
        FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
        FFT.complexToMagnitude(vReal, vImag, SAMPLES);

        // Calculate energy
        double energy = 0;
        int minBin = (WHEEZE_MIN_HZ * SAMPLES) / SAMPLING_FREQUENCY;
        int maxBin = (WHEEZE_MAX_HZ * SAMPLES) / SAMPLING_FREQUENCY;
        
        for (int j = minBin; j <= maxBin; j++) {
            energy += vReal[j];
        }

        sumEnergy += energy;
        if (energy > maxEnergy) maxEnergy = energy;
        if (energy < minEnergy) minEnergy = energy;

        Serial.print("Calibration reading ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(energy);

        delay(100);
    }

    double avgEnergy = sumEnergy / calibrationSamples;
    THRESHOLD = avgEnergy * 1.5; // Adjusted multiplier for your energy levels
    noiseFloor = minEnergy * 1.2;

    Serial.print("Calibration complete. Min Energy: ");
    Serial.print(minEnergy);
    Serial.print(" | Max Energy: ");
    Serial.print(maxEnergy);
    Serial.print(" | Avg Energy: ");
    Serial.print(avgEnergy);
    Serial.print(" | Threshold set to: ");
    Serial.println(THRESHOLD);
    Serial.print(" | Noise Floor set to: ");
    Serial.println(noiseFloor);
}