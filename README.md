# AsthmaVue
**A paedeatric monitoring device to aid children in managing asthma -- Imperial Maker Challenge 2024, 3rd Place**

AsthmaVue is a prototype device that detects breathing abnormalities (coughs, wheezes) in children with asthma, and alerts the child, or a designated adult. The project stems from my own experience managing my own asthma as a child, where it was particularly difficult to gauge whether I was close to an asthma attack.


## The problem
> [“Nearly 250,000 people die prematurely each year from asthma, and most of all these deaths are preventable. Globally, death rates from asthma in children range from 0 to 0.7 per 100,000 people”](https://pmc.ncbi.nlm.nih.gov/articles/PMC7052318/#:~:text=Nearly%20250%2C000%20people%20die%20prematurely,per%20100%2C000%20people%20%5B2%5D).

Children with asthma often lack the knowledge and experience needed to gauge when their symptoms are worsening. AsthmaVue aims to help quantify that uncertainty by providing a low-cost, accessible monitoring device that detects abnormal breathing events (wheezes, coughs) in real time and triggers and alert before an asthma attack.


## How it works
1. A MAX4466 electret microphone amplifier, placed in contact with the user's chest or neck, captures audio from the user's breathing
2. On startup, a **dynamic calibration** routine is initiated -- it takes a 20 readings as a baseline and automatically sets the detection threshold (average energy * 1.5) and noise floor (minimum energy * 1.2) to account for different environments
3. Audio is sampled at **2000 Hz** with **256 samples** per window
4. A **Hamming window** is applied before FFT to reduce spectral leakage at the edges of each sample window, in order to prevent false frequency peaks
5. FFT analysis idetifies energy peaks within the **100-1000 Hz** detection range, based on [respiratory literature on asthmatic breathing patterns in adult males](https://www.sciencedirect.com/science/article/abs/pii/S0012369216389644) (as that was the most convincing data I could find at the time)
6. A **moving average over 5 readings** smooths the energy signal to reduce noise-based false positives
7. A wheeze/cough event is confirmed only after **3 consecutive detections** within the designated frequency range
8. Alert is triggerd via one of the two output modes (see below)


## Hardware

| Component | Purpose |
|---|---|
| Raspberry Pi Pico (RP2040) | Microcontroller |
| MAX4466 electret microphone amplifier | Audio capture |
| Grove sound sensor | Initial testing (replaced) |


## Output modes
Two separate firmware files are provided as a COM port conflict prevents both outputs running simultaneously for the v1.0 model.

**`serialplotter_asthmavue.ino`**
Outputs CSV data (total energy, peak frequency, detection status) formatted for the software project SerialPlot (see dependencies). Used to visualise frequency peaks in real time during the challenge demo.

**`terminalAlert_asthmavue.ino`**
Outputs debug information into the Serial Monitor and triggers a clearly formatted terminal alert when a wheeze/cough event is confirmed:
```
========================
WHEEZE DETECTED!
Energy Level: [value]
Peak Frequency: [value] Hz
========================
```


## Signal processing summary
| Parameter | Value |
|---|---|
| Sampling frequency | 2000 Hz |
| Sample size | 256 |
| Windowing | Hamming |
| Wheeze frequency range | 100–1000 Hz |
| Energy smoothing | Moving average (n=5) |
| Consecutive detections required | 3 |
| Threshold setting | Dynamic (calibrated on startup) |


## Development
The prototype went through several iterations during the Imperial College Maker Challenge:

![Initial setup](images/01_initial_setup.jpg)
*Early breadboard prototype with Grove sound sensor*

![Iteration](images/02_iteration.jpg)
*Mid-development — switched to MAX4466, noted stethoscope as potential improvement*

![Challenge poster](images/03_poster.jpg)
*Imperial Maker Challenge poster with full technical documentation*

![Final demo](images/04_final_demo.jpg)
*Final prototype running live at the challenge, SerialPlot output visible on screen*


## Known limitations (v1.0)
- COM port conflict prevents terminal alerts and SerialPlot simultaneously
- Not yet portable -- requires USB connection to host machine
- Acoustic sensitivity varies with microphone placement
- Frequency thresholds should be calibrated in line with more relevant/recent clinical data


## Roadmap (v2.0+)
- Wireless connectivity -- push alerts to phone app (potentially)
- Simultaneous terminal + visualisation outputs
- Stethoscope attachment for improved acoustic isolation (or other alternatives)
- Onboard data logging
- Reduced form factor / wearable design
- Updated thresholds in line with emerging clinical validation


## Dependencies  
- [arduinoFFT](https://github.com/kosme/arduinoFFT) library
- SerialPlot by hyOzd ([github.com/hyOzd/serialplot](https://github.com/hyOzd/serialplot)) for real-time frequency visualisation
