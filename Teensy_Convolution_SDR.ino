/*********************************************************************************************
   (c) Frank DD4WH 2019_06_21

   "TEENSY CONVOLUTION SDR"

   SOFTWARE FOR A FAST CONVOLUTION-BASED RADIO

   HARDWARE NEEDED:
   - simple quadrature sampling detector board producing baseband IQ signals (Softrock, Elektor SDR etc.)
   (IQ boards with up to 192kHz bandwidth supported --> which basically means nearly 100% of the existing boards on the market)
   - Teensy audio board
   - Teensy 3.6 (No, Teensy 3.1/3.2/3.5 not supported)
   HARDWARE OPTIONAL:
   - Preselection: switchable RF lowpass or bandpass filter
   - digital step attenuator: PE4306 used in my setup


   SOFTWARE:
   - FFT Fast Convolution = Digital Convolution
   - with overlap - save = overlap-discard complex bandpass main filtering
   - spectral NR uses FFT-iFFT overlap-add with 50% overlap

   - in floating point 32bit
   - tested on Teensy 3.6 (using its FPU)
   - compile with 180MHz F_CPU, other speeds not supported

   Part of the evolution of this project has been documented here:
   https://forum.pjrc.com/threads/40188-Fast-Convolution-filtering-in-floating-point-with-Teensy-3-6/page2


   HISTORY OF IMPLEMENTED FEATURES
   - 12kHz to 30MHz Receive PLUS 76 - 108MHz: undersampling-by-3 with slightly reduced sensitivity (-9dB)
   - I & Q - correction in software (manual correction or automatic correction)
   - efficient frequency translation without multiplication
   - efficient spectrum display using a 256 point FFT on the first 256 samples of every 4096 sample-cycle
   - efficient AM demodulation with ARM functions
   - efficient DC elimination after AM demodulation
   - implemented nine different AM demodulation algorithms for comparison (only two could stand the test and one algorithm was finally left in the implementation)
   - real SAM - synchronous AM demodulation with phase determination by atan2f implemented from the wdsp lib
   - Stereo-SAM and sideband-selected SAM
   - sample rate from 48k to 234k and decimation-by-8 for efficient realtime calculations
   - spectrum Zoom function 1x, 2x, 4x, 512x, 1024x, 2048x, 4096x --> 4096x zoom with sub-Hz resolution
   - Automatic gain control (high end algorithm by Warren Pratt, wdsp)
   - plays MP3 and M4A (iTunes files) from SD card with the awesome lib by Frank Bösing (his old MP3 lib, not the new one)
   - automatic IQ amplitude and phase imbalance correction
   - dynamic frequency indicator figures and graticules on spectrum display x-axis
   - kind of menu system now working with many variables that can be set by the encoders
   - EEPROM save & load of important settings
   - wideband FM demodulation with deemphasis
   - automatic codec gain adjustment depending on the sample input level
   - spectrum display AGC to allow display of very small signals
   - spectrum display in WFM activated (alpha version . . .)
   - optimized automatic test whether mirror rejection is working - if not, codec is restarted automatically until we have working mirror rejection
   - display mirror rejection check ("IQtest" in red box)
   - activated integrated codec 5-band graphic equalizer
   - added digital attenuator PE4306 bit-banging SPI control [0 -31dB attenuation possible]
   - added backlight control for TFT in the menu
   - added analog gain display (analog codec gain AND attenuation displayed)
   - fixed major bug associated with too small "string" variables for printing, leading to annoying audio clicks
   - STEREO FM reception implemented and disabled spectrum display in WFM STEREO mode, because the digital noise of the refresh of the spectrum display does seriously distort audio
   - manual notch filter implemented [in the frequency domain: simply deletes bins before the iFFT]
   - bandwidth adjustment of manual notch filter implemented
   - graphical display of manual notch filters in the frequency domain
   - Michaels excellent noise blanker is working! Eliminates noise impulses very nicely and effectively!
   - leaky LMS algorithm from the wdsp lib implemented (but not working as expected . . .)
   - switched to complex filter coefficients for general filter in the fast convolution process
   - freely adjustable bandpasses & passband tuning in AM/SAM/SSB . . .
   - rebuilt convolution with more flexible choice of FFT size --> now default FFT size is 512, because of memory constraints when using 1024 . . .
   - decimation and interpolation filters are calculated with new algorithm and are calculated on-the-fly when changing filter characteristics --> much less hiss and ringing of the filters
   - Blackman-Harris four-term window for main FIR filter (as in PowerSDR)
   - first test of a 110kHz lowpass filter in the WFM path for FM (stereo) reception on VHF --> does work properly but causes strange effects (button swaps) because of memory constraints when assigning the FIR instances
   - changed default to 512tap FFT in order to have enough memory for MP3 playing and other things
   - updated Arduino to version 1.8.5 and Teensyduino to version 1.40 and had to change some of the code
   - implemented spectral noise reduction in the frequency domain by implementing another FFT-iFFT-overlap-add chain on the real audio output after the main filter
   - spectral weighting algorithm Kim et al. 2002 implemented[working!]
   - spectral weighting algorithm Romanin et al. 2009 / Schmitt et al. 2002 implemented (minimum statistics)[obsolete]
   - spectral weighting algorithm Romanin et al. 2009 implemented (voice activity detector)[working, without VAD now]
   - fixed bug in alias filter switching when changing bandpass filter coefficients
   - adjustment in finer filter frequency steps when below 500Hz (switch to 50Hz steps instead of 100Hz)
   - fixed several bugs in band switching and mode switching
   - final tweak of spectral NR algorithms finished (many parameters eliminated from menu)
   - for comparison added LMS and leaky LMS to NR menu choice (four NR algorithms to choose from: Kim, Romanin, leaky LMS, LMS)
   - changed spectral NR Romanin to newest version by Michael DL2FW [the final UHSDR version, 22.2.2018]
   - analog clock design
   - spectrum display FFT windowing bug fixed (thanks, Bob Larkin!)
   - ZoomFFT repaired and now fully functional for all magnifications (up to 2048x), additional IIR filters added, also added higher refresh rate!
   - incorporated many good ideas by Bob Larkin, thanks!
   - experimental new sample rates up to 353ksps . . . https://forum.pjrc.com/threads/42336-Reset-audio-board-codec-SGTL5000-in-realtime-processing/page3?highlight=sample+rate
   - add possibility to use PCB hardware by DO7JBH https://github.com/do7jbh/SSR-2
   - bugfix array out-of-bound, thanks bicycleguy for pointing me to this bug!
   - atan2f approximation: https://www.mikrocontroller.net/topic/atan2-funktion-mit-lookup-table-fuer-arm --> thanks Frank B for the hint !
   - bugfix band vs. bands --> cleanup and changed int band to int current_band
   - integrated automatic crc check on eePROM load and save (by Mike / bicycleguy, thanks!) - no more need to uncomment/comment during first time use of the software
   - added support for Bob Larkins RF Octave frontend filters http://www.janbob.com/electron/FilterBP1/FiltBP1.html
   - bugfix: only use local loop variables
   - bugfix: software now usable on different hardware versions: DO7JBH, DD4WH

   TODO:
   - get this software to run on the T4 with software switches, so we can use ONE software for T3.6 AND T4.0 
   - fix bug in Zoom_FFT --> lowpass IIR filters run with different sample rates, but are calculated for a fixed sample rate of 48ksps
   - implement separate interrupt to cope with UI (encoders, buttons, calculation of filter coefficients) in order to free audio interrupt
   - SSB autotune algorithm taken from Robert Dick
   - RTTY and CW decoder
   - BPSK decoder
   - UKW DX filters for WFM prior to FM demodulation (110kHz, 80kHz, 57kHz)
   - test dBm measurement according to filter passband
   - RDS decoding in wide FM reception mode ;-): very hard, but could be barely possible
   - finetune AGC parameters and make AGC HANG TIME, AGC HANG THRESHOLD and AGC HANG DECAY user-adjustable
   - record and playback IQ audio stream ;-)
   - read stations´ frequencies from SD card and display station names when tuned to a frequency
   - implement Motorola C-QUAM AM Stereo demodulation
   - CW peak filter (independently adjustable from notch filter)

   some parts of the code modified from and/or inspired by the following open sources:
   Teensy SDR (rheslip & DD4WH): https://github.com/DD4WH/Teensy-SDR-Rx [GNU GPL]
   UHSDR (M0NKA, KA7OEI, DF8OE, DB4PLE, DL2FW, DD4WH & other contributors): https://github.com/df8oe/UHSDR [GNU GPL]
   libcsdr (András Retzler): https://github.com/simonyiszk/csdr [BSD / GPL]
   wdsp (Warren Pratt): http://svn.tapr.org/repos_sdr_hpsdr/trunk/W5WC/PowerSDR_HPSDR_mRX_PS/Source/wdsp/ [GNU GPL]
   Wheatley (2011): cuteSDR https://github.com/satrian/cutesdr-se [BSD]
   Robert Dick (1999): Tune SSB Automatically. in QEX: http://www.arrl.org/files/file/QEX%20Binaries/1999/ssbtune.zip ["code is in the public domain . . .", thus I assume GNU GPL]
   sample-rate-change-on-the-fly code by Frank Bösing [MIT]
   GREAT THANKS FOR ALL THE HELP AND INPUT BY WALTER, WMXZ !
   Audio queue optimized by Pete El Supremo 2016_10_27, thanks Pete!
   An important hint on the implementation came from Alberto I2PHD, thanks for that!
   Thanks to Brian, bmillier for helping with codec restart code for the SGTL 5000 codec in the Teensy audio board!
   Thanks a lot to Michael DL2FW - without you the spectral noise reduction would not have been possible! Also you contributed the state-of-the-art Noise Blanker
   Bob Larkin, W7PUA, found a significant bug in the spectrum display FFT windowing and added lots of other very useful things, thanks a lot, Bob!
   and of course a great Thank You to Paul Stoffregen @ pjrc.com for providing the Teensy platform and its excellent audio library !

   Audio processing in float32_t with the NEW ARM CMSIS lib, --> https://forum.pjrc.com/threads/40590-Teensy-Convolution-SDR-(Software-Defined-Radio)?p=129081&viewfull=1#post129081

 ***********************************************************************************************************************************

   GNU GPL LICENSE v3

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>

 ************************************************************************************************************************************/

/*  If you use the hardware made by Frank DD4WH uncomment the next line */
//#define HARDWARE_DD4WH

/*  If you use the hardware made by FrankB uncomment the next line */
//#define HARDWARE_FRANKB

/*  If you use the hardware made by Dante DO7JBH [https://github.com/do7jbh/SSR-2], uncomment the next line */
#define HARDWARE_DO7JBH

/* only for debugging */
//#define DEBUG

/*  this prints out the ADC and DAC levels when NOT in SAM mode, primarily for debugging hardware
    recommendation: leave this commented */
//#define USE_ADC_DAC_display

/*  only for support of the hardware RF frontend filters designed by Bob Larkin, W7PUA
    http://www.janbob.com/electron/FilterBP1/FiltBP1.html
    adjust cutoff frequencies according to your needs in function setfreq */
#define USE_BOBS_FILTER

/*  flag to indicate to use the changes introduced by Bob Larkin, W7PUA
    recommendation: leave this uncommented */
#define USE_W7PUA

/*  use faster log calculations
    recommendation: leave this uncommented */
#define USE_LOG10FAST

/*  use faster atan2f calculation
    recommendation: leave this uncommented */
#define USE_ATAN2FAST

#define MP3 1

#if defined(__IMXRT1062__)
#define T4
#endif

/*  this allows simultaneous calculation of sin and cos to save processor time for SAM demodulation  */
extern "C"
{
  void sincosf(float err, float *s, float *c);
}

#include <Audio.h>
#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Metro.h>
#include <Bounce.h>
#include <arm_math.h>
#include <arm_const_structs.h>
#include <si5351.h>
#include <Encoder.h>
#if defined(MP3)
#include <play_sd_mp3.h> //mp3 decoder by Frank B
#include <play_sd_aac.h> // AAC decoder by Frank B
#endif
//#include "rtty.h"
//#include "cw_decoder.h"
#include <util/crc16.h> //mdrhere

#if defined(T4)
#include <SPIN.h>
#include <ILI9341_t3n.h>
#include <ili9341_t3n_font_Arial.h>
#else
#include <EEPROM.h>
#include <ILI9341_t3.h>
#include "font_Arial.h"
#endif


time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

// Settings for the hardware QSD
// Joris PCB uses a 27MHz crystal and CLOCK 2 output
// Elektor SDR PCB uses a 25MHz crystal and the CLOCK 1 output
//#define Si_5351_clock  SI5351_CLK1
#if defined(HARDWARE_DO7JBH) || defined(HARDWARE_FRANKB)
#define Si_5351_crystal 25000000
#else
#define Si_5351_crystal 27000000
#endif

#define Si_5351_clock  SI5351_CLK2

// Europe uses 9 kHz AM spacing, N.A. uses 10 (AM_SPACING_EU==0).  Others???  <PUA>
#define AM_SPACING_EU  1

unsigned long long calibration_factor = 1000000000 ;// 10002285;
long calibration_constant = 0;
// this is for the Joris PCB !
//long calibration_constant = 108000; // this is for the Elektor PCB !
unsigned long long hilfsf = 1000000000;

#ifdef HARDWARE_DO7JBH
// Optical Encoder connections
Encoder tune      (16, 17);
Encoder filter    (4, 5);
Encoder encoder3  (1, 2); //(26, 28);

Si5351 si5351;
#define MASTER_CLK_MULT  4  // QSD frontend requires 4x clock

#define BACKLIGHT_PIN   0  // unfortunately connected to 3V3 in DO7JBHs PCB 
#define TFT_DC          20
#define TFT_CS          21
#define TFT_RST         35  // 255 = unused. connect to 3.3V
#define TFT_MOSI        7
#define TFT_SCLK        14
#define TFT_MISO        12
// pins for digital attenuator board PE4306
//#define ATT_LE          24
//#define ATT_DATA        25
//#define ATT_CLOCK       28
// dummy definitions for Dantes hardware
#define ATT_LE          40
#define ATT_DATA        41
#define ATT_CLOCK       42
// prop shield LC used for audio speaker amp
//#define AUDIO_AMP_ENABLE 39

ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

// push-buttons
#define   BUTTON_1_PIN      A22 // encoder2 button = button3SW
#define   BUTTON_2_PIN      37 // BAND+ = button2SW
#define   BUTTON_3_PIN      30 // ???
#define   BUTTON_4_PIN      36 //
#define   BUTTON_5_PIN      38 // this is the pushbutton pin of the tune encoder
#define   BUTTON_6_PIN      8 // this is the pushbutton pin of the filter encoder
#define   BUTTON_7_PIN      39 // this is the menu button pin
#define   BUTTON_8_PIN      33  //27 // this is the pushbutton pin of encoder 3

const int8_t On_set    = 25; // hold switched on

Bounce button1 = Bounce(BUTTON_1_PIN, 50);
Bounce button2 = Bounce(BUTTON_2_PIN, 50);
Bounce button3 = Bounce(BUTTON_3_PIN, 50);
Bounce button4 = Bounce(BUTTON_4_PIN, 50);
Bounce button5 = Bounce(BUTTON_5_PIN, 50);
Bounce button6 = Bounce(BUTTON_6_PIN, 50);
Bounce button7 = Bounce(BUTTON_7_PIN, 50);
Bounce button8 = Bounce(BUTTON_8_PIN, 50);

#elif defined(HARDWARE_FRANKB)
Si5351 si5351;
// Optical Encoder connections
Encoder tune      (2, 3);
Encoder filter    (1, 2);
Encoder encoder3  (15, 16); //(26, 28);
#define MASTER_CLK_MULT  4  // QSD frontend requires 4x clock

#define PCF8574_ADR     0x20
#define PCF8574_INT     22
#define SDCARD_CS_PIN   BUILTIN_SDCARD
#define SDCARD_SENSE    24 //read 0: Card inserted, 1: no Card
#define ENCODER_1_A     2
#define ENCODER_1_B     3
#define ENCODER_2_A     4
#define ENCODER_2_B     14
#define ENCODER_3_A     15
#define ENCODER_3_B     16
#define TFT_DC          10  // only CS pin 
#define TFT_CS          9   // using standard pin
#define TFT_RST         255 // no reset
#define TFT_TOUCH_IRQ   5
#define TFT_TOUCH_CS    6
#define LED_PIN         13
ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST);

#else
// Optical Encoder connections
Encoder tune      (16, 17);
Encoder filter    (1, 2);
Encoder encoder3  (5, 4); //(26, 28);

Si5351 si5351;
#define MASTER_CLK_MULT  4  // QSD frontend requires 4x clock

#define BACKLIGHT_PIN   3
#define TFT_DC          20
#define TFT_CS          21
#define TFT_RST         32  // 255 = unused. connect to 3.3V
#define TFT_MOSI        7
#define TFT_SCLK        14
#define TFT_MISO        12
// pins for digital attenuator board PE4306
#define ATT_LE          24
#define ATT_DATA        25
#define ATT_CLOCK       28
// prop shield LC used for audio speaker amp
//#define AUDIO_AMP_ENABLE 39

ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

// push-buttons
#define   BUTTON_1_PIN      33
#define   BUTTON_2_PIN      34
#define   BUTTON_3_PIN      35
#define   BUTTON_4_PIN      36
#define   BUTTON_5_PIN      38 // this is the pushbutton pin of the tune encoder
#define   BUTTON_6_PIN       0 // this is the pushbutton pin of the filter encoder
#define   BUTTON_7_PIN      37 // this is the menu button pin
#define   BUTTON_8_PIN       8  //27 // this is the pushbutton pin of encoder 3

Bounce button1 = Bounce(BUTTON_1_PIN, 50);
Bounce button2 = Bounce(BUTTON_2_PIN, 50);
Bounce button3 = Bounce(BUTTON_3_PIN, 50);
Bounce button4 = Bounce(BUTTON_4_PIN, 50);
Bounce button5 = Bounce(BUTTON_5_PIN, 50);
Bounce button6 = Bounce(BUTTON_6_PIN, 50);
Bounce button7 = Bounce(BUTTON_7_PIN, 50);
Bounce button8 = Bounce(BUTTON_8_PIN, 50);

#endif


Metro five_sec = Metro(2000); // Set up a Metro
Metro ms_500 = Metro(500); // Set up a Metro
Metro encoder_check = Metro(100); // Set up a Metro
//Metro dbm_check = Metro(25);
uint8_t wait_flag = 0;

#ifdef HARDWARE_DO7JBH
const uint8_t Band1 = 26; // band selection pins for LPF relays, used with 2N7000: HIGH means LPF is activated
const uint8_t Band2 = 27; // always use only one LPF with HIGH, all others have to be LOW
// not used
const uint8_t Band3 = 30;
const uint8_t Band4 = 57; // 29: > 5.4MHz
const uint8_t Band5 = 26; // LW
#endif

#ifdef HARDWARE_DD4WH
const uint8_t Band1 = 31; // band selection pins for LPF relays, used with 2N7000: HIGH means LPF is activated
const uint8_t Band2 = 30; // always use only one LPF with HIGH, all others have to be LOW
const uint8_t Band3 = 27;
const uint8_t Band4 = 29; // 29: > 5.4MHz
const uint8_t Band5 = 26; // LW
#endif

#ifdef USE_BOBS_FILTER
const uint8_t Band_3M5_7M3 =    31;
const uint8_t Band_7M3_15M =    28;
const uint8_t Band_15M_30M =    29;
#endif

// this audio comes from the codec by I2S2
AudioInputI2S            i2s_in;

AudioRecordQueue         Q_in_L;
AudioRecordQueue         Q_in_R;
#if defined(MP3)
AudioPlaySdMp3           playMp3;
AudioPlaySdAac           playAac;
#endif
AudioMixer4              mixleft;
AudioMixer4              mixright;
AudioPlayQueue           Q_out_L;
AudioPlayQueue           Q_out_R;
AudioOutputI2S           i2s_out;

#ifdef USE_W7PUA
// Added hardware: A pair of 100K resistors connected to the L&R input pins of the Codec.
// These two resisors are connected together on the other end and go through a 2.2nF cap
// to the A21 DAC pin on the Teensy 3.6.
AudioPlayQueue           queueTP;        // Output packets for Twin Peak test
AudioOutputAnalog        dac1;           // Generate 24 KHz signal
AudioConnection          patchCord21(queueTP, 0, dac1, 0);
#endif

AudioConnection          patchCord1(i2s_in, 0, Q_in_L, 0);
AudioConnection          patchCord2(i2s_in, 1, Q_in_R, 0);

//AudioConnection          patchCord3(Q_out_L, 0, i2s_out, 1);
//AudioConnection          patchCord4(Q_out_R, 0, i2s_out, 0);
AudioConnection          patchCord3(Q_out_L, 0, mixleft, 0);
AudioConnection          patchCord4(Q_out_R, 0, mixright, 0);
#if defined(MP3)
AudioConnection          patchCord5(playMp3, 0, mixleft, 1);
AudioConnection          patchCord6(playMp3, 1, mixright, 1);
AudioConnection          patchCord7(playAac, 0, mixleft, 2);
AudioConnection          patchCord8(playAac, 1, mixright, 2);
#endif
AudioConnection          patchCord9(mixleft, 0,  i2s_out, 1);
AudioConnection          patchCord10(mixright, 0, i2s_out, 0);

AudioControlSGTL5000     sgtl5000_1;

int idx_t = 0;
//int idx = 0;
int32_t sum;
float32_t mean;
int n_L;
int n_R;
long int n_clear;
//float32_t notch_amp[1024];
//float32_t FFT_magn[4096];
float32_t FFT_spec[256];
float32_t FFT_spec_old[256];
int16_t pixelnew[256];
int16_t pixelold[256];
float32_t LPF_spectrum = 0.2;
float32_t spectrum_display_scale = 30.0; // 30.0
uint8_t show_spectrum_flag = 1;
uint8_t display_S_meter_or_spectrum_state = 0;
uint8_t bitnumber = 16; // test, how restriction to twelve bit alters sound quality

int16_t spectrum_brightness = 255;
uint8_t spectrum_mov_average = 0;
uint16_t SPECTRUM_DELETE_COLOUR = ILI9341_BLACK;
uint16_t SPECTRUM_DRAW_COLOUR = ILI9341_WHITE;
#define SPECTRUM_ZOOM_MIN         0
#define SPECTRUM_ZOOM_1           0
#define SPECTRUM_ZOOM_2           1
#define SPECTRUM_ZOOM_4           2
#define SPECTRUM_ZOOM_8           3
#define SPECTRUM_ZOOM_16          4
#define SPECTRUM_ZOOM_32          5
#define SPECTRUM_ZOOM_64          6
#define SPECTRUM_ZOOM_128         7
#define SPECTRUM_ZOOM_256         8
#define SPECTRUM_ZOOM_512         9
#define SPECTRUM_ZOOM_1024        10
#define SPECTRUM_ZOOM_2048        11
#define SPECTRUM_ZOOM_4096        12
#define SPECTRUM_ZOOM_MAX         11

int32_t spectrum_zoom = SPECTRUM_ZOOM_2;

// Text and position for the FFT spectrum display scale

#define SAMPLE_RATE_MIN               6
#define SAMPLE_RATE_8K                0
#define SAMPLE_RATE_11K               1
#define SAMPLE_RATE_16K               2
#define SAMPLE_RATE_22K               3
#define SAMPLE_RATE_32K               4
#define SAMPLE_RATE_44K               5
#define SAMPLE_RATE_48K               6
#define SAMPLE_RATE_50K               7
#define SAMPLE_RATE_88K               8
#define SAMPLE_RATE_96K               9
#define SAMPLE_RATE_100K              10
#define SAMPLE_RATE_101K              11
#define SAMPLE_RATE_176K              12
#define SAMPLE_RATE_192K              13
#define SAMPLE_RATE_234K              14
#define SAMPLE_RATE_281K              15
#define SAMPLE_RATE_353K              16 // does not work !
#define SAMPLE_RATE_MAX               15

//uint8_t sr =                     SAMPLE_RATE_96K;
uint8_t SAMPLE_RATE =            SAMPLE_RATE_100K;
uint8_t LAST_SAMPLE_RATE =       SAMPLE_RATE_100K;

typedef struct SR_Descriptor
{
  const uint8_t SR_n;
  const uint32_t rate;
  const char* const text;
  const char* const f1;
  const char* const f2;
  const char* const f3;
  const char* const f4;
  const float32_t x_factor;
  const uint8_t x_offset;
} SR_Desc;


const SR_Descriptor SR [17] =
{ // x_factor, x_offset and f1 to f4 are NOT USED ANYMORE !!!
  //   SR_n , rate, text, f1, f2, f3, f4, x_factor = pixels per f1 kHz in spectrum display
  {  SAMPLE_RATE_8K, 8000,  "  8k", " 1", " 2", " 3", " 4", 64.0, 11}, // not OK
  {  SAMPLE_RATE_11K, 11025, " 11k", " 1", " 2", " 3", " 4", 43.1, 17}, // not OK
  {  SAMPLE_RATE_16K, 16000, " 16k",  " 4", " 4", " 8", "12", 64.0, 1}, // OK
  {  SAMPLE_RATE_22K, 22050, " 22k",  " 5", " 5", "10", "15", 58.05, 6}, // OK
  {  SAMPLE_RATE_32K, 32000,  " 32k", " 5", " 5", "10", "15", 40.0, 24}, // OK, one more indicator?
  {  SAMPLE_RATE_44K, 44100,  " 44k", "10", "10", "20", "30", 58.05, 6}, // OK
  {  SAMPLE_RATE_48K, 48000,  " 48k", "10", "10", "20", "30", 53.33, 11}, // OK
  {  SAMPLE_RATE_50K, 50223,  " 50k", "10", "10", "20", "30", 53.33, 11}, // NOT OK
  {  SAMPLE_RATE_88K, 88200,  " 88k", "20", "20", "40", "60", 58.05, 6}, // OK
  {  SAMPLE_RATE_96K, 96000,  " 96k", "20", "20", "40", "60", 53.33, 12}, // OK
  {  SAMPLE_RATE_100K, 100000,  "100k", "20", "20", "40", "60", 53.33, 12}, // NOT OK
  {  SAMPLE_RATE_101K, 100466,  "101k", "20", "20", "40", "60", 53.33, 12}, // NOT OK
  {  SAMPLE_RATE_176K, 176400,  "176k", "40", "40", "80", "120", 58.05, 6}, // OK
  {  SAMPLE_RATE_192K, 192000,  "192k", "40", "40", "80", "120", 53.33, 12}, // not OK
  {  SAMPLE_RATE_234K, 234375,  "234k", "40", "40", "80", "120", 53.33, 12}, // NOT OK
  {  SAMPLE_RATE_281K, 281000,  "281k", "40", "40", "80", "120", 53.33, 12}, // NOT OK
  {  SAMPLE_RATE_353K, 352800,  "353k", "40", "40", "80", "120", 53.33, 12} // NOT OK
};
int32_t IF_FREQ = SR[SAMPLE_RATE].rate / 4;     // IF (intermediate) frequency
#define F_MAX 3700000000
#define F_MIN 1200000

// out of the nine implemented AM detectors, only
// two proved to be of acceptable quality:
// AM2 and AM_ME2
// however, SAM outperforms all demodulators ;-)
#define       DEMOD_MIN           0
#define       DEMOD_USB           0
#define       DEMOD_LSB           1
//#define       DEMOD_AM1           2
#define       DEMOD_AM2           2
//#define       DEMOD_AM3           4
//#define       DEMOD_AM_AE1        5
//#define       DEMOD_AM_AE2        6
//#define       DEMOD_AM_AE3        7
//#define       DEMOD_AM_ME1        8
#define       DEMOD_AM_ME2        26
//#define       DEMOD_AM_ME3       10
#define       DEMOD_SAM          3 // synchronous AM demodulation
#define       DEMOD_SAM_USB      27 // synchronous AM demodulation
#define       DEMOD_SAM_LSB      28 // synchronous AM demodulation
#define       DEMOD_SAM_STEREO   4 // SAM, with pseude-stereo effect
#define       DEMOD_IQ           5
#define       DEMOD_WFM          6
#define       DEMOD_DCF77        29 // set the clock with the time signal station DCF77
#define       DEMOD_AUTOTUNE     10 // AM demodulation with autotune function
#define       DEMOD_STEREO_DSB   17 // double sideband: SSB without eliminating the opposite sideband
#define       DEMOD_DSB          18 // double sideband: SSB without eliminating the opposite sideband
#define       DEMOD_STEREO_LSB   19
#define       DEMOD_AM_USB       20 // AM demodulation with lower sideband suppressed
#define       DEMOD_AM_LSB       21 // AM demodulation with upper sideband suppressed
#define       DEMOD_STEREO_USB   22
#define       DEMOD_MAX          6

uint8_t autotune_wait = 10;

#ifdef USE_W7PUA
#define BAND_VLF    0  // Revised <PUA>
#define BAND_LW     1
#define BAND_MW     2
#define BAND_160M   3
#define BAND_80M    4
#define BAND_75M    5
#define BAND_60M    6
#define BAND_49M    7
#define BAND_40M    8
#define BAND_41M    9
#define BAND_31M   10
#define BAND_30M   11
#define BAND_25M   12
#define BAND_22M   13
#define BAND_20M   14
#define BAND_19M   15
#define BAND_16M   16
#define BAND_17M   17
#define BAND_15M   18
#define BAND_12M   19
#define BAND_10M   20
#define BAND_UKW   21

#define FIRST_BAND   BAND_VLF
#define LAST_BAND    BAND_UKW
#define NUM_BANDS    22
#define STARTUP_BAND BAND_80M

//Added band limits and band type, gain correction on a band basis    <PUA>
// f, f_low, f_high, band_name, mode_number, HiCut, LoCut, RFgain, band_type
// Band type 0=broadcast, 1=ham, 2=other general coverage
// Frequency resolution is for si5351 programming and is 0.01 Hz stated in ULL
struct band {
  unsigned long long freq;      // Current frequency in Hz * 100
  unsigned long long fBandLow;  // Lower band edge
  unsigned long long fBandHigh; // Upper band edge
  const char* name; // name of band
  int mode;
  int FHiCut;
  int FLoCut;
  int RFgain;
  uint8_t band_type;
  float32_t gainCorrection; // is hardware dependent and has to be calibrated ONCE and hardcoded in the table below
  int AGC_thresh;
  int16_t pixel_offset;
};
// For use in structure band
#define BROADCAST_BAND 0
#define HAM_BAND 1
#define MISC_BAND 2
#define WFM_BAND 3

struct band bands[NUM_BANDS] = {
  7700000,    1200000,   14000000, "VLF", DEMOD_USB, 800, 100, 0, MISC_BAND,      6.0,     30,    2,
  22500000,   14000000,   52000000,  "LW", DEMOD_SAM, 3600, -3600, 0, BROADCAST_BAND, 6.0,     30,    2,
  100000000,   52000000,  170000000,  "MW", DEMOD_SAM, 3600, -3600, 0, BROADCAST_BAND, 7.0,     30,    2,
  185000000,  180000000,  200000000, "160", DEMOD_LSB, -100, -2700, 0, HAM_BAND,       6.0,     30,    2,
  370000000,  350000000,  380000000, "80M", DEMOD_LSB, -100, -2700, 15, HAM_BAND,      6.0,     30,    2,
  399500000,  390000000,  400000000, "75M", DEMOD_SAM, 3600, -3600, 7, BROADCAST_BAND, 6.0,     30,    2,
  485000000,  475000000,  510000000, "60M", DEMOD_SAM, 3600, -3600, 7, BROADCAST_BAND, 6.0,     30,    2,
  607000000,  590000000,  620000000, "49M", DEMOD_SAM, 3600, -3600, 0, BROADCAST_BAND, 5.0,     30,    2,
  710000000,  700000000,  730000000, "40M", DEMOD_LSB, -100, -2700, 0, HAM_BAND,       4.0,     30,    2,
  735000000,  720000000,  745000000, "41M", DEMOD_SAM, 3600, -3600, 0, BROADCAST_BAND, 4.0,     30,    2,
  960000000,  940000000,  990000000, "31M", DEMOD_SAM, 3600, -3600, 0, BROADCAST_BAND, 4.0,     30,    2,
  1012500000, 1010000000, 1015000000, "30M", DEMOD_USB, 2700,   100, 0, HAM_BAND,       4.0,     30,    2,
  1180000000, 1160000000, 1210000000, "25M", DEMOD_SAM, 4800, -4800, 2, BROADCAST_BAND, 3.0,     30,    2,
  1367000000, 1357000000, 1387000000, "22M", DEMOD_SAM, 3600, -3600, 2, BROADCAST_BAND, 7.0,     30,    2,
  1420000000, 1400000000, 1435000000, "20M", DEMOD_USB, 3600,   100, 0, HAM_BAND,       7.0,     30,    2,
  1514500000, 1510000000, 1580000000, "19M", DEMOD_SAM, 3600, -3600, 4, BROADCAST_BAND, 7.0,     30,    2,
  1770000000, 1748000000, 1790000000, "16M", DEMOD_SAM, 3600, -3600, 5, BROADCAST_BAND, 6.0,     30,    2,
  1810000000, 1806800000, 1816800000, "17M", DEMOD_USB, 3600,   100, 5, HAM_BAND,       6.0,     30,    2,
  2120000000, 2100000000, 2145000000, "15M", DEMOD_USB, 3600,   100, 5, HAM_BAND,       6.0,     30,    2,
  2492000000, 2489000000, 2499000000, "12M", DEMOD_USB, 3600,   100, 6, HAM_BAND,       6.0,     30,    2,
  2835000000, 2800000000, 2970000000, "10M", DEMOD_USB, 3600,   100, 6, HAM_BAND,       0.0,     30,    2,
  3500807200, 2910000000, 3590000000, "UKW", DEMOD_WFM, 3600, -3600, 15, WFM_BAND,      0.0,     30,    2
};

//
#else

#define BAND_LW     0
#define BAND_MW     1
#define BAND_120M   2
#define BAND_90M    3
#define BAND_75M    4
#define BAND_60M    5
#define BAND_49M    6
#define BAND_41M    7
#define BAND_31M    8
#define BAND_25M    9
#define BAND_22M   10
#define BAND_19M   11
#define BAND_16M   12
#define BAND_15M   13
#define BAND_13M   14
#define BAND_11M   15

#define FIRST_BAND BAND_LW
#define LAST_BAND  BAND_13M
#define NUM_BANDS  16
#define STARTUP_BAND BAND_MW    // 
struct band {
  unsigned long long freq; // frequency in Hz
  const char* name; // name of band
  int mode;
  int FHiCut;
  int FLoCut;
  int RFgain;
};
// f, band, mode, bandwidth, RFgain
struct band bands[NUM_BANDS] = {
  //  7750000 ,"VLF", DEMOD_AM, 3600,3600,0,
  22500000, "LW", DEMOD_SAM, 3600, -3600, 0,
  63900000, "MW",  DEMOD_SAM, 3600, -3600, 0,
  248500000, "120M",  DEMOD_SAM, 3600, -3600, 0,
  350000000, "90M",  DEMOD_LSB, 3600, -3600, 6,
  390500000, "75M",  DEMOD_SAM, 3600, -3600, 4,
  502500000, "60M",  DEMOD_SAM, 3600, -3600, 7,
  597000000, "49M",  DEMOD_SAM, 3600, -3600, 0,
  712000000, "41M",  DEMOD_SAM, 3600, -3600, 0,
  942000000, "31M",  DEMOD_SAM, 3600, -3600, 0,
  1173500000, "25M", DEMOD_SAM, 3600, -3600, 2,
  1357000000, "22M", DEMOD_SAM, 3600, -3600, 2,
  1514000000, "19M", DEMOD_SAM, 3600, -3600, 4,
  1748000000, "16M", DEMOD_SAM, 3600, -3600, 5,
  3175200000, "15M", DEMOD_WFM, 3600, -3600, 21,
  2145000000, "13M", DEMOD_SAM, 3600, -3600, 6,
  2567000000, "11M", DEMOD_SAM, 3600, -3600, 6
};

#endif

int current_band = STARTUP_BAND;


ulong samp_ptr = 0;

const int myInput = AUDIO_INPUT_LINEIN;

float32_t IQ_amplitude_correction_factor = 1.0038;
float32_t IQ_phase_correction_factor =  0.0058;
// experiment: automatic IQ imbalance correction
// Chang, R. C.-H. & C.-H. Lin (2010): Implementation of carrier frequency offset and
//     IQ imbalance copensation in OFDM systems. - Int J Electr Eng 17(4): 251-259.
float32_t K_est = 1.0;
float32_t K_est_old = 0.0;
float32_t K_est_mult = 1.0 / K_est;
float32_t P_est = 0.0;
float32_t P_est_old = 0.0;
float32_t P_est_mult = 1.0 / (sqrtf(1.0 - P_est * P_est));
float32_t Q_sum = 0.0;
float32_t I_sum = 0.0;
float32_t IQ_sum = 0.0;
uint8_t   IQ_state = 1;
uint32_t n_para = 512;
uint32_t IQ_counter = 0;
float32_t K_dirty = 0.868;
float32_t P_dirty = 1.0;
int8_t auto_IQ_correction = 1;
#define CHANG         0
#define MOSELEY       1
float32_t teta1 = 0.0;
float32_t teta2 = 0.0;
float32_t teta3 = 0.0;
float32_t teta1_old = 0.0;
float32_t teta2_old = 0.0;
float32_t teta3_old = 0.0;
float32_t M_c1 = 0.0;
float32_t M_c2 = 0.0;
uint8_t codec_restarts = 0;
uint32_t twinpeaks_counter = 0;
//uint8_t IQ_auto_counter = 0;
uint8_t twinpeaks_tested = 2; // initial value --> 2 !!
//float32_t asin_sum = 0.0;
//uint16_t asin_N = 0;
uint8_t write_analog_gain = 0;

boolean gEEPROM_current = false;  //mdrhere does the data in EEPROM match the current structure contents

#define BUFFER_SIZE 128

// complex FFT with the new library CMSIS V4.5
const static arm_cfft_instance_f32 *S;

// create coefficients on the fly for custom filters
// and let the algorithm define which FFT size you need
// input variables by the user
// Fpass, Fstop, Astop (stopband attenuation)
// fixed: sample rate, scale = 1.0 ???
// tested & working samplerates (processor load):
// 96k (46%), 88.2k, 48k, 44.1k (18%), 32k (13.6%),

float32_t LP_Fpass = 3500;

//uint32_t LP_F_help = 3500;
int LP_F_help = 3500;
float32_t LP_Fstop = 3600;
float32_t LP_Astop = 90;
//float32_t LP_Fpass_old = 0.0;

//int RF_gain = 0;
int audio_volume = 50;
//int8_t bass_gain_help = 0;
//int8_t midbass_gain_help = 30;
//int8_t mid_gain_help = 0;
//int8_t midtreble_gain_help = -10;
//int8_t treble_gain_help = -40;
float32_t bass = 0.0;
float32_t midbass = 0.0;
float32_t mid = 0.0;
float32_t midtreble = -0.1;
float32_t treble = - 0.4;
float32_t stereo_factor = 500.0;
uint8_t half_clip = 0;
uint8_t quarter_clip = 0;
uint8_t auto_codec_gain = 1;
int8_t RF_attenuation = 0;

/*********************************************************************************************************************************************************

   FIR main filter defined here
   also decimation etc.

 *********************************************************************************************************************************************************/
int16_t *sp_L;
int16_t *sp_R;
float32_t hh1 = 0.0;
float32_t hh2 = 0.0;
float32_t I_old = 0.2;
float32_t Q_old = 0.2;
float32_t I_old_old = 0.2;
float32_t Q_old_old = 0.2;
float32_t rawFM_old_L = 0.0;
float32_t rawFM_old_R = 0.0;
float32_t alt_WFM_audio = 0.0;
float32_t WFM_phase = 0.1;
float32_t WFM_lastphase = 0.1;
float32_t WFM_dphase = 0.1;


const uint8_t WFM_BLOCKS = 6;
// T = 1.0/sample_rate;
// alpha = 1 - e^(-T/tau);
// tau = 50µsec in Europe --> alpha = 0.099
// tau = 75µsec in the US -->
//
float32_t dt = 1.0 / 234000.0;
float32_t deemp_alpha = dt / (50e-6 + dt);
//float32_t m_alpha = 0.91;
//float32_t deemp_alpha = 0.099;
float32_t onem_deemp_alpha = 1.0 - deemp_alpha;
uint16_t autotune_counter = 0;

//const uint32_t FFT_L = 1024; // needs 89% of the memory !
const uint32_t FFT_L = 512; // needs 60% of the memory
float32_t FIR_Coef_I[(FFT_L / 2) + 1];
float32_t FIR_Coef_Q[(FFT_L / 2) + 1];
#define MAX_NUMCOEF (FFT_L / 2) + 1
#define TPI           6.28318530717959f
#define PIH           1.57079632679490f
#define FOURPI        2.0 * TPI
#define SIXPI         3.0 * TPI
uint32_t m_NumTaps = (FFT_L / 2) + 1;
uint8_t FIR_filter_window = 1;
//const float32_t DF1 = 4.0; // decimation factor
const float32_t DF1 = 4.0; // decimation factor
const float32_t DF2 = 2.0; // decimation factor
const float32_t DF = DF1 * DF2; // decimation factor
uint32_t FFT_length = FFT_L;
const uint32_t N_B = FFT_L / 2 / BUFFER_SIZE * (uint32_t)DF; // should be 16 with DF == 8 and FFT_L = 512
uint32_t N_BLOCKS = N_B;
uint32_t BUF_N_DF = BUFFER_SIZE * N_BLOCKS / (uint32_t)DF;
// decimation by 8 --> 32 / 8 = 4
const uint32_t N_DEC_B = N_B / (uint32_t)DF;
float32_t float_buffer_L [BUFFER_SIZE * N_B];
float32_t float_buffer_R [BUFFER_SIZE * N_B];

float32_t FFT_buffer [FFT_L * 2] __attribute__ ((aligned (4)));
float32_t last_sample_buffer_L [BUFFER_SIZE * N_DEC_B];
float32_t last_sample_buffer_R [BUFFER_SIZE * N_DEC_B];
uint8_t flagg = 0;
// complex iFFT with the new library CMSIS V4.5
const static arm_cfft_instance_f32 *iS;
float32_t iFFT_buffer [FFT_L * 2] __attribute__ ((aligned (4)));

// FFT instance for direct calculation of the filter mask
// from the impulse response of the FIR - the coefficients
const static arm_cfft_instance_f32 *maskS;
float32_t FIR_filter_mask [FFT_L * 2] __attribute__ ((aligned (4)));

const static arm_cfft_instance_f32 *spec_FFT;
float32_t buffer_spec_FFT [512] __attribute__ ((aligned (4)));

const static arm_cfft_instance_f32 *NR_FFT;
float32_t NR_FFT_buffer [512] __attribute__ ((aligned (4)));

const static arm_cfft_instance_f32 *NR_iFFT;
// we dont need this any more, we reuse the NR_FFT_buffer and save 2kbytes ;-)
//float32_t NR_iFFT_buffer [512] __attribute__ ((aligned (4)));

//#define USE_WFM_FILTER
#ifdef USE_WFM_FILTER
// FIR filters for FM reception on VHF
// --> not needed: reuse FIR_Coef_I and FIR_Coef_Q
//const uint16_t FIR_WFM_num_taps = 24;
const uint16_t FIR_WFM_num_taps = 36;
arm_fir_instance_f32 FIR_WFM_I;
float32_t FIR_WFM_I_state [WFM_BLOCKS * BUFFER_SIZE + FIR_WFM_num_taps - 1]; // numTaps+blockSize-1
arm_fir_instance_f32 FIR_WFM_Q;
float32_t FIR_WFM_Q_state [WFM_BLOCKS * BUFFER_SIZE + FIR_WFM_num_taps - 1];


/*const float32_t FIR_WFM_Coef[] =
  { // FIR Parks, Kaiser window, Iowa Hills FIR Filter designer
  // Fs = 384kHz, Fc = 110kHz, Kaiser beta 3.8, 24 taps, transition width 0.1
  0.004625798840274968, 0.003688870813408950,-0.013875557717045652,-0.012934044118920221, 0.012800134495445288,-0.012180950672587090,-0.036852238612996767, 0.037399877886350741, 0.022300481142125423,-0.120157367503476248, 0.057767190665062668, 0.512110566632996034, 0.512110566632996034, 0.057767190665062668,-0.120157367503476248, 0.022300481142125423, 0.037399877886350741,-0.036852238612996767,-0.012180950672587090, 0.012800134495445288,-0.012934044118920221,-0.013875557717045652, 0.003688870813408950, 0.004625798840274968
  };*/
/*const float32_t FIR_WFM_Coef_Q[] =
  { // FIR Parks, Kaiser window, Iowa Hills FIR Filter designer
  // Fs = 384kHz, Fc = 110kHz, Kaiser beta 3.8, 24 taps, transition width 0.1
  0.004625798840274968, 0.003688870813408950,-0.013875557717045652,-0.012934044118920221, 0.012800134495445288,-0.012180950672587090,-0.036852238612996767, 0.037399877886350741, 0.022300481142125423,-0.120157367503476248, 0.057767190665062668, 0.512110566632996034, 0.512110566632996034, 0.057767190665062668,-0.120157367503476248, 0.022300481142125423, 0.037399877886350741,-0.036852238612996767,-0.012180950672587090, 0.012800134495445288,-0.012934044118920221,-0.013875557717045652, 0.003688870813408950, 0.004625798840274968
  };*/
/*
  const float32_t FIR_WFM_Coef[] =
  { // FIR Parks, Kaiser window, Iowa Hills FIR Filter designer
  // Fs = 384kHz, Fc = 50kHz, Kaiser beta 3.8, 24 taps, transition width 0.1
  -181.6381870078256780E-6, 0.004975451565742671, 0.011653821670876443, 0.017479667256298442, 0.012647025837995754,-0.008440017290804718,-0.036310877512063008,-0.044440114225935128,-0.004516440266678295, 0.088328595562111187, 0.201661521326793242, 0.279840753414532517, 0.279840753414532517, 0.201661521326793242, 0.088328595562111187,-0.004516440266678295,-0.044440114225935128,-0.036310877512063008,-0.008440017290804718, 0.012647025837995754, 0.017479667256298442, 0.011653821670876443, 0.004975451565742671,-181.6381870078256780E-6
  };
*/
/* const float32_t FIR_WFM_Coef[] =
  { // FIR Parks, Kaiser window, Iowa Hills FIR Filter designer
  // Fs = 384kHz, Fc = 100kHz, Kaiser beta 5.6, 24 taps, transition width 0.1
  -591.1041711221489550E-6, -0.005129034097065280, -0.005799289015478366, 0.006642848237559112, 0.007498237818598354, -0.019653624598637193, -0.005803653295842078, 0.047380624795600332, -0.012275157191465273, -0.105901764377712204, 0.101477147249252955, 0.485358617041706242, 0.485358617041706242, 0.101477147249252955, -0.105901764377712204, -0.012275157191465273, 0.047380624795600332, -0.005803653295842078, -0.019653624598637193, 0.007498237818598354, 0.006642848237559112, -0.005799289015478366, -0.005129034097065280, -591.1041711221489550E-6
  };
*/
/*// läuft !
  const float32_t FIR_WFM_Coef[] =
  { // FIR Parks, Kaiser window, Iowa Hills FIR Filter designer
  // Fs = 384kHz, Fc = 50kHz, Kaiser beta 4.4, 36 taps, transition width 0.1
  592.5825463002674950E-6, 0.001320848265533199, 0.001790946732298105, 655.4821700000750300E-6,-0.002699678833953366,-0.006572566177789689,-0.006914035555941112,-392.0989712319586720E-6, 0.010906809316195571, 0.017765225520549485, 0.009201337532541980,-0.016701709482744322,-0.044809056787863510,-0.047124157952014738,-430.8639980401008530E-6, 0.093472653544572140, 0.201191877174437983, 0.273200564206967700, 0.273200564206967700, 0.201191877174437983, 0.093472653544572140,-430.8639980401008530E-6,-0.047124157952014738,-0.044809056787863510,-0.016701709482744322, 0.009201337532541980, 0.017765225520549485, 0.010906809316195571,-392.0989712319586720E-6,-0.006914035555941112,-0.006572566177789689,-0.002699678833953366, 655.4821700000750300E-6, 0.001790946732298105, 0.001320848265533199, 592.5825463002674950E-6
  };
*/
/*// läuft !
  const float32_t FIR_WFM_Coef[] =
  { // FIR Parks, Kaiser window, Iowa Hills FIR Filter designer
  // Fs = 384kHz, Fc = 80.01kHz, Kaiser beta 4.4, 36 taps, transition width 0.1
  -356.2239703155325400E-6, 428.8159583493475110E-6, 0.002768313624795094, 0.003935981937959658,-229.4460384396558650E-6,-0.005676569602896231,-0.001415582278380387, 0.010263042330857489, 0.007877742946416445,-0.014222913476341907,-0.020791118995262630, 0.014235320171826216, 0.042979252785258985,-0.003477333822901606,-0.081033679869946321,-0.037594629553007936, 0.182167448027930057, 0.405225811603383557, 0.405225811603383557, 0.182167448027930057,-0.037594629553007936,-0.081033679869946321,-0.003477333822901606, 0.042979252785258985, 0.014235320171826216,-0.020791118995262630,-0.014222913476341907, 0.007877742946416445, 0.010263042330857489,-0.001415582278380387,-0.005676569602896231,-229.4460384396558650E-6, 0.003935981937959658, 0.002768313624795094, 428.8159583493475110E-6,-356.2239703155325400E-6
  };*/

// läuft !
const float32_t FIR_WFM_Coef[] =
{ // FIR Parks, Kaiser window, Iowa Hills FIR Filter designer
  // Fs = 384kHz, Fc = 120kHz, Kaiser beta 4.4, 36 taps, transition width 0.1
  624.6850061499508230E-6, 0.002708497910576767, 463.7277605032298310E-6, -0.002993671768455668, 0.002974399474225367, 0.001851944115674062, -0.008023223272215739, 0.006351672953904165, 0.006981175200321031, -0.019534687805473273, 0.010792340741888977, 0.021004722459631749, -0.043391183038594551, 0.015053012768562642, 0.061299026906325028, -0.111919636212113038, 0.017679746690378837, 0.540918306257059944, 0.540918306257059944, 0.017679746690378837, -0.111919636212113038, 0.061299026906325028, 0.015053012768562642, -0.043391183038594551, 0.021004722459631749, 0.010792340741888977, -0.019534687805473273, 0.006981175200321031, 0.006351672953904165, -0.008023223272215739, 0.001851944115674062, 0.002974399474225367, -0.002993671768455668, 463.7277605032298310E-6, 0.002708497910576767, 624.6850061499508230E-6
};

#endif

/****************************************************************************************
    init decimation and interpolation
    two decimation stages and
    two interpolation stages
 ****************************************************************************************/
// number of FIR taps for decimation stages
// DF1 decimation factor for first stage
// DF2 decimation factor for second stage
// see Lyons 2011 chapter 10.2 for the theory
const float32_t n_att = 90.0; // desired stopband attenuation
//const float32_t n_samplerate = 96.0; // samplerate before decimation
//const float32_t n_desired_BW = 5.0; // desired max BW of the filters
const float32_t n_samplerate = 176.0; // samplerate before decimation
const float32_t n_desired_BW = 9.0; // desired max BW of the filters
const float32_t n_fstop1 = ( (n_samplerate / DF1) - n_desired_BW) / n_samplerate;
const float32_t n_fpass1 = n_desired_BW / n_samplerate;
const uint16_t n_dec1_taps = (1 + (uint16_t) (n_att / (22.0 * (n_fstop1 - n_fpass1))));
const float32_t n_fstop2 = ((n_samplerate / (DF1 * DF2)) - n_desired_BW) / (n_samplerate / DF1);
const float32_t n_fpass2 = n_desired_BW / (n_samplerate / DF1);
const uint16_t n_dec2_taps = (1 + (uint16_t) (n_att / (22.0 * (n_fstop2 - n_fpass2))));


// decimation with FIR lowpass
// pState points to a state array of size numTaps + blockSize - 1
arm_fir_decimate_instance_f32 FIR_dec1_I;
float32_t FIR_dec1_I_state [n_dec1_taps + BUFFER_SIZE * N_B - 1]; //
arm_fir_decimate_instance_f32 FIR_dec1_Q;
float32_t FIR_dec1_Q_state [n_dec1_taps + BUFFER_SIZE * N_B - 1];
float32_t FIR_dec1_coeffs[n_dec1_taps];

arm_fir_decimate_instance_f32 FIR_dec2_I;
float32_t FIR_dec2_I_state [n_dec2_taps + 511]; //(BUFFER_SIZE * N_B / (uint32_t)DF1) - 1];
arm_fir_decimate_instance_f32 FIR_dec2_Q;
float32_t FIR_dec2_Q_state [n_dec2_taps + 511]; //(BUFFER_SIZE * N_B / (uint32_t)DF1) - 1];
float32_t FIR_dec2_coeffs[n_dec2_taps];

//interpolation filters have almost the same no. of taps
// but have not been programmed to be designed dynamically, because num_Taps has to be a multiple integer of interpolation factor L
// interpolation with FIR lowpass
// pState is of length (numTaps/L)+blockSize-1 words where blockSize is the number of input samples processed by each call
arm_fir_interpolate_instance_f32 FIR_int1_I;
//float32_t FIR_int1_I_state [(16 / (uint32_t)DF2) + BUFFER_SIZE * N_B / (uint32_t)DF - 1];
//float32_t FIR_int1_I_state [(48 / (uint32_t)DF2) + BUFFER_SIZE * N_B / (uint32_t)DF - 1];
float32_t FIR_int1_I_state [24 + 255]; //(48 / (uint32_t)DF2) + BUFFER_SIZE * N_B / (uint32_t)DF - 1];
arm_fir_interpolate_instance_f32 FIR_int1_Q;
//float32_t FIR_int1_Q_state [(16 / (uint32_t)DF2) + BUFFER_SIZE * N_B / (uint32_t)DF - 1];
//float32_t FIR_int1_Q_state [(48 / (uint32_t)DF2) + BUFFER_SIZE * N_B / (uint32_t)DF - 1];
float32_t FIR_int1_Q_state [24 + 255]; //(48 / (uint32_t)DF2) + BUFFER_SIZE * N_B / (uint32_t)DF - 1];
float32_t FIR_int1_coeffs[48];

arm_fir_interpolate_instance_f32 FIR_int2_I;
//float32_t FIR_int2_I_state [(32 / (uint32_t)DF1) + BUFFER_SIZE * N_B / (uint32_t)DF1 - 1];
float32_t FIR_int2_I_state [8 + 511]; // (32 / (uint32_t)DF1) + BUFFER_SIZE * N_B / (uint32_t)DF1 - 1];
arm_fir_interpolate_instance_f32 FIR_int2_Q;
//float32_t FIR_int2_Q_state [(32 / (uint32_t)DF1) + BUFFER_SIZE * N_B / (uint32_t)DF1 - 1];
float32_t FIR_int2_Q_state [8 + 511]; //(32 / (uint32_t)DF1) + BUFFER_SIZE * N_B / (uint32_t)DF1 - 1];
float32_t FIR_int2_coeffs[32];

//////////////////////////////////////
// constants for display
//////////////////////////////////////
//int spectrum_y = 120; // upper edge
//#define USE_WATERFALL
int spectrum_y =              115; // upper edge
const int spectrum_x =              10;
const int spectrum_height =         96;
int spectrum_pos_centre_f =         64;
//const int spectrum_height =         56; // height of spectrum display
const int spectrum_WF_height =       96; // height of spectrum plus waterfall
const int BW_indicator_y =          spectrum_y + spectrum_WF_height + 5;
const int WATERFALL_TOP =           spectrum_y + spectrum_height + 4;
const int WATERFALL_BOTTOM =        spectrum_y + spectrum_WF_height + 4;
const int BAND_INDICATOR_X =        277;
const int BAND_INDICATOR_Y =        212;

//const int MAX_PIXEL     240
//uint8_t waterfall[40][255];

const int pos_x_smeter = 11; //5
int pos_y_smeter = (spectrum_y - 12); //94
int16_t s_w = 10;
uint8_t freq_flag[2] = {0, 0};
uint8_t digits_old [2][10] =
{ {9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
  {9, 9, 9, 9, 9, 9, 9, 9, 9, 9}
};
uint8_t erase_flag = 0;
uint8_t WFM_spectrum_flag = 4;
uint16_t leave_WFM = 4;
#define pos 50 // position of spectrum display, has to be < 124
#define pos_version 119 // position of version number printing
#define pos_x_tunestep 100
#define pos_y_tunestep 119 // = pos_y_menu 91
int pos_x_frequency = 12;// !!!5; //21 //100
int pos_y_frequency = 48; //52 //61  //119
#define notchpos spectrum_y + 6
#define notchL 15
#define notchColour ILI9341_YELLOW
int pos_centre_f = 64; //
int oldnotchF = 10000;
float32_t bin_BW = 1.0 / (DF * FFT_length) * SR[SAMPLE_RATE].rate;
// 1/8192 = 0,0001220703125
float32_t bin = 2000.0 / bin_BW;
float32_t notches[10] = {500.0, 1000.0, 1500.0, 2000.0, 2500.0, 3000.0, 3500.0, 4000.0, 4500.0, 5000.0};
uint8_t notches_on[2] = {0, 0};
uint16_t notches_BW[2] = {4, 4}; // no. of bins --> notch BW = no. of bins * bin_BW
int16_t notch_L[2] = {156, 180};
int16_t notch_R[2] = {166, 190};
int16_t notch_pixel_L[2] = {1, 2};
int16_t notch_pixel_R[2] = {2, 3};

uint8_t sch = 0;
float32_t dbm = -145.0;
float32_t dbm_old = -145.0;
float32_t dbmhz = -145.0;
float32_t m_AttackAvedbm = -73.0;
float32_t m_DecayAvedbm = -73.0;
float32_t m_AverageMagdbm = -73.0;
float32_t m_AttackAvedbmhz = -103.0;
float32_t m_DecayAvedbmhz = -103.0;
float32_t m_AverageMagdbmhz = -103.0;

#ifdef HARDWARE_DO7JBH
float32_t dbm_calibration = 3.0; //
#else
float32_t dbm_calibration = 22.0; //
#endif

// ALPHA = 1 - e^(-T/Tau), T = 0.02s (because dbm routine is called every 20ms!)
// Tau     ALPHA
//  10ms   0.8647
//  30ms   0.4866
//  50ms   0.3297
// 100ms   0.1812
// 500ms   0.0391
float32_t m_AttackAlpha = 0.1; //0.08; //0.2;
float32_t m_DecayAlpha  = 0.01; //0.02; //0.05;
int16_t pos_x_dbm = pos_x_smeter + 170;
int16_t pos_y_dbm = pos_y_smeter - 7;
#define DISPLAY_S_METER_DBM       0
#define DISPLAY_S_METER_DBMHZ     1
uint8_t display_dbm = DISPLAY_S_METER_DBM;
uint8_t dbm_state = 0;


#define TUNE_STEP_MIN   0
#define TUNE_STEP1   0    // shortwave
#define TUNE_STEP2   1   // fine tuning
#define TUNE_STEP3   2    //
#define TUNE_STEP4   3    //
#define TUNE_STEP_MAX 3
#define first_tunehelp 1
#define last_tunehelp 3
uint8_t tune_stepper = 0;
int tunestep = 5000; //TUNE_STEP1;
const char* tune_text = "Fast Tune";
uint8_t autotune_flag = 0;
int old_demod_mode = -99;

int8_t first_block = 1;
//uint32_t i = 0;
int32_t FFT_shift = 2048; // which means 1024 bins!

// used for AM demodulation
float32_t audiotmp = 0.0f;
float32_t w = 0.0f;
float32_t wold = 0.0f;
float32_t last_dc_level = 0.0f;
uint8_t audio_flag = 1;

typedef struct DEMOD_Descriptor
{ const uint8_t DEMOD_n;
  const char* const text;
} DEMOD_Desc;
const DEMOD_Descriptor DEMOD [16] =
{
  //   DEMOD_n, name
  {  DEMOD_USB, " USB  "},
  {  DEMOD_LSB, " LSB  "},
  //    {  DEMOD_AM1,  " AM 1 "},
  {  DEMOD_AM2,  " AM  "},
  //    {  DEMOD_AM3,  " AM 3 "},
  //    {  DEMOD_AM_AE1,  "AM-AE1"},
  //    {  DEMOD_AM_AE2,  "AM-AE2"},
  //    {  DEMOD_AM_AE3,  "AM-AE3"},
  //    {  DEMOD_AM_ME1,  "AM-ME1"},
  //    {  DEMOD_AM_ME3,  "AM-ME3"},
  {  DEMOD_SAM, " SAM  "},
  {  DEMOD_SAM_STEREO, "SAM-St"},
  {  DEMOD_IQ, " IQ "},
  {  DEMOD_WFM, " WFM "},
  {  DEMOD_SAM_USB, "SAM-U "},
  {  DEMOD_SAM_LSB, "SAM-L "},
  {  DEMOD_STEREO_LSB, "StLSB "},
  {  DEMOD_STEREO_USB, "StUSB "},
  {  DEMOD_DCF77, "DCF 77"},
  {  DEMOD_AUTOTUNE, "AUTO_T"},
  {  DEMOD_DSB, " DSB  "},
  {  DEMOD_STEREO_DSB, "StDSB "},
  {  DEMOD_AM_ME2,  "AM-ME2"},
};

// Menus !
#define MENU_F_HI_CUT                     0
#define MENU_SPECTRUM_ZOOM                1
#define MENU_SAMPLE_RATE                  2
#define MENU_SAVE_EEPROM                  3
#define MENU_LOAD_EEPROM                  4
#define MENU_PLAYER                       5
#define MENU_LPF_SPECTRUM                 6
#define MENU_SPECTRUM_OFFSET              7
#define MENU_SPECTRUM_DISPLAY_SCALE       8
#define MENU_IQ_AUTO                      9
#define MENU_IQ_AMPLITUDE                 10
#define MENU_IQ_PHASE                     11
#define MENU_CALIBRATION_FACTOR           12
#define MENU_CALIBRATION_CONSTANT         13
#define MENU_TIME_SET                     14
#define MENU_DATE_SET                     15
#define MENU_RESET_CODEC                  16
#define MENU_SPECTRUM_BRIGHTNESS          17
#define MENU_SHOW_SPECTRUM                18

#define first_menu                        0
#define last_menu                         18
#define start_menu                        0
int8_t Menu_pointer =                    start_menu;

#define MENU_VOLUME                       19
#define MENU_RF_GAIN                      20
#define MENU_RF_ATTENUATION               21
#define MENU_BASS                         22
#define MENU_MIDBASS                      23
#define MENU_MID                          24
#define MENU_MIDTREBLE                    25
#define MENU_TREBLE                       26
#define MENU_SAM_ZETA                     27
#define MENU_SAM_OMEGA                    28
#define MENU_SAM_CATCH_BW                 29
#define MENU_NOTCH_1                      30
#define MENU_NOTCH_1_BW                   31
//#define MENU_NOTCH_2                      31
//#define MENU_NOTCH_2_BW                   32
#define MENU_AGC_MODE                     32
#define MENU_AGC_THRESH                   33
#define MENU_AGC_DECAY                    34
#define MENU_AGC_SLOPE                    35
#define MENU_ANR_NOTCH                    36
#define MENU_ANR_TAPS                     37
#define MENU_ANR_DELAY                    38
#define MENU_ANR_MU                       39
#define MENU_ANR_GAMMA                    40
#define MENU_NB_THRESH                    41
#define MENU_NB_TAPS                      42
#define MENU_NB_IMPULSE_SAMPLES           43
#define MENU_STEREO_FACTOR                44
#define MENU_BIT_NUMBER                   45
#define MENU_F_LO_CUT                     46
//#define MENU_NR_L                         46
//#define MENU_NR_N                         47
#define MENU_NR_PSI                       47
#define MENU_NR_ALPHA                     48
#define MENU_NR_BETA                      49
#define MENU_NR_USE_X                     50
#define MENU_NR_USE_KIM                   51
#define MENU_NR_KIM_K                     52
#define MENU_LMS_NR_STRENGTH              53
//#define MENU_NR_VAD_ENABLE                53
//#define MENU_NR_VAD_THRESH                54
//#define MENU_NR_ENABLE                    55
#define MENU_AGC_HANG_ENABLE              54
#define MENU_AGC_HANG_TIME                55
#define MENU_AGC_HANG_THRESH              56
#define first_menu2                       19
#define last_menu2                        53
int8_t Menu2 =                           MENU_VOLUME;
uint8_t which_menu = 1;

typedef struct Menu_Descriptor
{
  const uint8_t no; // Menu ID
  const char* const text1; // upper text
  const char* text2; // lower text
  const uint8_t menu2; // 0 = belongs to Menu, 1 = belongs to Menu2
} Menu_D;


Menu_D Menus [last_menu2 + 1] {
  { MENU_F_HI_CUT, "  Filter", "Hi Cut", 0 },
  { MENU_SPECTRUM_ZOOM, " Spectr", " Zoom ", 0 },
  { MENU_SAMPLE_RATE, "Sample", " Rate ", 0 },
  { MENU_SAVE_EEPROM, " Save ", "Eeprom", 0 },
  { MENU_LOAD_EEPROM, " Load ", "Eeprom", 0 },
  { MENU_PLAYER, "  MP3  ", " Player", 0 },
  { MENU_LPF_SPECTRUM, "Spectr", " LPF  ", 0 },
  { MENU_SPECTRUM_OFFSET, "Spectr", "offset", 0 },
  { MENU_SPECTRUM_DISPLAY_SCALE, "Spectr", " scale ", 0 },
  { MENU_IQ_AUTO, "  IQ  ", " Auto ", 0 },
  { MENU_IQ_AMPLITUDE, "  IQ  ", " gain ", 0 },
  { MENU_IQ_PHASE, "   IQ  ", "  phase ", 0 },
  { MENU_CALIBRATION_FACTOR, "F-calib", "factor", 0 },
  { MENU_CALIBRATION_CONSTANT, "F-calib", "const", 0 },
  { MENU_TIME_SET, " Time ", " Set  ", 0},
  { MENU_DATE_SET, " Date ", " Set  ", 0},
  { MENU_RESET_CODEC, " Reset", " codec ", 0},
  { MENU_SPECTRUM_BRIGHTNESS, "Display", "  dim ", 0},
  { MENU_SHOW_SPECTRUM, " Show ", " spectr", 0},
  { MENU_VOLUME, "Volume", "      ", 1},
  { MENU_RF_GAIN, "   RF  ", "  gain ", 1},
  { MENU_RF_ATTENUATION, "   RF  ", " atten", 1},
  { MENU_BASS, "  Bass ", "  gain ", 1},
  { MENU_MIDBASS, "MidBas", "  gain ", 1},
  { MENU_MID, "  Mids ", "  gain ", 1},
  { MENU_MIDTREBLE, "Midtreb", "  gain ", 1},
  { MENU_TREBLE, "Treble", "  gain ", 1},
  { MENU_SAM_ZETA, "  SAM  ", "  zeta ", 1},
  { MENU_SAM_OMEGA, "  SAM  ", " omega ", 1},
  { MENU_SAM_CATCH_BW, "  SAM  ", "catchB", 1},
  { MENU_NOTCH_1, " notch ", "  freq ", 1},
  { MENU_NOTCH_1, " notch ", "  BW ", 1},
  //{ MENU_NOTCH_2, "notch 2", " freq ", 1},
  //{ MENU_NOTCH_2, "notch 2", "  BW ", 1},
  { MENU_AGC_MODE, "  AGC  ", "  mode  ", 1},
  { MENU_AGC_THRESH, "  AGC  ", " thresh ", 1},
  { MENU_AGC_DECAY, "  AGC  ", " decay ", 1},
  { MENU_AGC_SLOPE, "  AGC  ", " slope  ", 1},
  { MENU_ANR_NOTCH, "  LMS  ", "  type ", 1},
  { MENU_ANR_TAPS, "  LMS  ", "  taps ", 1},
  { MENU_ANR_DELAY, "  LMS  ", " delay ", 1},
  { MENU_ANR_MU, "  LMS  ", "  gain ", 1},
  { MENU_ANR_GAMMA, "  LMS  ", "  leak ", 1},
  { MENU_NB_THRESH, "  NB  ", " thresh ", 1},
  { MENU_NB_TAPS, "  NB  ", " taps ", 1},
  { MENU_NB_IMPULSE_SAMPLES, " NB # ", "impul.", 1},
  { MENU_STEREO_FACTOR, "Stereo", " factor", 1},
  { MENU_BIT_NUMBER, "  Bit ", "number", 1},
  { MENU_F_LO_CUT, "  Filter", "Lo Cut", 1 },
  //  { MENU_NR_L, "  L  ", "  NR ", 1 },
  //  { MENU_NR_N, "  N  ", "  NR ", 1 },
  { MENU_NR_PSI, " PSI ", "  NR ", 1 },
  { MENU_NR_ALPHA, " alpha ", "  NR ", 1 },
  { MENU_NR_BETA, " beta ", "  NR ", 1 },
  { MENU_NR_USE_X, "X or E", "  NR ", 1 },
  { MENU_NR_USE_KIM, " type ", "  NR ", 1 },
  { MENU_NR_KIM_K, "Kim K ", "  NR ", 1 },
  { MENU_LMS_NR_STRENGTH, " LMS ", "strengt", 1 },
  //  { MENU_NR_VAD_ENABLE, " VAD ", "  NR ", 1 },
  //  { MENU_NR_VAD_THRESH, " VAD ", "thresh", 1 },
  //  { MENU_NR_ENABLE, "spectral", "  NR ", 1 }
};
uint8_t eeprom_saved = 0;
uint8_t eeprom_loaded = 0;

#if defined(MP3)
// SD card & MP3 playing
int track;
int tracknum;
int trackext[255]; // 0= nothing, 1= mp3, 2= aac, 3= wav.
String tracklist[255];
File root;
char playthis[15];
boolean trackchange;
boolean paused;
int eeprom_adress = 1900;
#endif

uint8_t iFFT_flip = 0;

// AGC
//#define MAX_SAMPLE_RATE     (12000.0)
#define MAX_SAMPLE_RATE     (24000.0)
#define MAX_N_TAU           (8)
#define MAX_TAU_ATTACK      (0.01)
#define RB_SIZE       (int) (MAX_SAMPLE_RATE * MAX_N_TAU * MAX_TAU_ATTACK + 1)

int8_t AGC_mode = 1;
int pmode = 1; // if 0, calculate magnitude by max(|I|, |Q|), if 1, calculate sqrtf(I*I+Q*Q)
float32_t out_sample[2];
float32_t abs_out_sample;
float32_t tau_attack;
float32_t tau_decay;
int n_tau;
float32_t max_gain;
float32_t var_gain;
float32_t fixed_gain = 1.0;
float32_t max_input;
float32_t out_targ;
float32_t tau_fast_backaverage;
float32_t tau_fast_decay;
float32_t pop_ratio;
uint8_t hang_enable;
float32_t tau_hang_backmult;
float32_t hangtime;
float32_t hang_thresh;
float32_t tau_hang_decay;
float32_t ring[RB_SIZE * 2];
float32_t abs_ring[RB_SIZE];
//assign constants
unsigned ring_buffsize = RB_SIZE;
//do one-time initialization
int out_index = -1;
float32_t ring_max = 0.0;
float32_t volts = 0.0;
float32_t save_volts = 0.0;
float32_t fast_backaverage = 0.0;
float32_t hang_backaverage = 0.0;
int hang_counter = 0;
uint8_t decay_type = 0;
uint8_t state = 0;
int attack_buffsize;
uint32_t in_index;
float32_t attack_mult;
float32_t decay_mult;
float32_t fast_decay_mult;
float32_t fast_backmult;
float32_t onemfast_backmult;
float32_t out_target;
float32_t min_volts;
float32_t inv_out_target;
float32_t tmp;
float32_t slope_constant;
float32_t inv_max_input;
float32_t hang_level;
float32_t hang_backmult;
float32_t onemhang_backmult;
float32_t hang_decay_mult;
int agc_thresh = 30;
int agc_slope = 100;
int agc_decay = 100;
uint8_t agc_action = 0;
uint8_t agc_switch_mode = 0;

// new synchronous AM PLL & PHASE detector
// wdsp Warren Pratt, 2016
//float32_t Sin = 0.0;
//float32_t Cos = 0.0;
float32_t pll_fmax = +4000.0;
int zeta_help = 65;
float32_t zeta = (float32_t)zeta_help / 100.0; // PLL step response: smaller, slower response 1.0 - 0.1
float32_t omegaN = 200.0; // PLL bandwidth 50.0 - 1000.0

//pll
float32_t omega_min = TPI * - pll_fmax * DF / SR[SAMPLE_RATE].rate;
float32_t omega_max = TPI * pll_fmax * DF / SR[SAMPLE_RATE].rate;
float32_t g1 = 1.0 - exp(-2.0 * omegaN * zeta * DF / SR[SAMPLE_RATE].rate);
float32_t g2 = - g1 + 2.0 * (1 - exp(- omegaN * zeta * DF / SR[SAMPLE_RATE].rate) * cosf(omegaN * DF / SR[SAMPLE_RATE].rate * sqrtf(1.0 - zeta * zeta)));
float32_t phzerror = 0.0;
float32_t det = 0.0;
float32_t fil_out = 0.0;
float32_t del_out = 0.0;
float32_t omega2 = 0.0;

//fade leveler
float32_t tauR = 0.02; // original 0.02;
float32_t tauI = 1.4; // original 1.4;
float32_t dc = 0.0;
float32_t dc_insert = 0.0;
float32_t dcu = 0.0;
float32_t dc_insertu = 0.0;
float32_t mtauR = exp(- DF / (SR[SAMPLE_RATE].rate * tauR));
float32_t onem_mtauR = 1.0 - mtauR;
float32_t mtauI = exp(- DF / (SR[SAMPLE_RATE].rate * tauI));
float32_t onem_mtauI = 1.0 - mtauI;
uint8_t fade_leveler = 1;
uint8_t WDSP_SAM = 1;
#define SAM_PLL_HILBERT_STAGES 7
#define OUT_IDX   (3 * SAM_PLL_HILBERT_STAGES)
float32_t c0[SAM_PLL_HILBERT_STAGES];
float32_t c1[SAM_PLL_HILBERT_STAGES];
float32_t ai, bi, aq, bq;
float32_t ai_ps, bi_ps, aq_ps, bq_ps;
float32_t a[3 * SAM_PLL_HILBERT_STAGES + 3];     // Filter a variables
float32_t b[3 * SAM_PLL_HILBERT_STAGES + 3];     // Filter b variables
float32_t c[3 * SAM_PLL_HILBERT_STAGES + 3];     // Filter c variables
float32_t d[3 * SAM_PLL_HILBERT_STAGES + 3];     // Filter d variables
float32_t dsI;             // delayed sample, I path
float32_t dsQ;             // delayed sample, Q path
float32_t corr[2];
float32_t audio;
float32_t audiou;
//int j, k;
float32_t SAM_carrier = 0.0;
float32_t SAM_lowpass = 0.0;
float32_t SAM_carrier_freq_offset = 0.0;
uint16_t  SAM_display_count = 0;

//***********************************************************************
bool timeflag = 0;
const int8_t pos_x_date = 14;
const int8_t pos_y_date = 68;
const int16_t pos_x_time = 232; // 14;
const int16_t pos_y_time = pos_y_frequency; //114;
const int16_t pos_x_a_time = 298;
const int16_t pos_y_a_time = 21; //pos_y_frequency +16;
int helpmin; // definitions for time and date adjust - Menu
int helphour;
int helpday;
int helpmonth;
int helpyear;
int helpsec;
uint8_t hour10_old;
uint8_t hour1_old;
uint8_t minute10_old;
uint8_t minute1_old;
uint8_t second10_old;
uint8_t second1_old;
uint8_t precision_flag = 0;
int8_t mesz = -1;
int8_t mesz_old = 0;

const float displayscale = 0.25;
float32_t display_offset = -10.0;

uint16_t currentScale = 1;        // 20 dB/division
struct dispSc
{ const char *dbText;
  float32_t   dBScale;          // dB per division; includes the 10.0 multiplier for power to dB
  uint16_t    pixelsPerDB;      // Redundant, to some extent. Useful?
  uint16_t    baseOffset;       // Pixels of offset to keep base noise at the baseline
  float32_t   offsetIncrement;  // Change per step of the offset changer
};

struct dispSc displayScale[] =
{
  /*  "20 dB/", 10.0,   2,  25, 1,
    "10 dB/", 20.0,   4,  20, 0.5,
    "5 dB/",  40.0,   8, 38, 0.25,
    "2 dB/",  100.0, 20, 80, 0.1,
    "1 dB/",  200.0, 40, 140, 0.05
  */  "20 dB/", 10.0,   2,  24, 1,
  "10 dB/", 20.0,   4,  30, 0.5,
  "5 dB/",  40.0,   8, 58, 0.25,
  "2 dB/",  100.0, 20, 120, 0.1,
  "1 dB/",  200.0, 40, 200, 0.05
};

float32_t offsetDisplayDB = 0.0;
//int16_t offsetPixels = 0;

#define YTOP_LEVEL_DISP 73
// ADC Bar on left, DAC bar on right
#define ADC_BAR 10
#define DAC_BAR 100
uint16_t adcMaxLevel, dacMaxLevel;    // Plot to bar graph for most modes

uint64_t output12khz = 0ULL;

//const char* const Days[7] = { "Samstag", "Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag"};
const char* const Days[7] = { "Saturday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};

// Automatic noise reduction
// Variable-leak LMS algorithm
// taken from (c) Warren Pratts wdsp library 2016
// GPLv3 licensed
#define ANR_DLINE_SIZE 512 //funktioniert nicht, 128 & 256 OK                 // dline_size
int ANR_taps =     64; //64;                       // taps
int ANR_delay =    16; //16;                       // delay
int ANR_dline_size = ANR_DLINE_SIZE;
int ANR_buff_size = FFT_length / 2.0;
int ANR_position = 0;
float32_t ANR_two_mu =   0.0001;                     // two_mu --> "gain"
float32_t ANR_gamma =    0.1;                      // gamma --> "leakage"
float32_t ANR_lidx =     120.0;                      // lidx
//float32_t ANR_lidx_min = 0.0;                      // lidx_min
float32_t ANR_lidx_min = 120.0;                      // lidx_min
float32_t ANR_lidx_max = 200.0;                      // lidx_max
float32_t ANR_ngamma =   0.001;                      // ngamma
float32_t ANR_den_mult = 6.25e-10;                   // den_mult
float32_t ANR_lincr =    1.0;                      // lincr
float32_t ANR_ldecr =    3.0;                     // ldecr
int ANR_mask = ANR_dline_size - 1;
int ANR_in_idx = 0;
float32_t ANR_d [ANR_DLINE_SIZE];
float32_t ANR_w [ANR_DLINE_SIZE];
uint8_t ANR_on = 0;
uint8_t ANR_notch = 0;

// ordinary LMS noise reduction

float32_t   LMS_errsig1[256 + 10];
arm_lms_norm_instance_f32  LMS_Norm_instance;
arm_lms_instance_f32      LMS_instance;
float32_t                 LMS_StateF32[96 + 256];
float32_t                 LMS_NormCoeff_f32[96 + 256];
float32_t                 LMS_nr_delay[512 + 256];
int                       LMS_nr_strength = 5;

// spectral weighting noise reduction
// based on:
// Kim, H.-G. & D. Ruwisch (2002): Speech enhancement in non-stationary noise environments. – 7th International Conference on Spoken Language Processing [ICSLP 2002]. – ISCA Archive (http://www.isca-speech.org/archive)
//float32_t NR_FFT_buffer [256] // buffer for FFT, defined abo
//float32_t NR_iFFT_buffer [256] // buffer for inverse FFT
//#define USE_NOISEREDUCTION
#define NR_FFT_L    256
//uint8_t NR_enable = 0;
uint8_t NR_Kim = 0; // if 0, NR is off, if 1, use algorithm by Kim et al. 2002, if 2, use algorithm by Romanin et al. 2009
uint8_t NR_use_X = 0;
const uint32_t NR_add_counter = 128;
float32_t NR_sum = 0.0;
const uint8_t NR_L_frames = 3; // default 3 //4 //3//2 //4
const uint8_t NR_N_frames = 15; // default 24 //40 //12 //20 //18//12 //20
float32_t NR_PSI = 3.0; // default 3.0, range of 2.5 - 3.5 ?; 6.0 leads to strong reverb effects
float32_t NR_KIM_K = 1.0; // K is the strength of the KIm & Ruwisch noise reduction
int NR_KIM_K_int = 1000; // this is 1000 * K
//float32_t NR_alpha = 0.99; // default 0.99 --> range 0.98 - 0.9999; 0.95 acts much too hard: reverb effects
float32_t NR_alpha = 0.95; // default 0.99 --> range 0.98 - 0.9999; 0.95 acts much too hard: reverb effects
float32_t NR_onemalpha = (1.0 - NR_alpha);
//float32_t NR_beta = 0.25;
float32_t NR_beta = 0.85;
float32_t NR_onemtwobeta = (1.0 - (2.0 * NR_beta));
float32_t NR_onembeta = 1.0 - NR_beta;
float32_t NR_G_bin_m_1 = 0.0;
float32_t NR_G_bin_p_1 = 0.0;
int8_t NR_first_block = 1;
uint32_t NR_X_pointer = 0;
uint32_t NR_E_pointer = 0;
float32_t NR_T;
float32_t NR_output_audio_buffer [NR_FFT_L];
float32_t NR_last_iFFT_result [NR_FFT_L / 2];
float32_t NR_last_sample_buffer_L [NR_FFT_L / 2];
float32_t NR_last_sample_buffer_R [NR_FFT_L / 2];
float32_t NR_X[NR_FFT_L / 2][3]; // magnitudes (fabs) of the last four values of FFT results for 128 frequency bins
float32_t NR_E[NR_FFT_L / 2][NR_N_frames]; // averaged (over the last four values) X values for the last 20 FFT frames
float32_t NR_M[NR_FFT_L / 2]; // minimum of the 20 last values of E
float32_t NR_Nest[NR_FFT_L / 2][2]; //
//float32_t NR_Hk[NR_FFT_L / 2]; //
float32_t NR_vk; //

float32_t NR_lambda[NR_FFT_L / 2]; // SNR of each current bin
float32_t NR_Gts[NR_FFT_L / 2][2]; // time smoothed gain factors (current and last) for each of the 128 bins
float32_t NR_G[NR_FFT_L / 2]; // preliminary gain factors (before time smoothing) and after that contains the frequency smoothed gain factors

//float32_t NR_beta_Romanin = 0.85; // "best results with 0.85"
//float32_t NR_onembeta_Romanin = 1.0 - NR_beta_Romanin;
//float32_t NR_alpha_Romanin = 0.92; // "should be close to 1.0"
//float32_t NR_onemalpha_Romanin = 1.0 - NR_beta_Romanin;
float32_t NR_SNR_prio[NR_FFT_L / 2];
float32_t NR_SNR_post[NR_FFT_L / 2];
float32_t NR_SNR_post_pos; //[NR_FFT_L / 2];
float32_t NR_Hk_old[NR_FFT_L / 2];
uint8_t NR_VAD_enable = 1;
float32_t NR_VAD = 0.0;
float32_t NR_VAD_thresh = 6.0; // no idea how large this should be !?
uint8_t NR_first_time = 1;
float32_t NR_long_tone[NR_FFT_L / 2][2];
float32_t NR_long_tone_gain[NR_FFT_L / 2];
bool NR_long_tone_reset = true;
bool NR_long_tone_enable = false;
float32_t NR_long_tone_alpha = 0.9999;
float32_t NR_long_tone_thresh = 12000;
bool NR_gain_smooth_enable = false;
float32_t NR_gain_smooth_alpha = 0.25;
float32_t NR_temp_sum = 0.0;
int NR_VAD_delay = 0;
int NR_VAD_duration = 0; //takes the duration of the last vowel
// array of squareroot von Hann coefficients [256]
const float32_t sqrtHann[256] = {0, 0.01231966, 0.024637449, 0.036951499, 0.049259941, 0.061560906, 0.073852527, 0.086132939, 0.098400278, 0.110652682, 0.122888291, 0.135105247, 0.147301698, 0.159475791, 0.171625679, 0.183749518, 0.195845467, 0.207911691, 0.219946358, 0.231947641, 0.24391372, 0.255842778, 0.267733003, 0.279582593, 0.291389747, 0.303152674, 0.314869589, 0.326538713, 0.338158275, 0.349726511, 0.361241666, 0.372701992, 0.384105749, 0.395451207, 0.406736643, 0.417960345, 0.429120609, 0.440215741, 0.451244057, 0.462203884, 0.473093557, 0.483911424, 0.494655843, 0.505325184, 0.515917826, 0.526432163, 0.536866598, 0.547219547, 0.557489439, 0.567674716, 0.577773831, 0.587785252, 0.597707459, 0.607538946, 0.617278221, 0.626923806, 0.636474236, 0.645928062, 0.65528385, 0.664540179, 0.673695644, 0.682748855, 0.691698439, 0.700543038, 0.709281308, 0.717911923, 0.726433574, 0.734844967, 0.743144825, 0.75133189, 0.759404917, 0.767362681, 0.775203976, 0.78292761, 0.790532412, 0.798017227, 0.805380919, 0.812622371, 0.819740483, 0.826734175, 0.833602385, 0.840344072, 0.846958211, 0.853443799, 0.859799851, 0.866025404, 0.872119511, 0.878081248, 0.88390971, 0.889604013, 0.895163291, 0.900586702, 0.905873422, 0.911022649, 0.916033601, 0.920905518, 0.92563766, 0.930229309, 0.934679767, 0.938988361, 0.943154434, 0.947177357, 0.951056516, 0.954791325, 0.958381215, 0.961825643, 0.965124085, 0.968276041, 0.971281032, 0.974138602, 0.976848318, 0.979409768, 0.981822563, 0.984086337, 0.986200747, 0.988165472, 0.989980213, 0.991644696, 0.993158666, 0.994521895, 0.995734176, 0.996795325, 0.99770518, 0.998463604, 0.999070481, 0.99952572, 0.99982925, 0.999981027, 0.999981027, 0.99982925, 0.99952572, 0.999070481, 0.998463604, 0.99770518, 0.996795325, 0.995734176, 0.994521895, 0.993158666, 0.991644696, 0.989980213, 0.988165472, 0.986200747, 0.984086337, 0.981822563, 0.979409768, 0.976848318, 0.974138602, 0.971281032, 0.968276041, 0.965124085, 0.961825643, 0.958381215, 0.954791325, 0.951056516, 0.947177357, 0.943154434, 0.938988361, 0.934679767, 0.930229309, 0.92563766, 0.920905518, 0.916033601, 0.911022649, 0.905873422, 0.900586702, 0.895163291, 0.889604013, 0.88390971, 0.878081248, 0.872119511, 0.866025404, 0.859799851, 0.853443799, 0.846958211, 0.840344072, 0.833602385, 0.826734175, 0.819740483, 0.812622371, 0.805380919, 0.798017227, 0.790532412, 0.78292761, 0.775203976, 0.767362681, 0.759404917, 0.75133189, 0.743144825, 0.734844967, 0.726433574, 0.717911923, 0.709281308, 0.700543038, 0.691698439, 0.682748855, 0.673695644, 0.664540179, 0.65528385, 0.645928062, 0.636474236, 0.626923806, 0.617278221, 0.607538946, 0.597707459, 0.587785252, 0.577773831, 0.567674716, 0.557489439, 0.547219547, 0.536866598, 0.526432163, 0.515917826, 0.505325184, 0.494655843, 0.483911424, 0.473093557, 0.462203884, 0.451244057, 0.440215741, 0.429120609, 0.417960345, 0.406736643, 0.395451207, 0.384105749, 0.372701992, 0.361241666, 0.349726511, 0.338158275, 0.326538713, 0.314869589, 0.303152674, 0.291389747, 0.279582593, 0.267733003, 0.255842778, 0.24391372, 0.231947641, 0.219946358, 0.207911691, 0.195845467, 0.183749518, 0.171625679, 0.159475791, 0.147301698, 0.135105247, 0.122888291, 0.110652682, 0.098400278, 0.086132939, 0.073852527, 0.061560906, 0.049259941, 0.036951499, 0.024637449, 0.01231966, 0};


// noise blanker by Michael Wild
uint8_t NB_on = 0;
float32_t NB_thresh = 2.5;
int8_t NB_taps = 10;
int8_t NB_impulse_samples = 7;
uint8_t NB_test = 0;



// decimation with FIR lowpass for Zoom FFT
arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_I;
arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_Q;
float32_t Fir_Zoom_FFT_Decimate_I_state [4 + BUFFER_SIZE * N_B - 1];
float32_t Fir_Zoom_FFT_Decimate_Q_state [4 + BUFFER_SIZE * N_B - 1];

float32_t Fir_Zoom_FFT_Decimate_coeffs[4];

/****************************************************************************************
    init IIR filters
 ****************************************************************************************/
float32_t coefficient_set[5] = {0, 0, 0, 0, 0};
// 2-pole biquad IIR - definitions and Initialisation
const uint32_t N_stages_biquad_lowpass1 = 1;
float32_t biquad_lowpass1_state [N_stages_biquad_lowpass1 * 4];
float32_t biquad_lowpass1_coeffs[5 * N_stages_biquad_lowpass1] = {0, 0, 0, 0, 0};
arm_biquad_casd_df1_inst_f32 biquad_lowpass1;

const uint32_t N_stages_biquad_WFM = 4;
float32_t biquad_WFM_state [N_stages_biquad_WFM * 4];
float32_t biquad_WFM_coeffs[5 * N_stages_biquad_WFM] = {0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0};
arm_biquad_casd_df1_inst_f32 biquad_WFM;

const uint32_t N_stages_biquad_WFM_R = 4;
float32_t biquad_WFM_R_state [N_stages_biquad_WFM_R * 4];
//float32_t biquad_WFM_coeffs[5 * N_stages_biquad_WFM] = {0,0,0,0,0,  0,0,0,0,0,  0,0,0,0,0,  0,0,0,0,0};
arm_biquad_casd_df1_inst_f32 biquad_WFM_R;

//biquad_WFM_19k
const uint32_t N_stages_biquad_WFM_19k = 1;
float32_t biquad_WFM_19k_state [N_stages_biquad_WFM_19k * 4];
float32_t biquad_WFM_19k_coeffs[5 * N_stages_biquad_WFM_19k] = {0, 0, 0, 0, 0};
arm_biquad_casd_df1_inst_f32 biquad_WFM_19k;

//biquad_WFM_38k
const uint32_t N_stages_biquad_WFM_38k = 1;
float32_t biquad_WFM_38k_state [N_stages_biquad_WFM_38k * 4];
float32_t biquad_WFM_38k_coeffs[5 * N_stages_biquad_WFM_38k] = {0, 0, 0, 0, 0};
arm_biquad_casd_df1_inst_f32 biquad_WFM_38k;

// 4-stage IIRs for Zoom FFT, one each for I & Q
const uint32_t IIR_biquad_Zoom_FFT_N_stages = 4;
float32_t IIR_biquad_Zoom_FFT_I_state [IIR_biquad_Zoom_FFT_N_stages * 4];
float32_t IIR_biquad_Zoom_FFT_Q_state [IIR_biquad_Zoom_FFT_N_stages * 4];
arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_I;
arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_Q;
int zoom_sample_ptr = 0;
uint8_t zoom_display = 0;

const float32_t nuttallWindow256[] = {
  0.0000001, 0.0000073, 0.0000292, 0.0000663, 0.0001192, 0.0001891, 0.0002771, 0.0003851,
  0.0005147, 0.0006684, 0.0008485, 0.0010580, 0.0012998, 0.0015775, 0.0018947, 0.0022554,
  0.0026639, 0.0031248, 0.0036429, 0.0042235, 0.0048719, 0.0055940, 0.0063956, 0.0072832,
  0.0082631, 0.0093423, 0.0105278, 0.0118269, 0.0132470, 0.0147960, 0.0164817, 0.0183122,
  0.0202960, 0.0224414, 0.0247569, 0.0272514, 0.0299336, 0.0328123, 0.0358966, 0.0391953,
  0.0427173, 0.0464717, 0.0504671, 0.0547124, 0.0592163, 0.0639871, 0.0690332, 0.0743626,
  0.0799832, 0.0859024, 0.0921274, 0.0986651, 0.1055218, 0.1127036, 0.1202159, 0.1280637,
  0.1362515, 0.1447831, 0.1536618, 0.1628900, 0.1724698, 0.1824023, 0.1926880, 0.2033264,
  0.2143164, 0.2256560, 0.2373424, 0.2493718, 0.2617397, 0.2744405, 0.2874677, 0.3008139,
  0.3144707, 0.3284289, 0.3426782, 0.3572073, 0.3720040, 0.3870552, 0.4023469, 0.4178639,
  0.4335904, 0.4495095, 0.4656036, 0.4818541, 0.4982416, 0.5147460, 0.5313464, 0.5480212,
  0.5647480, 0.5815041, 0.5982659, 0.6150094, 0.6317101, 0.6483431, 0.6648832, 0.6813048,
  0.6975821, 0.7136890, 0.7295995, 0.7452874, 0.7607267, 0.7758911, 0.7907549, 0.8052924,
  0.8194782, 0.8332872, 0.8466949, 0.8596772, 0.8722106, 0.8842721, 0.8958396, 0.9068915,
  0.9174074, 0.9273674, 0.9367527, 0.9455454, 0.9537289, 0.9612875, 0.9682065, 0.9744726,
  0.9800736, 0.9849988, 0.9892383, 0.9927841, 0.9956291, 0.9977678, 0.9991959, 0.9999106,
  0.9999106, 0.9991959, 0.9977678, 0.9956291, 0.9927841, 0.9892383, 0.9849988, 0.9800736,
  0.9744726, 0.9682065, 0.9612875, 0.9537289, 0.9455454, 0.9367527, 0.9273674, 0.9174074,
  0.9068915, 0.8958396, 0.8842721, 0.8722106, 0.8596772, 0.8466949, 0.8332872, 0.8194782,
  0.8052924, 0.7907549, 0.7758911, 0.7607267, 0.7452874, 0.7295995, 0.7136890, 0.6975821,
  0.6813048, 0.6648832, 0.6483431, 0.6317101, 0.6150094, 0.5982659, 0.5815041, 0.5647480,
  0.5480212, 0.5313464, 0.5147460, 0.4982416, 0.4818541, 0.4656036, 0.4495095, 0.4335904,
  0.4178639, 0.4023469, 0.3870552, 0.3720040, 0.3572073, 0.3426782, 0.3284289, 0.3144707,
  0.3008139, 0.2874677, 0.2744405, 0.2617397, 0.2493718, 0.2373424, 0.2256560, 0.2143164,
  0.2033264, 0.1926880, 0.1824023, 0.1724698, 0.1628900, 0.1536618, 0.1447831, 0.1362515,
  0.1280637, 0.1202159, 0.1127036, 0.1055218, 0.0986651, 0.0921274, 0.0859024, 0.0799832,
  0.0743626, 0.0690332, 0.0639871, 0.0592163, 0.0547124, 0.0504671, 0.0464717, 0.0427173,
  0.0391953, 0.0358966, 0.0328123, 0.0299336, 0.0272514, 0.0247569, 0.0224414, 0.0202960,
  0.0183122, 0.0164817, 0.0147960, 0.0132470, 0.0118269, 0.0105278, 0.0093423, 0.0082631,
  0.0072832, 0.0063956, 0.0055940, 0.0048719, 0.0042235, 0.0036429, 0.0031248, 0.0026639,
  0.0022554, 0.0018947, 0.0015775, 0.0012998, 0.0010580, 0.0008485, 0.0006684, 0.0005147,
  0.0003851, 0.0002771, 0.0001891, 0.0001192, 0.0000663, 0.0000292, 0.0000073, 0.0000001
};


float32_t* mag_coeffs[11] =
{
  // for Index 0 [1xZoom == no zoom] the mag_coeffs will consist of  a NULL  ptr, since the filter is not going to be used in this  mode!
  (float32_t*)NULL,

  (float32_t*)(const float32_t[]) {
    // 2x magnify - index 1
    // 12kHz, sample rate 48k, 60dB stopband, elliptic
    // a1 and a2 negated! order: b0, b1, b2, a1, a2
    // Iowa Hills IIR Filter Designer, DD4WH Aug 16th 2016
    0.228454526413293696,
    0.077639329099949764,
    0.228454526413293696,
    0.635534925142242080,
    -0.170083307068779194,

    0.436788292542003964,
    0.232307972937606161,
    0.436788292542003964,
    0.365885230717786780,
    -0.471769788739400842,

    0.535974654742658707,
    0.557035600464780845,
    0.535974654742658707,
    0.125740787233286133,
    -0.754725697183384336,

    0.501116342273565607,
    0.914877831284765408,
    0.501116342273565607,
    0.013862536615004284,
    -0.930973052446900984
  },

  (float32_t*)(const float32_t[]) {
    // 4x magnify - index 2
    // 6kHz, sample rate 48k, 60dB stopband, elliptic
    // a1 and a2 negated! order: b0, b1, b2, a1, a2
    // Iowa Hills IIR Filter Designer, DD4WH Aug 16th 2016
    0.182208761527446556,
    -0.222492493114674145,
    0.182208761527446556,
    1.326111070880959810,
    -0.468036100821178802,

    0.337123762652097259,
    -0.366352718812586853,
    0.337123762652097259,
    1.337053579516321200,
    -0.644948386007929031,

    0.336163175380826074,
    -0.199246162162897811,
    0.336163175380826074,
    1.354952684569386670,
    -0.828032873168141115,

    0.178588201750411041,
    0.207271695028067304,
    0.178588201750411041,
    1.386486967455699220,
    -0.950935065984588657
  },

  (float32_t*)(const float32_t[]) {
    // 8x magnify - index 3
    // 3kHz, sample rate 48k, 60dB stopband, elliptic
    // a1 and a2 negated! order: b0, b1, b2, a1, a2
    // Iowa Hills IIR Filter Designer, DD4WH Aug 16th 2016
    0.185643392652478922,
    -0.332064345389014803,
    0.185643392652478922,
    1.654637402827731090,
    -0.693859842743674182,

    0.327519300813245984,
    -0.571358085216950418,
    0.327519300813245984,
    1.715375037176782860,
    -0.799055553586324407,

    0.283656142708241688,
    -0.441088976843048652,
    0.283656142708241688,
    1.778230635987093860,
    -0.904453944560528522,

    0.079685368654848945,
    -0.011231810140649204,
    0.079685368654848945,
    1.825046003243238070,
    -0.973184930412286708
  },

  (float32_t*)(const float32_t[]) {
    // 16x magnify - index 4
    // 1k5, sample rate 48k, 60dB stopband, elliptic
    // a1 and a2 negated! order: b0, b1, b2, a1, a2
    // Iowa Hills IIR Filter Designer, DD4WH Aug 16th 2016
    0.194769868656866380,
    -0.379098413160710079,
    0.194769868656866380,
    1.824436402073870810,
    -0.834877726226893380,

    0.333973874901496770,
    -0.646106479315673776,
    0.333973874901496770,
    1.871892825636887640,
    -0.893734096124207178,

    0.272903880596429671,
    -0.513507745397738469,
    0.272903880596429671,
    1.918161772571113750,
    -0.950461788366234739,

    0.053535383722369843,
    -0.069683422367188122,
    0.053535383722369843,
    1.948900719896301760,
    -0.986288064973853129
  },

  (float32_t*)(const float32_t[]) {
    // 32x magnify - index 5
    // 750Hz, sample rate 48k, 60dB stopband, elliptic
    // a1 and a2 negated! order: b0, b1, b2, a1, a2
    // Iowa Hills IIR Filter Designer, DD4WH Aug 16th 2016
    0.201507402588557594,
    -0.400273615727755550,
    0.201507402588557594,
    1.910767558906650840,
    -0.913508748356010480,

    0.340295203367131205,
    -0.674930558961690075,
    0.340295203367131205,
    1.939398230905991390,
    -0.945058078678563840,

    0.271859921641011359,
    -0.535453706265515361,
    0.271859921641011359,
    1.966439529620203740,
    -0.974705666636711099,

    0.047026497485465592,
    -0.084562104085501480,
    0.047026497485465592,
    1.983564238653704900,
    -0.993055129539134551
  },

  (float32_t*)(const float32_t[]) {
    // 64x magnify - index 6
    // 374Hz, sr 48k, 0.02dB ripple, 60dB stopband elliptic
    // DD4WH, 2018_03_24

    0.241056639221550989,
    -0.481274384783607956,
    0.241056639221550989,
    1.949355134029925550,
    -0.950194027689419740,

    0.348059943588306275,
    -0.694622621265274853,
    0.348059943588306275,
    1.966699951543778860,
    -0.968197217455116443,

    0.259592008997311219,
    -0.517100588623714774,
    0.259592008997311219,
    1.983085371558495740,
    -0.985168800929403399,

    0.042223607998797694,
    -0.082088490093798844,
    0.042223607998797694,
    1.993523066505831660,
    -0.995881792409628042
  },

  (float32_t*)(const float32_t[]) {
    // 128x magnify - index 7
    // 187Hz, sample rate 48k, ripple 0.02dB, 60dB stopband, elliptic
    // a1 and a2 negated! order: b0, b1, b2, a1, a2
    // Iowa Hills IIR Filter Designer, DD4WH 2018_03_24
    0.243976032331821663,
    -0.487739726489511083,
    0.243976032331821663,
    1.974570407912224380,
    -0.974782746086356844,

    0.350666090990641666,
    -0.700954871622642472,
    0.350666090990641666,
    1.983591708136026810,
    -0.983969018494667669,

    0.260268176176534360,
    -0.520013508234821287,
    0.260268176176534360,
    1.992032152306574270,
    -0.992554996424821700,

    0.041842895868125313,
    -0.083095418270055094,
    0.041842895868125313,
    1.997347796837673830,
    -0.997938170303869221
  },

  // TODO: calculate new coeffs!
  (float32_t*)(const float32_t[]) {
    // 256x magnify - index 8
    // 187Hz, sample rate 48k, ripple 0.02dB, 60dB stopband, elliptic
    // a1 and a2 negated! order: b0, b1, b2, a1, a2
    // Iowa Hills IIR Filter Designer, DD4WH 2018_03_24
    0.243976032331821663,
    -0.487739726489511083,
    0.243976032331821663,
    1.974570407912224380,
    -0.974782746086356844,

    0.350666090990641666,
    -0.700954871622642472,
    0.350666090990641666,
    1.983591708136026810,
    -0.983969018494667669,

    0.260268176176534360,
    -0.520013508234821287,
    0.260268176176534360,
    1.992032152306574270,
    -0.992554996424821700,

    0.041842895868125313,
    -0.083095418270055094,
    0.041842895868125313,
    1.997347796837673830,
    -0.997938170303869221
  },

  // TODO: calculate new coeffs!
  (float32_t*)(const float32_t[]) {
    // 512x magnify - index 9
    // 187Hz, sample rate 48k, ripple 0.02dB, 60dB stopband, elliptic
    // a1 and a2 negated! order: b0, b1, b2, a1, a2
    // Iowa Hills IIR Filter Designer, DD4WH 2018_03_24
    0.243976032331821663,
    -0.487739726489511083,
    0.243976032331821663,
    1.974570407912224380,
    -0.974782746086356844,

    0.350666090990641666,
    -0.700954871622642472,
    0.350666090990641666,
    1.983591708136026810,
    -0.983969018494667669,

    0.260268176176534360,
    -0.520013508234821287,
    0.260268176176534360,
    1.992032152306574270,
    -0.992554996424821700,

    0.041842895868125313,
    -0.083095418270055094,
    0.041842895868125313,
    1.997347796837673830,
    -0.997938170303869221
  },

  (float32_t*)(const float32_t[]) {
    // 1024x magnify - index 10
    // 187Hz, sample rate 48k, ripple 0.02dB, 60dB stopband, elliptic
    // a1 and a2 negated! order: b0, b1, b2, a1, a2
    // Iowa Hills IIR Filter Designer, DD4WH 2018_03_24
    0.243976032331821663,
    -0.487739726489511083,
    0.243976032331821663,
    1.974570407912224380,
    -0.974782746086356844,

    0.350666090990641666,
    -0.700954871622642472,
    0.350666090990641666,
    1.983591708136026810,
    -0.983969018494667669,

    0.260268176176534360,
    -0.520013508234821287,
    0.260268176176534360,
    1.992032152306574270,
    -0.992554996424821700,

    0.041842895868125313,
    -0.083095418270055094,
    0.041842895868125313,
    1.997347796837673830,
    -0.997938170303869221
  }
};

const uint16_t gradient[] = {
  0x0
  , 0x1
  , 0x2
  , 0x3
  , 0x4
  , 0x5
  , 0x6
  , 0x7
  , 0x8
  , 0x9
  , 0x10
  , 0x1F
  , 0x11F
  , 0x19F
  , 0x23F
  , 0x2BF
  , 0x33F
  , 0x3BF
  , 0x43F
  , 0x4BF
  , 0x53F
  , 0x5BF
  , 0x63F
  , 0x6BF
  , 0x73F
  , 0x7FE
  , 0x7FA
  , 0x7F5
  , 0x7F0
  , 0x7EB
  , 0x7E6
  , 0x7E2
  , 0x17E0
  , 0x3FE0
  , 0x67E0
  , 0x8FE0
  , 0xB7E0
  , 0xD7E0
  , 0xFFE0
  , 0xFFC0
  , 0xFF80
  , 0xFF20
  , 0xFEE0
  , 0xFE80
  , 0xFE40
  , 0xFDE0
  , 0xFDA0
  , 0xFD40
  , 0xFD00
  , 0xFCA0
  , 0xFC60
  , 0xFC00
  , 0xFBC0
  , 0xFB60
  , 0xFB20
  , 0xFAC0
  , 0xFA80
  , 0xFA20
  , 0xF9E0
  , 0xF980
  , 0xF940
  , 0xF8E0
  , 0xF8A0
  , 0xF840
  , 0xF800
  , 0xF802
  , 0xF804
  , 0xF806
  , 0xF808
  , 0xF80A
  , 0xF80C
  , 0xF80E
  , 0xF810
  , 0xF812
  , 0xF814
  , 0xF816
  , 0xF818
  , 0xF81A
  , 0xF81C
  , 0xF81E
  , 0xF81E
  , 0xF81E
  , 0xF81E
  , 0xF83E
  , 0xF83E
  , 0xF83E
  , 0xF83E
  , 0xF85E
  , 0xF85E
  , 0xF85E
  , 0xF85E
  , 0xF87E
  , 0xF87E
  , 0xF83E
  , 0xF83E
  , 0xF83E
  , 0xF83E
  , 0xF85E
  , 0xF85E
  , 0xF85E
  , 0xF85E
  , 0xF87E
  , 0xF87E
  , 0xF87E
  , 0xF87E
  , 0xF87E
  , 0xF87E
  , 0xF87E
  , 0xF87E
  , 0xF87E
  , 0xF87E
  , 0xF87E
  , 0xF87E
  , 0xF87E
  , 0xF88F
  , 0xF88F
  , 0xF88F
};

PROGMEM
void flexRamInfo(void)
{
#if defined(__IMXRT1052__) || defined(__IMXRT1062__)
  int itcm = 0;
  int dtcm = 0;
  int ocram = 0;
  Serial.print("FlexRAM-Banks: [");
  for (int i = 15; i >= 0; i--) {
    switch ((IOMUXC_GPR_GPR17 >> (i * 2)) & 0b11) {
      case 0b00: Serial.print("."); break;
      case 0b01: Serial.print("O"); ocram++; break;
      case 0b10: Serial.print("D"); dtcm++; break;
      case 0b11: Serial.print("I"); itcm++; break;
    }
  }
  Serial.print("] ITCM: ");
  Serial.print(itcm * 32);
  Serial.print(" KB, DTCM: ");
  Serial.print(dtcm * 32);
  Serial.print(" KB, OCRAM: ");
  Serial.print(ocram * 32);
#if defined(__IMXRT1062__)
  Serial.print("(+512)");
#endif
  Serial.println(" KB");
  extern unsigned long _stext;
  extern unsigned long _etext;
  extern unsigned long _sdata;
  extern unsigned long _ebss;
  extern unsigned long _flashimagelen;
  extern unsigned long _heap_start;

  Serial.print("MEM (static usage): ITCM:");
  Serial.print((unsigned)&_etext - (unsigned)&_stext);
  Serial.print(", DTCM:");
  Serial.print((unsigned)&_ebss - (unsigned)&_sdata);
  Serial.print("(");
  Serial.print(dtcm * 32768 - ((unsigned)&_ebss - (unsigned)&_sdata));
  Serial.print(" Bytes free)");
  Serial.print(", OCRAM:");
  Serial.print((unsigned)&_heap_start - 0x20200000);
  Serial.print(", Flash:");
  Serial.println((unsigned)&_flashimagelen);
  Serial.println();
#endif
}

PROGMEM
void setup() {
#ifdef HARDWARE_DO7JBH
  pinMode(On_set, OUTPUT);
  digitalWrite (On_set, HIGH);      // Hold switch on
#endif

  Serial.begin(115200);
  delay(100);

  // all the comments on memory settings and MP3 playing are for FFT size of 1024 !
  // for the large queue sizes at 192ksps sample rate we need a lot of buffers
  //  AudioMemory(130);  // good for 176ksps sample rate, but MP3 playing is not possible
  //  AudioMemory(120); // MP3 works! but in 176k strong distortion . . .
  //  AudioMemory(140); // high sample rates work! but no MP3 and no ZoomFFT
  AudioMemory(170); // no MP3,but Zoom FFT works quite well
  //     AudioMemory(200); // is this overkill?
  //    AudioMemory(100);
  delay(100);

  // get TIME from real time clock with 3V backup battery
  setSyncProvider(getTeensy3Time);

  flexRamInfo();

#if defined(MP3)
  // initialize SD card slot
  if (!(SD.begin(BUILTIN_SDCARD)))
  {
    // print a message
    Serial.println("Unable to access the SD card");
    delay(500);
  }
  //Starting to index the SD card for MP3/AAC.
  root = SD.open("/");

  // reads the last track what was playing.
#if defined(EEPROM_h)
  track = EEPROM.read(eeprom_adress);
#else
  track = 0;
#endif

  while (true) {

    File files =  root.openNextFile();
    if (!files) {
      //If no more files, break out.
      break;
    }
    String curfile = files.name(); //put file in string
    //look for MP3 or AAC files
    int m = curfile.lastIndexOf(".MP3");
    int a = curfile.lastIndexOf(".AAC");
    int a1 = curfile.lastIndexOf(".MP4");
    int a2 = curfile.lastIndexOf(".M4A");
    //int w = curfile.lastIndexOf(".WAV");

    // if returned results is more then 0 add them to the list.
    if (m > 0 || a > 0 || a1 > 0 || a2 > 0 ) {

      tracklist[tracknum] = files.name();
      if (m > 0) trackext[tracknum] = 1;
      if (a > 0) trackext[tracknum] = 2;
      if (a1 > 0) trackext[tracknum] = 2;
      if (a2 > 0) trackext[tracknum] = 2;
      //  if(w > 0) trackext[tracknum] = 3;
      tracknum++;
    }
    // close
    files.close();
  }
  //check if tracknum exist in tracklist from eeprom, like if you deleted some files or added.
  if (track > tracknum) {
    //if it is too big, reset to 0
#if defined(EEPROM_h)
    EEPROM.write(eeprom_adress, 0);
#endif
    track = 0;
  }
  //      Serial.print("tracknum = "); Serial.println(tracknum);

  tracklist[track].toCharArray(playthis, sizeof(tracklist[track]));
#endif //MP3

  /****************************************************************************************
      load saved settings from EEPROM
   ****************************************************************************************/
  // this can be left as-is, because fresh eePROM is detected if loaded for the first time - thanks to Mike / bicycleguy
  EEPROM_LOAD();
  // Enable the audio shield. select input. and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.adcHighPassFilterDisable(); // does not help too much!
  sgtl5000_1.lineInLevel(bands[current_band].RFgain);
  //  sgtl5000_1.lineOutLevel(31);
  sgtl5000_1.lineOutLevel(24);

  sgtl5000_1.audioPostProcessorEnable(); // enables the DAP chain of the codec post audio processing before the headphone out
  //  sgtl5000_1.eqSelect (2); // Tone Control
  sgtl5000_1.eqSelect (3); // five-band-graphic equalizer
  sgtl5000_1.eqBands (bass, midbass, mid, midtreble, treble); // (float bass, etc.) in % -100 to +100
  //  sgtl5000_1.eqBands (bass, treble); // (float bass, float treble) in % -100 to +100
  //  sgtl5000_1.enhanceBassEnable();
  sgtl5000_1.dacVolumeRamp();
  mixleft.gain(0, 1.0);
  mixright.gain(0, 1.0);
  sgtl5000_1.volume((float32_t)audio_volume / 100.0); //

#if defined(HARDWARE_FRANKB)
  Wire.beginTransmission(PCF8574_ADR);
  Wire.write(255); //Init port
  Wire.endTransmission();
#endif
#if defined(BACKLIGHT_PIN)
  pinMode(BACKLIGHT_PIN, OUTPUT );
  //  analogWriteFrequency(BACKLIGHT_PIN, 234375); // change PWM speed in order to prevent disturbance in the audio path
  //  analogWriteResolution(8); // set resolution to 8 bit: well, that´s overkill for backlight, 4 bit would be enough :-)
  // severe disturbance occurs (in the audio loudspeaker amp!) with the standard speed of 488.28Hz, which is well in the audible audio range
  analogWrite(BACKLIGHT_PIN, spectrum_brightness); // 0: dark, 255: bright
#endif
#if defined(BUTTON_1_PIN)
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_3_PIN, INPUT_PULLUP);
  pinMode(BUTTON_4_PIN, INPUT_PULLUP);
  pinMode(BUTTON_5_PIN, INPUT_PULLUP);
  pinMode(BUTTON_6_PIN, INPUT_PULLUP);
  pinMode(BUTTON_7_PIN, INPUT_PULLUP);
  pinMode(BUTTON_8_PIN, INPUT_PULLUP);
#endif

  pinMode(Band1, OUTPUT);  // LPF switches
  pinMode(Band2, OUTPUT);  //

#ifdef HARDWARE_DD4WH
  pinMode(Band3, OUTPUT);  //
  pinMode(Band4, OUTPUT);  //
  pinMode(Band5, OUTPUT);  //
  pinMode(ATT_LE, OUTPUT);
  pinMode(ATT_CLOCK, OUTPUT);
  pinMode(ATT_DATA, OUTPUT);
#endif

#ifdef USE_BOBS_FILTER
  pinMode(Band_3M5_7M3, OUTPUT);
  pinMode(Band_7M3_15M, OUTPUT);
  pinMode(Band_15M_30M, OUTPUT);
#endif

  // *******  Init latching relays <PUA>   ********
  /*  digitalWrite (Band1, HIGH); digitalWrite (Band2, HIGH); digitalWrite (Band3, HIGH);
    digitalWrite (Band4, HIGH); digitalWrite (Band5, HIGH);
    delay(2000);
    digitalWrite (Band1, LOW); digitalWrite (Band2, LOW); digitalWrite (Band3, LOW);
    digitalWrite (Band4, LOW); digitalWrite (Band5, LOW);
    delay(100);
    // For latching relays, this leaves all "bypass" in place. Will be set by set_freq()
  */

#ifdef USE_BOBS_FILTER
  // *******  Init latching relays <PUA>   ********
  digitalWrite (Band_3M5_7M3, HIGH);
  digitalWrite (Band_7M3_15M, HIGH);
  digitalWrite (Band_15M_30M, HIGH);
  delay(2000);
  digitalWrite (Band_3M5_7M3, LOW);
  digitalWrite (Band_7M3_15M, LOW);
  digitalWrite (Band_15M_30M, LOW);
  //  delay(2000);
  //  digitalWrite (Band_3M5_7M3, HIGH);
  delay(100);
  // HIGH == enabled, LOW == bypass
  // For latching relays, this leaves all "bypass" in place. Will be set by set_freq()
#endif

  //  pinMode(AUDIO_AMP_ENABLE, OUTPUT);
  //  digitalWrite(AUDIO_AMP_ENABLE, HIGH);


  tft.begin();
#if defined(HARDWARE_FRANKB)
  tft.setRotation( 1 );
#else
  tft.setRotation( 3 );
#endif
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 1);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setFont(Arial_14);
  tft.print("Teensy Convolution SDR");
  tft.setFont(Arial_10);
  prepare_spectrum_display();
  Init_Display_Clock();

  /****************************************************************************************
     set filter bandwidth
  ****************************************************************************************/
  //setup_mode(bands[current_band].mode);
  // this routine does all the magic of calculating the FIR coeffs (Bessel-Kaiser window)
  //    calc_FIR_coeffs (FIR_Coef, 513, (float32_t)LP_F_help, LP_Astop, 0, 0.0, (float)SR[SAMPLE_RATE].rate / DF);
  calc_cplx_FIR_coeffs (FIR_Coef_I, FIR_Coef_Q, m_NumTaps, (float32_t)bands[current_band].FLoCut, (float32_t)bands[current_band].FHiCut, (float)SR[SAMPLE_RATE].rate / DF);
  //    m_NumTaps = 513;
  //    N_BLOCKS = FFT_length / 2 / BUFFER_SIZE;

  /*  // set to zero all other coefficients in coefficient array
    for(i = 0; i < MAX_NUMCOEF; i++)
    {
    //    Serial.print (FIR_Coef[i]); Serial.print(", ");
        if (i >= m_NumTaps) FIR_Coef[i] = 0.0;
    }
  */
  /****************************************************************************************
     init complex FFTs
  ****************************************************************************************/
  switch (FFT_length)
  {
    case 1024:
      S = &arm_cfft_sR_f32_len1024;
      iS = &arm_cfft_sR_f32_len1024;
      maskS = &arm_cfft_sR_f32_len1024;
      break;
    case 512:
      S = &arm_cfft_sR_f32_len512;
      iS = &arm_cfft_sR_f32_len512;
      maskS = &arm_cfft_sR_f32_len512;
      break;
  }

  spec_FFT = &arm_cfft_sR_f32_len256;
  NR_FFT = &arm_cfft_sR_f32_len256;
  NR_iFFT = &arm_cfft_sR_f32_len256;

  /****************************************************************************************
     Calculate the FFT of the FIR filter coefficients once to produce the FIR filter mask
  ****************************************************************************************/
  init_filter_mask();

  /****************************************************************************************
     show Filter response
  ****************************************************************************************/
  /*    setI2SFreq (8000);
    //    SAMPLE_RATE = 8000;
      delay(200);
      for(uint32_t y=0; y < FFT_length * 2; y++)
      FFT_buffer[y] = 180 * FIR_filter_mask[y];
    //    calc_spectrum_mags(16,0.2);
    //    show_spectrum();
    //    delay(1000);
      SAMPLE_RATE = sr;
  */
  /****************************************************************************************
     Set sample rate
  ****************************************************************************************/
  setI2SFreq (SR[SAMPLE_RATE].rate);
  delay(200); // essential ?
  IF_FREQ = SR[SAMPLE_RATE].rate / 4;

  biquad_lowpass1.numStages = N_stages_biquad_lowpass1; // set number of stages
  biquad_lowpass1.pCoeffs = biquad_lowpass1_coeffs; // set pointer to coefficients file
  for (unsigned i = 0; i < 4 * N_stages_biquad_lowpass1; i++)
  {
    biquad_lowpass1_state[i] = 0.0; // set state variables to zero
  }
  biquad_lowpass1.pState = biquad_lowpass1_state; // set pointer to the state variables

  biquad_WFM.numStages = N_stages_biquad_WFM; // set number of stages
  biquad_WFM.pCoeffs = biquad_WFM_coeffs; // set pointer to coefficients file
  for (unsigned i = 0; i < 4 * N_stages_biquad_WFM; i++)
  {
    biquad_WFM_state[i] = 0.0; // set state variables to zero
  }
  biquad_WFM.pState = biquad_WFM_state; // set pointer to the state variables

  biquad_WFM_R.numStages = N_stages_biquad_WFM_R; // set number of stages
  biquad_WFM_R.pCoeffs = biquad_WFM_coeffs; // set pointer to coefficients file
  for (unsigned i = 0; i < 4 * N_stages_biquad_WFM_R; i++)
  {
    biquad_WFM_R_state[i] = 0.0; // set state variables to zero
  }
  biquad_WFM_R.pState = biquad_WFM_R_state; // set pointer to the state variables

  biquad_WFM_19k.numStages = N_stages_biquad_WFM_19k; // set number of stages
  biquad_WFM_19k.pCoeffs = biquad_WFM_19k_coeffs; // set pointer to coefficients file
  for (unsigned i = 0; i < 4 * N_stages_biquad_WFM_19k; i++)
  {
    biquad_WFM_19k_state[i] = 0.0; // set state variables to zero
  }
  biquad_WFM_19k.pState = biquad_WFM_19k_state; // set pointer to the state variables

  biquad_WFM_38k.numStages = N_stages_biquad_WFM_38k; // set number of stages
  biquad_WFM_38k.pCoeffs = biquad_WFM_38k_coeffs; // set pointer to coefficients file
  for (unsigned i = 0; i < 4 * N_stages_biquad_WFM_38k; i++)
  {
    biquad_WFM_38k_state[i] = 0.0; // set state variables to zero
  }
  biquad_WFM_38k.pState = biquad_WFM_38k_state; // set pointer to the state variables

  /****************************************************************************************
     set filter bandwidth of IIR filter
  ****************************************************************************************/
  // also adjust IIR AM filter
  // calculate IIR coeffs
  LP_F_help = bands[current_band].FHiCut;
  if (LP_F_help < - bands[current_band].FLoCut) LP_F_help = - bands[current_band].FLoCut;
  set_IIR_coeffs ((float32_t)LP_F_help, 1.3, (float32_t)SR[SAMPLE_RATE].rate / DF, 0); // 1st stage
  for (int i = 0; i < 5; i++)
  { // fill coefficients into the right file
    biquad_lowpass1_coeffs[i] = coefficient_set[i];
  }
  // IIR lowpass filter for wideband FM at 15k
  //  set_IIR_coeffs ((float32_t)15000, 0.54, (float32_t)192000, 0); // 1st stage
  set_IIR_coeffs ((float32_t)15000, 0.54, (float32_t)234375, 0); // 1st stage
  //   set_IIR_coeffs ((float32_t)15000, 0.7071, (float32_t)192000, 0); // 1st stage
  for (int i = 0; i < 5; i++)
  { // fill coefficients into the right file
    biquad_WFM_coeffs[i] = coefficient_set[i];
    biquad_WFM_coeffs[i + 10] = coefficient_set[i];
  }
  //  set_IIR_coeffs ((float32_t)15000, 1.3, (float32_t)192000, 0); // 1st stage
  set_IIR_coeffs ((float32_t)15000, 1.3, (float32_t)234375, 0); // 1st stage
  //   set_IIR_coeffs ((float32_t)15000, 0.7071, (float32_t)192000, 0); // 1st stage
  for (int i = 0; i < 5; i++)
  { // fill coefficients into the right file
    biquad_WFM_coeffs[i + 5] = coefficient_set[i];
    biquad_WFM_coeffs[i + 15] = coefficient_set[i];
  }

  // high Q IIR bandpass filter for wideband FM at 19k
  //  set_IIR_coeffs ((float32_t)19000, 1000.0, (float32_t)192000, 2); // 1st stage
  set_IIR_coeffs ((float32_t)19000, 1000.0, (float32_t)234375, 2); // 1st stage
  //   set_IIR_coeffs ((float32_t)19000, 10.0, (float32_t)192000, 2); // 1st stage
  for (int i = 0; i < 5; i++)
  { // fill coefficients into the right file
    biquad_WFM_19k_coeffs[i] = coefficient_set[i];
  }

  // high Q IIR bandpass filter for wideband FM at 38k
  //  set_IIR_coeffs ((float32_t)38000, 1000.0, (float32_t)192000, 2); // 1st stage
  set_IIR_coeffs ((float32_t)38000, 1000.0, (float32_t)234375, 2); // 1st stage
  //   set_IIR_coeffs ((float32_t)38000, 10.0, (float32_t)192000, 2); // 1st stage
  for (int i = 0; i < 5; i++)
  { // fill coefficients into the right file
    biquad_WFM_38k_coeffs[i] = coefficient_set[i];
  }

  set_tunestep();
  show_bandwidth ();

  /****************************************************************************************
     Initiate decimation and interpolation FIR filters
  ****************************************************************************************/

  // Decimation filter 1, M1 = DF1
  //    calc_FIR_coeffs (FIR_dec1_coeffs, 25, (float32_t)5100.0, 80, 0, 0.0, (float32_t)SR[SAMPLE_RATE].rate);
  calc_FIR_coeffs (FIR_dec1_coeffs, n_dec1_taps, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, (float32_t)SR[SAMPLE_RATE].rate);
  if (arm_fir_decimate_init_f32(&FIR_dec1_I, n_dec1_taps, (uint32_t)DF1 , FIR_dec1_coeffs, FIR_dec1_I_state, BUFFER_SIZE * N_BLOCKS)) {
    Serial.println("Init of decimation failed");
    while(1);
  }
  if (arm_fir_decimate_init_f32(&FIR_dec1_Q, n_dec1_taps, (uint32_t)DF1, FIR_dec1_coeffs, FIR_dec1_Q_state, BUFFER_SIZE * N_BLOCKS)) {
    Serial.println("Init of decimation failed");
    while(1);
  }

  // Decimation filter 2, M2 = DF2
  calc_FIR_coeffs (FIR_dec2_coeffs, n_dec2_taps, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, (float32_t)(SR[SAMPLE_RATE].rate / DF1));
  if (arm_fir_decimate_init_f32(&FIR_dec2_I, n_dec2_taps, (uint32_t)DF2, FIR_dec2_coeffs, FIR_dec2_I_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
    Serial.println("Init of decimation failed");
    while(1);
  }
  if (arm_fir_decimate_init_f32(&FIR_dec2_Q, n_dec2_taps, (uint32_t)DF2, FIR_dec2_coeffs, FIR_dec2_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
    Serial.println("Init of decimation failed");
    while(1);
  }

  // Interpolation filter 1, L1 = 2
  // not sure whether I should design with the final sample rate ??
  // yes, because the interpolation filter is AFTER the upsampling, so it has to be in the target sample rate!
  //    calc_FIR_coeffs (FIR_int1_coeffs, 8, (float32_t)5000.0, 80, 0, 0.0, 12000);
  //    calc_FIR_coeffs (FIR_int1_coeffs, 16, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, SR[SAMPLE_RATE].rate / 4.0);
  calc_FIR_coeffs (FIR_int1_coeffs, 48, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, SR[SAMPLE_RATE].rate / 4.0);
  //    if(arm_fir_interpolate_init_f32(&FIR_int1_I, (uint32_t)DF2, 16, FIR_int1_coeffs, FIR_int1_I_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF)) {
  if (arm_fir_interpolate_init_f32(&FIR_int1_I, (uint8_t)DF2, 48, FIR_int1_coeffs, FIR_int1_I_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF)) {
    Serial.println("Init of interpolation failed");
    while(1);
  }
  //    if(arm_fir_interpolate_init_f32(&FIR_int1_Q, (uint32_t)DF2, 16, FIR_int1_coeffs, FIR_int1_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF)) {
  if (arm_fir_interpolate_init_f32(&FIR_int1_Q, (uint8_t)DF2, 48, FIR_int1_coeffs, FIR_int1_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF)) {
    Serial.println("Init of interpolation failed");
    while(1);
  }

  // Interpolation filter 2, L2 = 4
  // not sure whether I should design with the final sample rate ??
  // yes, because the interpolation filter is AFTER the upsampling, so it has to be in the target sample rate!
  //    calc_FIR_coeffs (FIR_int2_coeffs, 4, (float32_t)5000.0, 80, 0, 0.0, 24000);
  //    calc_FIR_coeffs (FIR_int2_coeffs, 16, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, (float32_t)SR[SAMPLE_RATE].rate);
  calc_FIR_coeffs (FIR_int2_coeffs, 32, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, (float32_t)SR[SAMPLE_RATE].rate);

  //    if(arm_fir_interpolate_init_f32(&FIR_int2_I, (uint32_t)DF1, 16, FIR_int2_coeffs, FIR_int2_I_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
  if (arm_fir_interpolate_init_f32(&FIR_int2_I, (uint8_t)DF1, 32, FIR_int2_coeffs, FIR_int2_I_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
    Serial.println("Init of interpolation failed");
    while(1);
  }
  //    if(arm_fir_interpolate_init_f32(&FIR_int2_Q, (uint32_t)DF1, 16, FIR_int2_coeffs, FIR_int2_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
  if (arm_fir_interpolate_init_f32(&FIR_int2_Q, (uint8_t)DF1, 32, FIR_int2_coeffs, FIR_int2_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
    Serial.println("Init of interpolation failed");
    while(1);
  }

#ifdef USE_WFM_FILTER
  arm_fir_init_f32 (&FIR_WFM_I, FIR_WFM_num_taps, FIR_WFM_Coef, FIR_WFM_I_state, (uint32_t)WFM_BLOCKS * BUFFER_SIZE);
  arm_fir_init_f32 (&FIR_WFM_Q, FIR_WFM_num_taps, FIR_WFM_Coef, FIR_WFM_Q_state, (uint32_t)WFM_BLOCKS * BUFFER_SIZE);
  //    arm_fir_init_f32 (&FIR_WFM_I, FIR_WFM_num_taps, FIR_WFM_Coef_I, FIR_WFM_I_state, (uint32_t)(WFM_BLOCKS * BUFFER_SIZE));
  //    arm_fir_init_f32 (&FIR_WFM_Q, FIR_WFM_num_taps, FIR_WFM_Coef_Q, FIR_WFM_Q_state, (uint32_t)(WFM_BLOCKS * BUFFER_SIZE));

#endif

  set_dec_int_filters(); // here, the correct bandwidths are calculated and set accordingly

  /****************************************************************************************
     Coefficients for SAM sideband selection Hilbert filters
     Are these Hilbert transformers built from half-band filters??
  ****************************************************************************************/
  c0[0] = -0.328201924180698;
  c0[1] = -0.744171491539427;
  c0[2] = -0.923022915444215;
  c0[3] = -0.978490468768238;
  c0[4] = -0.994128272402075;
  c0[5] = -0.998458978159551;
  c0[6] = -0.999790306259206;

  c1[0] = -0.0991227952747244;
  c1[1] = -0.565619728761389;
  c1[2] = -0.857467122550052;
  c1[3] = -0.959123933111275;
  c1[4] = -0.988739372718090;
  c1[5] = -0.996959189310611;
  c1[6] = -0.999282492800792;

  /****************************************************************************************
     Zoom FFT: Initiate decimation and interpolation FIR filters AND IIR filters
  ****************************************************************************************/
  float32_t Fstop_Zoom = 0.5 * (float32_t) SR[SAMPLE_RATE].rate / (1 << spectrum_zoom);
  calc_FIR_coeffs (Fir_Zoom_FFT_Decimate_coeffs, 4, Fstop_Zoom, 60, 0, 0.0, (float32_t)SR[SAMPLE_RATE].rate);
  // Attention: max decimation rate is 128 !
  //  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_I, 4, 1 << spectrum_zoom, Fir_Zoom_FFT_Decimate_coeffs, Fir_Zoom_FFT_Decimate_I_state, BUFFER_SIZE * N_BLOCKS)) {
  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_I, 4, 128, Fir_Zoom_FFT_Decimate_coeffs, Fir_Zoom_FFT_Decimate_I_state, BUFFER_SIZE * N_BLOCKS)) {
    Serial.println("Init of decimation failed");
    while(1);
  }
  // same coefficients, but specific state variables
  //  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_Q, 4, 1 << spectrum_zoom, Fir_Zoom_FFT_Decimate_coeffs, Fir_Zoom_FFT_Decimate_Q_state, BUFFER_SIZE * N_BLOCKS)) {
  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_Q, 4, 128, Fir_Zoom_FFT_Decimate_coeffs, Fir_Zoom_FFT_Decimate_Q_state, BUFFER_SIZE * N_BLOCKS)) {
    Serial.println("Init of decimation failed");
    while(1);
  }

  IIR_biquad_Zoom_FFT_I.numStages = IIR_biquad_Zoom_FFT_N_stages; // set number of stages
  IIR_biquad_Zoom_FFT_Q.numStages = IIR_biquad_Zoom_FFT_N_stages; // set number of stages
  for (unsigned i = 0; i < 4 * IIR_biquad_Zoom_FFT_N_stages; i++)
  {
    IIR_biquad_Zoom_FFT_I_state[i] = 0.0; // set state variables to zero
    IIR_biquad_Zoom_FFT_Q_state[i] = 0.0; // set state variables to zero
  }
  IIR_biquad_Zoom_FFT_I.pState = IIR_biquad_Zoom_FFT_I_state; // set pointer to the state variables
  IIR_biquad_Zoom_FFT_Q.pState = IIR_biquad_Zoom_FFT_Q_state; // set pointer to the state variables

  // this sets the coefficients for the ZoomFFT decimation filter
  // according to the desired magnification mode
  // for 0 the mag_coeffs will a NULL  ptr, since the filter is not going to be used in this  mode!
  IIR_biquad_Zoom_FFT_I.pCoeffs = mag_coeffs[spectrum_zoom];
  IIR_biquad_Zoom_FFT_Q.pCoeffs = mag_coeffs[spectrum_zoom];

  Zoom_FFT_prep();

  /****************************************************************************************
     Initialize AGC variables
  ****************************************************************************************/

  AGC_prep();

  /****************************************************************************************
     IQ imbalance correction
  ****************************************************************************************/
  //        Serial.print("1 / K_est: "); Serial.println(1.0 / K_est);
  //        Serial.print("1 / sqrt(1 - P_est^2): "); Serial.println(P_est_mult);
  //        Serial.print("Phasenfehler in Grad: "); Serial.println(- asinf(P_est));


  Serial.print("decimation stage 1: no of taps: "); Serial.println(n_dec1_taps);

  Serial.print("decimation stage 2: no of taps: "); Serial.println(n_dec2_taps);
  Serial.print("fstop2: "); Serial.println(n_fstop2);
  Serial.print("fpass2: "); Serial.println(n_fpass2);


  /****************************************************************************************
     start local oscillator Si5351
  ****************************************************************************************/
  setAttenuator(RF_attenuation);
  si5351.init(SI5351_CRYSTAL_LOAD_10PF, Si_5351_crystal, calibration_constant);
  setfreq();
  delay(100);
  //show_frequency(bands[current_band].freq, 1);

  /****************************************************************************************
      Initialize band settings, set frequency, show frequency etc.
   ****************************************************************************************/
  set_band();

  /****************************************************************************************
      Initialize spectral noise reduction variables
   ****************************************************************************************/

  spectral_noise_reduction_init();
  Init_LMS_NR ();

#ifdef USE_W7PUA
  showSpectrumCorners();
  //    offsetPixels = displayScale[currentScale].baseOffset + (int16_t)(offsetDisplayDB*displayScale[currentScale].pixelsPerDB);
#endif

  /****************************************************************************************
     eePROM check by Mike bicycleguy
  ****************************************************************************************/
  if (gEEPROM_current == false) { //mdrhere
    EEPROM_SAVE();
    gEEPROM_current = true; //future proof, but not used after this
  }

  /****************************************************************************************
     begin to queue the audio from the audio library
  ****************************************************************************************/
  delay(100);
  Q_in_L.begin();
  Q_in_R.begin();

} // END SETUP



void loop() {
  //  asm(" wfi"); // does this save battery power ? https://forum.pjrc.com/threads/40315-Reducing-Power-Consumption
  elapsedMicros usec = 0;
  static float32_t phaseLO = 0.0;
  uint16_t xx;
  static uint16_t barGraphUpdate = 0;
  static uint16_t uB4, uAfter;
  static bool omitOutputFlag = false;

  /**********************************************************************************
      Get samples from queue buffers
   **********************************************************************************/
  // we have to ensure that we have enough audio samples: we need
  // N_BLOCKS = 16
  // decimate by 8
  // FFT512 point --> = 512 / 2 / 128 = 2
  // when these buffers are available, read them in, decimate and perform
  // the FFT - cmplx-mult - iFFT
  //

  // WIDE FM BROADCAST RECEPTION
  if (bands[current_band].mode == DEMOD_WFM)
  {
    if (Q_in_L.available() > 6 && Q_in_R.available() > 6 && Menu_pointer != MENU_PLAYER)
    {
      // get audio samples from the audio  buffers and convert them to float
      for (int i = 0; i < WFM_BLOCKS; i++)
      {
        sp_L = Q_in_L.readBuffer();
        sp_R = Q_in_R.readBuffer();

        switch (bitnumber)
        {
          case 15:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0));
              sp_R[xx] &= ~((1 << 0));
            }
            break;
          case 14:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1));
              sp_R[xx] &= ~((1 << 0) | (1 << 1));
            }
            break;
          case 13:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2));
            }
            break;
          case 12:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
            }
            break;
          case 11:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4));
            }
            break;
          case 10:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
            }
            break;
          case 9:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
            }
            break;
          case 8:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7));
            }
            break;
          case 7:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8));
            }
            break;
          case 6:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9));
            }
            break;
          case 5:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
            }
            break;
          case 4:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
            }
            break;
          case 3:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12));
            }
            break;
          default:
            break;
        }

        // convert to float one buffer_size
        // float_buffer samples are now standardized from > -1.0 to < 1.0
        arm_q15_to_float (sp_L, &float_buffer_L[BUFFER_SIZE * i], BUFFER_SIZE); // convert int_buffer to float 32bit
        arm_q15_to_float (sp_R, &float_buffer_R[BUFFER_SIZE * i], BUFFER_SIZE); // convert int_buffer to float 32bit
        Q_in_L.freeBuffer();
        Q_in_R.freeBuffer();
      }

      /*************************************************************************************************************************************

          Here would be the right place for an FM filter for FM DXing
          plans:
          - 120kHz filter (narrow FM)
          - 80kHz filter (DX)
          - 50kHz filter (extreme DX)
          do those filters have to be phase linear FIR filters ???
       *************************************************************************************************************************************/
#ifdef USE_WFM_FILTER
      // filter both: float_buffer_L and float_buffer_R
      arm_fir_f32 (&FIR_WFM_I, float_buffer_L, float_buffer_L, BUFFER_SIZE * WFM_BLOCKS);
      arm_fir_f32 (&FIR_WFM_Q, float_buffer_R, float_buffer_R, BUFFER_SIZE * WFM_BLOCKS);
#endif
      //      const float32_t WFM_scaling_factor = 0.009; //1.0;//0.01; // 0.01: good, 0.001: rauscht, 0.1: rauscht
      const float32_t WFM_scaling_factor = 0.24; //1.0;//0.01; // 0.01: good, 0.001: rauscht, 0.1: rauscht
      // 0.025 leads to distortion, 0.001 is much too low, 0.1 is quite good???
      //    arm_scale_f32(float_buffer_L, WFM_scaling_factor, float_buffer_L, BUFFER_SIZE * WFM_BLOCKS);
      //    arm_scale_f32(float_buffer_R, WFM_scaling_factor, float_buffer_R, BUFFER_SIZE * WFM_BLOCKS);

      // 0.01 much better than 0.1
      // 0.01 much better than 0.02
      // 0.015 is OK
      // 0.01 is OK --> standard ?
      // 0.011 is (OK)
      // 0.012 is very OK --> standard ?
      // 0.013 is unacceptable
      // 0.04 better than 0.01
      // 0.05 unacceptable
      // 0.1 much too high
      // 0.001 much too low, 0.005 too low
      // 0.009 --> PERFECT

      //#define LYONS_DEMOD_FM
      //#define CSDR_FM_DEMOD_ATAN
      //#define CSDR_FM_DEMOD_QUADRI
#define WFM_KA7OEI

#ifdef  LYONS_DEMOD_FM
      // distorted audio
      FFT_buffer[0] =   ((float_buffer_R[0] - Q_old_old) * I_old)
                        - ((float_buffer_L[0] - I_old_old) * Q_old);
      FFT_buffer[0] /=  float_buffer_L[0] * float_buffer_L[0] + float_buffer_R[0] * float_buffer_R[0];
      FFT_buffer[1] =   ((float_buffer_R[1] - Q_old) * float_buffer_L[0])
                        - ((float_buffer_L[1] - I_old) * float_buffer_R[0]);
      FFT_buffer[1] /=  float_buffer_L[1] * float_buffer_L[1] + float_buffer_R[1] * float_buffer_R[1];

      for (int i = 2; i < BUFFER_SIZE * WFM_BLOCKS; i++)
      {
        FFT_buffer[i] =   ((float_buffer_R[i] - float_buffer_R[i - 2]) * float_buffer_L[i - 1])
                          - ((float_buffer_L[i] - float_buffer_L[i - 2]) * float_buffer_R[i - 1]);
        FFT_buffer[i] /=  float_buffer_L[i] * float_buffer_L[i] + float_buffer_R[i] * float_buffer_R[i];
      }
      I_old = float_buffer_L[BUFFER_SIZE * WFM_BLOCKS - 1];
      Q_old = float_buffer_R[BUFFER_SIZE * WFM_BLOCKS - 1];
      I_old_old = float_buffer_L[BUFFER_SIZE * WFM_BLOCKS - 2];
      Q_old_old = float_buffer_R[BUFFER_SIZE * WFM_BLOCKS - 2];

      // maybe scaling helps??? --> nor really
      arm_scale_f32(FFT_buffer, 0.01, FFT_buffer, BUFFER_SIZE * WFM_BLOCKS);
#endif

#ifdef CSDR_FM_DEMOD_ATAN
      //Serial.println("CSDR");
      for (int i = 0; i < BUFFER_SIZE * WFM_BLOCKS; i++)
      {
        // WFM_phase = atan2f(float_buffer_R[i], float_buffer_L[i]);
        WFM_phase = atan2f(float_buffer_L[i], float_buffer_R[i]);
        WFM_dphase = WFM_phase - WFM_lastphase;
        if (WFM_dphase < -PI) WFM_dphase += 2 * PI;
        if (WFM_dphase > PI) WFM_dphase -= 2 * PI;
        FFT_buffer[i] = 0.1 * WFM_dphase / PI;
        WFM_lastphase = WFM_phase;
      }
#endif

#ifdef CSDR_FM_DEMOD_QUADRI
      // (qnow*ilast-inow*qlast)/(inow*inow+qnow*qnow)

      FFT_buffer[0] = 0.01 * (float_buffer_R[0] * I_old - float_buffer_L[0] * Q_old)
                      / (float_buffer_R[0] * float_buffer_R[0] + float_buffer_L[0] * float_buffer_L[0] + 1e-6);

      for (int i = 1; i < BUFFER_SIZE * WFM_BLOCKS; i++)
      {
        FFT_buffer[i] = 0.01 * (float_buffer_R[i] * float_buffer_L[i - 1] - float_buffer_L[i] * float_buffer_R[i - 1])
                        / (float_buffer_R[i] * float_buffer_R[i] + float_buffer_L[i] * float_buffer_L[i] + 1e-6);
      }
      I_old = float_buffer_L[BUFFER_SIZE * WFM_BLOCKS - 1];
      Q_old = float_buffer_R[BUFFER_SIZE * WFM_BLOCKS - 1];
#endif

#ifdef WFM_KA7OEI
      //      Serial.println("KA7OEI");

      // KA7OEI
#ifdef USE_ATAN2FAST
      FFT_buffer[0] = WFM_scaling_factor * arm_atan2_f32(I_old * float_buffer_R[0] - float_buffer_L[0] * Q_old,
                      I_old * float_buffer_L[0] + float_buffer_R[0] * Q_old);
#else
      FFT_buffer[0] = WFM_scaling_factor * atan2f(I_old * float_buffer_R[0] - float_buffer_L[0] * Q_old,
                      I_old * float_buffer_L[0] + float_buffer_R[0] * Q_old);
#endif
      for (int i = 1; i < BUFFER_SIZE * WFM_BLOCKS; i++)
      {

        // KA7OEI: http://ka7oei.blogspot.com/2015/11/adding-fm-to-mchf-sdr-transceiver.html

#ifdef USE_ATAN2FAST
        FFT_buffer[i] = WFM_scaling_factor * arm_atan2_f32(float_buffer_L[i - 1] * float_buffer_R[i] - float_buffer_L[i] * float_buffer_R[i - 1],
                        float_buffer_L[i - 1] * float_buffer_L[i] + float_buffer_R[i] * float_buffer_R[i - 1]);
#else
        FFT_buffer[i] = WFM_scaling_factor * atan2f(float_buffer_L[i - 1] * float_buffer_R[i] - float_buffer_L[i] * float_buffer_R[i - 1],
                        float_buffer_L[i - 1] * float_buffer_L[i] + float_buffer_R[i] * float_buffer_R[i - 1]);
#endif
      }

      // take care of last sample of each block
      I_old = float_buffer_L[BUFFER_SIZE * WFM_BLOCKS - 1];
      Q_old = float_buffer_R[BUFFER_SIZE * WFM_BLOCKS - 1];

#endif

      if (stereo_factor > 0.1f)
      {
        /*******************************************************************************************************

           STEREO first trial
           39.2% in MONO
         *******************************************************************************************************/
        for (int i = 0; i < BUFFER_SIZE * WFM_BLOCKS; i++)
        { // DC removal filter -----------------------
          w = FFT_buffer[i] + wold * 0.9999f; // yes, I want a superb bass response ;-)
          FFT_buffer[i] = w - wold;
          wold = w;
        }

        // THIS IS REALLY A GREAT MESS WITH ALL THE COPYING INTO THOSE DIFFERENT BUFFERS
        // BEWARE!

        // TODO: substitute all IIR filters by FIR filters with linear phase for better stereo resolution!

        // audio of L + R channel is in FFT_buffer

        // 1. BPF 19k for extracting the pilot tone
        arm_biquad_cascade_df1_f32 (&biquad_WFM_19k, FFT_buffer, float_buffer_L, BUFFER_SIZE * WFM_BLOCKS);
        // float_buffer_L --> 19k pilot

        // 2. if negative --> positive in order to rectify the wave
        for (int i = 0; i < BUFFER_SIZE * WFM_BLOCKS; i++)
        {
          if (float_buffer_L[i] < 0.0) float_buffer_L[i] = - float_buffer_L[i];
        }
        // float_buffer_L --> 19k pilot rectified

        // 2.b) eliminate DC
        for (int i = 0; i < BUFFER_SIZE * WFM_BLOCKS; i++)
        { // DC removal filter -----------------------
          w = float_buffer_L[i] + wold * 0.9999f; // yes, I want a superb bass response ;-)
          float_buffer_L[i] = w - wold;
          wold = w;
        }
        // float_buffer_L --> 19k pilot rectified & without DC

        // 3. BPF 38kHz to extract the double f pilot tone
        arm_biquad_cascade_df1_f32 (&biquad_WFM_38k, float_buffer_L, float_buffer_R, BUFFER_SIZE * WFM_BLOCKS);
        // float_buffer_R --> 38k pilot

        // make 38k pilot tone a rectangle in order to have it of equal size independent of signal strength
        // thanks Martin Oßmann for this hint!
        for (int i = 0; i < BUFFER_SIZE * WFM_BLOCKS; i++)
        {
          if (float_buffer_R[i] > 0) float_buffer_R[i] = 0.002;
          else float_buffer_R[i] = -0.002; // factor 0.002 * 1000 (=stereo_factor) = 2 ! multiplication because L-R is DSB signal
        }

        // 4. L-R = multiply audio with 38k carrier in order to produce audio L - R
        for (int i = 0; i < BUFFER_SIZE * WFM_BLOCKS; i++)
        {
          float_buffer_L[i] = stereo_factor * float_buffer_R[i] * FFT_buffer[i];
        }
        // float_buffer_L --> L-R

        // 6. Right channel:
        for (int i = 0; i < BUFFER_SIZE * WFM_BLOCKS; i++)
        {
          iFFT_buffer[i] = FFT_buffer[i] - float_buffer_L[i];
        }
        // iFFT_buffer --> RIGHT CHANNEL

        // 7. Left channel
        for (int i = 0; i < BUFFER_SIZE * WFM_BLOCKS; i++)
        {
          float_buffer_R[i] = FFT_buffer[i] + float_buffer_L[i];
        }
        // float_buffer_R --> LEFT CHANNEL

        // Right channel: lowpass filter with 15kHz Fstop & deemphasis
        rawFM_old_R = deemphasis_wfm_ff (iFFT_buffer, FFT_buffer, BUFFER_SIZE * WFM_BLOCKS, SR[SAMPLE_RATE].rate, rawFM_old_R);
        arm_biquad_cascade_df1_f32 (&biquad_WFM, FFT_buffer, float_buffer_L, BUFFER_SIZE * WFM_BLOCKS);

        // FFT_buffer --> RIGHT CHANNEL PERFECT AUDIO

        // Left channel: lowpass filter with 15kHz Fstop & deemphasis
        rawFM_old_L = deemphasis_wfm_ff (float_buffer_R, FFT_buffer, BUFFER_SIZE * WFM_BLOCKS, SR[SAMPLE_RATE].rate, rawFM_old_L);
        arm_biquad_cascade_df1_f32 (&biquad_WFM_R, FFT_buffer, iFFT_buffer, BUFFER_SIZE * WFM_BLOCKS);

      }
      //*****END of STEREO***********************************************************************************************************

      else // MONO
      { // in FFT_buffer is perfect audio, but with preemphasis on the treble


        // lowpass filter with 15kHz Fstop
        //        arm_biquad_cascade_df1_f32 (&biquad_WFM, FFT_buffer, float_buffer_R, BUFFER_SIZE * WFM_BLOCKS);
        //        arm_biquad_cascade_df1_f32 (&biquad_WFM, FFT_buffer, float_buffer_L, BUFFER_SIZE * WFM_BLOCKS);
        //    arm_copy_f32(FFT_buffer, float_buffer_L, BUFFER_SIZE * WFM_BLOCKS);
        //      arm_copy_f32(FFT_buffer, float_buffer_R, BUFFER_SIZE * WFM_BLOCKS);

        // de-emphasis with exponential averager
        float_buffer_L[0] = FFT_buffer[0] * deemp_alpha + alt_WFM_audio * onem_deemp_alpha;
        for (int idx = 1; idx < BUFFER_SIZE * WFM_BLOCKS; idx++)
        {
          float_buffer_L[idx] = FFT_buffer[idx] * 0.09 + float_buffer_L[idx - 1] * 0.91;
          iFFT_buffer[idx] = float_buffer_L[idx];
        }
        iFFT_buffer[0] = float_buffer_L[0];
        alt_WFM_audio = float_buffer_L[(BUFFER_SIZE * WFM_BLOCKS) - 1];
      }

      if (Q_in_L.available() >  25)
      {
        AudioNoInterrupts();
        Q_in_L.clear();
        n_clear ++; // just for debugging to check how often this occurs [about once in an hour of playing . . .]
        AudioInterrupts();
      }
      if (Q_in_R.available() >  25)
      {
        AudioNoInterrupts();
        Q_in_R.clear();
        n_clear ++; // just for debugging to check how often this occurs [about once in an hour of playing . . .]
        AudioInterrupts();
      }

      // it should be possible to do RDS decoding???
      // preliminary idea for the workflow:
      /*
         192ksp sample rate of I & Q 
        Calculate real signal by atan2f (Nyquist is now 96k)
        Highpass-filter at 54kHz  keep 54 – 96k
        Decimate-by-2 (96ksps, Nyquist 48k)
        57kHz == 39kHz
        Highpass 36kHz  keep 36-48k
        Decimate-by-3 (32ksps, Nyquist 16k)
        39kHz = 7kHz
        Lowpass 8k
        Decimate-by-2 (16ksps, Nyquist 8k)
        Mix to baseband with 7kHz oscillator
        Lowpass 4k
        Decimate-by-2 (8ksps, Nyquist 4k)
        Decode baseband RDS signal
      */

      //      arm_scale_f32(float_buffer_L, 3.0, float_buffer_L, BUFFER_SIZE * WFM_BLOCKS);
      //      arm_scale_f32(iFFT_buffer, 3.0, iFFT_buffer, BUFFER_SIZE * WFM_BLOCKS);

      for (int i = 0; i < WFM_BLOCKS; i++)
      {
        sp_L = Q_out_L.getBuffer();
        sp_R = Q_out_R.getBuffer();
        arm_float_to_q15 (&float_buffer_L[BUFFER_SIZE * i], sp_L, BUFFER_SIZE);
        arm_float_to_q15 (&iFFT_buffer[BUFFER_SIZE * i], sp_R, BUFFER_SIZE);
        Q_out_L.playBuffer(); // play it !
        Q_out_R.playBuffer(); // play it !
      }

      WFM_spectrum_flag++;
      if (show_spectrum_flag)
      {
        if (WFM_spectrum_flag == 2)
        {
          //            spectrum_zoom == SPECTRUM_ZOOM_1;
          zoom_display = 1;
          //            calc_256_magn();
          Zoom_FFT_prep();
          Zoom_FFT_exe(WFM_BLOCKS * BUFFER_SIZE);
        }
        else if (WFM_spectrum_flag >= 4)
        {
          //            if(stereo_factor < 0.1)
          {
            show_spectrum();
          }
          WFM_spectrum_flag = 0;
        }
      }

      sum = sum + usec;
      idx_t++;

      if (idx_t > 1000)
        //          if (five_sec.check() == 1)
      {
        tft.fillRect(227, 5, 45, 20, ILI9341_BLACK);
        tft.setCursor(227, 5);
        tft.setTextColor(ILI9341_GREEN);
        tft.setFont(Arial_9);
        mean = sum / idx_t;
        if (mean / 29.00 * SR[SAMPLE_RATE].rate / AUDIO_SAMPLE_RATE_EXACT / WFM_BLOCKS < 100.0)
        {
          tft.print (mean / 29.00 * SR[SAMPLE_RATE].rate / AUDIO_SAMPLE_RATE_EXACT / WFM_BLOCKS);
          tft.print("%");
        }
        else
        {
          tft.setTextColor(ILI9341_RED);
          tft.print("100%");
        }
        //          tft.print (mean/29.0 * SR[SAMPLE_RATE].rate / AUDIO_SAMPLE_RATE_EXACT / WFM_BLOCKS);tft.print("%");
        idx_t = 0;
        sum = 0;
        //          Serial.print(" n_clear = "); Serial.println(n_clear);
        //          Serial.print ("1 - Alpha = "); Serial.println(onem_deemp_alpha);
        //          Serial.print ("Alpha = "); Serial.println(deemp_alpha);

      }
    }
  }
  /**************************************************************************************************************************************************

     longwave / mediumwave / shortwave starts here

   **************************************************************************************************************************************************/

  else
    // are there at least N_BLOCKS buffers in each channel available ?
    if (Q_in_L.available() > N_BLOCKS + 0 && Q_in_R.available() > N_BLOCKS + 0 && Menu_pointer != MENU_PLAYER)
    {
      // get audio samples from the audio  buffers and convert them to float
      // read in 32 blocks á 128 samples in I and Q
      for (unsigned i = 0; i < N_BLOCKS; i++)
      {
        sp_L = Q_in_L.readBuffer();
        sp_R = Q_in_R.readBuffer();

        // Test: artificially restrict int16_t to 12bit resolution
        // lösche bits 0 bis 3
        switch (bitnumber)
        {
          case 15:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0));
              sp_R[xx] &= ~((1 << 0));
            }
            break;
          case 14:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1));
              sp_R[xx] &= ~((1 << 0) | (1 << 1));
            }
            break;
          case 13:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2));
            }
            break;
          case 12:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
            }
            break;
          case 11:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4));
            }
            break;
          case 10:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
            }
            break;
          case 9:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
            }
            break;
          case 8:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7));
            }
            break;
          case 7:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8));
            }
            break;
          case 6:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9));
            }
            break;
          case 5:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
            }
            break;
          case 4:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
            }
            break;
          case 3:
            for (int xx = 0; xx < 128; xx++)
            {
              sp_L[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12));
              sp_R[xx] &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12));
            }
            break;
          default:
            break;
        }

        adcMaxLevel = 0;
        // set clip state flags for codec gain adjustment in codec_gain()
        for (int xx = 0; xx < 128; xx++)
        {
          if (sp_L[xx] > adcMaxLevel)
            adcMaxLevel = sp_L[xx];
          if (sp_L[xx] > 10000)                    // 4096)
          {
            quarter_clip = 1;
            if (sp_L[xx] > 20000)                 //   8192)
            {
              half_clip = 1;
            }
          }
        }      // End, signal level check

        // convert to float one buffer_size
        // float_buffer samples are now standardized from > -1.0 to < 1.0
        arm_q15_to_float (sp_L, &float_buffer_L[BUFFER_SIZE * i], BUFFER_SIZE); // convert int_buffer to float 32bit
        arm_q15_to_float (sp_R, &float_buffer_R[BUFFER_SIZE * i], BUFFER_SIZE); // convert int_buffer to float 32bit
        Q_in_L.freeBuffer();
        Q_in_R.freeBuffer();
        //     blocks_read ++;
      }

      /*      // this is supposed to prevent overfilled queue buffers
            // rarely the Teensy audio queue gets a hickup
            // in that case this keeps the whole audio chain running smoothly
            if (Q_in_L.available() >  5)
            {
              AudioNoInterrupts();
              Q_in_L.clear();
              n_clear ++; // just for debugging to check how often this occurs [about once in an hour of playing . . .]
              AudioInterrupts();
            }
            if (Q_in_R.available() >  5)
            {
              AudioNoInterrupts();
              Q_in_R.clear();
              n_clear ++; // just for debugging to check how often this occurs [about once in an hour of playing . . .]
              AudioInterrupts();
            }
      */
      /***********************************************************************************************
          just for checking: plotting min/max and mean of the samples
       ***********************************************************************************************/
      //    if (ms_500.check() == 1)
      if (0)
      {
        float32_t sample_min = 0.0;
        float32_t sample_max = 0.0;
        float32_t sample_mean = 0.0;
        uint32_t min_index, max_index;
        arm_mean_f32(float_buffer_L, BUFFER_SIZE * N_BLOCKS, &sample_mean);
        arm_max_f32(float_buffer_L, BUFFER_SIZE * N_BLOCKS, &sample_max, &max_index);
        arm_min_f32(float_buffer_L, BUFFER_SIZE * N_BLOCKS, &sample_min, &min_index);
        Serial.print("Sample min: "); Serial.println(sample_min);
        Serial.print("Sample max: "); Serial.println(sample_max);
        Serial.print("Max index: "); Serial.println(max_index);

        Serial.print("Sample mean: "); Serial.println(sample_mean);
        Serial.print("FFT_length: "); Serial.println(FFT_length);
        Serial.print("N_BLOCKS: "); Serial.println(N_BLOCKS);
        Serial.println(BUFFER_SIZE * N_BLOCKS / 8);
      }


      /***********************************************************************************************
          IQ amplitude and phase correction
       ***********************************************************************************************/

      if (!auto_IQ_correction)
      {
        // Manual IQ amplitude correction
        // to be honest: we only correct the amplitude of the I channel ;-)
        arm_scale_f32 (float_buffer_L, IQ_amplitude_correction_factor, float_buffer_L, BUFFER_SIZE * N_BLOCKS);
        // IQ phase correction
        IQ_phase_correction(float_buffer_L, float_buffer_R, IQ_phase_correction_factor, BUFFER_SIZE * N_BLOCKS);
        //        Serial.println("Manual IQ correction");
      }
      else
        /*    {
                // introduce artificial amplitude imbalance
                arm_scale_f32 (float_buffer_R, 0.97, float_buffer_R, BUFFER_SIZE * N_BLOCKS);
                // introduce artificial phase imbalance
                IQ_phase_correction(float_buffer_L, float_buffer_R, +0.05, BUFFER_SIZE * N_BLOCKS);
            }
        */
        /*******************************************************************************************************

          algorithm by Moseley & Slump
        ********************************************************************************************************/

        if (MOSELEY)
        { // Moseley, N.A. & C.H. Slump (2006): A low-complexity feed-forward I/Q imbalance compensation algorithm.
          // in 17th Annual Workshop on Circuits, Nov. 2006, pp. 158–164.
          // http://doc.utwente.nl/66726/1/moseley.pdf

          if (twinpeaks_tested == 3) // delete "IQ test"-display after a while, when everything is OK
          {
            twinpeaks_counter++;
            if (twinpeaks_counter >= 200)
            { // delete IQ test message
              tft.fillRect(spectrum_x + 256 + 2, pos_y_time + 17, 320 - spectrum_x - 58, 31, ILI9341_BLACK);
              twinpeaks_tested = 1;
            }
          }
          if (twinpeaks_tested == 2)
          {
            twinpeaks_counter++;
#ifdef DEBUG
            Serial.print("twinpeaks counter = "); Serial.println(twinpeaks_counter);
#endif
            if (twinpeaks_counter == 1)
            {
              tft.fillRect(spectrum_x + 256 + 3, pos_y_time + 18, 49, 28, ILI9341_RED);
              tft.drawRect(spectrum_x + 256 + 2, pos_y_time + 17, 320 - spectrum_x - 258, 31, ILI9341_MAROON);
              tft.setCursor(spectrum_x + 256 + 6, pos_y_time + 19);
              tft.setFont(Arial_12);
              tft.setTextColor(ILI9341_WHITE);
              tft.print("IQtest");
            }
            tft.setCursor(pos_x_time + 55, pos_y_time + 19 + 14);
            tft.setFont(Arial_12);
            if (twinpeaks_counter)
            {
              tft.setTextColor(ILI9341_RED);
              tft.print(800 - twinpeaks_counter + 1);
            }
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(pos_x_time + 55, pos_y_time + 19 + 14);
            tft.print(800 - twinpeaks_counter);
          }
          //        if(twinpeaks_counter >= 500) // wait 500 cycles for the system to settle: compare fig. 11 in Moseley & Slump (2006)
          if (twinpeaks_counter >= 800 && twinpeaks_tested == 2) // wait 800 cycles for the system to settle: compare fig. 11 in Moseley & Slump (2006)
            // it takes quite a while until the automatic IQ correction has really settled (because of the built-in lowpass filter in the algorithm):
            // we take only the first 256 of the 4096 samples to calculate the IQ correction factors
            // 500 cycles x 4096 samples (@96ksps sample rate) = 21.33 sec
          {
            twinpeaks_tested = 0;
            twinpeaks_counter = 0;
#ifdef DEBUG
            Serial.println("twinpeaks_counter ready to test IQ balance !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1");
#endif
          }
          for (unsigned i = 0; i < n_para; i++)
          {
            teta1 += sign(float_buffer_L[i]) * float_buffer_R[i]; // eq (34)
            teta2 += sign(float_buffer_L[i]) * float_buffer_L[i]; // eq (35)
            teta3 += sign (float_buffer_R[i]) * float_buffer_R[i]; // eq (36)
          }
          teta1 = -0.01 * (teta1 / n_para) + 0.99 * teta1_old; // eq (34) and first order lowpass
          teta2 = 0.01 * (teta2 / n_para) + 0.99 * teta2_old; // eq (35) and first order lowpass
          teta3 = 0.01 * (teta3 / n_para) + 0.99 * teta3_old; // eq (36) and first order lowpass

          if (teta2 != 0.0) // prevent divide-by-zero
          {
            M_c1 = teta1 / teta2; // eq (30)
          }
          else
          {
            M_c1 = 0.0;
          }

          float32_t moseley_help = (teta2 * teta2);
          if (moseley_help > 0.0) // prevent divide-by-zero
          {
            moseley_help = (teta3 * teta3 - teta1 * teta1) / moseley_help; // eq (31)
          }
          if (moseley_help > 0.0)// prevent sqrtf of negative value
          {
            M_c2 = sqrtf(moseley_help); // eq (31)
          }
          else
          {
            M_c2 = 1.0;
          }
          // Test and fix of the "twinpeak syndrome"
          // which occurs sporadically and can -to our knowledge- only be fixed
          // by a reset of the codec
          // It can be identified by a totally non-existing mirror rejection,
          // so I & Q have essentially the same phase
          // We use this to identify the snydrome and reset the codec accordingly:
          // calculate phase between I & Q

          if (teta3 != 0.0 && twinpeaks_tested == 0) // prevent divide-by-zero
            // twinpeak_tested = 2 --> wait for system to warm up
            // twinpeak_tested = 0 --> go and test the IQ phase
            // twinpeak_tested = 1 --> tested, verified, go and have a nice day!
          {
#ifdef DEBUG
            Serial.println("HERE");
#endif
            // Moseley & Slump (2006) eq. (33)
            // this gives us the phase error between I & Q in radians
            float32_t phase_IQ = asinf(teta1 / teta3);
#ifdef DEBUG
            Serial.print("asinf = "); Serial.println(phase_IQ);
#endif
            if ((phase_IQ > 0.15 || phase_IQ < -0.15) && codec_restarts < 5)
              // threshold lowered, so we can be really sure to have IQ phase balance OK
              // threshold of 22.5 degrees phase shift == PI / 8 == 0.3926990817
              // hopefully your hardware is not so bad, that its phase error is more than 22 degrees ;-)
              // if it is that bad, adjust this threshold to maybe PI / 7 or PI / 6
            {
              reset_codec();
#ifdef DEBUG
              Serial.println("CODEC RESET");
#endif
              twinpeaks_tested = 2;
              codec_restarts++;
              // TODO: we should set a maximum number of codec resets
              // and print out a message, if twinpeaks remains after the
              // 5th reset for example --> could then be a severe hardware error !
              if (codec_restarts >= 4)
              {
                // PRINT OUT WARNING MESSAGE
#ifdef DEBUG
                Serial.println("Tried four times to reset your codec, but still IQ balance is very bad - hardware error ???");
#endif
                twinpeaks_tested = 3;
                tft.fillRect(spectrum_x + 256 + 3, pos_y_time + 18, 49, 28, ILI9341_RED);
                tft.setCursor(pos_x_time + 42, pos_y_time + 22);
                tft.setFont(Arial_12);
                tft.print("reset!");
              }
            }
            else
            {
              twinpeaks_tested = 3;
              twinpeaks_counter = 0;
#ifdef DEBUG
              Serial.println("IQ phase balance is OK, so enjoy radio reception !");
#endif
              tft.fillRect(spectrum_x + 256 + 3, pos_y_time + 18, 49, 28, ILI9341_NAVY);
              tft.drawRect(spectrum_x + 256 + 2, pos_y_time + 17, 320 - spectrum_x - 258, 31, ILI9341_MAROON);
              tft.setCursor(spectrum_x + 256 + 6, pos_y_time + 22);
              tft.setFont(Arial_12);
              tft.setTextColor(ILI9341_WHITE);
              tft.print("IQtest");
              tft.setCursor(pos_x_time + 55, pos_y_time + 22 + 14);
              tft.setFont(Arial_12);
              tft.print("OK !");
            }
          }


          teta1_old = teta1;
          teta2_old = teta2;
          teta3_old = teta3;

          // first correct Q and then correct I --> this order is crucially important!
          for (unsigned i = 0; i < BUFFER_SIZE * N_BLOCKS; i++)
          { // see fig. 5
            float_buffer_R[i] += M_c1 * float_buffer_L[i];
          }
          // see fig. 5
          arm_scale_f32 (float_buffer_L, M_c2, float_buffer_L, BUFFER_SIZE * N_BLOCKS);
        }

      /*******************************************************************************************************

        algorithm by Chang et al. 2010


      ********************************************************************************************************/

        else if (CHANG)
        {
          // this is the IQ imbalance correction algorithm by Chang et al. 2010
          // 1.) estimate K_est
          // 2.) correct for K_est_mult
          // 3.) estimate P_est
          // 4.) correct for P_est_mult

          // new experiment

          // calculate the coefficients for the imbalance correction
          // once at system start or when changing frequency band
          // IQ_state 0: do nothing --> automatic IQ imbalance correction switched off
          // IQ_state 1: estimate amplitude coefficient K_est
          // IQ_state 2: K_est estimated, wait for next stage
          // IQ_state 3: estimate phase coefficient P_est
          // IQ_state 4: everything calculated and corrected

          /*
              switch(IQ_state)
              {
                case 0:
                  break;
                case 1: // Chang & Lin (2010): eq. (9)
                    AudioNoInterrupts();
                    Q_sum = 0.0;
                    I_sum = 0.0;
                    for (i = 0; i < n_para; i++)
                    {
                         Q_sum += float_buffer_R[i] * float_buffer_R[i + n_para];
                         I_sum += float_buffer_L[i] * float_buffer_L[i + n_para];
                    }
                    K_est = sqrtf(Q_sum / I_sum);
                    K_est_mult = 1.0 / K_est;
                    IQ_state++;
                    Serial.print("New 1 / K_est: "); Serial.println(1.0 / K_est);
                    AudioInterrupts();
                  break;
                case 2: // Chang & Lin (2010): eq. (10)
                    AudioNoInterrupts();
                    IQ_sum = 0.0;
                    I_sum = 0.0;
                    for (i = 0; i < n_para; i++)
                    {
                         IQ_sum += float_buffer_L[i] * float_buffer_R[i + n_para];
                         I_sum += float_buffer_L[i] * float_buffer_L[i + n_para];
                    }
                    P_est = IQ_sum / I_sum;
                    P_est_mult = 1.0 / (sqrtf(1.0 - P_est * P_est));
                    IQ_state = 1;
                    Serial.print("1 / sqrt(1 - P_est^2): "); Serial.println(P_est_mult);
                    if(P_est > -1.0 && P_est < 1.0) {
                      Serial.print("New: Phasenfehler in Grad: "); Serial.println(- asinf(P_est));
                    }
                    AudioInterrupts();
                  break;
              }
          */

          // only correct, if signal strength is above a threshold
          //
          if (IQ_counter >= 0 && 1 )
          {
            // 1.)
            // K_est estimation
            Q_sum = 0.0;
            I_sum = 0.0;
            for (unsigned i = 0; i < n_para; i++)
            {
              Q_sum += float_buffer_R[i] * float_buffer_R[i + n_para];
              I_sum += float_buffer_L[i] * float_buffer_L[i + n_para];
            }
            if (I_sum != 0.0)
            {
              if (Q_sum / I_sum < 0) {
#ifdef DEBUG
                Serial.println("ACHTUNG WURZEL AUS NEGATIVER ZAHL");
                Serial.println("ACHTUNG WURZEL AUS NEGATIVER ZAHL");
                Serial.println("ACHTUNG WURZEL AUS NEGATIVER ZAHL");
                Serial.println("ACHTUNG WURZEL AUS NEGATIVER ZAHL");
                Serial.println("ACHTUNG WURZEL AUS NEGATIVER ZAHL");
                Serial.println("ACHTUNG WURZEL AUS NEGATIVER ZAHL");
                Serial.println("ACHTUNG WURZEL AUS NEGATIVER ZAHL");
                Serial.println("ACHTUNG WURZEL AUS NEGATIVER ZAHL");
                Serial.println("ACHTUNG WURZEL AUS NEGATIVER ZAHL");
#endif
                K_est = K_est_old;
              }
              else
              {
                if (IQ_counter != 0) K_est = 0.001 * sqrtf(Q_sum / I_sum) + 0.999 * K_est_old;
                else
                {
                  K_est = sqrtf(Q_sum / I_sum);
                }
                K_est_old = K_est;
              }
            }
            else K_est = K_est_old;
#ifdef DEBUG
            Serial.print("New 1 / K_est: "); Serial.println(100.0 / K_est);
#endif
            // 3.)
            // phase estimation
            IQ_sum = 0.0;
            for (unsigned i = 0; i < n_para; i++)
            { // amplitude correction inside the formula  --> K_est_mult !
              IQ_sum += float_buffer_L[i] * float_buffer_R[i + n_para];// * K_est_mult;
            }
            if (I_sum == 0.0) I_sum = IQ_sum;
            if (IQ_counter != 0) P_est = 0.001 * (IQ_sum / I_sum) + 0.999 * P_est_old;
            else P_est = (IQ_sum / I_sum);
            P_est_old = P_est;
            if (P_est > -1.0 && P_est < 1.0) P_est_mult = 1.0 / (sqrtf(1.0 - P_est * P_est));
            else P_est_mult = 1.0;
            // dirty fix !!!
#ifdef DEBUG
            Serial.print("1 / sqrt(1 - P_est^2): "); Serial.println(P_est_mult * 100.0);
#endif
            if (P_est > -1.0 && P_est < 1.0) {
#ifdef DEBUG
              Serial.print("New: Phasenfehler in Grad: "); Serial.println(- asinf(P_est) * 100.0);
#endif
            }

            // 4.)
            // Chang & Lin (2010): eq. 12; phase correction
            for (unsigned i = 0; i < BUFFER_SIZE * N_BLOCKS; i++)
            {
              float_buffer_R[i] = P_est_mult * float_buffer_R[i] - P_est * float_buffer_L[i];
            }
          }
          IQ_counter++;
          if (IQ_counter >= 1000) IQ_counter = 1;

        } // end if (Chang)


      /***********************************************************************************************
          Perform a 256 point FFT for the spectrum display on the basis of the first 256 complex values
          of the raw IQ input data
          this saves about 3% of processor power compared to calculating the magnitudes and means of
          the 4096 point FFT for the display [41.8% vs. 44.53%]
       ***********************************************************************************************/
      // only go there from here, if magnification == 1
      if (spectrum_zoom == SPECTRUM_ZOOM_1) // && display_S_meter_or_spectrum_state == 1)
      {
        //        Zoom_FFT_exe(BUFFER_SIZE * N_BLOCKS);
        zoom_display = 1;
        calc_256_magn();
      }
      display_S_meter_or_spectrum_state++;
      /**********************************************************************************
          Frequency translation by Fs/4 without multiplication
          Lyons (2011): chapter 13.1.2 page 646
          together with the savings of not having to shift/rotate the FFT_buffer, this saves
          about 1% of processor use (40.78% vs. 41.70% [AM demodulation])
       **********************************************************************************/
      // this is for +Fs/4 [moves receive frequency to the left in the spectrum display]
      for (unsigned i = 0; i < BUFFER_SIZE * N_BLOCKS; i += 4)
      { // float_buffer_L contains I = real values
        // float_buffer_R contains Q = imaginary values
        // xnew(0) =  xreal(0) + jximag(0)
        // leave as it is!
        // xnew(1) =  - ximag(1) + jxreal(1)
        hh1 = - float_buffer_R[i + 1];
        hh2 =   float_buffer_L[i + 1];
        float_buffer_L[i + 1] = hh1;
        float_buffer_R[i + 1] = hh2;
        // xnew(2) = -xreal(2) - jximag(2)
        hh1 = - float_buffer_L[i + 2];
        hh2 = - float_buffer_R[i + 2];
        float_buffer_L[i + 2] = hh1;
        float_buffer_R[i + 2] = hh2;
        // xnew(3) = + ximag(3) - jxreal(3)
        hh1 =   float_buffer_R[i + 3];
        hh2 = - float_buffer_L[i + 3];
        float_buffer_L[i + 3] = hh1;
        float_buffer_R[i + 3] = hh2;
      }

      // this is for -Fs/4 [moves receive frequency to the right in the spectrumdisplay]
      /*    for(i = 0; i < BUFFER_SIZE * N_BLOCKS; i += 4)
          {   // float_buffer_L contains I = real values
              // float_buffer_R contains Q = imaginary values
              // xnew(0) =  xreal(0) + jximag(0)
              // leave as it is!
              // xnew(1) =  ximag(1) - jxreal(1)
              hh1 = float_buffer_R[i + 1];
              hh2 = - float_buffer_L[i + 1];
              float_buffer_L[i + 1] = hh1;
              float_buffer_R[i + 1] = hh2;
              // xnew(2) = -xreal(2) - jximag(2)
              hh1 = - float_buffer_L[i + 2];
              hh2 = - float_buffer_R[i + 2];
              float_buffer_L[i + 2] = hh1;
              float_buffer_R[i + 2] = hh2;
              // xnew(3) = -ximag(3) + jxreal(3)
              hh1 = - float_buffer_R[i + 3];
              hh2 = float_buffer_L[i + 3];
              float_buffer_L[i + 3] = hh1;
              float_buffer_R[i + 3] = hh2;
          }
      */
      /***********************************************************************************************
          just for checking: plotting min/max and mean of the samples
       ***********************************************************************************************/
      //    if (ms_500.check() == 1)
      if (0)
      {
        float32_t sample_min = 0.0;
        float32_t sample_max = 0.0;
        float32_t sample_mean = 0.0;
        flagg = 1;
        uint32_t min_index, max_index;
        arm_mean_f32(float_buffer_L, BUFFER_SIZE * N_BLOCKS, &sample_mean);
        arm_max_f32(float_buffer_L, BUFFER_SIZE * N_BLOCKS, &sample_max, &max_index);
        arm_min_f32(float_buffer_L, BUFFER_SIZE * N_BLOCKS, &sample_min, &min_index);
#ifdef DEBUG
        Serial.print("VOR DECIMATION: ");
        Serial.print("Sample min: "); Serial.println(sample_min);
        Serial.print("Sample max: "); Serial.println(sample_max);
        Serial.print("Max index: "); Serial.println(max_index);

        Serial.print("Sample mean: "); Serial.println(sample_mean);
        //    Serial.print("FFT_length: "); Serial.println(FFT_length);
        //    Serial.print("N_BLOCKS: "); Serial.println(N_BLOCKS);
        //Serial.println(BUFFER_SIZE * N_BLOCKS / 8);
#endif
      }

      // SPECTRUM_ZOOM_2 and larger here after frequency conversion!
      if (spectrum_zoom != SPECTRUM_ZOOM_1)
      {
        //        Zoom_FFT_exe (BUFFER_SIZE * N_BLOCKS / 2); // perform Zoom FFT on half of the samples to save memory --> does not work for magnifications larger than 4
        Zoom_FFT_exe (BUFFER_SIZE * N_BLOCKS); // there seems to be a BUG here, because the blocksize has to be adjusted according to magnification,
        // does not work for magnifications > 8
      }
      if (zoom_display)
      {
        if (show_spectrum_flag)
        {
          show_spectrum();
        }
        zoom_display = 0;
        //zoom_sample_ptr = 0;
      }

      /**********************************************************************************
          S-Meter & dBm-display
       **********************************************************************************/

      if (display_S_meter_or_spectrum_state == 2)
      {
        Calculatedbm();
      }
      else if (display_S_meter_or_spectrum_state == 4)
      {
        Display_dbm();
        display_S_meter_or_spectrum_state = 0;
      }

      /**********************************************************************************
          Decimation
       **********************************************************************************/
      // decimation-by-4 in-place!
      arm_fir_decimate_f32(&FIR_dec1_I, float_buffer_L, float_buffer_L, BUFFER_SIZE * N_BLOCKS);
      arm_fir_decimate_f32(&FIR_dec1_Q, float_buffer_R, float_buffer_R, BUFFER_SIZE * N_BLOCKS);

      // decimation-by-2 in-place
      arm_fir_decimate_f32(&FIR_dec2_I, float_buffer_L, float_buffer_L, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1);
      arm_fir_decimate_f32(&FIR_dec2_Q, float_buffer_R, float_buffer_R, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1);

      /***********************************************************************************************
          just for checking: plotting min/max and mean of the samples
       ***********************************************************************************************/
      //    if (flagg == 1)
#ifdef DEBUG
      {
        if(0)
        {
          flagg = 0;
          float32_t sample_min = 0.0;
          float32_t sample_max = 0.0;
          float32_t sample_mean = 0.0;
          uint32_t min_index, max_index;
          arm_mean_f32(float_buffer_L, BUFFER_SIZE * N_BLOCKS / 8, &sample_mean);
          arm_max_f32(float_buffer_L, BUFFER_SIZE * N_BLOCKS / 8, &sample_max, &max_index);
          arm_min_f32(float_buffer_L, BUFFER_SIZE * N_BLOCKS / 8, &sample_min, &min_index);
  
          Serial.print("NACH DECIMATION: ");
          Serial.print("Sample min: "); Serial.println(sample_min);
          Serial.print("Sample max: "); Serial.println(sample_max);
          Serial.print("Max index: "); Serial.println(max_index);
  
          Serial.print("Sample mean: "); Serial.println(sample_mean);
          //    Serial.print("FFT_length: "); Serial.println(FFT_length);
          //    Serial.print("N_BLOCKS: "); Serial.println(N_BLOCKS);
          //Serial.println(BUFFER_SIZE * N_BLOCKS / 8);
        }
      }
#endif
      /**********************************************************************************
          Digital convolution
       **********************************************************************************/
      //  basis for this was Lyons, R. (2011): Understanding Digital Processing.
      //  "Fast FIR Filtering using the FFT", pages 688 - 694
      //  numbers for the steps taken from that source
      //  Method used here: overlap-and-save

      // 4.) ONLY FOR the VERY FIRST FFT: fill first samples with zeros
      if (first_block) // fill real & imaginaries with zeros for the first BLOCKSIZE samples
      {
        for (unsigned i = 0; i < BUFFER_SIZE * N_BLOCKS / (uint32_t)(DF / 2.0); i++)
        {
          FFT_buffer[i] = 0.0;
        }
        first_block = 0;
      }
      else

        // HERE IT STARTS for all other instances
        // 6 a.) fill FFT_buffer with last events audio samples
        for (unsigned i = 0; i < BUFFER_SIZE * N_BLOCKS / (uint32_t)(DF); i++)
        {
          FFT_buffer[i * 2] = last_sample_buffer_L[i]; // real
          FFT_buffer[i * 2 + 1] = last_sample_buffer_R[i]; // imaginary
        }


      // copy recent samples to last_sample_buffer for next time!
      for (unsigned i = 0; i < BUFFER_SIZE * N_BLOCKS / (uint32_t)(DF); i++)
      {
        last_sample_buffer_L [i] = float_buffer_L[i];
        last_sample_buffer_R [i] = float_buffer_R[i];
      }

      // 6. b) now fill recent audio samples into FFT_buffer (left channel: re, right channel: im)
      for (unsigned i = 0; i < BUFFER_SIZE * N_BLOCKS / (uint32_t)(DF); i++)
      {
        FFT_buffer[FFT_length + i * 2] = float_buffer_L[i]; // real
        FFT_buffer[FFT_length + i * 2 + 1] = float_buffer_R[i]; // imaginary
      }

      // 6. c) and do windowing: Nuttall window
      // window does not work properly, I think
      // --> stuttering with the frequency of the FFT update depending on the sample size
      /*
            for(i = 0; i < FFT_length; i++)
        //      for(i = FFT_length/2; i < FFT_length; i++)
            { // SIN^2 window
                float32_t SINF = (sinf(PI * (float32_t)i / 1023.0f));
                SINF = SINF * SINF;
                FFT_buffer[i * 2] = SINF * FFT_buffer[i * 2];
                FFT_buffer[i * 2 + 1] = SINF * FFT_buffer[i * 2 + 1];
        // Nuttal
        //          FFT_buffer[i * 2] = (0.355768 - (0.487396*arm_cos_f32((TPI*(float32_t)i)/(float32_t)2047)) + (0.144232*arm_cos_f32((FOURPI*(float32_t)i)/(float32_t)2047)) - (0.012604*arm_cos_f32((SIXPI*(float32_t)i)/(float32_t)2047))) * FFT_buffer[i * 2];
        //          FFT_buffer[i * 2 + 1] = (0.355768 - (0.487396*arm_cos_f32((TPI*(float32_t)i)/(float32_t)2047)) + (0.144232*arm_cos_f32((FOURPI*(float32_t)i)/(float32_t)2047)) - (0.012604*arm_cos_f32((SIXPI*(float32_t)i)/(float32_t)2047))) * FFT_buffer[i * 2 + 1];
            }
      */

      // perform complex FFT
      // calculation is performed in-place the FFT_buffer [re, im, re, im, re, im . . .]
      arm_cfft_f32(S, FFT_buffer, 0, 1);

      // AUTOTUNE, slow down process in order for Si5351 to settle
      if (autotune_flag != 0)
      {
        if (autotune_counter < autotune_wait) autotune_counter++;
        else
        {
          autotune_counter = 0;
          autotune();
        }
      }

      /*
        // frequency shift & choice of FFT_bins for "demodulation"
          switch (demod_mode)
          {
            case DEMOD_AM:
        ////////////////////////////////////////////////////////////////////////////////
        // AM demodulation - shift everything FFT_length / 4 bins around IF
        // leave both: USB & LSB
        ////////////////////////////////////////////////////////////////////////////////
        // shift USB part by FFT_shift
          for(i = 0; i < FFT_shift; i++)
          {
              FFT_buffer[i] = FFT_buffer[i + FFT_length * 2 - FFT_shift];
          }

        // now shift LSB part by FFT_shift
          for(i = (FFT_length * 2) - FFT_shift - 2; i > FFT_length - FFT_shift;i--)
           {
              FFT_buffer[i + FFT_shift] = FFT_buffer[i]; // shift LSB part by FFT_shift bins
           }
           break;
        ///// END AM-demodulation, seems to be OK (at least sounds ok)
        ////////////////////////////////////////////////////////////////////////////////
        // USB DEMODULATION
        ////////////////////////////////////////////////////////////////////////////////
          case DEMOD_USB:
          case DEMOD_STEREO_USB:
          for(i = FFT_length / 2; i < FFT_length ; i++) // maybe this is not necessary? yes, it is!
           {
              FFT_buffer[i] = FFT_buffer [i - FFT_shift]; // shift USB, 2nd part, to the right
           }

          // frequency shift of the USB 1st part - but this is essential
          for(i = FFT_length * 2 - FFT_shift; i < FFT_length * 2; i++)
          {
              FFT_buffer[i - FFT_length * 2 + FFT_shift] = FFT_buffer[i];
          }
          // fill LSB with zeros
          for(i = FFT_length; i < FFT_length *2; i++)
           {
              FFT_buffer[i] = 0.0; // fill rest with zero
           }
          break;
        // END USB: audio seems to be OK
        ////////////////////////////////////////////////////////////////////////////////
          case DEMOD_LSB:
          case DEMOD_STEREO_LSB:
          case DEMOD_DCF77:
        ////////////////////////////////////////////////////////////////////////////////
        // LSB demodulation
        ////////////////////////////////////////////////////////////////////////////////
          for(i = 0; i < FFT_length ; i++)
           {
              FFT_buffer[i] = 0.0; // fill USB with zero
           }
          // frequency shift of the LSB part
          ////////////////////////////////////////////////////////////////////////////////
          for(i = (FFT_length * 2) - FFT_shift - 2; i > FFT_length - FFT_shift;i--)
           {
              FFT_buffer[i + FFT_shift] = FFT_buffer[i]; // shift LSB part by FFT_shift bins
           }
        // fill USB with zeros
          for(i = FFT_length; i < FFT_length + FFT_shift; i++)
           {
              FFT_buffer[i] = 0.0; // fill rest with zero
           }
          break;
        // END LSB: audio seems to be OK
        ////////////////////////////////////////////////////////////////////////////////
          } // END switch demod_mode
      */
#if 0
      // choice of FFT_bins for "demodulation" WITHOUT frequency translation
      // (because frequency translation has already been done in time domain with
      // "frequency translation without multiplication" - DSP trick R. Lyons (2011))
      switch (band[bands].mode)
      {
        //      case DEMOD_AM1:
        case DEMOD_AUTOTUNE:
        case DEMOD_DSB:
        case DEMOD_SAM_USB:
        case DEMOD_SAM_LSB:
        case DEMOD_IQ:

          ////////////////////////////////////////////////////////////////////////////////
          // AM & SAM demodulation
          // DO NOTHING !
          // leave both: USB & LSB
          ////////////////////////////////////////////////////////////////////////////////
          break;
        ////////////////////////////////////////////////////////////////////////////////
        // USB DEMODULATION
        ////////////////////////////////////////////////////////////////////////////////
        case DEMOD_USB:
        case DEMOD_STEREO_USB:
          // fill LSB with zeros
          for (int i = FFT_length; i < FFT_length * 2; i++)
          {
            FFT_buffer[i] = 0.0;
          }
          break;
        ////////////////////////////////////////////////////////////////////////////////
        case DEMOD_LSB:
        case DEMOD_STEREO_LSB:
        case DEMOD_DCF77:
          ////////////////////////////////////////////////////////////////////////////////
          // LSB demodulation
          ////////////////////////////////////////////////////////////////////////////////
          for (int i = 4; i < FFT_length ; i++)
            // when I delete all the FFT_buffer products (from i = 0 to FFT_length), LSB is much louder! --> effect of the AGC !!1
            // so, I leave indices 0 to 3 in the buffer
          {
            FFT_buffer[i] = 0.0; // fill USB with zero
          }
          break;
          ////////////////////////////////////////////////////////////////////////////////
      } // END switch demod_mode
#endif


      // complex multiply with filter mask
      arm_cmplx_mult_cmplx_f32 (FFT_buffer, FIR_filter_mask, iFFT_buffer, FFT_length);
      //    arm_copy_f32(FFT_buffer, iFFT_buffer, FFT_length * 2);


      // filter by just deleting bins - principle of Linrad
      // only works properly when we have the right window function!?

      // (automatic) notch filter = Tone killer --> the name is stolen from SNR ;-)
      // first test, we set a notch filter at 1kHz
      // which bin is that?
      // positive & negative frequency -1kHz and +1kHz --> delete 2 bins
      // we are not deleting one bin, but five bins for the test
      // 1024 bins in 12ksps = 11.71Hz per bin
      // SR[SAMPLE_RATE].rate / 8.0 / 1024 = bin BW

      // 1000Hz / 11.71Hz = bin 85.333
      //

      ////////////////////////////////////////////////////////////////////////////////
      // Manual notch filter
      ////////////////////////////////////////////////////////////////////////////////

      /*  for(int n = 0; n < 2; n++)
        {
          bin = notches[n] * 2.0 / bin_BW;
          if(notches_on[n])
          {
            for(int zz = bin - notches_BW[n]; zz < bin + notches_BW[n] + 1; zz++)
            {
                iFFT_buffer[zz] = 0.0;
                iFFT_buffer[2047 - zz] = 0.0;
            }
          }
        }
      */
      if (notches_on[0])
      {
        if (notches[0] > 0 && notch_L[0] > 0)
        {
          for (int16_t zz = notch_L[0]; zz < notch_R[0] + 1; zz++)
          {
            iFFT_buffer[zz] = 0.0;
          }
        }
        else if (notches[0] < 0 && notch_R[0] > 0 && notch_L[0] > 0)
        {
          for (int16_t zz = notch_L[0]; zz < notch_R[0] + 1; zz++)
          {
            iFFT_buffer[2046 - zz] = 0.0;
          }
        }
      }


      /*
        for(int zz = bin; zz < FFT_length; zz++)
        {
            iFFT_buffer[zz] = 0.0;
            iFFT_buffer[2048 - zz] = 0.0;
        }
      */

      // highpass 234Hz
      // never delete the carrier !!! ;-)
      // always preserve bin 0 ()
      /*  for(int zz = 5; zz < 40; zz++)
        {
            iFFT_buffer[zz] = 0.0;
            iFFT_buffer[2047 - zz] = 0.0;
        }

        // lowpass
        for(int zz = bin; zz < FFT_length; zz++)
        {
            iFFT_buffer[zz] = 0.0;
            iFFT_buffer[2047 - zz] = 0.0;
        }
      */
      // automatic notch:
      // detect bins where the power level remains the same for at least 200ms
      // set these bins to zero
      /*  for(i = 0; i < 1024; i++)
        {
            float32_t bin_p = sqrtf((FFT_buffer[i * 2] * FFT_buffer[i * 2]) + (FFT_buffer[i * 2 + 1] * FFT_buffer[i * 2 + 1]));
            notch_amp[i] = bin_p;
        }
      */

      // spectral noise reduction
      // if sample rate = 96kHz, we are in 12ksps now, because we decimated by 8

      // perform iFFT (in-place)
      arm_cfft_f32(iS, iFFT_buffer, 1, 1);
      /*
        // apply window the other way round !
            for(i = 0; i < FFT_length; i++)
        //      for(i = FFT_length / 2; i < FFT_length; i++)
            { // SIN^2 window
                float32_t SINF = (sinf(PI * (float32_t)i / 1023.0));
                SINF = (SINF * SINF);
                SINF = 1.0f / SINF;
                iFFT_buffer[i * 2] = SINF * iFFT_buffer[i * 2];
                if(iFFT_flip == 1)
                {
                    iFFT_buffer[i * 2 + 1] = SINF * iFFT_buffer[i * 2 + 1];
                }
                else
                {
                    iFFT_buffer[i * 2 + 1] = - SINF * iFFT_buffer[i * 2 + 1];
                }
        //          Serial.print(iFFT_buffer[i*2]); Serial.print("  ");
            }
            if(iFFT_flip == 1) iFFT_flip = 0;
            else iFFT_flip = 1;
      */

      /**********************************************************************************
          AGC - automatic gain control

          we´re back in time domain
          AGC acts upon I & Q before demodulation on the decimated audio data in iFFT_buffer
       **********************************************************************************/

      AGC();

      /**********************************************************************************
          Demodulation
       **********************************************************************************/

      // our desired output is a combination of the real part (left channel) AND the imaginary part (right channel) of the second half of the FFT_buffer
      // which one and how they are combined is dependent upon the demod_mode . . .


      if (bands[current_band].mode == DEMOD_SAM || bands[current_band].mode == DEMOD_SAM_LSB || bands[current_band].mode == DEMOD_SAM_USB || bands[current_band].mode == DEMOD_SAM_STEREO)
      { // taken from Warren Pratt´s WDSP, 2016
        // http://svn.tapr.org/repos_sdr_hpsdr/trunk/W5WC/PowerSDR_HPSDR_mRX_PS/Source/wdsp/

        for (unsigned i = 0; i < FFT_length / 2; i++)
        {
          float32_t Sin, Cos;
          sincosf(phzerror, &Sin, &Cos);
          //Sin = sinf(phzerror);
          //Cos = cosf(phzerror);
          ai = Cos * iFFT_buffer[FFT_length + i * 2];
          bi = Sin * iFFT_buffer[FFT_length + i * 2];
          aq = Cos * iFFT_buffer[FFT_length + i * 2 + 1];
          bq = Sin * iFFT_buffer[FFT_length + i * 2 + 1];

          if (bands[current_band].mode != DEMOD_SAM)
          {
            a[0] = dsI;
            b[0] = bi;
            c[0] = dsQ;
            d[0] = aq;
            dsI = ai;
            dsQ = bq;

            for (int j = 0; j < SAM_PLL_HILBERT_STAGES; j++)
            {
              int k = 3 * j;
              a[k + 3] = c0[j] * (a[k] - a[k + 5]) + a[k + 2];
              b[k + 3] = c1[j] * (b[k] - b[k + 5]) + b[k + 2];
              c[k + 3] = c0[j] * (c[k] - c[k + 5]) + c[k + 2];
              d[k + 3] = c1[j] * (d[k] - d[k + 5]) + d[k + 2];
            }
            ai_ps = a[OUT_IDX];
            bi_ps = b[OUT_IDX];
            bq_ps = c[OUT_IDX];
            aq_ps = d[OUT_IDX];

            for (int j = OUT_IDX + 2; j > 0; j--)
            {
              a[j] = a[j - 1];
              b[j] = b[j - 1];
              c[j] = c[j - 1];
              d[j] = d[j - 1];
            }
          }

          corr[0] = +ai + bq;
          corr[1] = -bi + aq;

          switch (bands[current_band].mode)
          {
            case DEMOD_SAM:
              {
                audio = corr[0];
                break;
              }
            case DEMOD_SAM_USB:
              {
                audio = (ai_ps - bi_ps) + (aq_ps + bq_ps);
                break;
              }
            case DEMOD_SAM_LSB:
              {
                audio = (ai_ps + bi_ps) - (aq_ps - bq_ps);
                break;
              }
            case DEMOD_SAM_STEREO:
              {
                audio = (ai_ps + bi_ps) - (aq_ps - bq_ps);
                audiou = (ai_ps - bi_ps) + (aq_ps + bq_ps);
                break;
              }
          }
          if (fade_leveler)
          {
            dc = mtauR * dc + onem_mtauR * audio;
            dc_insert = mtauI * dc_insert + onem_mtauI * corr[0];
            audio = audio + dc_insert - dc;
          }
          float_buffer_L[i] = audio;
          if (bands[current_band].mode == DEMOD_SAM_STEREO)
          {
            if (fade_leveler)
            {
              dcu = mtauR * dcu + onem_mtauR * audiou;
              dc_insertu = mtauI * dc_insertu + onem_mtauI * corr[0];
              audiou = audiou + dc_insertu - dcu;
            }
            float_buffer_R[i] = audiou;
          }
#ifdef USE_ATAN2FAST
          det = arm_atan2_f32(corr[1], corr[0]);
#else
          det = atan2f(corr[1], corr[0]);
#endif
          //            Serial.println(corr[1] * 100000);
          //            Serial.println(corr[0] * 100000);
          // is not at all faster than atan2f !
          //            det = atan2_fast(corr[1], corr[0]);

          del_out = fil_out;
          omega2 = omega2 + g2 * det;
          if (omega2 < omega_min) omega2 = omega_min;
          else if (omega2 > omega_max) omega2 = omega_max;
          fil_out = g1 * det + omega2;
          phzerror = phzerror + del_out;

          // wrap round 2PI, modulus
          while (phzerror >= TPI) phzerror -= TPI;
          while (phzerror < 0.0) phzerror += TPI;
        }
        if (bands[current_band].mode != DEMOD_SAM_STEREO)
        {
          arm_copy_f32(float_buffer_L, float_buffer_R, FFT_length / 2);
        }
        //        SAM_display_count++;
        //        if(SAM_display_count > 50) // to display the exact carrier frequency that the PLL is tuned to
        //        if(0)
        // in the small frequency display
        // we calculate carrier offset here and the display function is
        // then called in main loop every 100ms
        { // to make this smoother, a simple lowpass/exponential averager here . . .
          SAM_carrier = 0.08 * (omega2 * SR[SAMPLE_RATE].rate) / (DF * TPI);
          SAM_carrier = SAM_carrier + 0.92 * SAM_lowpass;
          SAM_carrier_freq_offset =  (int)SAM_carrier;
          //            SAM_display_count = 0;
          SAM_lowpass = SAM_carrier;
          show_frequency(bands[current_band].freq, 0);
        }
      }
      else if (bands[current_band].mode == DEMOD_IQ)
      {
        for (unsigned i = 0; i < FFT_length / 2; i++)
        {
          float_buffer_L[i] = iFFT_buffer[FFT_length + i * 2];
          float_buffer_R[i] = iFFT_buffer[FFT_length + i * 2 + 1];
        }
      }
      else
        /*      if(demod_mode == DEMOD_AM1)
              { // E(t) = sqrt(I*I + Q*Q) with arm function --> moving average DC removal --> lowpass IIR 2nd order
                // this AM demodulator with arm_cmplx_mag_f32 saves 1.23% of processor load at 96ksps compared to using arm_sqrt_f32
                arm_cmplx_mag_f32(&iFFT_buffer[FFT_length], float_buffer_R, FFT_length/2);
                // DC removal
                last_dc_level = fastdcblock_ff(float_buffer_R, float_buffer_L, FFT_length / 2, last_dc_level);
                // lowpass-filter the AM output to reduce high frequency noise produced by the envelope demodulator
                // see Whiteley 2011 for an explanation of this problem
                // 45.18% with IIR; 43.29% without IIR
                arm_biquad_cascade_df1_f32 (&biquad_lowpass1, float_buffer_L, float_buffer_R, FFT_length / 2);
                arm_copy_f32(float_buffer_R, float_buffer_L, FFT_length/2);
              }
              else */
        if (bands[current_band].mode == DEMOD_AM2)
        { // // E(t) = sqrtf(I*I + Q*Q) --> highpass IIR 1st order for DC removal --> lowpass IIR 2nd order
          for (unsigned i = 0; i < FFT_length / 2; i++)
          { //
            audiotmp = sqrtf(iFFT_buffer[FFT_length + (i * 2)] * iFFT_buffer[FFT_length + (i * 2)]
                             + iFFT_buffer[FFT_length + (i * 2) + 1] * iFFT_buffer[FFT_length + (i * 2) + 1]);
            // DC removal filter -----------------------
            w = audiotmp + wold * 0.9999f; // yes, I want a superb bass response ;-)
            float_buffer_L[i] = w - wold;
            wold = w;
          }
          arm_biquad_cascade_df1_f32 (&biquad_lowpass1, float_buffer_L, float_buffer_R, FFT_length / 2);
          arm_copy_f32(float_buffer_R, float_buffer_L, FFT_length / 2);
        }
        else
          /*      if(bands[current_band].mode == DEMOD_AM3)
                { // // E(t) = sqrt(I*I + Q*Q) --> highpass IIR 1st order for DC removal --> no lowpass
                    for(i = 0; i < FFT_length / 2; i++)
                    { //
                          audiotmp = sqrtf(iFFT_buffer[FFT_length + (i * 2)] * iFFT_buffer[FFT_length + (i * 2)]
                                              + iFFT_buffer[FFT_length + (i * 2) + 1] * iFFT_buffer[FFT_length + (i * 2) + 1]);
                          // DC removal filter -----------------------
                          w = audiotmp + wold * 0.9999f; // yes, I want a superb bass response ;-)
                          float_buffer_L[i] = w - wold;
                          wold = w;
                    }
                    arm_copy_f32(float_buffer_L, float_buffer_R, FFT_length/2);
                }
                else
                if(bands[current_band].mode == DEMOD_AM_AE1)
                {  // E(n) = |I| + |Q| --> moving average DC removal --> lowpass 2nd order IIR
                  // Approximate envelope detection, Lyons (2011): page 756
                   // E(n) = |I| + |Q| --> lowpass (here I use a 2nd order IIR instead of the 1st order IIR of the original publication)
                    for(i = 0; i < FFT_length / 2; i++)
                    {
                          float_buffer_R[i] = abs(iFFT_buffer[FFT_length + (i * 2)]) + abs(iFFT_buffer[FFT_length + (i * 2) + 1]);
                    }
                  // DC removal
                  last_dc_level = fastdcblock_ff(float_buffer_R, float_buffer_L, FFT_length / 2, last_dc_level);
                  arm_biquad_cascade_df1_f32 (&biquad_lowpass1, float_buffer_L, float_buffer_R, FFT_length / 2);
                  arm_copy_f32(float_buffer_R, float_buffer_L, FFT_length/2);
                }
                else
                if(bands[current_band].mode == DEMOD_AM_AE2)
                {  // E(n) = |I| + |Q| --> highpass IIR 1st order for DC removal --> lowpass 2nd order IIR
                   // Approximate envelope detection, Lyons (2011): page 756
                   // E(n) = |I| + |Q| --> lowpass (here I use a 2nd order IIR instead of the 1st order IIR of the original publication)
                    for(i = 0; i < FFT_length / 2; i++)
                    {
                      audiotmp = abs(iFFT_buffer[FFT_length + (i * 2)]) + abs(iFFT_buffer[FFT_length + (i * 2) + 1]);
                      // DC removal filter -----------------------
                      w = audiotmp + wold * 0.9999f; // yes, I want a superb bass response ;-)
                      float_buffer_L[i] = w - wold;
                      wold = w;
                    }
                  arm_biquad_cascade_df1_f32 (&biquad_lowpass1, float_buffer_L, float_buffer_R, FFT_length / 2);
                  arm_copy_f32(float_buffer_R, float_buffer_L, FFT_length/2);
                }
                else
                if(bands[current_band].mode == DEMOD_AM_AE3)
                {  // E(n) = |I| + |Q| --> highpass IIR 1st order for DC removal --> NO lowpass
                   // Approximate envelope detection, Lyons (2011): page 756
                   // E(n) = |I| + |Q| --> lowpass 1st order IIR
                    for(i = 0; i < FFT_length / 2; i++)
                    {
                      audiotmp = abs(iFFT_buffer[FFT_length + (i * 2)]) + abs(iFFT_buffer[FFT_length + (i * 2) + 1]);
                      // DC removal filter -----------------------
                      w = audiotmp + wold * 0.9999f; // yes, I want a superb bass response ;-)
                      float_buffer_L[i] = w - wold;
                      wold = w;
                    }
                  arm_copy_f32(float_buffer_L, float_buffer_R, FFT_length/2);
                }
                else
                if(bands[current_band].mode == DEMOD_AM_ME1)
                {  // E(n) = alpha * max [I, Q] + beta * min [I, Q] --> moving average DC removal --> lowpass 2nd order IIR
                   // Magnitude estimation Lyons (2011): page 652 / libcsdr
                    for(i = 0; i < FFT_length / 2; i++)
                    {
                          float_buffer_R[i] = alpha_beta_mag(iFFT_buffer[FFT_length + (i * 2)], iFFT_buffer[FFT_length + (i * 2) + 1]);
                    }
                  // DC removal
                  last_dc_level = fastdcblock_ff(float_buffer_R, float_buffer_L, FFT_length / 2, last_dc_level);
                  arm_biquad_cascade_df1_f32 (&biquad_lowpass1, float_buffer_L, float_buffer_R, FFT_length / 2);
                  arm_copy_f32(float_buffer_R, float_buffer_L, FFT_length/2);
                }
                else */
          if (bands[current_band].mode == DEMOD_AM_ME2)
          { // E(n) = alpha * max [I, Q] + beta * min [I, Q] --> highpass 1st order DC removal --> lowpass 2nd order IIR
            // Magnitude estimation Lyons (2011): page 652 / libcsdr
            for (unsigned i = 0; i < FFT_length / 2; i++)
            {
              audiotmp = alpha_beta_mag(iFFT_buffer[FFT_length + (i * 2)], iFFT_buffer[FFT_length + (i * 2) + 1]);
              // DC removal filter -----------------------
              w = audiotmp + wold * 0.9999f; // yes, I want a superb bass response ;-)
              float_buffer_L[i] = w - wold;
              wold = w;
            }
            arm_biquad_cascade_df1_f32 (&biquad_lowpass1, float_buffer_L, float_buffer_R, FFT_length / 2);
            arm_copy_f32(float_buffer_R, float_buffer_L, FFT_length / 2);
          }
          else
            /*      if(bands[current_band].mode == DEMOD_AM_ME3)
                  {  // E(n) = alpha * max [I, Q] + beta * min [I, Q] --> highpass 1st order DC removal --> NO lowpass
                     // Magnitude estimation Lyons (2011): page 652 / libcsdr
                      for(i = 0; i < FFT_length / 2; i++)
                      {
                            audiotmp = alpha_beta_mag(iFFT_buffer[FFT_length + (i * 2)], iFFT_buffer[FFT_length + (i * 2) + 1]);
                            // DC removal filter -----------------------
                            w = audiotmp + wold * 0.9999f; // yes, I want a superb bass response ;-)
                            float_buffer_L[i] = w - wold;
                            wold = w;
                      }
                    arm_copy_f32(float_buffer_L, float_buffer_R, FFT_length/2);
                  }
                  else */
            for (unsigned i = 0; i < FFT_length / 2; i++)
            {
              if (bands[current_band].mode == DEMOD_USB || bands[current_band].mode == DEMOD_LSB || bands[current_band].mode == DEMOD_DCF77 || bands[current_band].mode == DEMOD_AUTOTUNE)
              {
                float_buffer_L[i] = iFFT_buffer[FFT_length + (i * 2)];
                // for SSB copy real part in both outputs
                float_buffer_R[i] = float_buffer_L[i];
              }
              else if (bands[current_band].mode == DEMOD_STEREO_LSB || bands[current_band].mode == DEMOD_STEREO_USB) // creates a pseudo-stereo effect
                // could be good for copying faint CW signals
              {
                float_buffer_L[i] = iFFT_buffer[FFT_length + (i * 2)];
                float_buffer_R[i] = iFFT_buffer[FFT_length + (i * 2) + 1];
              }
            }



      /**********************************************************************************
          EXPERIMENTAL: variable-leak LMS algorithm
          can be switched to NOISE REDUCTION or AUTOMATIC NOTCH FILTER
          (c) Warren Pratt, wdsp 2016
          licensed under GPLv3
          only one channel --> float_buffer_L --> NR --> float_buffer_R
       **********************************************************************************/
      if (ANR_on)
      {
        xanr();
        if (!ANR_notch) arm_scale_f32(float_buffer_R, 4.0, float_buffer_R, FFT_length / 2);
        arm_copy_f32(float_buffer_R, float_buffer_L, FFT_length / 2);
      }

      /**********************************************************************************
          EXPERIMENTAL: noise blanker
         by Michael Wild
       **********************************************************************************/
      if (NB_on != 0)
      {
        noiseblanker(float_buffer_L, float_buffer_R);
        arm_copy_f32(float_buffer_R, float_buffer_L, FFT_length / 2);
      }

      if (NR_Kim != 0)
      {
        /**********************************************************************************
            EXPERIMENTAL STATION FOR SPECTRAL NOISE REDUCTION
            FFT - iFFT Convolution

           thanks a lot for your support, Michael DL2FW !
         **********************************************************************************/

        if (NR_Kim == 1)
        {
          ////////////////////////////////////////////////////////////////////////////////////////////////////////
          // this is exactly the implementation by
          // Kim & Ruwisch 2002 - 7th International Conference on Spoken Language Processing Denver, Colorado, USA
          // with two exceptions:
          // 1.) we use power instead of magnitude for X
          // 2.) we need to clamp for negative gains . . .
          ////////////////////////////////////////////////////////////////////////////////////////////////////////

          // perform a loop two times (each time process 128 new samples)
          // FFT 256 points
          // frame step 128 samples
          // half-overlapped data buffers

          uint8_t VAD_low = 0;
          uint8_t VAD_high = 127;
          float32_t lf_freq; // = (offset - width/2) / (12000 / NR_FFT_L); // bin BW is 46.9Hz [12000Hz / 256 bins] @96kHz
          float32_t uf_freq;
          if (bands[current_band].FLoCut <= 0 && bands[current_band].FHiCut >= 0)
          {
            lf_freq = 0.0;
            uf_freq = fmax(-(float32_t)bands[current_band].FLoCut, (float32_t)bands[current_band].FHiCut);
          }
          else
          {
            if (bands[current_band].FLoCut > 0)
            {
              lf_freq = (float32_t)bands[current_band].FLoCut;
              uf_freq = (float32_t)bands[current_band].FHiCut;
            }
            else
            {
              uf_freq = -(float32_t)bands[current_band].FLoCut;
              lf_freq = -(float32_t)bands[current_band].FHiCut;
            }
          }
          // / rate DF SR[SAMPLE_RATE].rate/DF
          lf_freq /= ((SR[SAMPLE_RATE].rate / DF) / NR_FFT_L); // bin BW is 46.9Hz [12000Hz / 256 bins] @96kHz
          uf_freq /= ((SR[SAMPLE_RATE].rate / DF) / NR_FFT_L);

          VAD_low = (int)lf_freq;
          VAD_high = (int)uf_freq;
          if (VAD_low == VAD_high)
          {
            VAD_high++;
          }
          if (VAD_low < 1)
          {
            VAD_low = 1;
          }
          else if (VAD_low > NR_FFT_L / 2 - 2)
          {
            VAD_low = NR_FFT_L / 2 - 2;
          }
          if (VAD_high < 1)
          {
            VAD_high = 1;
          }
          else if (VAD_high > NR_FFT_L / 2)
          {
            VAD_high = NR_FFT_L / 2;
          }

          for (int k = 0; k < 2; k++)
          {
            // NR_FFT_buffer is 512 floats big
            // interleaved r, i, r, i . . .

            // fill first half of FFT_buffer with last events audio samples
            for (int i = 0; i < NR_FFT_L / 2; i++)
            {
              NR_FFT_buffer[i * 2] = NR_last_sample_buffer_L[i]; // real
              NR_FFT_buffer[i * 2 + 1] = 0.0; // imaginary
            }

            // copy recent samples to last_sample_buffer for next time!
            for (int i = 0; i < NR_FFT_L  / 2; i++)
            {
              NR_last_sample_buffer_L [i] = float_buffer_L[i + k * (NR_FFT_L / 2)];
            }

            // now fill recent audio samples into second half of FFT_buffer
            for (int i = 0; i < NR_FFT_L / 2; i++)
            {
              NR_FFT_buffer[NR_FFT_L + i * 2] = float_buffer_L[i + k * (NR_FFT_L / 2)]; // real
              NR_FFT_buffer[NR_FFT_L + i * 2 + 1] = 0.0;
            }

            /////////////////////////////////
            // WINDOWING
#if 1
            // perform windowing on 256 real samples in the NR_FFT_buffer
            for (int idx = 0; idx < NR_FFT_L; idx++)
            { // Hann window
              float32_t temp_sample = 0.5 * (float32_t)(1.0 - (cosf(PI * 2.0 * (float32_t)idx / (float32_t)((NR_FFT_L) - 1))));
              NR_FFT_buffer[idx * 2] *= temp_sample;
            }
#endif

#if 0

            // perform windowing on 256 real samples in the NR_FFT_buffer
            for (int idx = 0; idx < NR_FFT_L; idx++)
            { // sqrt Hann window
              NR_FFT_buffer[idx * 2] *= sqrtHann[idx];
            }
#endif

            // NR_FFT 256
            // calculation is performed in-place the FFT_buffer [re, im, re, im, re, im . . .]
            arm_cfft_f32(NR_FFT, NR_FFT_buffer, 0, 1);

            // pass-thru
            //    arm_copy_f32(NR_FFT_buffer, NR_iFFT_buffer, NR_FFT_L * 2);

            /******************************************************************************************************
                Noise reduction starts here

                PROBLEM: negative gain values results!
             ******************************************************************************************************/

            // for debugging
            //  for(int idx = 0; idx < 5; idx++)
            //  {
            //      Serial.println(NR_FFT_buffer[idx]);
            //  }
            // the buffer contents are negative and positive, so taking absolute values for magnitude detection does seem to make some sense ;-)

            // NR_FFT_buffer contains interleaved 256 real and 256 imaginary values {r, i, r, i, r, i, . . .}
            // as far as I know, the first 128 contain the real part of the respective channel, maybe I am wrong???

            // the strategy is to take ONLY the real values (one channel) to estimate the noise reduction gain factors (in order to save processor time)
            // and then apply the same gain factors to both channel
            // we will see (better: hear) whether that makes sense or not

            // 2. MAGNITUDE CALCULATION  we save the absolute values of the bin results (bin magnitudes) in an array of 128 x 4 results in time [float32_t
            // [BTW: could we subsititue this step with a simple one pole IIR ?]
            // NR_X [128][4] contains the bin magnitudes
            // 2a copy current results into NR_X

            for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // take first 128 bin values of the FFT result
            { // it seems that taking power works better than taking magnitude . . . !?
              //NR_X[bindx][NR_X_pointer] = sqrtf(NR_FFT_buffer[bindx * 2] * NR_FFT_buffer[bindx * 2] + NR_FFT_buffer[bindx * 2 + 1] * NR_FFT_buffer[bindx * 2 + 1]);
              NR_X[bindx][NR_X_pointer] = (NR_FFT_buffer[bindx * 2] * NR_FFT_buffer[bindx * 2] + NR_FFT_buffer[bindx * 2 + 1] * NR_FFT_buffer[bindx * 2 + 1]);
            }

            // 3. AVERAGING: We average over these L_frames (eg. 4) results (for every bin) and save the result in float32_t NR_E[128, 20]:
            //    we do this for the last 20 averaged results.
            // 3a calculate average of the four values and save in E

            //            for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // take first 128 bin values of the FFT result
            for (int bindx = VAD_low; bindx < VAD_high; bindx++) // take first 128 bin values of the FFT result
            {
              NR_sum = 0.0;
              for (int j = 0; j < NR_L_frames; j++)
              { // sum up the L_frames |X|
                NR_sum = NR_sum + NR_X[bindx][j];
              }
              // divide sum of L_frames |X| by L_frames to calculate the average and save in NR_E
              NR_E[bindx][NR_E_pointer] = NR_sum / (float32_t)NR_L_frames;
            }

            // 4.  MINIMUM DETECTION: We search for the minimum in the last N_frames (eg. 20) results for E and save this minimum (for every bin): float32_t M[128]
            // 4a minimum search in all E values and save in M

            //            for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // take first 128 bin values of the FFT result
            for (int bindx = VAD_low; bindx < VAD_high; bindx++) // take first 128 bin values of the FFT result
            {
              // we have to reset the minimum value to the first E value every time we start with a bin
              NR_M[bindx] = NR_E[bindx][0];
              // therefore we start with the second E value (index j == 1)
              for (uint8_t j = 1; j < NR_N_frames; j++)
              { //
                if (NR_E[bindx][j] < NR_M[bindx])
                {
                  NR_M[bindx] = NR_E[bindx][j];
                }
              }
            }
            ////////////////////////////////////////////////////
            // TODO: make min-search more efficient
            ////////////////////////////////////////////////////

            // 5.  SNR CALCULATION: We calculate the signal-noise-ratio of the current frame T = X / M for every bin. If T > PSI {lambda = M}
            //     else {lambda = E} (float32_t lambda [128])

            //            for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // take first 128 bin values of the FFT result
            for (int bindx = VAD_low; bindx < VAD_high; bindx++) // take first 128 bin values of the FFT result
            {
              NR_T = NR_X[bindx][NR_X_pointer] / NR_M[bindx]; // dies scheint mir besser zu funktionieren !
              if (NR_T > NR_PSI)
              {
                NR_lambda[bindx] = NR_M[bindx];
              }
              else
              {
                NR_lambda[bindx] = NR_E[bindx][NR_E_pointer];
              }
            }

#ifdef DEBUG
            // for debugging
            for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
            {
              Serial.print((NR_lambda[bindx]), 6);
              Serial.print("   ");
            }
            Serial.println("-------------------------");
#endif

            // lambda is always positive
            // > 1 for bin 0 and bin 1, decreasing with bin number

            // 6.  SMOOTHED GAIN COMPUTATION: Calculate time smoothed gain factors float32_t Gts [128, 2],
            //     float32_t G[128]: G = 1 – (lambda / X); apply temporal smoothing: Gts (f, 0) = alpha * Gts (f, 1) + (1 – alpha) * G(f)

            //            for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // take first 128 bin values of the FFT result
            for (int bindx = VAD_low; bindx < VAD_high; bindx++) // take first 128 bin values of the FFT result
            {
              // the original equation is dividing by X. But this leads to negative gain factors sometimes!
              // better divide by E ???
              // we could also set NR_G to zero if its negative . . .

              if (NR_use_X)
              {
                NR_G[bindx] = 1.0 - (NR_lambda[bindx] * NR_KIM_K / NR_X[bindx][NR_X_pointer]);
                if (NR_G[bindx] < 0.0) NR_G[bindx] = 0.0;
              }
              else
              {
                NR_G[bindx] = 1.0 - (NR_lambda[bindx] * NR_KIM_K / NR_E[bindx][NR_E_pointer]);
                if (NR_G[bindx] < 0.0) NR_G[bindx] = 0.0;
              }

              // time smoothing
              NR_Gts[bindx][0] = NR_alpha * NR_Gts[bindx][1] + (NR_onemalpha) * NR_G[bindx];
              NR_Gts[bindx][1] = NR_Gts[bindx][0]; // copy for next FFT frame
            }

            // NR_G is always positive, however often 0.0

            // for debugging
#ifdef DEBUG
            for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
            {
              Serial.print((NR_Gts[bindx][0]), 6);
              Serial.print("   ");
            }
            Serial.println("-------------------------");
#endif

            // NR_Gts is always positive, bin 0 and bin 1 large, about 1.2 to 1.5, all other bins close to 0.2

            // 7.  Frequency smoothing of gain factors (recycle G array): G (f) = beta * Gts(f-1,0) + (1 – 2*beta) * Gts(f , 0) + beta * Gts(f + 1,0)

            for (int bindx = 1; bindx < ((NR_FFT_L / 2) - 1); bindx++) // take first 128 bin values of the FFT result
            {
              NR_G[bindx] = NR_beta * NR_Gts[bindx - 1][0] + NR_onemtwobeta * NR_Gts[bindx][0] + NR_beta * NR_Gts[bindx + 1][0];
            }
            // take care of bin 0 and bin NR_FFT_L/2 - 1
            NR_G[0] = (NR_onemtwobeta + NR_beta) * NR_Gts[0][0] + NR_beta * NR_Gts[1][0];
            NR_G[(NR_FFT_L / 2) - 1] = NR_beta * NR_Gts[(NR_FFT_L / 2) - 2][0] + (NR_onemtwobeta + NR_beta) * NR_Gts[(NR_FFT_L / 2) - 1][0];


            //old, probably right
            //          NR_G[0] = NR_beta * NR_G_bin_m_1 + NR_onemtwobeta * NR_Gts[0][0] + NR_beta * NR_Gts[1][0];
            //          NR_G[NR_FFT_L / 2 - 1] = NR_beta * NR_Gts[NR_FFT_L / 2 - 2][0] + (NR_onemtwobeta + NR_beta) * NR_Gts[NR_FFT_L / 2 - 1][0];
            // save gain for bin 0 for next frame
            //          NR_G_bin_m_1 = NR_Gts[NR_FFT_L / 2 - 1][0];

            // for debugging
#ifdef DEBUG
            for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
            {
              Serial.print((NR_G[bindx]), 6);
              Serial.print("   ");
            }
            Serial.println("-------------------------");
#endif

            // 8.  SPECTRAL WEIGHTING: Multiply current FFT results with NR_FFT_buffer for 256 bins with the 256 bin-specific gain factors G

            for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // try 128:
            {
              NR_FFT_buffer[bindx * 2] = NR_FFT_buffer [bindx * 2] * NR_G[bindx]; // real part
              NR_FFT_buffer[bindx * 2 + 1] = NR_FFT_buffer [bindx * 2 + 1] * NR_G[bindx]; // imag part
              NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 2] = NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 2] * NR_G[bindx]; // real part conjugate symmetric
              NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 1] = NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 1] * NR_G[bindx]; // imag part conjugate symmetric
            }

            // DEBUG
#ifdef DEBUG
            for (int bindx = 20; bindx < 21; bindx++)
            {
              Serial.println("************************************************");
              Serial.print("E: "); Serial.println(NR_E[bindx][NR_E_pointer]);
              Serial.print("MIN: "); Serial.println(NR_M[bindx]);
              Serial.print("lambda: "); Serial.println(NR_lambda[bindx]);
              Serial.print("X: "); Serial.println(NR_X[bindx][NR_X_pointer]);
              Serial.print("lanbda / X: "); Serial.println(NR_lambda[bindx] / NR_X[bindx][NR_X_pointer]);
              Serial.print("Gts: "); Serial.println(NR_Gts[bindx][0]);
              Serial.print("Gts old: "); Serial.println(NR_Gts[bindx][1]);
              Serial.print("Gfs: "); Serial.println(NR_G[bindx]);
            }
#endif

            // increment pointer AFTER everything has been processed !
            // 2b ++NR_X_pointer --> increment pointer for next FFT frame
            NR_X_pointer = NR_X_pointer + 1;
            if (NR_X_pointer >= NR_L_frames)
            {
              NR_X_pointer = 0;
            }
            // 3b ++NR_E_pointer
            NR_E_pointer = NR_E_pointer + 1;
            if (NR_E_pointer >= NR_N_frames)
            {
              NR_E_pointer = 0;
            }


#if 0
            for (int idx = 1; idx < 20; idx++)
              // bins 2 to 29 attenuated
              // set real values to 0.1 of their original value
            {
              NR_iFFT_buffer[idx * 2] *= 0.1;
              NR_iFFT_buffer[NR_FFT_L * 2 - ((idx + 1) * 2)] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
              NR_iFFT_buffer[idx * 2 + 1] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
              NR_iFFT_buffer[NR_FFT_L * 2 - ((idx + 1) * 2) + 1] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
            }
#endif

            // NR_iFFT
            // perform iFFT (in-place)
            arm_cfft_f32(NR_iFFT, NR_FFT_buffer, 1, 1);

#if 0
            // perform windowing on 256 real samples in the NR_FFT_buffer
            for (int idx = 0; idx < NR_FFT_L; idx++)
            { // sqrt Hann window
              NR_FFT_buffer[idx * 2] *= sqrtHann[idx];
            }
#endif

            // do the overlap & add

            for (int i = 0; i < NR_FFT_L / 2; i++)
            { // take real part of first half of current iFFT result and add to 2nd half of last iFFT_result
              NR_output_audio_buffer[i + k * (NR_FFT_L / 2)] = NR_FFT_buffer[i * 2] + NR_last_iFFT_result[i];
            }

            for (int i = 0; i < NR_FFT_L / 2; i++)
            {
              NR_last_iFFT_result[i] = NR_FFT_buffer[NR_FFT_L + i * 2];
            }

            // end of "for" loop which repeats the FFT_iFFT_chain two times !!!
          }

          for (int i = 0; i < NR_FFT_L; i++)
          {
            float_buffer_L [i] = NR_output_audio_buffer[i]; // * 9.0; // * 5.0;
            float_buffer_R [i] = float_buffer_L [i];
          }


        } // end of Kim et al. 2002 algorithm
        else if (NR_Kim == 2)
        {
          spectral_noise_reduction();
        }
        else if (NR_Kim == 4)
        {
          LMS_NoiseReduction (256, float_buffer_L);
          for (int i = 0; i < NR_FFT_L; i++)
          {
            float_buffer_R [i] = float_buffer_L [i];
          }
        }

        /////////////////////////////////////////////////////////////////////
      }

#if 0
      /**********************************************************************************
          SSB-AUTOTUNE
       **********************************************************************************/
      SSB_AUTOTUNE_counter++;

      if (SSB_AUTOTUNE_counter > 10)
      {
        SSB_AUTOTUNE_ccor(512, float_buffer_L, FFT_buffer);

        // we recycle FFT_buffer
        // interleaved r, i, r, i . . .
        for (int i = 0; i < 512; i++)
        {
          FFT_buffer[i * 2] = float_buffer_L[i]; // real
          FFT_buffer[i * 2 + 1] = 0.0; // imaginary
        }
        /////////////////////////////////7
        // WINDOWING
        // perform windowing
        for (int idx = 0; idx < 512; idx++)
        { // Hann window
          float32_t temp_sample = 0.5 * (float32_t)(1.0 - (cosf(PI * 2.0 * (float32_t)idx / (float32_t)((512) - 1))));
          FFT_buffer[idx * 2] *= temp_sample;
        }
        SSB_AUTOTUNE_counter = 0;
      }

#endif
      /**********************************************************************************
          CW DECODER
       **********************************************************************************/
      //cw_decoder();

      /**********************************************************************************
          INTERPOLATION
       **********************************************************************************/
      // re-uses iFFT_buffer[2048] and FFT_buffer !!!

      // receives 512 samples and makes 4096 samples out of it
      // interpolation-by-2
      // interpolation-in-place does not work
      arm_fir_interpolate_f32(&FIR_int1_I, float_buffer_L, iFFT_buffer, BUFFER_SIZE * N_BLOCKS / (uint32_t)(DF));
      arm_fir_interpolate_f32(&FIR_int1_Q, float_buffer_R, FFT_buffer, BUFFER_SIZE * N_BLOCKS / (uint32_t)(DF));

      // interpolation-by-4
      arm_fir_interpolate_f32(&FIR_int2_I, iFFT_buffer, float_buffer_L, BUFFER_SIZE * N_BLOCKS / (uint32_t)(DF1));
      arm_fir_interpolate_f32(&FIR_int2_Q, FFT_buffer, float_buffer_R, BUFFER_SIZE * N_BLOCKS / (uint32_t)(DF1));

      // scale after interpolation
      //      float32_t interpol_scale = 8.0;
      //      if(bands[current_band].mode == DEMOD_LSB || bands[current_band].mode == DEMOD_USB) interpol_scale = 16.0;
      //      if(bands[current_band].mode == DEMOD_USB) interpol_scale = 16.0;
      arm_scale_f32(float_buffer_L, DF, float_buffer_L, BUFFER_SIZE * N_BLOCKS);
      arm_scale_f32(float_buffer_R, DF, float_buffer_R, BUFFER_SIZE * N_BLOCKS);

#ifdef USE_ADC_DAC_display
      /**********************************************************************
                CONVERT TO INTEGER AND PLAY AUDIO
       **********************************************************************/
      if (omitOutputFlag)
        omitOutputFlag = false;
      else
      {
        for (int i = 0; i < N_BLOCKS; i++)
        {
          sp_L = Q_out_L.getBuffer();    // two of these is about 5 usec
          sp_R = Q_out_R.getBuffer();
          if (i == 15)      // A rough indicator, so only a sampling of data
          {
            dacMaxLevel = 0;
            for (int xx = 0; xx < 128; xx++)
            {
              if (sp_L[xx] > dacMaxLevel)
                dacMaxLevel = sp_L[xx];
            }
          }
          arm_float_to_q15 (&float_buffer_L[BUFFER_SIZE * i], sp_L, BUFFER_SIZE); // two of these is about 32 usec
          arm_float_to_q15 (&float_buffer_R[BUFFER_SIZE * i], sp_R, BUFFER_SIZE);
          uB4 = (uint16_t)usec;
          Q_out_L.playBuffer(); // play it !  two of these is about 4 usec
          Q_out_R.playBuffer(); // play it !
          uAfter = (uint16_t)usec;
          if ((uAfter - uB4) > 20)  {
#ifdef DEBUG
            Serial.println(uAfter - uB4);
#endif
            omitOutputFlag = true;
          }
        }       // Over 16 blocks
      }

      // Update level bar graphs   <PUA>
      if ((barGraphUpdate++) > 4  &&  bands[current_band].mode != DEMOD_SAM  &&  bands[current_band].mode != DEMOD_SAM_STEREO)
      {
        displayLevel(adcMaxLevel, dacMaxLevel);
        barGraphUpdate = 0;
      }

      // *******************************************************************


#else
      // **********************************************************************************
      //    CONVERT TO INTEGER AND PLAY AUDIO
      // **********************************************************************************
      for (unsigned  i = 0; i < N_BLOCKS; i++)
      {
        sp_L = Q_out_L.getBuffer();
        sp_R = Q_out_R.getBuffer();
        arm_float_to_q15 (&float_buffer_L[BUFFER_SIZE * i], sp_L, BUFFER_SIZE);
        arm_float_to_q15 (&float_buffer_R[BUFFER_SIZE * i], sp_R, BUFFER_SIZE);
        Q_out_L.playBuffer(); // play it !
        Q_out_R.playBuffer(); // play it !
      }
#endif

      /**********************************************************************************
          PRINT ROUTINE FOR ELAPSED MICROSECONDS
       **********************************************************************************/

      sum = sum + usec;
      idx_t++;
      if (idx_t > 40) {
        tft.fillRect(227, 5, 45, 20, ILI9341_BLACK);
        tft.setCursor(227, 5);
        tft.setTextColor(ILI9341_GREEN);
        tft.setFont(Arial_9);
        mean = sum / idx_t;
        if (mean / 29.00 / N_BLOCKS * SR[SAMPLE_RATE].rate / AUDIO_SAMPLE_RATE_EXACT < 100.0)
        {
          tft.print (mean / 29.00 / N_BLOCKS * SR[SAMPLE_RATE].rate / AUDIO_SAMPLE_RATE_EXACT);
          tft.print("%");
        }
        else
        {
          tft.setTextColor(ILI9341_RED);
          tft.print("100%");
        }
#ifdef DEBUG
        Serial.print (mean);
        Serial.print (" microsec for ");
        Serial.print (N_BLOCKS);
        Serial.print ("  stereo blocks    ");
        Serial.println();
        Serial.print (" n_clear    ");
        Serial.println(n_clear);
#endif
        idx_t = 0;
        sum = 0;
        tft.setTextColor(ILI9341_WHITE);
      }
      /*
          if(zoom_display)
          {
            show_spectrum();
            zoom_display = 0;
          zoom_sample_ptr = 0;
          }
      */
      if (bands[current_band].mode == DEMOD_DCF77)
      {
      }
      if (auto_codec_gain == 1)
      {
        codec_gain();
      }
    } // end of if(audio blocks available)

  /**********************************************************************************
      PRINT ROUTINE FOR AUDIO LIBRARY PROCESSOR AND MEMORY USAGE
   **********************************************************************************/
  if (0) //(five_sec.check() == 1)
  {
    Serial.print("Proc = ");
    Serial.print(AudioProcessorUsage());
    Serial.print(" (");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("),  Mem = ");
    Serial.print(AudioMemoryUsage());
    Serial.print(" (");
    Serial.print(AudioMemoryUsageMax());
    Serial.println(")");
    AudioProcessorUsageMaxReset();
    AudioMemoryUsageMaxReset();
    //      eeprom_saved = 0;
    //      eeprom_loaded = 0;
  }
  //    show_spectrum();

  if (encoder_check.check() == 1)
  {
    encoders();
    buttons();
    //    displayClock();
    show_analog_gain();
    //        Serial.print("SAM carrier frequency = "); Serial.println(SAM_carrier_freq_offset);
  }

  if (ms_500.check() == 1)
  {
    wait_flag = 0;
    displayClock();
  }

  //    if(dbm_check.check() == 1) Calculatedbm();
#if defined(MP3)
  if (Menu_pointer == MENU_PLAYER)
  {
    if (trackext[track] == 1)
    {
#ifdef DEBUG
      Serial.println("MP3" );
#endif
      playFileMP3(playthis);
    }
    else if (trackext[track] == 2)
    {
#ifdef DEBUG
      Serial.println("aac");
#endif
      playFileAAC(playthis);
    }
    if (trackchange == true)
    { //when your track is finished, go to the next one. when using buttons, it will not skip twice.
      nexttrack();
    }
  }
#endif
} // end loop


void xanr () // variable leak LMS algorithm for automatic notch or noise reduction
{ // (c) Warren Pratt wdsp library 2016
  int idx;
  float32_t c0, c1;
  float32_t y, error, sigma, inv_sigp;
  float32_t nel, nev;

  for (int i = 0; i < ANR_buff_size; i++)
  {
    //      ANR_d[ANR_in_idx] = in_buff[2 * i + 0];
    ANR_d[ANR_in_idx] = float_buffer_L[i];

    y = 0;
    sigma = 0;

    for (int j = 0; j < ANR_taps; j++)
    {
      idx = (ANR_in_idx + j + ANR_delay) & ANR_mask;
      y += ANR_w[j] * ANR_d[idx];
      sigma += ANR_d[idx] * ANR_d[idx];
    }
    inv_sigp = 1.0 / (sigma + 1e-10);
    error = ANR_d[ANR_in_idx] - y;

    if (ANR_notch) float_buffer_R[i] = error; // NOTCH FILTER
    else  float_buffer_R[i] = y; // NOISE REDUCTION

    if ((nel = error * (1.0 - ANR_two_mu * sigma * inv_sigp)) < 0.0) nel = -nel;
    if ((nev = ANR_d[ANR_in_idx] - (1.0 - ANR_two_mu * ANR_ngamma) * y - ANR_two_mu * error * sigma * inv_sigp) < 0.0) nev = -nev;
    if (nev < nel)
    {
      if ((ANR_lidx += ANR_lincr) > ANR_lidx_max) ANR_lidx = ANR_lidx_max;
      else if ((ANR_lidx -= ANR_ldecr) < ANR_lidx_min) ANR_lidx = ANR_lidx_min;
    }
    ANR_ngamma = ANR_gamma * (ANR_lidx * ANR_lidx) * (ANR_lidx * ANR_lidx) * ANR_den_mult;

    c0 = 1.0 - ANR_two_mu * ANR_ngamma;
    c1 = ANR_two_mu * error * inv_sigp;

    for (int j = 0; j < ANR_taps; j++)
    {
      idx = (ANR_in_idx + j + ANR_delay) & ANR_mask;
      ANR_w[j] = c0 * ANR_w[j] + c1 * ANR_d[idx];
    }
    ANR_in_idx = (ANR_in_idx + ANR_mask) & ANR_mask;
  }
}


void IQ_phase_correction (float32_t *I_buffer, float32_t *Q_buffer, float32_t factor, uint32_t blocksize)
{
  float32_t temp_buffer[blocksize];
  if (factor < 0.0)
  { // mix a bit of I into Q
    arm_scale_f32 (I_buffer, factor, temp_buffer, blocksize);
    arm_add_f32 (Q_buffer, temp_buffer, Q_buffer, blocksize);
  }
  else
  { // // mix a bit of Q into I
    arm_scale_f32 (Q_buffer, factor, temp_buffer, blocksize);
    arm_add_f32 (I_buffer, temp_buffer, I_buffer, blocksize);
  }
} // end IQ_phase_correction

void AGC_prep()
{
  float32_t tmp;
  float32_t sample_rate = (float32_t)SR[SAMPLE_RATE].rate / DF;
  // Start variables taken from wdsp
  // RXA.c !!!!
  /*
      0.001,                      // tau_attack
      0.250,                      // tau_decay
      4,                        // n_tau
      10000.0,                    // max_gain
      1.5,                      // var_gain
      1000.0,                     // fixed_gain
      1.0,                      // max_input
      1.0,                      // out_target
      0.250,                      // tau_fast_backaverage
      0.005,                      // tau_fast_decay
      5.0,                      // pop_ratio
      1,                        // hang_enable
      0.500,                      // tau_hang_backmult
      0.250,                      // hangtime
      0.250,                      // hang_thresh
      0.100);                     // tau_hang_decay
  */
  /*  GOOD WORKING VARIABLES
      max_gain = 1.0;                    // max_gain
      var_gain = 0.0015; // 1.5                      // var_gain
      fixed_gain = 1.0;                     // fixed_gain
      max_input = 1.0;                 // max_input
      out_target = 0.00005; //0.0001; // 1.0                // out_target

  */
  tau_attack = 0.001;               // tau_attack
  //    tau_decay = 0.250;                // tau_decay
  n_tau = 1;                        // n_tau

  //    max_gain = 1000.0; // 1000.0; max gain to be applied??? or is this AGC threshold = knee level?
  fixed_gain = 0.7; // if AGC == OFF
  max_input = 2.0; //
  out_targ = 0.3; // target value of audio after AGC
  //    var_gain = 32.0;  // slope of the AGC --> this is 10 * 10^(slope / 20) --> for 10dB slope, this is 30
  var_gain = powf (10.0, (float32_t)agc_slope / 200.0); // 10 * 10^(slope / 20)

  tau_fast_backaverage = 0.250;    // tau_fast_backaverage
  tau_fast_decay = 0.005;          // tau_fast_decay
  pop_ratio = 5.0;                 // pop_ratio
  hang_enable = 0;                 // hang_enable
  tau_hang_backmult = 0.500;       // tau_hang_backmult
  hangtime = 0.250;                // hangtime
  hang_thresh = 0.250;             // hang_thresh
  tau_hang_decay = 0.100;          // tau_hang_decay

  //calculate internal parameters
  if (agc_switch_mode)
  {
    switch (AGC_mode)
    {
      case 0: //agcOFF
        break;
      case 2: //agcLONG
        hangtime = 2.000;
        agc_decay = 2000;
        break;
      case 3: //agcSLOW
        hangtime = 1.000;
        agc_decay = 500;
        break;
      case 4: //agcMED
        hang_thresh = 1.0;
        hangtime = 0.000;
        agc_decay = 250;
        break;
      case 5: //agcFAST
        hang_thresh = 1.0;
        hangtime = 0.000;
        agc_decay = 50;
        break;
      case 1: //agcFrank
        hang_enable = 0;
        hang_thresh = 0.100; // from which level on should hang be enabled
        hangtime = 2.000; // hang time, if enabled
        tau_hang_backmult = 0.500; // time constant exponential averager

        agc_decay = 4000; // time constant decay long
        tau_fast_decay = 0.05;          // tau_fast_decay
        tau_fast_backaverage = 0.250; // time constant exponential averager
        //      max_gain = 1000.0; // max gain to be applied??? or is this AGC threshold = knee level?
        //      fixed_gain = 1.0; // if AGC == OFF
        //      max_input = 1.0; //
        //      out_targ = 0.2; // target value of audio after AGC
        //      var_gain = 30.0;  // slope of the AGC -->

        /*    // sehr gut!
               hang_thresh = 0.100;
              hangtime = 2.000;
              tau_decay = 2.000;
              tau_hang_backmult = 0.500;
              tau_fast_backaverage = 0.250;
              out_targ = 0.0004;
              var_gain = 0.001; */
        break;
      default:
        break;
    }
    agc_switch_mode = 0;
  }
  tau_decay = (float32_t)agc_decay / 1000.0;
  //  max_gain = powf (10.0, (float32_t)agc_thresh / 20.0);
  max_gain = powf (10.0, (float32_t)bands[current_band].AGC_thresh / 20.0);

  attack_buffsize = (int)ceil(sample_rate * n_tau * tau_attack);
  //Serial.println(attack_buffsize);
  in_index = attack_buffsize + out_index;
  attack_mult = 1.0 - expf(-1.0 / (sample_rate * tau_attack));
  //Serial.print("attack_mult = ");
  //Serial.println(attack_mult * 1000);
  decay_mult = 1.0 - expf(-1.0 / (sample_rate * tau_decay));
  //Serial.print("decay_mult = ");
  //Serial.println(decay_mult * 1000);
  fast_decay_mult = 1.0 - expf(-1.0 / (sample_rate * tau_fast_decay));
  //Serial.print("fast_decay_mult = ");
  //Serial.println(fast_decay_mult * 1000);
  fast_backmult = 1.0 - expf(-1.0 / (sample_rate * tau_fast_backaverage));
  //Serial.print("fast_backmult = ");
  //Serial.println(fast_backmult * 1000);

  onemfast_backmult = 1.0 - fast_backmult;

  out_target = out_targ * (1.0 - expf(-(float32_t)n_tau)) * 0.9999;
  //  out_target = out_target * (1.0 - expf(-(float32_t)n_tau)) * 0.9999;
  //Serial.print("out_target = ");
  //Serial.println(out_target * 1000);
  min_volts = out_target / (var_gain * max_gain);
  inv_out_target = 1.0 / out_target;

  tmp = log10f(out_target / (max_input * var_gain * max_gain));
  if (tmp == 0.0)
    tmp = 1e-16;
  slope_constant = (out_target * (1.0 - 1.0 / var_gain)) / tmp;
  //  Serial.print("slope_constant = ");
  //  Serial.println(slope_constant * 1000);

  inv_max_input = 1.0 / max_input;

  tmp = powf (10.0, (hang_thresh - 1.0) / 0.125);
  hang_level = (max_input * tmp + (out_target /
                                   (var_gain * max_gain)) * (1.0 - tmp)) * 0.637;

  hang_backmult = 1.0 - expf(-1.0 / (sample_rate * tau_hang_backmult));
  onemhang_backmult = 1.0 - hang_backmult;

  hang_decay_mult = 1.0 - expf(-1.0 / (sample_rate * tau_hang_decay));
}


void AGC()
{
  int k;
  float32_t mult;

  if (AGC_mode == 0)  // AGC OFF
  {
    for (unsigned i = 0; i < FFT_length / 2; i++)
    {
      iFFT_buffer[FFT_length + 2 * i + 0] = fixed_gain * iFFT_buffer[FFT_length + 2 * i + 0];
      iFFT_buffer[FFT_length + 2 * i + 1] = fixed_gain * iFFT_buffer[FFT_length + 2 * i + 1];
    }
    return;
  }

  for (unsigned i = 0; i < FFT_length / 2; i++)
  {
    if (++out_index >= (int)ring_buffsize)
      out_index -= ring_buffsize;
    if (++in_index >= ring_buffsize)
      in_index -= ring_buffsize;

    out_sample[0] = ring[2 * out_index + 0];
    out_sample[1] = ring[2 * out_index + 1];
    abs_out_sample = abs_ring[out_index];
    ring[2 * in_index + 0] = iFFT_buffer[FFT_length + 2 * i + 0];
    ring[2 * in_index + 1] = iFFT_buffer[FFT_length + 2 * i + 1];
    if (pmode == 0) // MAGNITUDE CALCULATION
      abs_ring[in_index] = max(fabs(ring[2 * in_index + 0]), fabs(ring[2 * in_index + 1]));
    else
      abs_ring[in_index] = sqrtf(ring[2 * in_index + 0] * ring[2 * in_index + 0] + ring[2 * in_index + 1] * ring[2 * in_index + 1]);

    fast_backaverage = fast_backmult * abs_out_sample + onemfast_backmult * fast_backaverage;
    hang_backaverage = hang_backmult * abs_out_sample + onemhang_backmult * hang_backaverage;

    if ((abs_out_sample >= ring_max) && (abs_out_sample > 0.0))
    {
      ring_max = 0.0;
      k = out_index;
      for (int j = 0; j < attack_buffsize; j++)
      {
        if (++k == (int)ring_buffsize)
          k = 0;
        if (abs_ring[k] > ring_max)
          ring_max = abs_ring[k];
      }
    }
    if (abs_ring[in_index] > ring_max)
      ring_max = abs_ring[in_index];

    if (hang_counter > 0)
      --hang_counter;
    //      Serial.println(ring_max);
    //      Serial.println(volts);

    switch (state)
    {
      case 0:
        {
          if (ring_max >= volts)
          {
            volts += (ring_max - volts) * attack_mult;
          }
          else
          {
            if (volts > pop_ratio * fast_backaverage)
            {
              state = 1;
              volts += (ring_max - volts) * fast_decay_mult;
            }
            else
            {
              if (hang_enable && (hang_backaverage > hang_level))
              {
                state = 2;
                hang_counter = (int)(hangtime * SR[SAMPLE_RATE].rate / DF);
                decay_type = 1;
              }
              else
              {
                state = 3;
                volts += (ring_max - volts) * decay_mult;
                decay_type = 0;
              }
            }
          }
          break;
        }
      case 1:
        {
          if (ring_max >= volts)
          {
            state = 0;
            volts += (ring_max - volts) * attack_mult;
          }
          else
          {
            if (volts > save_volts)
            {
              volts += (ring_max - volts) * fast_decay_mult;
            }
            else
            {
              if (hang_counter > 0)
              {
                state = 2;
              }
              else
              {
                if (decay_type == 0)
                {
                  state = 3;
                  volts += (ring_max - volts) * decay_mult;
                }
                else
                {
                  state = 4;
                  volts += (ring_max - volts) * hang_decay_mult;
                }
              }
            }
          }
          break;
        }
      case 2:
        {
          if (ring_max >= volts)
          {
            state = 0;
            save_volts = volts;
            volts += (ring_max - volts) * attack_mult;
          }
          else
          {
            if (hang_counter == 0)
            {
              state = 4;
              volts += (ring_max - volts) * hang_decay_mult;
            }
          }
          break;
        }
      case 3:
        {
          if (ring_max >= volts)
          {
            state = 0;
            save_volts = volts;
            volts += (ring_max - volts) * attack_mult;
          }
          else
          {
            volts += (ring_max - volts) * decay_mult;
          }
          break;
        }
      case 4:
        {
          if (ring_max >= volts)
          {
            state = 0;
            save_volts = volts;
            volts += (ring_max - volts) * attack_mult;
          }
          else
          {
            volts += (ring_max - volts) * hang_decay_mult;
          }
          break;
        }
    }
    if (volts < min_volts)
    {
      volts = min_volts; // no AGC action is taking place
      agc_action = 0;
    }
    else
    {
      // LED indicator for AGC action
      agc_action = 1;
    }
    //      Serial.println(volts * inv_out_target);
#ifdef USE_LOG10FAST
    mult = (out_target - slope_constant * min (0.0, log10f_fast(inv_max_input * volts))) / volts;
#else
    mult = (out_target - slope_constant * min (0.0, log10f(inv_max_input * volts))) / volts;
#endif
    //    Serial.println(mult * 1000);
    //      Serial.println(volts * 1000);
    iFFT_buffer[FFT_length + 2 * i + 0] = out_sample[0] * mult;
    iFFT_buffer[FFT_length + 2 * i + 1] = out_sample[1] * mult;
  }
}

void filter_bandwidth()
{
  AudioNoInterrupts();
  sgtl5000_1.dacVolume(0.0);
  calc_cplx_FIR_coeffs (FIR_Coef_I, FIR_Coef_Q, m_NumTaps, (float32_t)bands[current_band].FLoCut, (float32_t)bands[current_band].FHiCut, (float)SR[SAMPLE_RATE].rate / DF);
  init_filter_mask();

  // also adjust IIR AM filter
  int filter_BW_highest = bands[current_band].FHiCut;
  if (filter_BW_highest < - bands[current_band].FLoCut) filter_BW_highest = - bands[current_band].FLoCut;
  set_IIR_coeffs ((float32_t)filter_BW_highest, 1.3, (float32_t)SR[SAMPLE_RATE].rate / DF, 0); // 1st stage
  for (int i = 0; i < 5; i++)
  {
    biquad_lowpass1_coeffs[i] = coefficient_set[i];
  }

  // and adjust decimation and interpolation filters
  set_dec_int_filters();

  show_bandwidth ();

  sgtl5000_1.dacVolume(1.0);
  delay(1);
  AudioInterrupts();

} // end filter_bandwidth

void calc_FIR_coeffs (float * coeffs_I, int numCoeffs, float32_t fc, float32_t Astop, int type, float dfc, float Fsamprate)
// pointer to coefficients variable, no. of coefficients to calculate, frequency where it happens, stopband attenuation in dB,
// filter type, half-filter bandwidth (only for bandpass and notch)
{ // modified by WMXZ and DD4WH after
  // Wheatley, M. (2011): CuteSDR Technical Manual. www.metronix.com, pages 118 - 120, FIR with Kaiser-Bessel Window
  // assess required number of coefficients by
  //     numCoeffs = (Astop - 8.0) / (2.285 * TPI * normFtrans);
  // selecting high-pass, numCoeffs is forced to an even number for better frequency response

  float32_t Beta;
  float32_t izb;
  float fcf = fc;
  int nc = numCoeffs;
  fc = fc / Fsamprate;
  dfc = dfc / Fsamprate;
  // calculate Kaiser-Bessel window shape factor beta from stop-band attenuation
  if (Astop < 20.96)
    Beta = 0.0;
  else if (Astop >= 50.0)
    Beta = 0.1102 * (Astop - 8.71);
  else
    Beta = 0.5842 * powf((Astop - 20.96), 0.4) + 0.07886 * (Astop - 20.96);

  izb = Izero (Beta);
  if (type == 0) // low pass filter
    //     {  fcf = fc;
  { fcf = fc * 2.0;
    nc =  numCoeffs;
  }
  else if (type == 1) // high-pass filter
  { fcf = -fc;
    nc =  2 * (numCoeffs / 2);
  }
  else if ((type == 2) || (type == 3)) // band-pass filter
  {
    fcf = dfc;
    nc =  2 * (numCoeffs / 2); // maybe not needed
  }
  else if (type == 4) // Hilbert transform
  {
    nc =  2 * (numCoeffs / 2);
    // clear coefficients
    for (int ii = 0; ii < 2 * (nc - 1); ii++) coeffs_I[ii] = 0;
    // set real delay
    coeffs_I[nc] = 1;

    // set imaginary Hilbert coefficients
    for (int ii = 1; ii < (nc + 1); ii += 2)
    {
      if (2 * ii == nc) continue;
      float x = (float)(2 * ii - nc) / (float)nc;
      float w = Izero(Beta * sqrtf(1.0f - x * x)) / izb; // Kaiser window
      coeffs_I[2 * ii + 1] = 1.0f / (PIH * (float)(ii - nc / 2)) * w ;
    }
    return;
  }

  for (int ii = - nc, jj = 0; ii < nc; ii += 2, jj++)
  {
    float x = (float)ii / (float)nc;
    float w = Izero(Beta * sqrtf(1.0f - x * x)) / izb; // Kaiser window
    coeffs_I[jj] = fcf * m_sinc(ii, fcf) * w;

  }

  if (type == 1)
  {
    coeffs_I[nc / 2] += 1;
  }
  else if (type == 2)
  {
    for (int jj = 0; jj < nc + 1; jj++) coeffs_I[jj] *= 2.0f * cosf(PIH * (2 * jj - nc) * fc);
  }
  else if (type == 3)
  {
    for (int jj = 0; jj < nc + 1; jj++) coeffs_I[jj] *= -2.0f * cosf(PIH * (2 * jj - nc) * fc);
    coeffs_I[nc / 2] += 1;
  }

} // END calc_FIR_coeffs


//////////////////////////////////////////////////////////////////////
//  Call to setup filter parameters
// SampleRate in Hz
// FLowcut is low cutoff frequency of filter in Hz
// FHicut is high cutoff frequency of filter in Hz
// Offset is the CW tone offset frequency
// cutoff frequencies range from -SampleRate/2 to +SampleRate/2
//  HiCut must be greater than LowCut
//    example to make 2700Hz USB filter:
//  SetupParameters( 100, 2800, 0, 48000);
//////////////////////////////////////////////////////////////////////

//void calc_cplx_FIR_coeffs (float * coeffs_I, float * coeffs_Q, int numCoeffs, float32_t fc, float32_t Astop, int type, float dfc, float SampleRate)
void calc_cplx_FIR_coeffs (float * coeffs_I, float * coeffs_Q, int numCoeffs, float32_t FLoCut, float32_t FHiCut, float SampleRate)
// pointer to coefficients variable, no. of coefficients to calculate, frequency where it happens, stopband attenuation in dB,
// filter type, half-filter bandwidth (only for bandpass and notch)

//void CFastFIR::SetupParameters( TYPEREAL FLoCut, TYPEREAL FHiCut,
//                TYPEREAL Offset, TYPEREAL SampleRate)
{

  //calculate some normalized filter parameters
  float32_t nFL = FLoCut / SampleRate;
  float32_t nFH = FHiCut / SampleRate;
  float32_t nFc = (nFH - nFL) / 2.0; //prototype LP filter cutoff
  float32_t nFs = PI * (nFH + nFL); //2 PI times required frequency shift (FHiCut+FLoCut)/2
  float32_t fCenter = 0.5 * (float32_t)(numCoeffs - 1); //floating point center index of FIR filter

  //  for (int i = 0; i < FFT_length; i++) // WRONG! causes overflow, because coeffs_I is of length [FFT_length / 2 + 1]
  for (int i = 0; i < numCoeffs; i++) //zero pad entire coefficient buffer
  {
    coeffs_I[i] = 0.0;
    coeffs_Q[i] = 0.0;
  }

  //create LP FIR windowed sinc, sin(x)/x complex LP filter coefficients
  for (int i = 0; i < numCoeffs; i++)
  {
    float32_t x = (float32_t)i - fCenter;
    float32_t z;
    if ( abs((float)i - fCenter) < 0.01) //deal with odd size filter singularity where sin(0)/0==1
      z = 2.0 * nFc;
    else
      switch (FIR_filter_window) {
        case 1:    // 4-term Blackman-Harris --> this is what Power SDR uses
          z = (float32_t)sinf(TWO_PI * x * nFc) / (PI * x) *
              (0.35875 - 0.48829 * cosf( (TWO_PI * i) / (numCoeffs - 1) )
               + 0.14128 * cosf( (FOURPI * i) / (numCoeffs - 1) )
               - 0.01168 * cosf( (SIXPI * i) / (numCoeffs - 1) ) );
          break;
        case 2:
          z = (float32_t)sinf(TWO_PI * x * nFc) / (PI * x) *
              (0.355768 - 0.487396 * cosf( (TWO_PI * i) / (numCoeffs - 1) )
               + 0.144232 * cosf( (FOURPI * i) / (numCoeffs - 1) )
               - 0.012604 * cosf( (SIXPI * i) / (numCoeffs - 1) ) );
          break;
        case 3: // cosine
          z = (float32_t)sinf(TWO_PI * x * nFc) / (PI * x) *
              cosf((PI * (float32_t)i) / (numCoeffs - 1));
          break;
        case 4: // Hann
          z = (float32_t)sinf(TWO_PI * x * nFc) / (PI * x) *
              0.5 * (float32_t)(1.0 - (cosf(PI * 2 * (float32_t)i / (float32_t)(numCoeffs - 1))));
          break;
        default: // Blackman-Nuttall window
          z = (float32_t)sinf(TWO_PI * x * nFc) / (PI * x) *
              (0.3635819
               - 0.4891775 * cosf( (TWO_PI * i) / (numCoeffs - 1) )
               + 0.1365995 * cosf( (FOURPI * i) / (numCoeffs - 1) )
               - 0.0106411 * cosf( (SIXPI * i) / (numCoeffs - 1) ) );
          break;
      }
    //shift lowpass filter coefficients in frequency by (hicut+lowcut)/2 to form bandpass filter anywhere in range
    coeffs_I[i]  =  z * cosf(nFs * x);
    coeffs_Q[i] = z * sinf(nFs * x);
  }
}

/*
  void calc_cplx_FIR_coeffs (float * coeffs_I, float * coeffs_Q, int numCoeffs, float32_t fc, float32_t Astop, int type, float dfc, float Fsamprate)
    // pointer to coefficients variable, no. of coefficients to calculate, frequency where it happens, stopband attenuation in dB,
    // filter type, half-filter bandwidth (only for bandpass and notch)
  {  // modified by WMXZ and DD4WH after
    // Wheatley, M. (2011): CuteSDR Technical Manual. www.metronix.com, pages 118 - 120, FIR with Kaiser-Bessel Window
    // assess required number of coefficients by
    //     numCoeffs = (Astop - 8.0) / (2.285 * TPI * normFtrans);
    // selecting high-pass, numCoeffs is forced to an even number for better frequency response

     float32_t nFs = TWO_PI * 200.0/96000.0;
     int ii,jj;
     float32_t Beta;
     float32_t izb;
     float fcf = fc;
     int nc = numCoeffs;
     fc = fc / Fsamprate;
     dfc = dfc / Fsamprate;
     // calculate Kaiser-Bessel window shape factor beta from stop-band attenuation
     if (Astop < 20.96)
       Beta = 0.0;
     else if (Astop >= 50.0)
       Beta = 0.1102 * (Astop - 8.71);
     else
       Beta = 0.5842 * powf((Astop - 20.96), 0.4) + 0.07886 * (Astop - 20.96);

     izb = Izero (Beta);
     if(type == 0) // low pass filter
  //     {  fcf = fc;
     {  fcf = fc * 2.0;
      nc =  numCoeffs;
     }
     else if(type == 1) // high-pass filter
     {  fcf = -fc;
      nc =  2*(numCoeffs/2);
     }
     else if ((type == 2) || (type==3)) // band-pass filter
     {
       fcf = dfc;
       nc =  2*(numCoeffs/2); // maybe not needed
     }
     else if (type==4)  // Hilbert transform
   {
         nc =  2*(numCoeffs/2);
       // clear coefficients
       for(ii=0; ii< 2*(nc-1); ii++) coeffs_I[ii]=0;
       // set real delay
       coeffs_I[nc]=1;

       // set imaginary Hilbert coefficients
       for(ii=1; ii< (nc+1); ii+=2)
       {
         if(2*ii==nc) continue;
       float x =(float)(2*ii - nc)/(float)nc;
       float w = Izero(Beta*sqrtf(1.0f - x*x))/izb; // Kaiser window
       coeffs_I[2*ii+1] = 1.0f/(PIH*(float)(ii-nc/2)) * w ;
       }
       return;
   }
     float32_t test;
     for(ii= - nc, jj=0; ii< nc; ii+=2,jj++)
     {
       float x =(float)ii/(float)nc;
       float w = Izero(Beta*sqrtf(1.0f - x*x))/izb; // Kaiser window
  //       coeffs[jj] = fcf * m_sinc(ii,fcf) * w;
       test = fcf * m_sinc(ii,fcf) * w;
       coeffs_I[jj] = test * cosf(nFs * ii); // create complex coefficients
       coeffs_Q[jj] = test * sinf(nFs * ii);
     }

     if(type==1)
     {
       coeffs_I[nc/2] += 1;
     }
     else if (type==2)
     {
         for(jj=0; jj< nc+1; jj++) coeffs_I[jj] *= 2.0f*cosf(PIH*(2*jj-nc)*fc);
     }
     else if (type==3)
     {
         for(jj=0; jj< nc+1; jj++) coeffs_I[jj] *= -2.0f*cosf(PIH*(2*jj-nc)*fc);
       coeffs_I[nc/2] += 1;
     }

  } // END calc_FIR_coeffs
*/
float m_sinc(int m, float fc)
{ // fc is f_cut/(Fsamp/2)
  // m is between -M and M step 2
  //
  float x = m * PIH;
  if (m == 0)
    return 1.0f;
  else
    return sinf(x * fc) / (fc * x);
}
/*
  void calc_FIR_lowpass_coeffs (float32_t Scale, float32_t Astop, float32_t Fpass, float32_t Fstop, float32_t Fsamprate)
  { // Wheatley, M. (2011): CuteSDR Technical Manual. www.metronix.com, pages 118 - 120, FIR with Kaiser-Bessel Window
    int n;
    float32_t Beta;
    float32_t izb;
    // normalized F parameters
    float32_t normFpass = Fpass / Fsamprate;
    float32_t normFstop = Fstop / Fsamprate;
    float32_t normFcut = (normFstop + normFpass) / 2.0; // lowpass filter 6dB cutoff
    // calculate Kaiser-Bessel window shape factor beta from stopband attenuation
    if (Astop < 20.96)
    {
      Beta = 0.0;
    }
    else if (Astop >= 50.0)
    {
      Beta = 0.1102 * (Astop - 8.71);
    }
    else
    {
      Beta = 0.5842 * powf((Astop - 20.96), 0.4) + 0.07886 * (Astop - 20.96);
    }
    // estimate number of taps
    m_NumTaps = (int32_t)((Astop - 8.0) / (2.285 * K_2PI * (normFstop - normFpass)) + 1.0);
    if (m_NumTaps > MAX_NUMCOEF)
    {
      m_NumTaps = MAX_NUMCOEF;
    }
    if (m_NumTaps < 3)
    {
      m_NumTaps = 3;
    }
    float32_t fCenter = 0.5 * (float32_t) (m_NumTaps - 1);
    izb = Izero (Beta);
    for (n = 0; n < m_NumTaps; n++)
    {
      float32_t x = (float32_t) n - fCenter;
      float32_t c;
      // create ideal Sinc() LP filter with normFcut
        if ( abs((float)n - fCenter) < 0.01)
      { // deal with odd size filter singularity where sin(0) / 0 == 1
        c = 2.0 * normFcut;
  //        Serial.println("Hello, here at odd FIR filter size");
      }
      else
      {
        c = (float32_t) sinf(K_2PI * x * normFcut) / (K_PI * x);
  //        c = (float32_t) sinf(K_PI * x * normFcut) / (K_PI * x);
      }
      x = ((float32_t) n - ((float32_t) m_NumTaps - 1.0) / 2.0) / (((float32_t) m_NumTaps - 1.0) / 2.0);
      FIR_Coef[n] = Scale * c * Izero (Beta * sqrt(1 - (x * x) )) / izb;
    }
  } // END calc_lowpass_coeffs
*/
float32_t Izero (float32_t x)
{
  float32_t x2 = x / 2.0;
  float32_t summe = 1.0;
  float32_t ds = 1.0;
  float32_t di = 1.0;
  float32_t errorlimit = 1e-9;
  float32_t tmp;
  do
  {
    tmp = x2 / di;
    tmp *= tmp;
    ds *= tmp;
    summe += ds;
    di += 1.0;
  }   while (ds >= errorlimit * summe);
  return (summe);
}  // END Izero

// set samplerate code by Frank Bösing
void setI2SFreq(int freq) {
#if defined(T4)
  // PLL between 27*24 = 648MHz und 54*24=1296MHz
  int n1 = 4; //SAI prescaler 4 => (n1*n2) = multiple of 4
  int n2 = 1 + (24000000 * 27) / (freq * 256 * n1);
  double C = ((double)freq * 256 * n1 * n2) / 24000000;

  int nfact = C;
  int ndiv = 10000;
  int nmult = C * ndiv - (nfact * ndiv);

  CCM_ANALOG_PLL_AUDIO = 0;
  CCM_ANALOG_PLL_AUDIO |= CCM_ANALOG_PLL_AUDIO_ENABLE
                          | CCM_ANALOG_PLL_AUDIO_POST_DIV_SELECT(2) // 2: 1/4; 1: 1/2; 0: 1/1
                          | CCM_ANALOG_PLL_AUDIO_DIV_SELECT(nfact);

  CCM_ANALOG_PLL_AUDIO_NUM   = nmult & CCM_ANALOG_PLL_AUDIO_NUM_MASK;
  CCM_ANALOG_PLL_AUDIO_DENOM = ndiv & CCM_ANALOG_PLL_AUDIO_DENOM_MASK;
  while (!(CCM_ANALOG_PLL_AUDIO & CCM_ANALOG_PLL_AUDIO_LOCK)) {}; //Wait for pll-lock

  const int div_post_pll = 1; // other values: 2,4
  CCM_ANALOG_MISC2 &= ~(CCM_ANALOG_MISC2_DIV_MSB | CCM_ANALOG_MISC2_DIV_LSB);
  if (div_post_pll > 1) CCM_ANALOG_MISC2 |= CCM_ANALOG_MISC2_DIV_LSB;
  if (div_post_pll > 3) CCM_ANALOG_MISC2 |= CCM_ANALOG_MISC2_DIV_MSB;

  CCM_CSCMR1 = (CCM_CSCMR1 & ~(CCM_CSCMR1_SAI1_CLK_SEL_MASK))
               | CCM_CSCMR1_SAI1_CLK_SEL(2); // &0x03 // (0,1,2): PLL3PFD0, PLL5, PLL4
  CCM_CS1CDR = (CCM_CS1CDR & ~(CCM_CS1CDR_SAI1_CLK_PRED_MASK | CCM_CS1CDR_SAI1_CLK_PODF_MASK))
               | CCM_CS1CDR_SAI1_CLK_PRED(n1 - 1) // &0x07
               | CCM_CS1CDR_SAI1_CLK_PODF(n2 - 1); // &0x3f

  IOMUXC_GPR_GPR1 = (IOMUXC_GPR_GPR1 & ~(IOMUXC_GPR_GPR1_SAI1_MCLK1_SEL_MASK))
                    | (IOMUXC_GPR_GPR1_SAI1_MCLK_DIR | IOMUXC_GPR_GPR1_SAI1_MCLK1_SEL(0));	//Select MCLK

  //Serial.printf("n1:%d n2:%d C:%f c0:%d c1:%d c2:%d\n",  n1, n2, C, nfact, nmult, ndiv);
#else
  typedef struct {
    uint8_t mult;
    uint16_t div;
  } tmclk;

  const int numfreqs = 20;
  //  const int samplefreqs[numfreqs] = { 8000, 11025, 16000, 22050, 32000, 44100, (int)44117.64706 , 48000, 88200, (int)44117.64706 * 2, 96000, 100000, 176400, (int)44117.64706 * 4, 192000};
  const int samplefreqs[numfreqs] = { 8000, 11025, 16000, 22050, 32000, 44100, (int)44117.64706 , 48000, 50223, 88200, (int)44117.64706 * 2,
                                      96000, 100000, 100466, 176400, (int)44117.64706 * 4, 192000, 234375, 281000, 352800
                                    };

#if (F_PLL==180000000)
  //{ 8000, 11025, 16000, 22050, 32000, 44100, (int)44117.64706 , 48000,
  const tmclk clkArr[numfreqs] = {{46, 4043}, {49, 3125}, {73, 3208}, {98, 3125}, {183, 4021}, {196, 3125}, {16, 255}, {128, 1875},
    // 50223, 88200, (int)44117.64706 * 2, 96000, 100466, 176400, (int)44117.64706 * 4, 192000, 234375, 281000, 352800};
    {1, 14}, {107, 853}, {32, 255}, {219, 1604}, {224, 1575}, {1, 7}, {214, 853}, {64, 255}, {219, 802}, {1, 3}, {2, 5} , {1, 2}
  };
#endif


  for (int f = 0; f < numfreqs; f++) {
    if ( freq == samplefreqs[f] ) {
      while (I2S0_MCR & I2S_MCR_DUF) ;
      I2S0_MDR = I2S_MDR_FRACT((clkArr[f].mult - 1)) | I2S_MDR_DIVIDE((clkArr[f].div - 1));
      return;
    }
  }
#endif //Teensy4
#if 1
  Serial.print("Set Samplerate ");
  Serial.print(freq / 1000);
  Serial.println(" kHz");
#endif
} // end set_I2S

void init_filter_mask()
{
  /****************************************************************************************
     Calculate the FFT of the FIR filter coefficients once to produce the FIR filter mask
  ****************************************************************************************/
  // the FIR has exactly m_NumTaps and a maximum of (FFT_length / 2) + 1 taps = coefficients, so we have to add (FFT_length / 2) -1 zeros before the FFT
  // in order to produce a FFT_length point input buffer for the FFT
  // copy coefficients into real values of first part of buffer, rest is zero
#if 1

  for (unsigned i = 0; i < m_NumTaps; i++)
  {
    // try out a window function to eliminate ringing of the filter at the stop frequency
    //             sd.FFT_Samples[i] = (float32_t)((0.53836 - (0.46164 * arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN-1)))) * sd.FFT_Samples[i]);
    /*                   float32_t SINF = (sinf(PI * (float32_t)(i/2.0) / (float32_t)(513.0)));
                      SINF = SINF * SINF;


            FIR_filter_mask[i * 2] = FIR_Coef [i] * SINF;
    */
    FIR_filter_mask[i * 2] = FIR_Coef_I [i];
    FIR_filter_mask[i * 2 + 1] = FIR_Coef_Q [i];
  }

  for (unsigned i = FFT_length + 1; i < FFT_length * 2; i++)
  {
    FIR_filter_mask[i] = 0.0;
  }
#endif
#if 0

  // pass-thru
  // b0=1
  // all others zero

  FIR_filter_mask[0] = 1;
  for (unsigned i = 1; i < FFT_length * 2; i++)
  {
    FIR_filter_mask[i] = 0.0;
  }


#endif
  // FFT of FIR_filter_mask
  // perform FFT (in-place), needs only to be done once (or every time the filter coeffs change)
  arm_cfft_f32(maskS, FIR_filter_mask, 0, 1);

  ///////////////////////////////////////////////////////////////77
  // PASS-THRU only for TESTING
  /////////////////////////////////////////////////////////////77
  /*
    // hmmm, unclear, whether [1,1] or [1,0] or [0,1] are pass-thru filter-mask-multipliers??
    // empirically, [1,0] sounds most natural = pass-thru
    for(i = 0; i < FFT_length * 2; i+=2)
    {
          FIR_filter_mask [i] = 1.0; // real
          FIR_filter_mask [i + 1] = 0.0; // imaginary
    }
  */
} // end init_filter_mask


void Zoom_FFT_prep()
{ // take value of spectrum_zoom and initialize IIR lowpass and FIR decimation filters for the right values

  float32_t Fstop_Zoom = 0.5 * (float32_t) SR[SAMPLE_RATE].rate / (1 << spectrum_zoom);
  //    Serial.print("Fstop =  "); Serial.println(Fstop_Zoom);
  calc_FIR_coeffs (Fir_Zoom_FFT_Decimate_coeffs, 4, Fstop_Zoom, 60, 0, 0.0, (float32_t)SR[SAMPLE_RATE].rate);

  if (spectrum_zoom < 7)
  {
    Fir_Zoom_FFT_Decimate_I.M = (1 << spectrum_zoom);
    Fir_Zoom_FFT_Decimate_Q.M = (1 << spectrum_zoom);
    IIR_biquad_Zoom_FFT_I.pCoeffs = mag_coeffs[spectrum_zoom];
    IIR_biquad_Zoom_FFT_Q.pCoeffs = mag_coeffs[spectrum_zoom];
  }
  else
  { // we have to decimate by 128 for all higher magnifications, arm routine does not allow for higher decimations
    Fir_Zoom_FFT_Decimate_I.M = 128;
    Fir_Zoom_FFT_Decimate_Q.M = 128;
    IIR_biquad_Zoom_FFT_I.pCoeffs = mag_coeffs[7];
    IIR_biquad_Zoom_FFT_Q.pCoeffs = mag_coeffs[7];
  }

  zoom_sample_ptr = 0;
}


void Zoom_FFT_exe (uint32_t blockSize)
{
  /*********************************************************************************************
     see Lyons 2011 for a general description of the ZOOM FFT
   *********************************************************************************************

    Repair of the ZoomFFT, 2018_03_24

    REPAIR PLAN:

    For FFT size  FFT_L = 512
    N_BLOCKS = 16
    BUFFER_SIZE = 128
     2048 samples arriving every round

     Block of samples (each round gives us BUFFER_SIZE * N_BLOCKS)
     Lowpass filter (BUFFER_SIZE * N_BLOCKS)
     Downsample. This produces (BUFFER_SIZE * N_BLOCKS / 1<<magnify) samples
     Put those (BUFFER_SIZE * N_BLOCKS / 1<<magnify) into FFT_ring_buffer
     Copy FFT_ring_buffer into buffer_spec_FFT
     For spectrum_zoom <= 8, the number of samples used for the FFT is larger or equal to 256
     For spectrum_zoom >= 16, the number of new samples used is smaller than 256  put into ringbuffer
     If enough samples (256!), apply window and do 256-point-FFT on buffer_spec_FFT [or do FFT every time!]
     Display spectrum if FFT has been freshly calculated

  ************************************************/

  float32_t x_buffer[2048];
  float32_t y_buffer[2048];
  static float32_t FFT_ring_buffer_x[256];
  static float32_t FFT_ring_buffer_y[256];
  int sample_no = 256;
  // sample_no is 256, in high magnify modes it is smaller!
  // but it must never be > 256

  sample_no = BUFFER_SIZE * N_BLOCKS / (1 << spectrum_zoom);
  if (sample_no > 256)
  {
    sample_no = 256;
  }

  if (spectrum_zoom != SPECTRUM_ZOOM_1)
  {
    if (bands[current_band].mode == DEMOD_WFM)
    {
      arm_biquad_cascade_df1_f32 (&IIR_biquad_Zoom_FFT_I, float_buffer_R, x_buffer, blockSize);
      arm_biquad_cascade_df1_f32 (&IIR_biquad_Zoom_FFT_Q, iFFT_buffer, y_buffer, blockSize);
    }
    else // all modes except wide FM Rx
    {
      // lowpass filter
      arm_biquad_cascade_df1_f32 (&IIR_biquad_Zoom_FFT_I, float_buffer_L, x_buffer, blockSize);
      arm_biquad_cascade_df1_f32 (&IIR_biquad_Zoom_FFT_Q, float_buffer_R, y_buffer, blockSize);
    }
    // decimation
    arm_fir_decimate_f32(&Fir_Zoom_FFT_Decimate_I, x_buffer, x_buffer, blockSize);
    arm_fir_decimate_f32(&Fir_Zoom_FFT_Decimate_Q, y_buffer, y_buffer, blockSize);

    //this puts the sample_no samples into the ringbuffer -->
    // the right order has to be thought about!
    // we take all the samples from zoom_sample_ptr to 256 and
    // then all samples from 0 to zoom_sampl_ptr - 1

    // fill into ringbuffer
    for (int i = 0; i < sample_no; i++)
    { // interleave real and imaginary input values [real, imag, real, imag . . .]
      FFT_ring_buffer_x[zoom_sample_ptr] = x_buffer[i];
      FFT_ring_buffer_y[zoom_sample_ptr] = y_buffer[i];
      zoom_sample_ptr++;
      if (zoom_sample_ptr >= 256) zoom_sample_ptr = 0;
    }
  }
  else  // this is for NON-ZOOM --> will never be executed because that case will be treated in another function
  {

    // put samples into buffer and apply windowing
    for (int i = 0; i < sample_no; i++)
    { // interleave real and imaginary input values [real, imag, real, imag . . .]
      // apply Hann window
      // Nuttall window
      //      buffer_spec_FFT[zoom_sample_ptr] = (0.355768 - (0.487396 * cosf((TPI * (float32_t)i) / (float32_t)255)) +
      //                                          (0.144232 * cosf((FOURPI * (float32_t)i) / (float32_t)255)) - (0.012604 * cosf((SIXPI * (float32_t)i) / (float32_t)255))) * float_buffer_L[i];
      buffer_spec_FFT[zoom_sample_ptr] = float_buffer_L[i] * nuttallWindow256[zoom_sample_ptr * 2];
      //      buffer_spec_FFT[zoom_sample_ptr] = (0.355768 - (0.487396 * cosf((TPI * (float32_t)i) / (float32_t)255)) +
      //                                          (0.144232 * cosf((FOURPI * (float32_t)i) / (float32_t)255)) - (0.012604 * cosf((SIXPI * (float32_t)i) / (float32_t)255))) * float_buffer_R[i];
      buffer_spec_FFT[zoom_sample_ptr] = float_buffer_R[i] * nuttallWindow256[zoom_sample_ptr * 2 + 1];
      zoom_sample_ptr++;

    }
  } // end of NON-ZOOM

  // I have to think about this:
  // when do we want to display a new spectrum?
  // if we wait for zoom_sample_ptr to be 255,
  // it can last more than a few seconds in 4096x Zoom
  // maybe we calculate an FFT and display the spectrum every time this function is called?

  //  if (zoom_sample_ptr >= 255)
  {
    //    zoom_sample_ptr = 0;
    zoom_display = 1;

    // copy from ringbuffer to FFT_buffer
    // in the right order and
    // apply FFT window here
    // Nuttall window
    // zoom_sample_ptr points to the oldest sample now

    float32_t multiplier = (float32_t)spectrum_zoom;
    if (spectrum_zoom > SPECTRUM_ZOOM_8) // && spectrum_zoom < SPECTRUM_ZOOM_1024)
    {
      multiplier = (float32_t)(1 << spectrum_zoom);
    }
    for (int idx = 0; idx < 256; idx++)
    {
      buffer_spec_FFT[idx * 2 + 0] =  multiplier * FFT_ring_buffer_x[zoom_sample_ptr] * nuttallWindow256[idx];
      buffer_spec_FFT[idx * 2 + 1] =  multiplier * FFT_ring_buffer_y[zoom_sample_ptr] * nuttallWindow256[idx];
      zoom_sample_ptr++;
      if (zoom_sample_ptr >= 256) zoom_sample_ptr = 0;
    }


    //***************
    // adjust lowpass filter coefficient, so that
    // "spectrum display smoothness" is the same across the different sample rates
    // and the same across different magnify modes . . .
    float32_t LPFcoeff = LPF_spectrum * (AUDIO_SAMPLE_RATE_EXACT / SR[SAMPLE_RATE].rate);

    //    onem_LPFcoeff *= (float32_t)(1 << spectrum_zoom) / N_BLOCKS / BUFFER_SIZE;
    //    LPFcoeff += onem_LPFcoeff;
    //    if (spectrum_zoom > 10) LPFcoeff = 0.90;
    if (LPFcoeff > 1.0) LPFcoeff = 1.0;
    if (LPFcoeff < 0.001) LPFcoeff = 0.001;
    float32_t onem_LPFcoeff = 1.0 - LPFcoeff;

    //      if(spectrum_zoom >= 7) LPFcoeff = 1.0; // FIXME
    // save old pixels for lowpass filter
    for (int i = 0; i < 256; i++)
    {
      pixelold[i] = pixelnew[i];
    }
    // perform complex FFT
    // calculation is performed in-place the FFT_buffer [re, im, re, im, re, im . . .]
    arm_cfft_f32(spec_FFT, buffer_spec_FFT, 0, 1);
    // calculate mag = I*I + Q*Q,
    // and simultaneously put them into the right order
    if (NR_Kim == 1 || NR_Kim == 2)
    {
      for (int i = 0; i < 128; i++)
      {
        FFT_spec[i * 2] = NR_G[i];
        FFT_spec[i * 2 + 1] = NR_G[i];
      }
    }
    else
    {

      for (int i = 0; i < 128; i++)
      {
        FFT_spec[i + 128] = (buffer_spec_FFT[i * 2] * buffer_spec_FFT[i * 2] + buffer_spec_FFT[i * 2 + 1] * buffer_spec_FFT[i * 2 + 1]);
        FFT_spec[i] = (buffer_spec_FFT[(i + 128) * 2] * buffer_spec_FFT[(i + 128)  * 2] + buffer_spec_FFT[(i + 128)  * 2 + 1] * buffer_spec_FFT[(i + 128)  * 2 + 1]);
      }
    }
    // apply low pass filter and scale the magnitude values and convert to int for spectrum display
    // apply spectrum AGC
    //
    for (int16_t x = 0; x < 256; x++)
    {
      FFT_spec[x] = LPFcoeff * FFT_spec[x] + onem_LPFcoeff * FFT_spec_old[x];
      FFT_spec_old[x] = FFT_spec[x];
    }
    float32_t min_spec = 10000.0;
#ifdef USE_W7PUA
    for (int16_t x = 0; x < 256; x++)
    {
      // pixelnew[x] = DISPLAY_OFFSET_PIXELS + (int16_t)(spectrum_display_scale*10.0*log10f(FFT_spec[x])); <PUA>
#ifdef USE_LOG10FAST
      //      pixelnew[x] = offsetPixels + (int16_t)(displayScale[currentScale].dBScale*log10f_fast(FFT_spec[x]));
      pixelnew[x] = displayScale[currentScale].baseOffset + bands[current_band].pixel_offset + (int16_t)(displayScale[currentScale].dBScale * log10f_fast(FFT_spec[x]));
#else
      //      pixelnew[x] = offsetPixels + (int16_t)(displayScale[currentScale].dBScale*log10f(FFT_spec[x]));
      pixelnew[x] = displayScale[currentScale].baseOffset + bands[current_band].pixel_offset + (int16_t)(displayScale[currentScale].dBScale * log10f(FFT_spec[x]));
#endif
      if (pixelnew[x] > 220)   pixelnew[x] = 220;
    }
#else
    for (int16_t x = 0; x < 256; x++)
    {
#ifdef USE_LOG10FAST
      help = 10.0 * log10f_fast(FFT_spec[x] + 1.0) * spectrum_display_scale;
#else
      help = 10.0 * log10f(FFT_spec[x] + 1.0) * spectrum_display_scale;
#endif
      help = help + display_offset;
      if (help < min_spec) min_spec = help;
      if (help < 1) help = 1.0;
      // insert display offset, AGC etc. here
      pixelnew[x] = (int16_t) (help);
    }
    display_offset -= min_spec * 0.03;
#endif
  }
}

void codec_gain()
{
  static uint32_t timer = 0;
  //      sgtl5000_1.lineInLevel(bands[current_band].RFgain);
  timer ++;
  if (timer > 10000) timer = 10000;
  if (half_clip == 1)      // did clipping almost occur?
  {
    if (timer >= 100)     // has enough time passed since the last gain decrease?
    {
      if (bands[current_band].RFgain != 0)       // yes - is this NOT zero?
      {
        bands[current_band].RFgain -= 1;    // decrease gain one step, 1.5dB
        if (bands[current_band].RFgain < 0)
        {
          bands[current_band].RFgain = 0;
        }
        timer = 0;  // reset the adjustment timer
        AudioNoInterrupts();
        sgtl5000_1.lineInLevel(bands[current_band].RFgain);
        AudioInterrupts();
        if (Menu2 == MENU_RF_GAIN) show_menu();
      }
    }
  }
  else if (quarter_clip == 0)     // no clipping occurred
  {
    if (timer >= 500)       // has it been long enough since the last increase?
    {
      bands[current_band].RFgain += 1;    // increase gain by one step, 1.5dB
      timer = 0;  // reset the timer to prevent this from executing too often
      if (bands[current_band].RFgain > 15)
      {
        bands[current_band].RFgain = 15;
      }
      AudioNoInterrupts();
      sgtl5000_1.lineInLevel(bands[current_band].RFgain);
      AudioInterrupts();
      if (Menu2 == MENU_RF_GAIN) show_menu();
    }
  }
  half_clip = 0;      // clear "half clip" indicator that tells us that we should decrease gain
  quarter_clip = 0;   // clear indicator that, if not triggered, indicates that we can increase gain
}


#ifdef USE_MULTIPLE_FFT_DISPLAY
// calc_256_magn() takes the current input data, in 2048 word chunks, float_buffer_L[] and  _R[],
// uses some 256 word blocks to get the display spectrum.  Originally only one block. now nDisplay blocks. <PUA>
void calc_256_magn()
{
  static uint64_t lastTime, newTime;
  // Get more spectral smoothing with nDisplay
  static uint8_t nDisplay = 8;       // Max 8. Processor load is nDisplay*0.5%
  uint16_t x, jjj, pixelMax;


  // For spectrum search
  static uint16_t printCount = 0;
  static unsigned long long  fr = 277000000ULL; // in 1/100 Hz


  float32_t aaa;
  float32_t spec_help = 0.0;
  // adjust lowpass filter coefficient, so that
  // "spectrum display smoothness" is the same across the different sample rates
  float32_t LPFcoeff = LPF_spectrum * (AUDIO_SAMPLE_RATE_EXACT / SR[SAMPLE_RATE].rate);
  if (LPFcoeff > 1.0) LPFcoeff = 1.0;

  for (int i = 0; i < 256; i++)
  {
    pixelold[i] = pixelnew[i];
  }

  // Do multiple 256 blocks and add powers.  Start by zeroing power sum:
  for (int i = 0; i < 256; i++)
    FFT_spec[i] = 0.0;;

  for (int jjj = 0; jjj < nDisplay; jjj++)
  {
    // put 256 samples into buffer and apply windowing
    for (int i = 0; i < 256; i++)
    { // interleave real and imaginary input values [real, imag, real, imag . . .]
      // apply  window
      // Thanks, Bob for pointing me to the bug! fixed now:
      // Use table of window values
      buffer_spec_FFT[i * 2] =   nuttallWindow256[i] * float_buffer_L[256 * jjj + i];
      buffer_spec_FFT[i * 2 + 1] = nuttallWindow256[i] * float_buffer_R[256 * jjj + i];
    } // End i over all samples

    // Perform complex FFT. Calculation is performed in-place the FFT_buffer [re, im, re, im, re, im . . .]
    // Thus the array given to the fft is 512 floats (256 Complex).  Results are in-place. Prototype:
    // void   arm_cfft_f32 (const arm_cfft_instance_f32 *S, float32_t *p1, uint8_t ifftFlag, uint8_t bitReverseFlag)
    // From cppInitFilters.h:    spec_FFT = &arm_cfft_sR_f32_len256;
    arm_cfft_f32(spec_FFT, buffer_spec_FFT, 0, 1);

    // calculate power spectra and put into FFT_spec

    if (NR_Kim == 1 || NR_Kim == 2)
    {
      for (int i = 0; i < 128; i++)
      {
        FFT_spec[i * 2] = NR_G[i];
        FFT_spec[i * 2 + 1] = NR_G[i];
      }
    }
    else
    {
      for (int i = 0; i < 128; i++)
      {
        // From complex FFT the "negative frequencies" are mirrors of the frequencies above fs/2.  So, we get
        // frequencies from 0 to fs by re-arranging the coefficients.  These are powers (not Volts):                <PUA>
        FFT_spec[i + 128] += buffer_spec_FFT[2 * i] *   buffer_spec_FFT[2 * i] +
                             buffer_spec_FFT[2 * i + 1] * buffer_spec_FFT[2 * i + 1];
        FFT_spec[i] +=     buffer_spec_FFT[2 * (i + 128)] *   buffer_spec_FFT[2 * (i + 128)] +
                           buffer_spec_FFT[2 * (i + 128) + 1] * buffer_spec_FFT[2 * (i + 128) + 1]; // Non-coherent integration
      }
    }
  }   // End of jjj


  //Serial.println(pixelnew[75]);

  for (int x = 0; x < 256; x++)
  {
    FFT_spec_old[x] = FFT_spec[x];    //<< ANYBODY NEED?
#ifdef USE_LOG10FAST
    //       pixelnew[x] = offsetPixels + (int16_t) (displayScale[currentScale].dBScale*log10f_fast(FFT_spec[x]));
    pixelnew[x] = displayScale[currentScale].baseOffset + bands[current_band].pixel_offset + (int16_t) (displayScale[currentScale].dBScale * log10f_fast(FFT_spec[x]));
#else
    //       pixelnew[x] = offsetPixels + (int16_t) (displayScale[currentScale].dBScale*log10f(FFT_spec[x]));
    pixelnew[x] = displayScale[currentScale].baseOffset + bands[current_band].pixel_offset + (int16_t) (displayScale[currentScale].dBScale * log10f(FFT_spec[x]));
#endif

    // Here -6 is about the bottom of the display and +74 is the top.  It can go 10 higher and still display.
    if (pixelnew[x] < -6)
      pixelnew[x] = -6;
  }
} // end calc_256_magn
#else
void calc_256_magn()
{
  float32_t spec_help = 0.0;
  // adjust lowpass filter coefficient, so that
  // "spectrum display smoothness" is the same across the different sample rates
  float32_t LPFcoeff = LPF_spectrum * (AUDIO_SAMPLE_RATE_EXACT / SR[SAMPLE_RATE].rate);
  if (LPFcoeff > 1.0) LPFcoeff = 1.0;

  for (int i = 0; i < 256; i++)
  {
    pixelold[i] = pixelnew[i];
  }

  // put samples into buffer and apply windowing
  for (int i = 0; i < 256; i++)
  { // interleave real and imaginary input values [real, imag, real, imag . . .]
    // apply Hann window
    // cosf is much much faster than arm_cos_f32 !
    //          buffer_spec_FFT[i * 2] = 0.5 * (float32_t)((1 - (cosf(PI*2 * (float32_t)i / (float32_t)(512-1)))) * float_buffer_L[i]);
    //          buffer_spec_FFT[i * 2 + 1] = 0.5 * (float32_t)((1 - (cosf(PI*2 * (float32_t)i / (float32_t)(512-1)))) * float_buffer_R[i]);
    // Nuttall window
    //    buffer_spec_FFT[i * 2] = (0.355768 - (0.487396 * cosf((TPI * (float32_t)i) / (float32_t)255)) +
    //                              (0.144232 * cosf((FOURPI * (float32_t)i) / (float32_t)511)) - (0.012604 * cosf((SIXPI * (float32_t)i) / (float32_t)255))) * float_buffer_L[i];
    //    buffer_spec_FFT[i * 2 + 1] = (0.355768 - (0.487396 * cosf((TPI * (float32_t)i) / (float32_t)255)) +
    //                                  (0.144232 * cosf((FOURPI * (float32_t)i) / (float32_t)511)) - (0.012604 * cosf((SIXPI * (float32_t)i) / (float32_t)255))) * float_buffer_R[i];
    // Thanks, Bob for pointing me to the bug! fixed now:
    buffer_spec_FFT[i * 2] =      float_buffer_L[i] * nuttallWindow256[i];
    buffer_spec_FFT[i * 2 + 1] =  float_buffer_R[i] * nuttallWindow256[i];
  }

  //  Hanning 1.36
  //sc.FFT_Windat[i] = 0.5 * (float32_t)((1 - (arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN2-1)))) * sc.FFT_Samples[i]);
  // Hamming 1.22
  //sc.FFT_Windat[i] = (float32_t)((0.53836 - (0.46164 * arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN2-1)))) * sc.FFT_Samples[i]);
  // Blackman 1.75
  // float32_t help_sample = (0.42659 - (0.49656*arm_cos_f32((2.0*PI*(float32_t)i)/(buff_len-1.0))) + (0.076849*arm_cos_f32((4.0*PI*(float32_t)i)/(buff_len-1.0)))) * sc.FFT_Samples[i];
  // Nuttall
  //             s = (0.355768 - (0.487396*arm_cos_f32((2*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) + (0.144232*arm_cos_f32((4*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) - (0.012604*arm_cos_f32((6*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1))) * sd.FFT_Samples[i];

  // perform complex FFT
  // calculation is performed in-place the FFT_buffer [re, im, re, im, re, im . . .]
  arm_cfft_f32(spec_FFT, buffer_spec_FFT, 0, 1);
  // calculate magnitudes and put into FFT_spec
  // we do not need to calculate magnitudes with square roots, it would seem to be sufficient to
  // calculate mag = I*I + Q*Q, because we are doing a log10-transformation later anyway
  // and simultaneously put them into the right order
  // 38.50%, saves 0.05% of processor power and 1kbyte RAM ;-)

  if (NR_Kim == 1 || NR_Kim == 2)
  {
    for (int i = 0; i < 128; i++)
    {
      FFT_spec[i * 2] = NR_G[i];
      FFT_spec[i * 2 + 1] = NR_G[i];
    }
  }
  else
  {
    for (int i = 0; i < 128; i++)
    {
      FFT_spec[i + 128] = (buffer_spec_FFT[i * 2] * buffer_spec_FFT[i * 2] + buffer_spec_FFT[i * 2 + 1] * buffer_spec_FFT[i * 2 + 1]);
      FFT_spec[i] = (buffer_spec_FFT[(i + 128) * 2] * buffer_spec_FFT[(i + 128)  * 2] + buffer_spec_FFT[(i + 128)  * 2 + 1] * buffer_spec_FFT[(i + 128)  * 2 + 1]);
    }
  }

  // apply low pass filter and scale the magnitude values and convert to int for spectrum display
  for (int16_t x = 0; x < 256; x++)
  {
    spec_help = LPFcoeff * FFT_spec[x] + (1.0 - LPFcoeff) * FFT_spec_old[x];
    FFT_spec_old[x] = spec_help;
    // insert display offset, AGC etc. here
    //    spec_help = 10.0 * log10f(spec_help + 1.0);
    //    pixelnew[x] = (int16_t) (spec_help * spectrum_display_scale);
#ifdef USE_LOG10FAST
    //    pixelnew[x] = offsetPixels + (int16_t) (displayScale[currentScale].dBScale*log10f_fast(spec_help));
    pixelnew[x] = displayScale[currentScale].baseOffset + bands[current_band].pixel_offset + (int16_t) (displayScale[currentScale].dBScale * log10f_fast(spec_help));
#else
    //    pixelnew[x] = offsetPixels + (int16_t) (displayScale[currentScale].dBScale*log10f(spec_help));
    pixelnew[x] = displayScale[currentScale].baseOffset + bands[current_band].pixel_offset + (int16_t) (displayScale[currentScale].dBScale * log10f(spec_help));
#endif

  }
} // end calc_256_magn
#endif
/*
  // leave this here for REFERENCE !
  void calc_spectrum_mags(uint32_t zoom, float32_t LPFcoeff) {
      float32_t help, help2;
      LPFcoeff = LPFcoeff * (AUDIO_SAMPLE_RATE_EXACT / SR[SAMPLE_RATE].rate);
      if(LPFcoeff > 1.0) LPFcoeff = 1.0;
      for(i = 0; i < 256; i++)
      {
          pixelold[i] = pixelnew[i];
      }
      // this saves absolutely NO processor power at 96ksps in comparison to arm_cmplx_mag
  //      for(i = 0; i < FFT_length; i++)
  //      {
  //          FFT_magn[i] = alpha_beta_mag(FFT_buffer[(i * 2)], FFT_buffer[(i*2)+1]);
  //      }

      // this is slower than arm_cmplx_mag_f32 (43.3% vs. 45.5% MCU usage)
  //      for(i = 0; i < FFT_length; i++)
  //      {
  //          FFT_magn[i] = sqrtf(FFT_buffer[(i*2)] * FFT_buffer[(i*2)] + FFT_buffer[(i*2) + 1] * FFT_buffer[(i*2) + 1]);
  //      }

      arm_cmplx_mag_f32(FFT_buffer, FFT_magn, FFT_length);  // calculates sqrt(I*I + Q*Q) for each frequency bin of the FFT

      ////////////////////////////////////////////////////////////////////////
      // now calculate average of the bin values according to spectrum zoom
      // this assumes FFT_shift of 1024, ie. 5515Hz with an FFT_length of 4096
      ////////////////////////////////////////////////////////////////////////

      // TODO: adapt to different FFT lengths
      for(i = 0; i < FFT_length / 2; i+=zoom)
      {
              arm_mean_f32(&FFT_magn[i], zoom, &FFT_spec[i / zoom + 128]);
      }
      for(i = FFT_length / 2; i < FFT_length; i+=zoom)
      {
              arm_mean_f32(&FFT_magn[i], zoom, &FFT_spec[i / zoom - 128]);
      }

      // low pass filtering of the spectrum pixels to smooth/slow down spectrum in the time domain
      // ynew = LPCoeff * x + (1-LPCoeff) * yprevious;
      for(int16_t x = 0; x < 256; x++)
      {
           help = LPFcoeff * FFT_spec[x] + (1.0 - LPFcoeff) * FFT_spec_old[x];
           FFT_spec_old[x] = help;
              // insert display offset, AGC etc. here
              // a single log10 here needs another 22% of processor use on a Teensy 3.6 (96k sample rate)!
  //           help = 50.0 * log10(help+1.0);
             // sqrtf is a little bit faster than arm_sqrt_f32 !
             help2 = sqrtf(help);
  //           arm_sqrt_f32(help, &help2);
           pixelnew[x] = (int16_t) (help2 * 10);
      }

  } // end calc_spectrum_mags
*/


#ifdef USE_W7PUA
// Spectrum display when zoom is being used??  <PUA>
void show_spectrum()
{
  // For Reference
  //   spectrum_y = 120;       // Upper edge
  //   spectrum_x = 10;
  //   spectrum_height = 90;        // Lower edge 210
  //   spectrum_pos_centre_f = 64;
  //   pos_x_smeter = 11; //5
  //   pos_y_smeter = (spectrum_y - 12);    //  94
  //   s_w = 10;
  //=========================
  float wtf;
  int16_t y_old, y_new, y1_new, y1_old;
  int16_t y1_old_minus = 0;
  int16_t y1_new_minus = 0;
  static uint16_t p = WATERFALL_TOP, cnt = WATERFALL_BOTTOM;
  static uint8_t first_time_full = 0;
  static int WF_cnt = 0;
  leave_WFM++;
  if (leave_WFM == 2)
  {
    // clear spectrum display
    tft.fillRect(0, spectrum_y + 4, 320, 240 - spectrum_y + 4, ILI9341_BLACK);
    prepare_spectrum_display();
    show_bandwidth();
  }
  if (leave_WFM == 1000) leave_WFM = 1000;


  // Spectrum area top is spectrum_y=120, bottom is 210
  // Vertical freq markers run 215 to 225
  // Horizontal is 5 to 5+255
  // First cut at a plot grid  <PUA>
  int h = spectrum_height + 3;
  tft.drawFastHLine (spectrum_x - 1, spectrum_y - 2, 254, ILI9341_MAROON);
  tft.drawFastHLine (spectrum_x - 1, spectrum_y + 18, 254, ILI9341_MAROON);
  tft.drawFastHLine (spectrum_x - 1, spectrum_y + 38, 254, ILI9341_MAROON);
  tft.drawFastHLine (spectrum_x - 1, spectrum_y + 58, 254, ILI9341_MAROON);
#ifndef USE_WATERFALL
  tft.drawFastHLine (spectrum_x - 1, spectrum_y + 78, 254, ILI9341_MAROON);
  tft.drawFastHLine (spectrum_x - 1, spectrum_y + h, 254, ILI9341_MAROON);
#endif
  tft.drawFastVLine (spectrum_x - 1,   spectrum_y, h, ILI9341_MAROON);
  if (spectrum_zoom != SPECTRUM_ZOOM_1)                 //  Change the color for the center frequency
  {
    tft.drawFastVLine (spectrum_x + 63,  spectrum_y, h, ILI9341_MAROON);
    tft.drawFastVLine (spectrum_x + 127, spectrum_y, h, ILI9341_GREEN);
  }
  else
  {
    tft.drawFastVLine (spectrum_x + 63,  spectrum_y, h, ILI9341_GREEN);
    tft.drawFastVLine (spectrum_x + 127, spectrum_y, h, ILI9341_MAROON);
  }
  tft.drawFastVLine (spectrum_x + 191, spectrum_y, h, ILI9341_MAROON);
  tft.drawFastVLine (spectrum_x + 255, spectrum_y, h, ILI9341_MAROON);


  // ScrollAreaDefinition(uint16_t TopFixedArea, uint16_t VerticalScrollingArea, uint16_t BottomFixedArea)
  // summ must be 320
  //   tft.ScrollAreaDefinition(WATERFALL_TOP, WATERFALL_BOTTOM - WATERFALL_TOP,20);
  //     tft.ScrollAreaDefinition(100, 200, 20);

  // Draw spectrum display
  for (int16_t x = 0; x < 254; x++)
  {
    if ((x > 1) && (x < 255))
    {
      if (spectrum_mov_average)
      {
        // moving window - weighted average of 5 points of the spectrum to smooth spectrum in the frequency domain
        // weights:  x: 50% , x-1/x+1: 36%, x+2/x-2: 14%
        y_new = pixelnew[x] * 0.5 + pixelnew[x - 1] * 0.18 + pixelnew[x + 1] * 0.18 + pixelnew[x - 2] * 0.07 + pixelnew[x + 2] * 0.07;
        y_old = pixelold[x] * 0.5 + pixelold[x - 1] * 0.18 + pixelold[x + 1] * 0.18 + pixelold[x - 2] * 0.07 + pixelold[x + 2] * 0.07;
      }
      else   // not spectrum_mov_average
      {
        y_new = pixelnew[x];
        y_old = pixelold[x];
      }
    }
    else    // x at edges, i.e., x not between 2 and 254
    {
      y_new = pixelnew[x];
      y_old = pixelold[x];
    }
    //    if (y_old > (spectrum_height - 7))
    //       y_old = (spectrum_height - 7);
    //    if (y_new > (spectrum_height - 7))
    //       y_new = (spectrum_height - 7);
    if (y_old > (spectrum_height - 1))
      y_old = (spectrum_height - 1);
    if (y_new > (spectrum_height - 1))
      y_new = (spectrum_height - 1);
    if (y_old < 0)
      y_old = 0;
    if (y_new < 0)
      y_new = 0;
    y1_old  = (spectrum_y + spectrum_height - 1) - y_old;
    y1_new  = (spectrum_y + spectrum_height - 1) - y_new;

    if (x == 0)
    {
      y1_old_minus = y1_old;
      y1_new_minus = y1_new;
    }
    if (x == 254)
    {
      y1_old_minus = y1_old;
      y1_new_minus = y1_new;
    }
    // set colours for margin of notch box or notch box
    if (notches_on[0])
    {
      if (x == (notch_pixel_L[0] - spectrum_x - 1) || x == (notch_pixel_R[0] - spectrum_x) ) // margin
        SPECTRUM_DELETE_COLOUR = ILI9341_MAROON;
      else if (x > (notch_pixel_L[0] - spectrum_x) && x < (notch_pixel_R[0] - spectrum_x)) // inside notch box
        SPECTRUM_DELETE_COLOUR = ILI9341_NAVY;
      else // outside notch box
        SPECTRUM_DELETE_COLOUR = ILI9341_BLACK;
    }



    // DELETE OLD LINE/POINT
    if (y1_old - y1_old_minus > 1)
    { // plot line upwards
      tft.drawFastVLine(x + spectrum_x, y1_old_minus + 1, y1_old - y1_old_minus, SPECTRUM_DELETE_COLOUR);
    }
    else if (y1_old - y1_old_minus < -1)
    { // plot line downwards
      tft.drawFastVLine(x + spectrum_x, y1_old, y1_old_minus - y1_old, SPECTRUM_DELETE_COLOUR);
    }
    else
    {
      tft.drawPixel(x + spectrum_x, y1_old, SPECTRUM_DELETE_COLOUR); // delete old pixel
    }

    // DRAW NEW LINE/POINT
    if (y1_new - y1_new_minus > 1)
    { // plot line upwards
      tft.drawFastVLine(x + spectrum_x, y1_new_minus + 1, y1_new - y1_new_minus, SPECTRUM_DRAW_COLOUR);
    }
    else if (y1_new - y1_new_minus < -1)
    { // plot line downwards
      tft.drawFastVLine(x + spectrum_x, y1_new, y1_new_minus - y1_new, SPECTRUM_DRAW_COLOUR);
    }
    else
    {
      tft.drawPixel(x + spectrum_x, y1_new, SPECTRUM_DRAW_COLOUR); // write new pixel
    }

    y1_new_minus = y1_new;
    y1_old_minus = y1_old;


    // WATERFALL
    //wtf = abs(avg);
    //uint8_t value = map(20*log10f(wtf), 0,116 , 1, 117);
    //wtf = gradient[y_new];
    //tft.drawPixel(x + spectrum_x, WATERFALL_BOTTOM, wtf);
    //waterfall[WF_cnt][x] = y_new;

  } // End for(...) Draw 254 spectral points

  /*
    if (cnt ==  WATERFALL_TOP)
     cnt = WATERFALL_BOTTOM;

    if( first_time_full)
    {   //Serial.println("setScroll");
       tft.setScroll(cnt--);
       if (p == WATERFALL_TOP)
       {
           p = WATERFALL_BOTTOM;
       }
       p = p - 1;
    }
    else
    {
       p = p + 1;
    }
    if (p == WATERFALL_BOTTOM - 1)
    {
       //Serial.println ("first_time_full!");
       first_time_full = 1;
    } */

  /*
    // Waterfall without hardware scrolling (because TFT does not allow horizontal scrolling)
    for(int i = 0; i < 40; i++)

    {
        for (int j = 0; j < 254; j++)
        {
          wtf = gradient[waterfall[i][j]];
          tft.drawPixel(j + spectrum_x, i + WATERFALL_TOP, wtf);
        }

    }

    WF_cnt++;
    if(WF_cnt > 40) WF_cnt = 0;
  */
  //  showSpectrumCorners();
} // End show_spectrum()
#else
void show_spectrum()
{
  int16_t y_old, y_new, y1_new, y1_old;
  int16_t y1_old_minus = 0;
  int16_t y1_new_minus = 0;
  leave_WFM++;
  if (leave_WFM == 2)
  {
    // clear spectrum display
    tft.fillRect(0, spectrum_y + 4, 320, 240 - spectrum_y + 4, ILI9341_BLACK);
    prepare_spectrum_display();
    show_bandwidth();
  }
  if (leave_WFM == 1000) leave_WFM = 1000;



#if 0
  // Spectrum area top is spectrum_y=120, bottom is 210
  // Vertical freq markers run 215 to 225
  // Horizontal is 5 to 5+255
  // First cut at a plot grid:
  tft.drawFastHLine (spectrum_x, 135, 254, ILI9341_ORANGE);
  tft.drawFastHLine (spectrum_x, 155, 254, ILI9341_ORANGE);
  tft.drawFastHLine (spectrum_x, 175, 254, ILI9341_ORANGE);
  tft.drawFastHLine (spectrum_x, 195, 254, ILI9341_ORANGE);
  tft.drawFastHLine (spectrum_x, 215, 254, ILI9341_ORANGE);

  tft.drawFastVLine (spectrum_x - 1,   135, 80, ILI9341_ORANGE);
  tft.drawFastVLine (spectrum_x + 63,  135, 80, ILI9341_ORANGE);
  tft.drawFastVLine (spectrum_x + 127, 135, 80, ILI9341_ORANGE);
  tft.drawFastVLine (spectrum_x + 191, 135, 80, ILI9341_ORANGE);
  tft.drawFastVLine (spectrum_x + 255, 135, 80, ILI9341_ORANGE);
#endif
  // Draw spectrum display


  for (int16_t x = 0; x < 254; x++)
  {
    if ((x > 1) && (x < 255))
      // moving window - weighted average of 5 points of the spectrum to smooth spectrum in the frequency domain
      // weights:  x: 50% , x-1/x+1: 36%, x+2/x-2: 14%
    {
      if (spectrum_mov_average)
      {
        y_new = pixelnew[x] * 0.5 + pixelnew[x - 1] * 0.18 + pixelnew[x + 1] * 0.18 + pixelnew[x - 2] * 0.07 + pixelnew[x + 2] * 0.07;
        y_old = pixelold[x] * 0.5 + pixelold[x - 1] * 0.18 + pixelold[x + 1] * 0.18 + pixelold[x - 2] * 0.07 + pixelold[x + 2] * 0.07;
      }
      else
      {
        y_new = pixelnew[x];
        y_old = pixelold[x];
      }
    }
    else
    {
      y_new = pixelnew[x];
      y_old = pixelold[x];
    }
    if (y_old > (spectrum_height - 7))
    {
      y_old = (spectrum_height - 7);
    }

    if (y_new > (spectrum_height - 7))
    {
      y_new = (spectrum_height - 7);
    }
    y1_old  = (spectrum_y + spectrum_height - 1) - y_old;
    y1_new  = (spectrum_y + spectrum_height - 1) - y_new;

    if (x == 0)
    {
      y1_old_minus = y1_old;
      y1_new_minus = y1_new;
    }
    if (x == 254)
    {
      y1_old_minus = y1_old;
      y1_new_minus = y1_new;
    }
    // set colours for margin of notch box or notch box
    if (notches_on[0])
    {
      if (x == (notch_pixel_L[0] - spectrum_x - 1) || x == (notch_pixel_R[0] - spectrum_x) ) // margin
      {
        SPECTRUM_DELETE_COLOUR = ILI9341_MAROON;
      }
      else if (x > (notch_pixel_L[0] - spectrum_x) && x < (notch_pixel_R[0] - spectrum_x)) // inside notch box
      {
        SPECTRUM_DELETE_COLOUR = ILI9341_NAVY;
      }
      else // outside notch box
      {
        SPECTRUM_DELETE_COLOUR = ILI9341_BLACK;
      }
    }


    {
      // DELETE OLD LINE/POINT
      if (y1_old - y1_old_minus > 1)
      { // plot line upwards
        tft.drawFastVLine(x + spectrum_x, y1_old_minus + 1, y1_old - y1_old_minus, SPECTRUM_DELETE_COLOUR);
      }
      else if (y1_old - y1_old_minus < -1)
      { // plot line downwards
        tft.drawFastVLine(x + spectrum_x, y1_old, y1_old_minus - y1_old, SPECTRUM_DELETE_COLOUR);
      }
      else
      {
        tft.drawPixel(x + spectrum_x, y1_old, SPECTRUM_DELETE_COLOUR); // delete old pixel
      }

      // DRAW NEW LINE/POINT
      if (y1_new - y1_new_minus > 1)
      { // plot line upwards
        tft.drawFastVLine(x + spectrum_x, y1_new_minus + 1, y1_new - y1_new_minus, SPECTRUM_DRAW_COLOUR);
      }
      else if (y1_new - y1_new_minus < -1)
      { // plot line downwards
        tft.drawFastVLine(x + spectrum_x, y1_new, y1_new_minus - y1_new, SPECTRUM_DRAW_COLOUR);
      }
      else
      {
        tft.drawPixel(x + spectrum_x, y1_new, SPECTRUM_DRAW_COLOUR); // write new pixel
      }

      y1_new_minus = y1_new;
      y1_old_minus = y1_old;

    }
  } // end for loop

} // END show_spectrum
#endif

void show_bandwidth ()
{
  //  AudioNoInterrupts();
  // M = demod_mode, FU & FL upper & lower frequency
  // this routine prints the frequency bars under the spectrum display
  // and displays the bandwidth bar indicating demodulation bandwidth
  if (spectrum_zoom != SPECTRUM_ZOOM_1) spectrum_pos_centre_f = 128;
  else spectrum_pos_centre_f = 64;
  float32_t pixel_per_khz = (1 << spectrum_zoom) * 256.0 / SR[SAMPLE_RATE].rate * 1000.0;
  int len = (int)((bands[current_band].FHiCut - bands[current_band].FLoCut) / 1000.0 * pixel_per_khz);
  int pos_left = (int)(bands[current_band].FLoCut / 1000.0 * pixel_per_khz) + spectrum_pos_centre_f + spectrum_x;

  if (bands[current_band].mode == DEMOD_SAM_USB)
  {
    len = (int)(bands[current_band].FHiCut / 1000.0 * pixel_per_khz);
    pos_left = spectrum_pos_centre_f + spectrum_x;
  }
  else if (bands[current_band].mode == DEMOD_SAM_LSB)
  {
    len = - (int)(bands[current_band].FLoCut / 1000.0 * pixel_per_khz);
    pos_left = spectrum_pos_centre_f + spectrum_x - len;
  }

  if (pos_left < spectrum_x) pos_left = spectrum_x;
  if (len > (256 - pos_left + spectrum_x)) len = 256 - pos_left + spectrum_x;

  //  int pos_y = spectrum_y + spectrum_height+2;        // + 2;     // 212      Move down below base line?  <PUA>
#ifdef USE_W7PUA
  int pos_y = BW_indicator_y;
#else
  int pos_y = spectrum_y + spectrum_height + 2;
#endif

  tft.drawFastHLine(spectrum_x - 1, pos_y, 258, ILI9341_BLACK); // erase old indicator
  tft.drawFastHLine(spectrum_x - 1, pos_y + 1, 258, ILI9341_BLACK); // erase old indicator
  tft.drawFastHLine(spectrum_x - 1, pos_y + 2, 258, ILI9341_BLACK); // erase old indicator
  tft.drawFastHLine(spectrum_x - 1, pos_y + 3, 258, ILI9341_BLACK); // erase old indicator

  tft.drawFastHLine(pos_left, pos_y + 0, len, ILI9341_YELLOW);
  tft.drawFastHLine(pos_left, pos_y + 1, len, ILI9341_YELLOW);
  tft.drawFastHLine(pos_left, pos_y + 2, len, ILI9341_YELLOW);
  tft.drawFastHLine(pos_left, pos_y + 3, len, ILI9341_YELLOW);

#if 0
  if (leL > spectrum_pos_centre_f + 1) leL = spectrum_pos_centre_f + 2;
  if ((leU + spectrum_pos_centre_f) > 255) leU = 256 - spectrum_pos_centre_f;

  // draw upper sideband indicator
  tft.drawFastHLine(spectrum_pos_centre_f + spectrum_x + 1, pos_y, leU, ILI9341_YELLOW);
  tft.drawFastHLine(spectrum_pos_centre_f + spectrum_x + 1, pos_y + 1, leU, ILI9341_YELLOW);
  tft.drawFastHLine(spectrum_pos_centre_f + spectrum_x + 1, pos_y + 2, leU, ILI9341_YELLOW);
  tft.drawFastHLine(spectrum_pos_centre_f + spectrum_x + 1, pos_y + 3, leU, ILI9341_YELLOW);
  // draw lower sideband indicator
  tft.drawFastHLine(spectrum_pos_centre_f  + spectrum_x - leL + 1, pos_y, leL + 1, ILI9341_YELLOW);
  tft.drawFastHLine(spectrum_pos_centre_f  + spectrum_x - leL + 1, pos_y + 1, leL + 1, ILI9341_YELLOW);
  tft.drawFastHLine(spectrum_pos_centre_f  + spectrum_x - leL + 1, pos_y + 2, leL + 1, ILI9341_YELLOW);
  tft.drawFastHLine(spectrum_pos_centre_f  + spectrum_x - leL + 1, pos_y + 3, leL + 1, ILI9341_YELLOW);
#endif

  //print bandwidth !
  tft.fillRect(10, 24, 240, 21, ILI9341_BLACK);
  tft.setCursor(10, 25);
  tft.setFont(Arial_9);
  tft.setTextColor(ILI9341_WHITE);
  tft.print(DEMOD[bands[current_band].mode].text);
  tft.setCursor(70, 25);
  tft.printf("%02.1f kHz", (bands[current_band].mode != DEMOD_SAM_USB) ? (float)(bands[current_band].FLoCut / 1000.0f) : 0.0f);
  tft.setCursor(130, 25);
  tft.printf("%02.1f kHz", (bands[current_band].mode != DEMOD_SAM_LSB) ? (float)(float)(bands[current_band].FHiCut / 1000.0f) : 0.0f);
  tft.setCursor(180, 25);
  //tft.print("   SR: ");
  tft.print("  ");
  tft.print(SR[SAMPLE_RATE].text);
  show_tunestep();
  tft.setTextColor(ILI9341_WHITE); // set text color to white for other print routines not to get confused ;-)
  //  AudioInterrupts();
}  // end show_bandwidth

void prepare_spectrum_display()
{
  uint16_t base_y = spectrum_y + spectrum_WF_height + 4;
  //    uint16_t b_x = spectrum_x + SR[SAMPLE_RATE].x_offset;
  //    float32_t x_f = SR[SAMPLE_RATE].x_factor;

  tft.fillRect(0, base_y, 320, 240 - base_y, ILI9341_BLACK);
  //  tft.drawRect(spectrum_x - 2, spectrum_y + 2, 257, spectrum_height, ILI9341_MAROON);
  // receive freq indicator line
  //    tft.drawFastVLine(spectrum_x + spectrum_pos_centre_f, spectrum_y + spectrum_height - 18, 20, ILI9341_GREEN);
  /*
      // vertical lines
      tft.drawFastVLine(b_x - 4, base_y + 1, 10, ILI9341_YELLOW);
      tft.drawFastVLine(b_x - 3, base_y + 1, 10, ILI9341_YELLOW);
      tft.drawFastVLine( x_f * 2.0 + b_x,  base_y + 1, 10, ILI9341_YELLOW);
      tft.drawFastVLine( x_f * 2.0 + 1.0 + b_x,  base_y + 1, 10, ILI9341_YELLOW);
      if(x_f * 3.0 + b_x < 256+b_x) {
      tft.drawFastVLine( x_f * 3.0 + b_x,  base_y + 1, 10, ILI9341_YELLOW);
      tft.drawFastVLine( x_f * 3.0 + 1.0 + b_x,  base_y + 1, 10, ILI9341_YELLOW);
      }
      if(x_f * 4.0 + b_x < 256+b_x) {
      tft.drawFastVLine( x_f * 4.0 + b_x,  base_y + 1, 10, ILI9341_YELLOW);
      tft.drawFastVLine( x_f * 4.0 + 1.0 + b_x,  base_y + 1, 10, ILI9341_YELLOW);
      }
      if(x_f * 5.0 + b_x < 256+b_x) {
      tft.drawFastVLine( x_f * 5.0 + b_x,  base_y + 1, 10, ILI9341_YELLOW);
      tft.drawFastVLine( x_f * 5.0 + 1.0 + b_x,  base_y + 1, 10, ILI9341_YELLOW);
      }
      tft.drawFastVLine( x_f * 0.5 + b_x,  base_y + 1, 6, ILI9341_YELLOW);
      tft.drawFastVLine( x_f * 1.5 + b_x,  base_y + 1, 6, ILI9341_YELLOW);
      tft.drawFastVLine( x_f * 2.5 + b_x,  base_y + 1, 6, ILI9341_YELLOW);
      if(x_f * 3.5 + b_x < 256+b_x) {
      tft.drawFastVLine( x_f * 3.5 + b_x,  base_y + 1, 6, ILI9341_YELLOW);
      }
      if(x_f * 4.5 + b_x < 256+b_x) {
          tft.drawFastVLine( x_f * 4.5 + b_x,  base_y + 1, 6, ILI9341_YELLOW);
      }
      // text
      tft.setTextColor(ILI9341_WHITE);
      tft.setFont(Arial_8);
      int text_y_offset = 15;
      int text_x_offset = - 5;
      // zero
      tft.setCursor (b_x + text_x_offset - 1, base_y + text_y_offset);
      tft.print(SR[SAMPLE_RATE].f1);
      tft.setCursor (b_x + x_f * 2 + text_x_offset, base_y + text_y_offset);
      tft.print(SR[SAMPLE_RATE].f2);
      tft.setCursor (b_x + x_f *3 + text_x_offset, base_y + text_y_offset);
      tft.print(SR[SAMPLE_RATE].f3);
      tft.setCursor (b_x + x_f *4 + text_x_offset, base_y + text_y_offset);
      tft.print(SR[SAMPLE_RATE].f4);
    //    tft.setCursor (b_x + text_x_offset + 256, base_y + text_y_offset);
      tft.print(" kHz");
      tft.setFont(Arial_10);
  */
  // draw S-Meter layout
  tft.drawFastHLine (pos_x_smeter, pos_y_smeter - 1, 9 * s_w, ILI9341_WHITE);
  tft.drawFastHLine (pos_x_smeter, pos_y_smeter + 6, 9 * s_w, ILI9341_WHITE);
  tft.fillRect(pos_x_smeter, pos_y_smeter - 3, 2, 2, ILI9341_WHITE);
  tft.fillRect(pos_x_smeter + 8 * s_w, pos_y_smeter - 3, 2, 2, ILI9341_WHITE);
  tft.fillRect(pos_x_smeter + 2 * s_w, pos_y_smeter - 3, 2, 2, ILI9341_WHITE);
  tft.fillRect(pos_x_smeter + 4 * s_w, pos_y_smeter - 3, 2, 2, ILI9341_WHITE);
  tft.fillRect(pos_x_smeter + 6 * s_w, pos_y_smeter - 3, 2, 2, ILI9341_WHITE);
  tft.fillRect(pos_x_smeter + 7 * s_w, pos_y_smeter - 4, 2, 3, ILI9341_WHITE);
  tft.fillRect(pos_x_smeter + 3 * s_w, pos_y_smeter - 4, 2, 3, ILI9341_WHITE);
  tft.fillRect(pos_x_smeter + 5 * s_w, pos_y_smeter - 4, 2, 3, ILI9341_WHITE);
  tft.fillRect(pos_x_smeter + s_w, pos_y_smeter - 4, 2, 3, ILI9341_WHITE);
  tft.fillRect(pos_x_smeter + 9 * s_w, pos_y_smeter - 4, 2, 3, ILI9341_WHITE);
  tft.drawFastHLine (pos_x_smeter + 9 * s_w, pos_y_smeter - 1, 3 * s_w * 2 + 2, ILI9341_GREEN);

  tft.drawFastHLine (pos_x_smeter + 9 * s_w, pos_y_smeter + 6, 3 * s_w * 2 + 2, ILI9341_GREEN);
  tft.fillRect(pos_x_smeter + 11 * s_w, pos_y_smeter - 4, 2, 3, ILI9341_GREEN);
  tft.fillRect(pos_x_smeter + 13 * s_w, pos_y_smeter - 4, 2, 3, ILI9341_GREEN);
  tft.fillRect(pos_x_smeter + 15 * s_w, pos_y_smeter - 4, 2, 3, ILI9341_GREEN);

  tft.drawFastVLine (pos_x_smeter - 1, pos_y_smeter - 1, 8, ILI9341_WHITE);
  tft.drawFastVLine (pos_x_smeter + 15 * s_w + 2, pos_y_smeter - 1, 8, ILI9341_GREEN);

  tft.setCursor(pos_x_smeter - 1, pos_y_smeter - 15);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_8);
  tft.print("S 1");
  tft.setCursor(pos_x_smeter + 28, pos_y_smeter - 15);
  tft.print("3");
  tft.setCursor(pos_x_smeter + 48, pos_y_smeter - 15);
  tft.print("5");
  tft.setCursor(pos_x_smeter + 68, pos_y_smeter - 15);
  tft.print("7");
  tft.setCursor(pos_x_smeter + 88, pos_y_smeter - 15);
  tft.print("9");
  tft.setCursor(pos_x_smeter + 120, pos_y_smeter - 15);
  tft.print("+20dB");
  FrequencyBarText();
  show_menu();
  show_notch((int)notches[0], bands[current_band].mode);
  showSpectrumCorners();

} // END prepare_spectrum_display

void showSpectrumCorners(void)
{
  tft.fillRect(12, spectrum_y + 3, 33, 10, ILI9341_BLACK);
  tft.setCursor(12, spectrum_y + 3);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_8);
  tft.print(displayScale[currentScale].dbText);

  tft.fillRect(240, spectrum_y + 3, 20, 10, ILI9341_BLACK);
  tft.setCursor(240, spectrum_y + 3);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_8);
  //tft.print(display_offset);
//  tft.printf("%4.1f", offsetDisplayDB);
  tft.printf("%4d", bands[current_band].pixel_offset);
}

void FrequencyBarText()
{ // This function draws the frequency bar at the bottom of the spectrum scope, putting markers at every graticule and the full frequency
  // (rounded to the nearest kHz) in the "center".  (by KA7OEI, 20140913) modified from the mcHF source code
  float   freq_calc;
  ulong   i;
  char    txt[16], *c;
  float   grat;
  int centerIdx;
  const int pos_grat_y = 20;
  grat = (float)(SR[SAMPLE_RATE].rate / 8000.0) / (float)(1 << spectrum_zoom); // 1, 2, 4, 8, 16, 32, 64 . . . 4096

  /*    if(spectrum_zoom == SPECTRUM_SUPER_ZOOM)
      {
          grat = (float)((SR[SAMPLE_RATE].rate / 8000.0) / 4096); //
      } */

  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_8);
  // clear print area for frequency text
  //    tft.fillRect(0, spectrum_y + spectrum_height + pos_grat_y, 320, 8, ILI9341_BLACK);
  tft.fillRect(0, spectrum_y + spectrum_WF_height + 5, 320, 240 - spectrum_y - spectrum_height - 5, ILI9341_BLACK);

  freq_calc = (float)(bands[current_band].freq / SI5351_FREQ_MULT);      // get current frequency in Hz
  if (bands[current_band].mode == DEMOD_WFM)
  { // undersampling mode with 3x undersampling
    // grat *= 5.0;
    freq_calc = 3.0 * freq_calc + 0.75 * SR[SAMPLE_RATE].rate;
  }

  if (spectrum_zoom == 0)        //
  {
    freq_calc += (float32_t)SR[SAMPLE_RATE].rate / 4.0;
  }

  if (spectrum_zoom < 3)
  {
    freq_calc = roundf(freq_calc / 1000); // round graticule frequency to the nearest kHz
  }
  else if (spectrum_zoom < 5)
  {
    freq_calc = roundf(freq_calc / 100) / 10; // round graticule frequency to the nearest 100Hz
  }
  else if (spectrum_zoom == 5) // 32x
  {
    freq_calc = roundf(freq_calc / 50) / 20; // round graticule frequency to the nearest 50Hz
  }
  else if (spectrum_zoom < 8) //
  {
    freq_calc = roundf(freq_calc / 10) / 100 ; // round graticule frequency to the nearest 10Hz
  }
  else
  {
    freq_calc = roundf(freq_calc) / 1000; // round graticule frequency to the nearest 1Hz
  }

  if (spectrum_zoom != 0)     centerIdx = 0;
  else centerIdx = -2;

  {
    // remainder of frequency/graticule markings
    //        const static int idx2pos[2][9] = {{0,26,58,90,122,154,186,218, 242},{0,26,58,90,122,154,186,209, 229} };
    // positions for graticules: first for spectrum_zoom < 3, then for spectrum_zoom > 2
    const static int idx2pos[2][9] = {{ -10, 21, 52, 83, 123, 151, 186, 218, 252}, { -10, 21, 50, 86, 123, 154, 178, 218, 252} };
    //        const static int centerIdx2pos[] = {62,94,130,160,192};
    const static int centerIdx2pos[] = {62, 94, 140, 160, 192};

    /**************************************************************************************************
       CENTER FREQUENCY PRINT
     **************************************************************************************************/
    if (spectrum_zoom < 3)
    {
      snprintf(txt, 16, "  %lu  ", (ulong)(freq_calc + (centerIdx * grat))); // build string for center frequency precision 1khz
    }
    else
    {
      float disp_freq = freq_calc + (centerIdx * grat);
      int bignum = (int)disp_freq;
      if (spectrum_zoom < 8)
      {
        int smallnum = (int)roundf((disp_freq - bignum) * 100);
        snprintf(txt, 16, "  %u.%02u  ", bignum, smallnum); // build string for center frequency precision 100Hz/10Hz/1Hz
      }
      else
      {
        int smallnum = (int)roundf((disp_freq - bignum) * 1000);
        snprintf(txt, 16, "  %u.%03u  ", bignum, smallnum); // build string for center frequency precision 100Hz/10Hz/1Hz
      }
    }

    i = centerIdx2pos[centerIdx + 2] - ((strlen(txt) - 4) * 4); // calculate position of center frequency text

    tft.setCursor(spectrum_x + i, spectrum_y + spectrum_WF_height + pos_grat_y);
    tft.print(txt);

    /**************************************************************************************************
       PRINT ALL OTHER FREQUENCIES (NON-CENTER)
     **************************************************************************************************/

    for (int idx = -4; idx < 5; idx++)
    {
      int pos_help = idx2pos[spectrum_zoom < 3 ? 0 : 1][idx + 4];
      if (idx != centerIdx)
      {
        if (spectrum_zoom < 3)
        {
          snprintf(txt, 16, " %lu ", (ulong)(freq_calc + (idx * grat))); // build string for middle-left frequency (1khz precision)
          c = &txt[strlen(txt) - 3]; // point at 2nd character from the end
        }
        else
        {
          float disp_freq = freq_calc + (idx * grat);
          int bignum = (int)disp_freq;
          if (spectrum_zoom < 8)
          {
            int smallnum = (int)roundf((disp_freq - bignum) * 100);
            snprintf(txt, 16, "  %u.%02u  ", bignum, smallnum); // build string for center frequency precision 100Hz/10Hz/1Hz
          }
          else
          {
            int smallnum = (int)roundf((disp_freq - bignum) * 1000);
            snprintf(txt, 16, "  %u.%03u  ", bignum, smallnum); // build string for center frequency precision 100Hz/10Hz/1Hz
          }
          c = &txt[strlen(txt) - 5]; // point at 5th character from the end
        }

        tft.setCursor(spectrum_x + pos_help, spectrum_y + spectrum_WF_height + pos_grat_y);
        tft.print(txt);
        // insert draw vertical bar HERE:



      }
      if (spectrum_zoom > 2 || freq_calc > 1000)
      {
        idx++;
      }
    }
  }
  tft.setFont(Arial_10);

  //**************************************************************************
  uint16_t base_y = spectrum_y + spectrum_WF_height + 4;

  //    float32_t pixel_per_khz = (1<<spectrum_zoom) * 256.0 / SR[SAMPLE_RATE].rate * 1000.0;

  // center line
  if (spectrum_zoom == 0)
  {
    tft.drawFastVLine(spectrum_x + 62, base_y + 1, 10, ILI9341_RED);
    tft.drawFastVLine(spectrum_x + 65, base_y + 1, 10, ILI9341_RED);
    tft.drawFastVLine(spectrum_x + 63, base_y + 1, 10, ILI9341_RED);
    tft.drawFastVLine(spectrum_x + 64, base_y + 1, 10, ILI9341_RED);
    tft.drawFastVLine(spectrum_x + 127, base_y + 1, 10, ILI9341_YELLOW);
  }
  else
  {
    tft.drawFastVLine(spectrum_x + 126, base_y + 1, 10, ILI9341_RED);
    tft.drawFastVLine(spectrum_x + 129, base_y + 1, 10, ILI9341_RED);
    tft.drawFastVLine(spectrum_x + 127, base_y + 1, 10, ILI9341_RED);
    tft.drawFastVLine(spectrum_x + 128, base_y + 1, 10, ILI9341_RED);
    tft.drawFastVLine(spectrum_x + 63, base_y + 1, 10, ILI9341_YELLOW);
  }
  // vertical lines
  tft.drawFastVLine(spectrum_x, base_y + 1, 10, ILI9341_YELLOW);
  tft.drawFastVLine(spectrum_x - 1, base_y + 1, 10, ILI9341_YELLOW);
  tft.drawFastVLine(spectrum_x + 255, base_y + 1, 10, ILI9341_YELLOW);
  tft.drawFastVLine(spectrum_x + 256, base_y + 1, 10, ILI9341_YELLOW);
  tft.drawFastVLine(spectrum_x + 191, base_y + 1, 10, ILI9341_YELLOW);

  if (spectrum_zoom < 3 && freq_calc <= 1000)
  {
    tft.drawFastVLine(spectrum_x + 31, base_y + 1, 10, ILI9341_YELLOW);
    tft.drawFastVLine(spectrum_x + 95, base_y + 1, 10, ILI9341_YELLOW);
    tft.drawFastVLine(spectrum_x + 159, base_y + 1, 10, ILI9341_YELLOW);
    tft.drawFastVLine(spectrum_x + 223, base_y + 1, 10, ILI9341_YELLOW);
  }
  showFreqBand();
  show_bandwidth();
}

//showFreqBand() Display the frequency bandin lower left corner.  <PUA>
// White for Broadcast band, Yellow for Ham band, Red for out-of-band.
void showFreqBand(void)
{
  tft.setTextColor(ILI9341_RED);      // Color for out-of-band tuning
  if (bands[current_band].freq >= bands[current_band].fBandLow  && bands[current_band].freq <= bands[current_band].fBandHigh)
  {
    if (bands[current_band].band_type == BROADCAST_BAND)
      tft.setTextColor(ILI9341_WHITE);
    else if (bands[current_band].band_type == HAM_BAND)
      tft.setTextColor(ILI9341_ORANGE);
    else
      tft.setTextColor(ILI9341_GREEN);
  }
  tft.setFont(Arial_12);
  //  tft.setCursor(277, 212);
  tft.setCursor(BAND_INDICATOR_X, BAND_INDICATOR_Y); // 277,212
  tft.print(bands[current_band].name);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// The two levels are the maximum observed since the last update on (0, 32768). They are displayed
// as equivalent bits (0,15) at 1/4 bit per pixel.
void displayLevel(uint16_t adcLevel, uint16_t dacLevel)
{
  uint16_t adcPixels, dacPixels;
  static uint16_t adcPixels_old, dacPixels_old;

#if 0
Prototypes temp ref:
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
#endif

  uint16_t barColor;

  tft.fillRect(ADC_BAR + 1, YTOP_LEVEL_DISP + 5, 60, 3, ILI9341_BLACK);
  tft.fillRect(DAC_BAR + 1, YTOP_LEVEL_DISP + 5, 60, 3, ILI9341_BLACK);

  // log10(32768)=4.515.  To occupy 60 pixels, mult by 13.2877
#ifdef USE_LOG10FAST
  adcPixels = (uint16_t)(13.2877 * log10f_fast((float32_t)adcLevel));
  dacPixels = (uint16_t)(13.2877 * log10f_fast((float32_t)dacLevel));
#else
  adcPixels = (uint16_t)(13.2877 * log10f((float32_t)adcLevel));
  dacPixels = (uint16_t)(13.2877 * log10f((float32_t)dacLevel));
#endif
  //  adcPixels = (uint16_t)(0.9 * (float32_t)adcPixels_old + 0.1 * (float32_t)adcPixels);
  //  dacPixels = (uint16_t)(0.9 * (float32_t)dacPixels_old + 0.1 * (float32_t)dacPixels);

  if (adcPixels > 55)
    barColor = ILI9341_RED;
  else
    barColor = ILI9341_WHITE;
  tft.drawFastHLine (ADC_BAR + 1, YTOP_LEVEL_DISP + 5,  adcPixels, barColor); // ADC Bar Graph
  tft.drawFastHLine (ADC_BAR + 1, YTOP_LEVEL_DISP + 6,  adcPixels, barColor);
  tft.drawFastHLine (ADC_BAR + 1, YTOP_LEVEL_DISP + 7,  adcPixels, barColor);

  if (dacPixels > 55)
    barColor = ILI9341_RED;
  else
    barColor = ILI9341_WHITE;
  tft.drawFastHLine (DAC_BAR + 1, YTOP_LEVEL_DISP + 5,  dacPixels, barColor); // DAC Bar Graph
  tft.drawFastHLine (DAC_BAR + 1, YTOP_LEVEL_DISP + 6,  dacPixels, barColor);
  tft.drawFastHLine (DAC_BAR + 1, YTOP_LEVEL_DISP + 7,  dacPixels, barColor);
}

// Draw the frames for ADC & DAC bar graphs, but not the inside bar.
void prepareLevelDisplay()
{
  //  uint16_t jjj;

  tft.fillRect(0, YTOP_LEVEL_DISP - 2, 200, 14, ILI9341_BLACK);

  // draw ADC and DAC layouts
  tft.drawFastHLine (ADC_BAR, YTOP_LEVEL_DISP + 3,  61, ILI9341_ORANGE); // Top
  tft.drawFastHLine (ADC_BAR, YTOP_LEVEL_DISP + 9,  61, ILI9341_ORANGE); // Bottom
  tft.drawFastVLine (ADC_BAR, YTOP_LEVEL_DISP + 4,   5, ILI9341_ORANGE); // Left
  tft.drawFastVLine (ADC_BAR + 61, YTOP_LEVEL_DISP + 3, 7, ILI9341_ORANGE); // Right

  for (int jjj = 0; jjj < 8; jjj++)
    tft.drawFastVLine (4 + ADC_BAR + 8 * jjj, YTOP_LEVEL_DISP, 3, ILI9341_ORANGE); // Tic's

  tft.setCursor(ADC_BAR + 63, YTOP_LEVEL_DISP + 2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_8);
  tft.print("ADC");

  // draw ADC and DAC layouts
  tft.drawFastHLine (DAC_BAR, YTOP_LEVEL_DISP + 3,  61, ILI9341_ORANGE); // Top
  tft.drawFastHLine (DAC_BAR, YTOP_LEVEL_DISP + 9,  61, ILI9341_ORANGE); // Bottom
  tft.drawFastVLine (DAC_BAR, YTOP_LEVEL_DISP + 4,   5, ILI9341_ORANGE); // Left
  tft.drawFastVLine (DAC_BAR + 61, YTOP_LEVEL_DISP + 3, 7, ILI9341_ORANGE); // Right

  for (int jjj = 0; jjj < 8; jjj++)
    tft.drawFastVLine (4 + DAC_BAR + 8 * jjj, YTOP_LEVEL_DISP, 3, ILI9341_ORANGE); // Tic's

  tft.setCursor(DAC_BAR + 63, YTOP_LEVEL_DISP + 2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_8);
  tft.print("DAC");

} // END prepareLevelDisplay
///////////////////////////////////////////////////////////////////////////



float32_t alpha_beta_mag(float32_t  inphase, float32_t  quadrature)
// (c) András Retzler
// taken from libcsdr: https://github.com/simonyiszk/csdr
{
  // Min RMS Err      0.947543636291 0.392485425092
  // Min Peak Err     0.960433870103 0.397824734759
  // Min RMS w/ Avg=0 0.948059448969 0.392699081699
  const float32_t alpha = 0.960433870103; // 1.0; //0.947543636291;
  const float32_t beta =  0.397824734759;
  /* magnitude ~= alpha * max(|I|, |Q|) + beta * min(|I|, |Q|) */
  float32_t abs_inphase = fabs(inphase);
  float32_t abs_quadrature = fabs(quadrature);
  if (abs_inphase > abs_quadrature) {
    return alpha * abs_inphase + beta * abs_quadrature;
  } else {
    return alpha * abs_quadrature + beta * abs_inphase;
  }
}
/*
  void amdemod_estimator_cf(complexf* input, float *output, int input_size, float alpha, float beta)
  { //  (c) András Retzler
  //  taken from libcsdr: https://github.com/simonyiszk/csdr
  //concept is explained here:
  //http://www.dspguru.com/dsp/tricks/magnitude-estimator

  //default: optimize for min RMS error
  if(alpha==0)
  {
    alpha=0.947543636291;
    beta=0.392485425092;
  }

  //@amdemod_estimator
  for (int i=0; i<input_size; i++)
  {
    float abs_i=iof(input,i);
    if(abs_i<0) abs_i=-abs_i;
    float abs_q=qof(input,i);
    if(abs_q<0) abs_q=-abs_q;
    float max_iq=abs_i;
    if(abs_q>max_iq) max_iq=abs_q;
    float min_iq=abs_i;
    if(abs_q<min_iq) min_iq=abs_q;

    output[i]=alpha*max_iq+beta*min_iq;
  }
  }

*/

float32_t fastdcblock_ff(float32_t* input, float32_t* output, int input_size, float32_t last_dc_level)
{ //  (c) András Retzler
  //  taken from libcsdr: https://github.com/simonyiszk/csdr
  //this DC block filter does moving average block-by-block.
  //this is the most computationally efficient
  //input and output buffer is allowed to be the same
  //http://www.digitalsignallabs.com/dcblock.pdf
  float32_t avg = 0.0;
  for (int i = 0; i < input_size; i++) //@fastdcblock_ff: calculate block average
  {
    avg += input[i];
  }
  avg /= input_size;

  float32_t avgdiff = avg - last_dc_level;
  //DC removal level will change lineraly from last_dc_level to avg.
  for (int i = 0; i < input_size; i++) //@fastdcblock_ff: remove DC component
  {
    float32_t dc_removal_level = last_dc_level + avgdiff * ((float32_t)i / input_size);
    output[i] = input[i] - dc_removal_level;
  }
  return avg;
}

void set_IIR_coeffs (float32_t f0, float32_t Q, float32_t sample_rate, uint8_t filter_type)
{
  //     set_IIR_coeffs ((float32_t)2400.0, 1.3, (float32_t)SR[SAMPLE_RATE].rate, 0);
  /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Cascaded biquad (notch, peak, lowShelf, highShelf) [DD4WH, april 2016]
    ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
  // DSP Audio-EQ-cookbook for generating the coeffs of the filters on the fly
  // www.musicdsp.org/files/Audio-EQ-Cookbook.txt  [by Robert Bristow-Johnson]
  //
  // the ARM algorithm assumes the biquad form
  // y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2]
  //
  // However, the cookbook formulae by Robert Bristow-Johnson AND the Iowa Hills IIR Filter designer
  // use this formula:
  //
  // y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] - a1 * y[n-1] - a2 * y[n-2]
  //
  // Therefore, we have to use negated a1 and a2 for use with the ARM function
  if (f0 > sample_rate / 2.0) f0 = sample_rate / 2.0;
  float32_t w0 = f0 * (TPI / sample_rate);
  float32_t sinW0 = sinf(w0);
  float32_t alpha = sinW0 / (Q * 2.0);
  float32_t cosW0 = cosf(w0);
  float32_t scale = 1.0 / (1.0 + alpha);

  if (filter_type == 0)
  { // lowpass coeffs

    /* b0 */ coefficient_set[0] = ((1.0 - cosW0) / 2.0) * scale;
    /* b1 */ coefficient_set[1] = (1.0 - cosW0) * scale;
    /* b2 */ coefficient_set[2] = coefficient_set[0];
    /* a1 */ coefficient_set[3] = (2.0 * cosW0) * scale; // negated
    /* a2 */     coefficient_set[4] = (-1.0 + alpha) * scale; // negated
  }
  else if (filter_type == 2)
  {
    //BPF:        H(s) = (s/Q) / (s^2 + s/Q + 1)      (constant 0 dB peak gain)

    /* b0 */ coefficient_set[0] =  alpha * scale;
    /* b1 */ coefficient_set[1] =  0.0;
    /* b2 */ coefficient_set[2] =  - alpha * scale;
    //     a0 =   1 + alpha
    /* a1 */ coefficient_set[3] =  2.0 * cosW0 * scale; // negated
    /* a2 */ coefficient_set[4] =  alpha - 1.0; // negated
  }
  else if (filter_type == 3) // notch
  {

  }

}

int ExtractDigit(unsigned long long int n, int k) {
  switch (k) {
    case 0: return n % 10;
    case 1: return n / 10 % 10;
    case 2: return n / 100 % 10;
    case 3: return n / 1000 % 10;
    case 4: return n / 10000 % 10;
    case 5: return n / 100000 % 10;
    case 6: return n / 1000000 % 10;
    case 7: return n / 10000000 % 10;
    case 8: return n / 100000000 % 10;
    case 9: return n / 1000000000 % 10;
    default: return 0;
  }
}

// show frequency
void show_frequency(unsigned long long freq, uint8_t text_size) {
  // text_size 0 --> small display
  // text_size 1 --> large main display
  int color = ILI9341_WHITE;
  int8_t font_width = 8;
  int8_t sch_help = 0;
  freq = freq / SI5351_FREQ_MULT;
  if (bands[current_band].mode == DEMOD_WFM)
  {
    // old undersampling 5 times MODE, now switched to 3 times undersampling
    //        freq = freq * 5 + 1.25 * SR[SAMPLE_RATE].rate; // undersampling of f/5 and correction, because no IF is used in WFM mode
    freq = freq * 3 + 0.75 * (unsigned long long)SR[SAMPLE_RATE].rate; // undersampling of f/3 and correction, because no IF is used in WFM mode
    erase_flag = 1;
  }
  if (text_size == 0) // small SAM carrier display
  {
    if (freq_flag[0] == 0)
    { // print text first time we´re here
      //      tft.setCursor(pos_x_frequency + 10, pos_y_frequency + 26);
      tft.setCursor(0, pos_y_frequency + 26);
      tft.setFont(Arial_8);
      tft.print("  ");
      tft.setCursor(pos_x_frequency + 10, pos_y_frequency + 26);
      tft.setTextColor(ILI9341_ORANGE);
      tft.print("SAM carrier ");
    }
    sch_help = 9;
    freq += SAM_carrier_freq_offset;
    tft.setFont(Arial_10);
    pos_x_frequency = pos_x_frequency + 68;
    pos_y_frequency = pos_y_frequency + 24;
    color = ILI9341_GREEN;
  }
  else // large main frequency display
  {
    sch_help = 9;
    tft.setFont(Arial_18);
    font_width = 16;
  }
  tft.setTextColor(color);
  uint8_t zaehler;
  uint8_t digits[10];
  zaehler = 9; //8;

  while (zaehler--) {
    digits[zaehler] = ExtractDigit (freq, zaehler);
    //              Serial.print(digits[zaehler]);
    //              Serial.print(".");
    // 7: 10Mhz, 6: 1Mhz, 5: 100khz, 4: 10khz, 3: 1khz, 2: 100Hz, 1: 10Hz, 0: 1Hz
  }
  //  Serial.print("xxxxxxxxxxxxx");

  zaehler = 9; //8;
  while (zaehler--) { // counts from 8 to 0
    if (zaehler < 6) sch = sch_help; // (khz)
    if (zaehler < 3) sch = sch_help * 2; //18; // (Hz)
    if (digits[zaehler] != digits_old[text_size][zaehler] || !freq_flag[text_size]) { // digit has changed (or frequency is displayed for the first time after power on)
      if (zaehler == 8) {
        sch = 0;
        tft.setCursor(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency); // set print position
        //        tft.fillRect(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency, font_width, 18, ILI9341_BLACK); // delete old digit
        tft.fillRect(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency, font_width, font_width + 2, ILI9341_BLACK); // delete old digit
        if (digits[8] != 0) tft.print(digits[zaehler]); // write new digit in white
      }
      if (zaehler == 7) {
        sch = 0;
        tft.setCursor(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency); // set print position
        //        tft.fillRect(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency, font_width, 18, ILI9341_BLACK); // delete old digit
        tft.fillRect(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency, font_width, font_width + 2, ILI9341_BLACK); // delete old digit
        if (digits[7] != 0 || digits[8] != 0) tft.print(digits[zaehler]); // write new digit in white
      }
      if (zaehler == 6) {
        sch = 0;
        tft.setCursor(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency); // set print position
        //        tft.fillRect(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency, font_width, 18, ILI9341_BLACK); // delete old digit
        tft.fillRect(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency, font_width, font_width + 2, ILI9341_BLACK); // delete old digit
        if (digits[6] != 0 || digits[7] != 0 || digits[8] != 0) tft.print(digits[zaehler]); // write new digit in white
      }
      if (zaehler == 5) {
        sch = 9;
        tft.setCursor(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency); // set print position
        //        tft.fillRect(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency, font_width, 18, ILI9341_BLACK); // delete old digit
        tft.fillRect(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency, font_width, font_width + 2, ILI9341_BLACK); // delete old digit
        if (digits[5] != 0 || digits[6] != 0 || digits[7] != 0 || digits[8] != 0) tft.print(digits[zaehler]); // write new digit in white
      }

      if (zaehler < 5) {
        // print the digit
        tft.setCursor(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency); // set print position
        //        tft.fillRect(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency, font_width, 18, ILI9341_BLACK); // delete old digit
        tft.fillRect(pos_x_frequency + font_width * (8 - zaehler) + sch, pos_y_frequency, font_width, font_width + 2, ILI9341_BLACK); // delete old digit
        tft.print(digits[zaehler]); // write new digit in white
      }
      digits_old[text_size][zaehler] = digits[zaehler];
    }
  }

  // reset to previous values!

  // print small yellow points between blocks of three digits
  if (text_size == 0)
  {
    if (digits[7] == 0 && digits[6] == 0 && digits[8] == 0)
      tft.fillRect(pos_x_frequency + font_width * 3 + 2, pos_y_frequency + 8, 2, 2, ILI9341_BLACK);
    else    tft.fillRect(pos_x_frequency + font_width * 3 + 2, pos_y_frequency + 8, 2, 2, ILI9341_YELLOW);
    tft.fillRect(pos_x_frequency + font_width * 8 - 4, pos_y_frequency + 8, 2, 2, ILI9341_YELLOW);
    pos_y_frequency -= 24;
    pos_x_frequency -= 68;
  }
  else
  {
    tft.setFont(Arial_10);
    if (digits[7] == 0 && digits[6] == 0 && digits[8] == 0)
      tft.fillRect(pos_x_frequency + font_width * 3 + 2 , pos_y_frequency + 15, 3, 3, ILI9341_BLACK);
    else    tft.fillRect(pos_x_frequency + font_width * 3 + 2, pos_y_frequency + 15, 3, 3, ILI9341_YELLOW);
    tft.fillRect(pos_x_frequency + font_width * 7 - 6, pos_y_frequency + 15, 3, 3, ILI9341_YELLOW);
    if (!freq_flag[text_size]) {
      tft.setCursor(pos_x_frequency + font_width * 9 + 21, pos_y_frequency + 7); // set print position
      tft.setTextColor(ILI9341_GREEN);
      tft.print("Hz");
    }

  }
  freq_flag[text_size] = 1;
  //      Serial.print("SAM carrier frequency = "); Serial.println((int)freq);

} // END VOID SHOW-FREQUENCY

void switch_RF_filters()
{
    static int16_t lastFilter;
  static int16_t currentFilter;
//#elif defined(Band1) && defined(Band2) && defined(Band3) && defined(Band4) && defined(Band5)
#ifdef HARDWARE_DD4WH
  // LPF switching follows here
  // Five filter banks there:
  // longwave LPF 295kHz, mediumwave I LPF 955kHz, mediumwave II LPF 2MHz, tropical bands LPF 5.4MHz, others LPF LPF 30MHz
  // LW: Band5
  // MW: Band3 (up to 955kHz)
  // MW: Band1 (up tp 1996kHz)
  // SW: Band2 (up to 5400kHz)
  // SW: Band4 (up up up)
  //
  // LOWPASS 955KHZ
  if (((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) < 955001 * SI5351_FREQ_MULT) && ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) > 300001 * SI5351_FREQ_MULT)) {
    digitalWrite (Band3, HIGH); 
#ifdef DEBUG
    Serial.println ("Band3: 300kHz bis 955kHz");
#endif    
    digitalWrite (Band1, LOW); digitalWrite (Band2, LOW); digitalWrite (Band4, LOW); digitalWrite (Band5, LOW);
  } // end if

  // LOWPASS 2MHZ
  if (((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) > 955000 * SI5351_FREQ_MULT) && ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) < 1996001 * SI5351_FREQ_MULT)) {
    digitalWrite (Band1, HIGH);//Serial.println ("Band1");
#ifdef DEBUG
    Serial.println ("Band1: 955kHz bis 1996kHz");
#endif    
    digitalWrite (Band5, LOW); digitalWrite (Band3, LOW); digitalWrite (Band4, LOW); digitalWrite (Band2, LOW);
  } // end if

  //LOWPASS 5.4MHZ
  if (((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) > 1996000 * SI5351_FREQ_MULT) && ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) < 5400001 * SI5351_FREQ_MULT)) {
    digitalWrite (Band2, HIGH);//Serial.println ("Band2");
    digitalWrite (Band4, LOW); digitalWrite (Band3, LOW); digitalWrite (Band1, LOW); digitalWrite (Band5, LOW);
#ifdef DEBUG
    Serial.println ("Band2: 1996kHz bis 5.4MHz");
#endif    
  } // end if

  // LOWPASS 30MHZ --> OK
  if ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) > 5400000 * SI5351_FREQ_MULT) {
    // && ((bands[current_band].freq + IF_FREQ) < 12500001)) {
    digitalWrite (Band4, HIGH);//Serial.println ("Band4");
    digitalWrite (Band1, LOW); digitalWrite (Band3, LOW); digitalWrite (Band2, LOW); digitalWrite (Band5, LOW);
#ifdef DEBUG
    Serial.println ("Band4: > 5.4MHz zwischen 5.4MHz und 12MHz Geistersignale !");
#endif    
  } // end if
  // I took out the 12.5MHz lowpass and inserted the 30MHz instead - I have to live with 3rd harmonic images in the range 5.4 - 12Mhz now
  // maybe this is more important than the 5.4 - 2Mhz filter ?? Maybe swap them sometime, because I only got five filter relays . . .

  // this is the brandnew longwave LPF (cutoff ca. 295kHz) --> OK
  if ((bands[current_band].freq - IF_FREQ * SI5351_FREQ_MULT) < 300000 * SI5351_FREQ_MULT) {
    digitalWrite (Band5, HIGH);//Serial.println ("Band5");
    digitalWrite (Band2, LOW); digitalWrite (Band3, LOW); digitalWrite (Band4, LOW); digitalWrite (Band1, LOW);
#ifdef DEBUG
    Serial.println ("Band5: < 295kHz");
#endif    
  }
#endif // HARDWARE_DD4WH
  
#ifdef HARDWARE_DO7JBH
  //***************************************************************************
  // Bandpass Filter switch
  // Bnd2 Bnd1  Frequency Range
  // 0  0     700khz  1.6MHz
  // 0  1     1.6Mhz  4.16MHz
  // 1  0     4.16MHz 11MHz
  // 1    1     11MHz   29MHz
  //***************************************************************************

  // this switches the four DO7JBH filters

  if ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) < 160000000)  {
      digitalWrite (Band1, LOW); digitalWrite (Band2, LOW);
//    digitalWrite (Band1, HIGH); digitalWrite (Band2, LOW);
    Serial.println("< 1.6MHz filter");
  } // end if
  if (((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) > 160000000) && ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) < 416000000)) {
    digitalWrite (Band1, HIGH); digitalWrite (Band2, LOW);
    Serial.println("< 4.16MHz filter");
  } // end if
  if (((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) > 416000000) && ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) < 1100000000)) {
    digitalWrite (Band1, LOW); digitalWrite (Band2, HIGH);
    Serial.println("< 11MHz filter");
  } // end if
  if ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) > 1100000000) {
    digitalWrite (Band1, HIGH); digitalWrite (Band2, HIGH);
    Serial.println(" > 11MHz filter");
  } // end if
  // for wideband FM reception, temporarily short-circuited filter for < 1600kHz in hardware by DO7JBH
  if ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) > 3000000000) {
    digitalWrite (Band1, LOW); digitalWrite (Band2, LOW);
    Serial.println(" UKW filter");
  } // end if
#endif

#ifdef USE_BOBS_FILTER

  if ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) < 160000000)  {
    currentFilter = 1;
  } // end if
  else if (((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) < 350000000)) {
    currentFilter = 2;
  } // end if
  else if (((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) < 730000000)) {
    currentFilter = 3;
  } // end if
  else if (((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) < 1500000000)) {
    currentFilter = 4;
  } // end if
  else if ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) < 3000000000) {
    currentFilter = 5;
  } // end if
  // for wideband FM reception, temporarily short-circuited filter for < 1600kHz in hardware by DO7JBH
  else if ((bands[current_band].freq + IF_FREQ * SI5351_FREQ_MULT) > 3000000000) {
    currentFilter = 6;
  } // end if
  else
    currentFilter = 99;

  Serial.print("LastFilter = "); Serial.println(lastFilter);
  Serial.print("CurrentFilter = "); Serial.println(currentFilter);

  if (currentFilter == lastFilter)
    return;
  lastFilter = currentFilter;

  if (currentFilter == 1) // < 1.6 MHz
  {
    digitalWrite (Band_3M5_7M3, LOW);
    digitalWrite (Band_7M3_15M, LOW);
    digitalWrite (Band_15M_30M, LOW);
    Serial.println("3.5MHz filter OFF");
  }
  else if (currentFilter == 2) // < 3.5 MHz
  {
    digitalWrite (Band_3M5_7M3, LOW);
    digitalWrite (Band_7M3_15M, LOW);
    digitalWrite (Band_15M_30M, LOW);
    Serial.println("3.5MHz filter OFF");
  }
  else if (currentFilter == 3) // < 7.3 MHz
  {
    digitalWrite (Band_3M5_7M3, HIGH);
    digitalWrite (Band_7M3_15M, LOW);
    digitalWrite (Band_15M_30M, LOW);
    Serial.println("3.5MHz filter ON");
  }
  else if (currentFilter == 4) // < 15 MHz
  {
    digitalWrite (Band_3M5_7M3, LOW);
    digitalWrite (Band_7M3_15M, HIGH);
    digitalWrite (Band_15M_30M, LOW);
    Serial.println("3.5MHz filter OFF");
  }
  else if (currentFilter == 5) // < 30 MHz
  {
    digitalWrite (Band_3M5_7M3, LOW);
    digitalWrite (Band_7M3_15M, LOW);
    digitalWrite (Band_15M_30M, HIGH);
    Serial.println("3.5MHz filter OFF");
  }
  else if (currentFilter == 6) // > 30 MHz
  {
    digitalWrite (Band_3M5_7M3, LOW);
    digitalWrite (Band_7M3_15M, LOW);
    digitalWrite (Band_15M_30M, LOW);
    Serial.println("3.5MHz filter OFF");
  }
  // Bypass all relays (no RF filtering)
  else
  {
    digitalWrite (Band_3M5_7M3, LOW);
    digitalWrite (Band_7M3_15M, LOW);
    digitalWrite (Band_15M_30M, LOW);
  }
#endif
}   // end switch_RF_filters()

void setfreq () {
  // Changes for Bobs Octave Filters:  18 March 2018  W7PUA <<<<<<
  // http://www.janbob.com/electron/FilterBP1/FiltBP1.html


  // NEVER USE AUDIONOINTERRUPTS HERE: that introduces annoying clicking noise with every frequency change
  //   hilfsf = (bands[current_band].freq +  IF_FREQ) * 10000000 * MASTER_CLK_MULT * SI5351_FREQ_MULT;
  hilfsf = (bands[current_band].freq +  IF_FREQ * SI5351_FREQ_MULT) * 1000000000 * MASTER_CLK_MULT; // SI5351_FREQ_MULT is 100ULL;
  hilfsf = hilfsf / calibration_factor;
  si5351.set_freq(hilfsf, Si_5351_clock);
  if (bands[current_band].mode == DEMOD_AUTOTUNE)
  {
    autotune_flag = 1;
  }
  FrequencyBarText();
  switch_RF_filters();
} // end setfreq

#if defined(HARDWARE_FRANKB)
//emulate buttons
class tbutton {
  public:
    void update() {};
    bool fallingEdge() {
      return false;
    };
};
tbutton button1, button2, button3, button4, button5, button6, button7, button8;
#endif


void buttons() {
  button1.update(); // BAND --
  button2.update(); // BAND ++
  button3.update(); // change band[bands].mode
  button4.update(); // MENU --
  button5.update(); // tune encoder button
  button6.update(); // filter encoder button
  button7.update(); // MENU ++
  button8.update(); // menu2 button
  eeprom_saved = 0;
  eeprom_loaded = 0;

  if ( button1.fallingEdge()) {

    if (Menu_pointer == MENU_PLAYER)
    {
#if defined(MP3)
      prevtrack();
#endif
    }
    else
    {
      int last_band = current_band;
      current_band--;
      if (current_band < FIRST_BAND) current_band = LAST_BAND; // cycle thru radio bands
      // set frequency_print flag to 0
      AudioNoInterrupts();
      sgtl5000_1.dacVolume(0.0);
      //setup_mode(bands[current_band].mode);
      freq_flag[1] = 0;
      set_band();
      control_filter_f();
      filter_bandwidth();
      set_tunestep();
      show_menu();
      prepare_spectrum_display();
      leave_WFM = 0;
      AGC_prep();
      if (bands[current_band].mode == DEMOD_WFM)
      { // if switched to WFM: set sample rate to 234ksps, switch off spectrum
        show_spectrum_flag = 0;
        LAST_SAMPLE_RATE = SAMPLE_RATE;
        SAMPLE_RATE = SAMPLE_RATE_234K;
        setI2SFreq(SAMPLE_RATE);
        set_samplerate();
        show_frequency(bands[current_band].freq, 1);
      }
      else
      {
        if (bands[last_band].mode == DEMOD_WFM && SAMPLE_RATE != LAST_SAMPLE_RATE)
        { // switch from WFM to any other mode
          show_spectrum_flag = 1;
          SAMPLE_RATE = LAST_SAMPLE_RATE;
          setI2SFreq(SAMPLE_RATE);
          set_samplerate();
        }
      }
      delay(1);
      sgtl5000_1.dacVolume(1.0);
      AudioInterrupts();
    }
  }
  if ( button2.fallingEdge()) {
    if (Menu_pointer == MENU_PLAYER)
    {
#if defined(MP3)
      pausetrack();
#endif
    }
    else
    {
      AudioNoInterrupts();
      int last_band = current_band;
      current_band++;
      if (current_band > LAST_BAND) current_band = FIRST_BAND; // cycle thru radio bands
      // set frequency_print flag to 0
      sgtl5000_1.dacVolume(0.0);
      //setup_mode(bands[current_band].mode);
      freq_flag[1] = 0;
      set_band();
      set_tunestep();
      //control_filter_f();
      filter_bandwidth();
      show_menu();
      prepare_spectrum_display();
      leave_WFM = 0;
      AGC_prep();
      if (bands[current_band].mode == DEMOD_WFM)
      { // if switched to WFM: set sample rate to 234ksps, switch off spectrum
        show_spectrum_flag = 0;
        LAST_SAMPLE_RATE = SAMPLE_RATE;
        SAMPLE_RATE = SAMPLE_RATE_234K;
        setI2SFreq(SAMPLE_RATE);
        set_samplerate();
        show_frequency(bands[current_band].freq, 1);
      }
      else
      {
        if (bands[last_band].mode == DEMOD_WFM && SAMPLE_RATE != LAST_SAMPLE_RATE)
        { // switch from WFM to any other mode
          show_spectrum_flag = 1;
          SAMPLE_RATE = LAST_SAMPLE_RATE;
          setI2SFreq(SAMPLE_RATE);
          set_samplerate();
        }
      }
      sgtl5000_1.dacVolume(1.0);
      delay(1);
      AudioInterrupts();
    }
  }
  if ( button3.fallingEdge()) {  // cycle through DEMOD modes
    if (Menu_pointer == MENU_PLAYER)
    {
#if defined(MP3)
      nexttrack();
#endif
    }
    else
    { int old_mode = bands[current_band].mode;
      bands[current_band].mode++;
      if (bands[current_band].mode > DEMOD_MAX) bands[current_band].mode = DEMOD_MIN; // cycle thru demod modes
      AudioNoInterrupts();
      sgtl5000_1.dacVolume(0.0);
      setup_mode(bands[current_band].mode);
      show_frequency(bands[current_band].freq, 1);
      control_filter_f();
      filter_bandwidth();
      leave_WFM = 0;
      prepare_spectrum_display();
      if (twinpeaks_tested == 3 && twinpeaks_counter >= 200) write_analog_gain = 1;
      show_analog_gain();

      if (bands[current_band].mode == DEMOD_WFM)
      { // if switched to WFM: set sample rate to 234ksps, switch off spectrum
        show_spectrum_flag = 0;
        LAST_SAMPLE_RATE = SAMPLE_RATE;
        SAMPLE_RATE = SAMPLE_RATE_234K;
        setI2SFreq(SAMPLE_RATE);
        set_samplerate();
        show_frequency(bands[current_band].freq, 1);
      }
      else
      {
        if (old_mode == DEMOD_WFM && SAMPLE_RATE != LAST_SAMPLE_RATE)
        { // switch from WFM to any other mode
          show_spectrum_flag = 1;
          SAMPLE_RATE = LAST_SAMPLE_RATE;
          setI2SFreq(SAMPLE_RATE);
          set_samplerate();
        }
      }

#ifdef USE_ADC_DAC_display
      if (bands[current_band].mode == DEMOD_SAM  ||  bands[current_band].mode == DEMOD_SAM_STEREO)
        //Serial.println(" ");
        tft.fillRect(0, YTOP_LEVEL_DISP, 200, 13, ILI9341_BLACK);
      else
        prepareLevelDisplay();
#endif

      idx_t = 0;
      delay(10);
      AudioInterrupts();
      sgtl5000_1.dacVolume(1.0);
    }
  }
  if ( button4.fallingEdge()) {
    // toggle thru menu
    if (which_menu == 1)
    {
      if (--Menu_pointer < first_menu) Menu_pointer = last_menu;
    }
    else
    {
      if (--Menu2 < first_menu2) Menu2 = last_menu2;
    }
    if (Menu_pointer == MENU_PLAYER)
    {
      Menu2 = MENU_VOLUME;
      setI2SFreq (AUDIO_SAMPLE_RATE_EXACT);
      delay(200); // essential ?
      //                  audio_flag = 0;
      Q_in_L.end();
      Q_in_R.end();
      Q_in_L.clear();
      Q_in_R.clear();
      mixleft.gain(0, 0.0);
      mixright.gain(0, 0.0);
      mixleft.gain(1, 0.1);
      mixright.gain(1, 0.1);
      mixleft.gain(2, 0.1);
      mixright.gain(2, 0.1);
    }
#if defined(MP3)
    if (Menu_pointer == (MENU_PLAYER - 1) || (Menu_pointer == last_menu && MENU_PLAYER == first_menu))
    {
      // stop all playing
      playMp3.stop();
      playAac.stop();
      delay(200);
      setI2SFreq (SR[SAMPLE_RATE].rate);
      delay(200); // essential ?
      mixleft.gain(0, 1.0);
      mixright.gain(0, 1.0);
      mixleft.gain(1, 0.0);
      mixright.gain(1, 0.0);
      mixleft.gain(2, 0.0);
      mixright.gain(2, 0.0);
      prepare_spectrum_display();
      Q_in_L.clear();
      Q_in_R.clear();
      Q_in_L.begin();
      Q_in_R.begin();
    }
#endif
    show_menu();

  }
  if ( button5.fallingEdge()) { // cycle thru tune steps
    if (++tune_stepper > TUNE_STEP_MAX) tune_stepper = TUNE_STEP_MIN;
    set_tunestep();
  }
  if (button6.fallingEdge()) {
    if (Menu_pointer == MENU_SAVE_EEPROM)
    {
      EEPROM_SAVE();
      eeprom_saved = 1;
      show_menu();
    }
    else if (Menu_pointer == MENU_LOAD_EEPROM)
    {
      EEPROM_LOAD();
      eeprom_loaded = 1;
      show_menu();
    }
    else if (Menu_pointer == MENU_IQ_AUTO)
    {
      if (auto_IQ_correction == 0)
        auto_IQ_correction = 1;
      else auto_IQ_correction = 0;
      show_menu();
    }
    else if (Menu_pointer == MENU_RESET_CODEC)
    {
      reset_codec();
    } // END RESET_CODEC
    else if (Menu_pointer == MENU_SHOW_SPECTRUM)
    {
      if (show_spectrum_flag == 0)
      {
        show_spectrum_flag = 1;
      }
      else
      {
        show_spectrum_flag = 0;
      }
      show_menu();
    }

    else autotune_flag = 1;
    //                Serial.println("Flag gesetzt!");

  }
  if (button7.fallingEdge()) {
    // toggle thru menu
    if (which_menu == 1)
    {
      if (++Menu_pointer > last_menu) Menu_pointer = first_menu;
    }
    else
    {
      if (++Menu2 > last_menu2) Menu2 = first_menu2;
    }
    //               Serial.println("MENU BUTTON pressed");
    if (Menu_pointer == MENU_PLAYER)
    {
      Menu2 = MENU_VOLUME;
      setI2SFreq (AUDIO_SAMPLE_RATE_EXACT);
      delay(200); // essential ?
      //                  audio_flag = 0;
      Q_in_L.end();
      Q_in_R.end();
      Q_in_L.clear();
      Q_in_R.clear();
      mixleft.gain(0, 0.0);
      mixright.gain(0, 0.0);
      mixleft.gain(1, 0.1);
      mixright.gain(1, 0.1);
      mixleft.gain(2, 0.1);
      mixright.gain(2, 0.1);
    }
#if defined(MP3)
    if (Menu_pointer == (MENU_PLAYER + 1) || (Menu_pointer == 0 && MENU_PLAYER == last_menu))
    {
      // stop all playing
      playMp3.stop();
      playAac.stop();
      delay(200);
      setI2SFreq (SR[SAMPLE_RATE].rate);
      delay(200); // essential ?
      mixleft.gain(0, 1.0);
      mixright.gain(0, 1.0);
      mixleft.gain(1, 0.0);
      mixright.gain(1, 0.0);
      mixleft.gain(2, 0.0);
      mixright.gain(2, 0.0);
      prepare_spectrum_display();
      Q_in_L.clear();
      Q_in_R.clear();
      Q_in_L.begin();
      Q_in_R.begin();
    }
#endif
    show_menu();
  }
  if (button8.fallingEdge()) {
    // toggle thru menu2
    if (Menu2 == MENU_NOTCH_1)
    {
      if (notches_on[0] == 0) notches_on[0] = 1;
      else notches_on[0] = 0;
      show_notch((int)notches[0], bands[current_band].mode);
      show_menu();
    }
    else if (Menu2 == MENU_ANR_TAPS || Menu2 == MENU_ANR_DELAY || Menu2 == MENU_ANR_MU || Menu2 == MENU_ANR_GAMMA)
    {
      if (ANR_on == 0) ANR_on = 1;
      else ANR_on = 0;
      show_menu();
    }
    else if (Menu2 == MENU_ANR_NOTCH)
    {
      //                    if(ANR_on == 0) ANR_on = 1;
      //                    else ANR_on = 0;
      if (ANR_notch == 0) ANR_notch = 1;
      else ANR_notch = 0;
      show_menu();
    }
    else if (Menu2 == MENU_NB_THRESH)
    {
      if (NB_on == 0) NB_on = 1;
      else NB_on = 0;
      show_menu();
    }
    else if (Menu2 == MENU_NR_USE_KIM)
    {
      if (NR_Kim == 0)
      {
        NR_Kim = 1;
        NR_beta = 0.25;
        NR_onemtwobeta = (1.0 - (2.0 * NR_beta));
      }
      else if (NR_Kim == 1)
      {
        NR_Kim = 2;
        NR_beta = 0.85;
        NR_onemtwobeta = (1.0 - (2.0 * NR_beta));
      }
      else if (NR_Kim == 2)
      {
        NR_Kim = 3;
        ANR_on = 1;
      }
      else if (NR_Kim == 3)
      {
        NR_Kim = 4;
        ANR_on = 0; // switch off leaky LMS
      }
      else
      {
        NR_Kim = 0; // switch off spectral NR
      }
      show_menu();
    }
    /*    else if (Menu2 == MENU_NR_ENABLE)
        {
          if (NR_enable == 0) NR_enable = 1;
          else NR_enable = 0;
          show_menu();
        } */
    /*    else if (Menu2 == MENU_NR_VAD_ENABLE)
        {
          if (NR_VAD_enable == 0) NR_VAD_enable = 1;
          else NR_VAD_enable = 0;
          show_menu();
        } */

    else if (Menu2 == MENU_NR_USE_X)
    {
      if (NR_use_X == 0) NR_use_X = 1;
      else NR_use_X = 0;
      show_menu();
    }

    else if (Menu2 == MENU_NB_IMPULSE_SAMPLES)
    {
      if (NB_test == 0) NB_test = 5;
      else if (NB_test == 5) NB_test = 9;
      else NB_test = 0;
      show_menu();
    }
    //else if (++Menu2 > last_menu2) Menu2 = first_menu2;
    which_menu = 2;
    //               Serial.println("MENU2 BUTTON pressed");
    show_menu();
  }
}


void show_menu()
{
  // two blue boxes show the setting for each encoder
  // menu  = filter    --> encoder under display
  // menu2 = encoder3  --> left encoder
  // define txt-string for display
  // position constant: spectrum_y
  // Menus[Menu_pointer].no
  // upper text menu
  float32_t BW_help = 100.0;
  int color1 = ILI9341_WHITE;
  int color2 = ILI9341_WHITE;
  if (which_menu == 1) color1 = ILI9341_DARKGREY;
  else color2 = ILI9341_DARKGREY;
  spectrum_y += 2;
  tft.fillRect(spectrum_x + 256 + 2, spectrum_y, 320 - spectrum_x - 258, 31, ILI9341_NAVY);
  tft.drawRect(spectrum_x + 256 + 2, spectrum_y, 320 - spectrum_x - 258, 31, ILI9341_MAROON);
  tft.setCursor(spectrum_x + 256 + 5, spectrum_y + 4);
  tft.setTextColor(color2);
  tft.setFont(Arial_10);
  tft.print(Menus[Menu_pointer].text1);
  // lower text menu
  tft.setCursor(spectrum_x + 256 + 5, spectrum_y + 16);
  tft.print(Menus[Menu_pointer].text2);
  // upper text menu2
  tft.setTextColor(color1);
  tft.fillRect(spectrum_x + 256 + 2, spectrum_y + 30, 320 - spectrum_x - 258, 31, ILI9341_NAVY);
  tft.drawRect(spectrum_x + 256 + 2, spectrum_y + 30, 320 - spectrum_x - 258, 31, ILI9341_MAROON);
  tft.setCursor(spectrum_x + 256 + 5, spectrum_y + 30 + 4);
  tft.print(Menus[Menu2].text1);
  // lower text menu2
  tft.setCursor(spectrum_x + 256 + 5, spectrum_y + 30 + 16);
  tft.print(Menus[Menu2].text2);
  tft.setTextColor(ILI9341_WHITE);
  // third box: shows the value of the parameter being changed
  tft.fillRect(spectrum_x + 256 + 2, spectrum_y + 60, 320 - spectrum_x - 258, 31, ILI9341_NAVY);
  tft.drawRect(spectrum_x + 256 + 2, spectrum_y + 60, 320 - spectrum_x - 258, 31, ILI9341_MAROON);
  // print value
  tft.setFont(Arial_14);
  tft.setCursor(spectrum_x + 256 + 12, spectrum_y + 31 + 31 + 7);
  if (which_menu == 1) // print value of Menu parameter
  {
    switch (Menu_pointer)
    {
      case MENU_SPECTRUM_ZOOM:
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.print(1 << spectrum_zoom);
        break;
      case MENU_IQ_AMPLITUDE:
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.printf("%01.3f", IQ_amplitude_correction_factor);
        break;
      case MENU_IQ_PHASE:
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.printf("%01.3f", IQ_phase_correction_factor);
        //                tft.print(IQ_phase_correction_factor);
        break;
      case MENU_CALIBRATION_FACTOR:
        tft.setFont(Arial_8);
        tft.setCursor(spectrum_x + 256 + 1, spectrum_y + 31 + 31 + 7);
        tft.printf("%9d", (int)(calibration_factor / 1000));
        break;
      case MENU_CALIBRATION_CONSTANT:
        tft.setFont(Arial_8);
        tft.setCursor(spectrum_x + 256 + 1, spectrum_y + 31 + 31 + 7);
        tft.printf("%9d", (int)(calibration_constant));
        break;
      case MENU_LPF_SPECTRUM:
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.printf("%01.4f", LPF_spectrum);
        break;
      case MENU_SPECTRUM_DISPLAY_SCALE:
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 6, spectrum_y + 31 + 31 + 7);
        tft.print(displayScale[currentScale].dbText);
        break;
      case MENU_SPECTRUM_OFFSET:
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 6, spectrum_y + 31 + 31 + 7);
//        tft.printf("%4.0f", offsetDisplayDB);        
        tft.printf("%4d", bands[current_band].pixel_offset);
        break;
      case MENU_SAVE_EEPROM:
        if (eeprom_saved)
        {
          tft.setFont(Arial_11);
          tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
          tft.print("saved!");
        }
        else
        {
          tft.setFont(Arial_11);
          tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
          tft.print("press!");
        }
        break;
      case MENU_LOAD_EEPROM:
        if (eeprom_loaded)
        {
          tft.setFont(Arial_11);
          tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
          tft.print("loaded!");
        }
        else
        {
          tft.setFont(Arial_11);
          tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
          tft.print("press!");
        }
        break;
      case MENU_IQ_AUTO:
        if (auto_IQ_correction)
        {
          tft.print("ON ");
        }
        else
        {
          tft.print("OFF ");
        }
        break;
      case MENU_RESET_CODEC:
        tft.setFont(Arial_11);
        tft.print("DO IT");
        break;
      case MENU_SPECTRUM_BRIGHTNESS:
        tft.setFont(Arial_11);
        tft.printf("%3d", spectrum_brightness);
        break;
      case MENU_SHOW_SPECTRUM:
        if (show_spectrum_flag)
        {
          tft.print("YES");
        }
        else
        {
          tft.print("NO");
        }
        break;
      case MENU_TIME_SET:
        break;
      case MENU_DATE_SET:
        break;
    }
  }
  else
  { // print value of Menu2 parameter
    int ANR_colour = ILI9341_WHITE;
    if (ANR_on) ANR_colour = ILI9341_RED;
    switch (Menu2)
    {
      case MENU_RF_GAIN:
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 6, spectrum_y + 31 + 31 + 7);
        tft.printf("%02.1fdB", (float)(bands[current_band].RFgain * 1.5));
        break;
      case MENU_RF_ATTENUATION:
        tft.setFont(Arial_11);
        tft.printf("%2ddB", RF_attenuation);
        break;
      case MENU_VOLUME:
        tft.print(audio_volume);
        break;
      case MENU_SAM_ZETA:
        tft.print(zeta);
        break;
      case MENU_TREBLE:
        tft.printf("%2.0f", treble * 100.0);
        break;
      case MENU_MIDTREBLE:
        tft.printf("%2.0f", midtreble * 100.0);
        break;
      case MENU_BASS:
        tft.printf("%2.0f", bass * 100.0);
        break;
      case MENU_MIDBASS:
        tft.printf("%2.0f", midbass * 100.0);
        break;
      case MENU_MID:
        tft.printf("%2.0f", mid * 100.0);
        break;
      case MENU_SAM_OMEGA:
        //                tft.setFont(Arial_11);
        //                tft.setCursor(spectrum_x + 256 + 6, spectrum_y + 31 + 31 + 7);
        tft.printf("%3.0f", omegaN);
        break;
      case MENU_SAM_CATCH_BW:
        tft.setFont(Arial_12);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.printf("%4.0f", pll_fmax);
        break;
      case MENU_NOTCH_1:
        tft.setFont(Arial_12);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        if (notches_on[0])
        {
          tft.setTextColor(ILI9341_RED);
        }
        else
        {
          tft.setTextColor(ILI9341_WHITE);
        }
        tft.printf("%4.0f", notches[0]);
        break;
      case MENU_NOTCH_1_BW:
        tft.setFont(Arial_12);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        BW_help = (float32_t)notches_BW[0] * bin_BW * 1000.0 * 2.0;
        BW_help = roundf(BW_help / 10) / 100 ; // round  frequency to the nearest 10Hz
        tft.printf("%3.0f", BW_help);
        break;
      /*            case MENU_NOTCH_2:
                      tft.setFont(Arial_12);
                      tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
                      if(notches_on[1])
                      {
                          tft.setTextColor(ILI9341_RED);
                      }
                      else
                      {
                          tft.setTextColor(ILI9341_WHITE);
                      }
                      sprintf(string,"%4.0f", notches[1]);
                      tft.print(string);
                  break;
                  case MENU_NOTCH_2_BW:
                      tft.setFont(Arial_11);
                      tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
                      BW_help = (float32_t)notches_BW[1] * bin_BW * 1000.0 * 2.0;
                      BW_help = roundf(BW_help / 10) / 100 ; // round  frequency to the nearest 10Hz
                      sprintf(string,"%3.0f", BW_help);
                      tft.print(string);
                  break;
      */
      case MENU_AGC_MODE:
        tft.setFont(Arial_10);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        switch (AGC_mode)
        {
          case 0:
            tft.print("OFF");
            break;
          case 1:
            tft.print("LON+");
            break;
          case 2:
            tft.print("LONG");
            break;
          case 3:
            tft.print("SLOW");
            break;
          case 4:
            tft.print("MED");
            break;
          case 5:
            tft.print("FAST");
            break;
        }
        break;
      case MENU_AGC_THRESH:
        tft.printf("%3d", bands[current_band].AGC_thresh);
        break;
      case MENU_AGC_DECAY:
        tft.printf("%3d", agc_decay / 10);
        break;
      case MENU_AGC_SLOPE:
        tft.printf("%3d", agc_slope);
        break;
      case MENU_ANR_NOTCH:
        tft.setTextColor(ANR_colour);
        tft.setFont(Arial_10);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        switch (ANR_notch)
        {
          case 0:
            tft.print("  NR");
            break;
          case 1:
            tft.print("Notch");
            break;
        }
        break;
      case MENU_ANR_TAPS:
        tft.setTextColor(ANR_colour);
        tft.printf("%3d", ANR_taps);
        break;
      case MENU_ANR_DELAY:
        tft.setTextColor(ANR_colour);
        tft.printf("%3d", ANR_delay);
        break;
      case MENU_ANR_MU:
        tft.setTextColor(ANR_colour);
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.printf("%1.3f", ANR_two_mu * 1000.0);
        break;
      case MENU_ANR_GAMMA:
        tft.setTextColor(ANR_colour);
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.printf("%4.0f", ANR_gamma * 1000.0);
        break;
      case MENU_NB_THRESH:
        tft.setFont(Arial_12);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        if (NB_on)
        {
          tft.setTextColor(ILI9341_RED);
        }
        else
        {
          tft.setTextColor(ILI9341_WHITE);
        }
        tft.printf("%2.1f", NB_thresh);
        break;
      case MENU_NB_TAPS:
        tft.setFont(Arial_12);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        if (NB_on)
        {
          tft.setTextColor(ILI9341_RED);
        }
        else
        {
          tft.setTextColor(ILI9341_WHITE);
        }
        tft.printf("%2d", NB_taps);
        break;
      case MENU_NB_IMPULSE_SAMPLES:
        tft.setFont(Arial_12);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        if (NB_test == 9)
        {
          tft.setTextColor(ILI9341_DARKGREEN);
        }
        else if (NB_test == 5)
        {
          tft.setTextColor(ILI9341_NAVY);
        }

        else
        {
          tft.setTextColor(ILI9341_WHITE);
        }
        tft.printf("%2d", NB_impulse_samples);
        break;
      case MENU_STEREO_FACTOR:
        tft.setFont(Arial_10);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.printf("%5.0f", stereo_factor);
        break;
      case MENU_BIT_NUMBER:
        tft.printf("%2d", bitnumber);
        break;
      /*      case MENU_NR_ENABLE:
              tft.setFont(Arial_10);
              tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
              switch (NR_enable)
              {
                case 0:
                  tft.print(" OFF ");
                  break;
                case 1:
                  tft.print(" ON  ");
                  break;
              }
              tft.setTextColor(ILI9341_WHITE);
              break; */
      case MENU_NR_USE_KIM:
        tft.setFont(Arial_10);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        switch (NR_Kim)
        {
          case 0:
            tft.print("  OFF");
            break;
          case 1:
            tft.print(" Kim ");
            break;
          case 2:
            tft.print("Roman");
            break;
          case 3:
            tft.print("le.LMS");
            break;
          case 4:
            tft.print(" LMS ");
            break;
        }
        break;
      /*      case MENU_NR_VAD_ENABLE:
              tft.setFont(Arial_10);
              tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
              switch (NR_VAD_enable)
              {
                case 0:
                  tft.print(" OFF ");
                  break;
                case 1:
                  tft.print(" ON ");
                  break;
              }
              break; */
      case MENU_NR_USE_X:
        tft.setFont(Arial_10);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        switch (NR_use_X)
        {
          case 0:
            tft.print(" E ");
            break;
          case 1:
            tft.print(" X  ");
            break;
        }
        break;

      /*      case MENU_NR_L:
              sprintf(menu_string, "%2d", NR_L_frames);
              tft.print(menu_string);
              break;
            case MENU_NR_N:
              sprintf(menu_string, "%2d", NR_N_frames);
              tft.print(menu_string);
              break; */
      case MENU_NR_ALPHA:
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.printf("%1.4f", NR_alpha);
        break;
      case MENU_NR_BETA:
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.printf("%1.4f", NR_beta);
        break;
      case MENU_NR_KIM_K:
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.printf("%1.4f", NR_KIM_K);
        break;
      case MENU_LMS_NR_STRENGTH:
        tft.printf("%2d", LMS_nr_strength);
        break;
      /*      case MENU_NR_VAD_THRESH:
              tft.setFont(Arial_11);
              tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
              sprintf(menu_string, "%5.1f", NR_VAD_thresh);
              tft.print(menu_string);
              break; */
      case MENU_NR_PSI:
        tft.setFont(Arial_11);
        tft.setCursor(spectrum_x + 256 + 8, spectrum_y + 31 + 31 + 7);
        tft.printf("%1.1f", NR_PSI);
        break;
    }
  }
  tft.setTextColor(ILI9341_WHITE);
  spectrum_y -= 2;
}


void set_tunestep()
{
  if (1) //bands[current_band].mode != DEMOD_WFM)
  {
    switch (tune_stepper)
    {
      case 0:
        if (current_band == BAND_MW || current_band == BAND_LW)
        {
          if (AM_SPACING_EU)
            tunestep = 9000;
          else
            tunestep = 10000;
        }
        else
        {
          tunestep = 5000;
        }
        break;
      case 1:
        tunestep = 100;
        break;
      case 2:
        tunestep = 500;
        break;
      case 3:
        tunestep = 1;
        break;
    }
  }
  else
  {
    switch (tune_stepper)
    {
      case 0:
        if (current_band == BAND_MW || current_band == BAND_LW)
        {
          tunestep = 3000;
        }
        else
        {
          tunestep = 5000 / 3;
        }
        break;
      case 1:
        tunestep = 100 / 3;
        break;
      case 2:
        tunestep = 500 / 3;
        break;
      case 3:
        tunestep = 1 / 3;
        break;
    }
  }
  /*

                  if(tune_stepper == 0)
                  if(band == BAND_MW || band == BAND_LW) tunestep = 9000; else tunestep = 5000;
                  else if (tune_stepper == 1) tunestep = 100;
                  else if (tune_stepper == 2) tunestep = 1000;
                  else if (tune_stepper == 3) tunestep = 1;
                  else tunestep = 5000;
                  if(band[bands].mode == DEMOD_WFM) tunestep =
  */
  show_tunestep();

}

void autotune() {
  // Lyons (2011): chapter 13.15 page 702
  // this uses the FFT_buffer DIRECTLY after the 1024 point FFT
  // and calculates the magnitudes
  // after that, for finetuning, a quadratic interpolation is performed
  // 1.) determine bins that are inside the filterbandwidth,
  //     depending on filter bandwidth AND bands[current_band].mode
  // 2.) calculate magnitudes from the real & imaginary values in the FFT buffer
  //     and bring them in the right order and put them into
  //     iFFT_buffer [that is recycled for this function and filled with other values afterwards]
  // 3.) perform carrier frequency estimation
  // 4.) tune to the estimated carrier frequency with an accuracy of 0.01Hz ;-)
  // --> in reality, we achieve about 0.2Hz accuracy, not bad

  const float32_t buff_len = FFT_length * 2.0;
  const float32_t bin_BW = (float32_t) (SR[SAMPLE_RATE].rate * 2.0 / DF / (buff_len));
  //    const int buff_len_int = FFT_length * 2;
  float32_t bw_LSB = 0.0;
  float32_t bw_USB = 0.0;
  //    float32_t help_freq = (float32_t)(bands[current_band].freq +  IF_FREQ * SI5351_FREQ_MULT);

  //  determine posbin (where we receive at the moment)
  // FFT_lengh is 1024
  // FFT_buffer is already frequency-translated !
  // so we do not need to worry about that IF stuff
  const int posbin = FFT_length / 2; //
  bw_LSB = -(float32_t)bands[current_band].FLoCut;
  bw_USB = (float32_t)bands[current_band].FHiCut;
  // include 500Hz of the other sideband into the search bandwidth
  if (bw_LSB < 1.0) bw_LSB = 500.0;
  if (bw_USB < 1.0) bw_USB = 500.0;

  //    Serial.print("bw_LSB = "); Serial.println(bw_LSB);
  //    Serial.print("bw_USB = "); Serial.println(bw_USB);

  // calculate upper and lower limit for determination of maximum magnitude
  const float32_t Lbin = (float32_t)posbin - round(bw_LSB / bin_BW);
  const float32_t Ubin = (float32_t)posbin + round(bw_USB / bin_BW); // the bin on the upper sideband side

  // put into second half of iFFT_buffer
  arm_cmplx_mag_f32(FFT_buffer, &iFFT_buffer[FFT_length], FFT_length);  // calculates sqrt(I*I + Q*Q) for each frequency bin of the FFT

  ////////////////////////////////////////////////////////////////////////
  // now bring into right order and copy in first half of iFFT_buffer
  ////////////////////////////////////////////////////////////////////////

  for (unsigned i = 0; i < FFT_length / 2; i++)
  {
    iFFT_buffer[i] = iFFT_buffer[i + FFT_length + FFT_length / 2];
  }
  for (unsigned i = FFT_length / 2; i < FFT_length; i++)
  {
    iFFT_buffer[i] = iFFT_buffer[i + FFT_length / 2];
  }

  //####################################################################
  if (autotune_flag == 1)
  {
    // look for maximum value and save the bin # for frequency delta calculation
    float32_t maximum = 0.0;
    float32_t maxbin = 1.0;
    float32_t delta = 0.0;

    for (int c = (int)Lbin; c <= (int)Ubin; c++)   // search for FFT bin with highest value = carrier and save the no. of the bin in maxbin
    {
      if (maximum < iFFT_buffer[c])
      {
        maximum = iFFT_buffer[c];
        maxbin = c;
      }
    }

    // ok, we have found the maximum, now save first delta frequency
    delta = (maxbin - (float32_t)posbin) * bin_BW;
    //        Serial.print("maxbin = ");
    //        Serial.println(maxbin);
    //        Serial.print("posbin = ");
    //        Serial.println(posbin);

    bands[current_band].freq = bands[current_band].freq  + (long long)(delta * SI5351_FREQ_MULT);
    setfreq();
    show_frequency(bands[current_band].freq, 1);
    //        Serial.print("delta = ");
    //        Serial.println(delta);
    autotune_flag = 2;
  }
  else
  {
    // ######################################################

    // and now: fine-tuning:
    //  get amplitude values of the three bins around the carrier

    float32_t bin1 = iFFT_buffer[posbin - 1];
    float32_t bin2 = iFFT_buffer[posbin];
    float32_t bin3 = iFFT_buffer[posbin + 1];

    if (bin1 + bin2 + bin3 == 0.0) bin1 = 0.00000001; // prevent divide by 0

    // estimate frequency of carrier by three-point-interpolation of bins around maxbin
    // formula by (Jacobsen & Kootsookos 2007) equation (4) P=1.36 for Hanning window FFT function
    // but we have unwindowed data here !
    // float32_t delta = (bin_BW * (1.75 * (bin3 - bin1)) / (bin1 + bin2 + bin3));
    // maybe this is the right equation for unwindowed magnitude data ?
    // performance is not too bad ;-)
    float32_t delta = (bin_BW * ((bin3 - bin1)) / (2 * bin2 - bin1 - bin3));
    if (delta > bin_BW) delta = 0.0; // just in case something went wrong

    bands[current_band].freq = bands[current_band].freq  + (long long)(delta * SI5351_FREQ_MULT);
    setfreq();
    show_frequency(bands[current_band].freq, 1);

    if (bands[current_band].mode == DEMOD_AUTOTUNE)
    {
      autotune_flag = 0;
    }
    else
    {
      // empirically derived: it seems good to perform the whole tuning some 5 to 10 times
      // in order to be perfect on the carrier frequency
      if (autotune_flag < 6)
      {
        autotune_flag++;
      }
      else
      {
        autotune_flag = 0;
        AudioNoInterrupts();
        Q_in_L.clear();
        Q_in_R.clear();
        AudioInterrupts();
      }
    }
    //        Serial.print("DELTA 2 = ");
    //        Serial.println(delta);
  }

} // end function autotune


void show_tunestep() {
  tft.fillRect(227, 25, 35, 21, ILI9341_BLACK);
  tft.setCursor(227, 25);
  tft.setFont(Arial_9);
  tft.setTextColor(ILI9341_GREEN);
  //tft.print("Step: ");
  if (tunestep == 9000)
  {
    tft.print("9k");
  }
  else if (tunestep == 5000)
  {
    tft.print("5k");
  }
  else
  {
    tft.print(tunestep);
  }
}

void set_band () {
  //         show_band(bands[current_band].name); // show new band
  old_demod_mode = -99; // used in setup_mode and when changing bands, so that LoCut and HiCut are not changed!
  setup_mode(bands[current_band].mode);
  sgtl5000_1.lineInLevel(bands[current_band].RFgain, bands[current_band].RFgain);
  //         setup_RX(bands[current_band].mode, bands[current_band].bandwidthU, bands[current_band].bandwidthL);  // set up the audio chain for new mode
  setfreq();
  show_frequency(bands[current_band].freq, 1);
  filter_bandwidth();
}

/* #########################################################################

    void setup_mode

    set up radio for RX modes - USB, LSB
   #########################################################################
*/

void setup_mode(int MO) {
  // bands[current_band].mode
  // USB -> LSB -> AM -> SAM -> SAM Stereo -> IQ -> WFM ->
  int temp;

  if (old_demod_mode != -99) // first time when radio is switched on and when changing bands
  {
    if (MO == DEMOD_LSB) // switch from USB to LSB
    {
      temp = bands[current_band].FHiCut;
      bands[current_band].FHiCut = - bands[current_band].FLoCut;
      bands[current_band].FLoCut = - temp;
    }
    else     if (MO == DEMOD_AM2) // switch from LSB to AM
    {
      bands[current_band].FHiCut = - bands[current_band].FLoCut;
    }
    else     if (MO == DEMOD_SAM_STEREO) // switch from SAM to SAM_STEREO
    {
      temp = bands[current_band].FHiCut;
      if (temp < - bands[current_band].FLoCut)
      {
        temp = - bands[current_band].FLoCut;
      }
      bands[current_band].FHiCut = temp;
      bands[current_band].FLoCut = temp;
    }
  }
  show_bandwidth();
  if (bands[current_band].mode == DEMOD_WFM)
  {
    tft.fillRect(spectrum_x + 256 + 2, pos_y_time + 17, 320 - spectrum_x - 258, 31, ILI9341_BLACK);
  }
  //    Q_in_L.clear();
  //    Q_in_R.clear();
  tft.fillRect(pos_x_frequency + 10, pos_y_frequency + 24, 210, 16, ILI9341_BLACK);
  freq_flag[0] = 0;
  //    show_notch((int)notches[0], bands[current_band].mode);
  old_demod_mode = bands[current_band].mode; // set old_mode flag for next time, at the moment only used for first time radio is switched on . . .
} // end void setup_mode


void set_samplerate ()
{
  AudioNoInterrupts();
  if (SAMPLE_RATE > SAMPLE_RATE_MAX) SAMPLE_RATE = SAMPLE_RATE_MAX;
  if (SAMPLE_RATE < SAMPLE_RATE_MIN) SAMPLE_RATE = SAMPLE_RATE_MIN;
  setI2SFreq (SR[SAMPLE_RATE].rate);
  delay(100);
  IF_FREQ = SR[SAMPLE_RATE].rate / 4;
  // if in WFM mode, reset the start frequency in order to have nice 25kHz steps
  if (bands[current_band].mode == DEMOD_WFM)
  {
    //          uint64_t prec_help = (95400000 - 0.75 * (uint64_t)SR[SAMPLE_RATE].rate) / 0.03;
    //          bands[current_band].freq = (unsigned long long)(prec_help);
    dt = 1.0 / (float32_t)SR[SAMPLE_RATE].rate;
    deemp_alpha = dt / (50e-6 + dt);
    onem_deemp_alpha = 1.0 - deemp_alpha;
    // IIR lowpass filter for wideband FM at 15k
    set_IIR_coeffs ((float32_t)15000, 0.54, (float32_t)SR[SAMPLE_RATE].rate, 0); // 1st stage
    for (int i = 0; i < 5; i++)
    { // fill coefficients into the right file
      biquad_WFM_coeffs[i] = coefficient_set[i];
      biquad_WFM_coeffs[i + 10] = coefficient_set[i];
    }
    set_IIR_coeffs ((float32_t)15000, 1.3, (float32_t)SR[SAMPLE_RATE].rate, 0); // 1st stage
    for (int i = 0; i < 5; i++)
    { // fill coefficients into the right file
      biquad_WFM_coeffs[i + 5] = coefficient_set[i];
      biquad_WFM_coeffs[i + 15] = coefficient_set[i];
    }

    // high Q IIR bandpass filter for wideband FM at 19k
    set_IIR_coeffs ((float32_t)19000, 1000.0, (float32_t)SR[SAMPLE_RATE].rate, 2); // 1st stage
    for (int i = 0; i < 5; i++)
    { // fill coefficients into the right file
      biquad_WFM_19k_coeffs[i] = coefficient_set[i];
    }

    // high Q IIR bandpass filter for wideband FM at 38k
    set_IIR_coeffs ((float32_t)38000, 1000.0, (float32_t)SR[SAMPLE_RATE].rate, 2); // 1st stage
    for (int i = 0; i < 5; i++)
    { // fill coefficients into the right file
      biquad_WFM_38k_coeffs[i] = coefficient_set[i];
    }
  }
  // this sets the frequency, but without knowing the IF!
  setfreq();
  prepare_spectrum_display(); // show new frequency scale
  //          LP_Fpass_old = 0; // cheat the filter_bandwidth function ;-)
  // before calculating the filter, we have to assure, that the filter bandwidth is not larger than
  // sample rate / 19.0
  // TODO: change bands[current_band].bandwidthU and L !!!

  //if(LP_F_help > SR[SAMPLE_RATE].rate / 19.0) LP_F_help = SR[SAMPLE_RATE].rate / 19.0;
  control_filter_f(); // check, if filter bandwidth is within bounds
  filter_bandwidth(); // calculate new FIR & IIR coefficients according to the new sample rate
  show_bandwidth();
  set_SAM_PLL();

  // NEW: this code is now in set_dec_int_filters() and is called by filter_bandwidth()
  AudioInterrupts();
}

void encoders () {
  static long encoder_pos = 0, last_encoder_pos = 0;
  long encoder_change;
  static long encoder2_pos = 0, last_encoder2_pos = 0;
  long encoder2_change;
  static long encoder3_pos = 0, last_encoder3_pos = 0;
  long encoder3_change;
  unsigned long long old_freq;

  // tune radio and adjust settings in menus using encoder switch
  encoder_pos = tune.read();

  if (encoder_pos != last_encoder_pos) {
    encoder_change = (encoder_pos - last_encoder_pos);
    last_encoder_pos = encoder_pos;

    if (((current_band == BAND_LW) || (current_band == BAND_MW)) && (tunestep == 5000))
    {
      tune_stepper = 0;
      set_tunestep();
      show_tunestep();
    }
    if (((current_band != BAND_LW) && (current_band != BAND_MW)) && (tunestep == 9000))
    {
      tune_stepper = 0;
      set_tunestep();
      show_tunestep();
    }

    if (tunestep == 1)
    {
      if (encoder_change <= 4 && encoder_change > 0) encoder_change = 4;
      else if (encoder_change >= -4 && encoder_change < 0) encoder_change = - 4;
    }
    long long tune_help1;
    if (bands[current_band].mode == DEMOD_WFM)
    { // hopefully tunes FM stations in 25kHz steps ;-)
      // 25000000 / 3 = 833333.3333f
      //
      tune_help1 = (long long)(833333.3333 * round((double)encoder_change / 4.0));
    }
    else
    {
      tune_help1 = (long long)tunestep  * SI5351_FREQ_MULT * (long long)round((double)encoder_change / 4.0);
    }
    //    long long tune_help1 = tunestep  * SI5351_FREQ_MULT * encoder_change;
    old_freq = bands[current_band].freq;
    bands[current_band].freq += (long long)tune_help1;  // tune the master vfo
    if (bands[current_band].freq > F_MAX) bands[current_band].freq = F_MAX;
    if (bands[current_band].freq < F_MIN) bands[current_band].freq = F_MIN;
    if (bands[current_band].freq != old_freq)
    {
      Q_in_L.clear();
      Q_in_R.clear();
      setfreq();
      show_frequency(bands[current_band].freq, 1);
      return;
    }
    show_menu();
  }
  ////////////////////////////////////////////////
  encoder2_pos = filter.read();
  if (encoder2_pos != last_encoder2_pos)
  {
    encoder2_change = (encoder2_pos - last_encoder2_pos);
    last_encoder2_pos = encoder2_pos;
    which_menu = 1;
    if (Menu_pointer == MENU_F_HI_CUT)
    {
      if (abs(bands[current_band].FHiCut) < 500)
      {
        bands[current_band].FHiCut = bands[current_band].FHiCut + encoder2_change * 12.5;
      }
      else
      {
        bands[current_band].FHiCut = bands[current_band].FHiCut + encoder2_change * 25.0;
      }
      control_filter_f();
      // set Menu2 to MENU_F_LO_CUT
      Menu2 = MENU_F_LO_CUT;
      filter_bandwidth();
      setfreq();
      show_frequency(bands[current_band].freq, 1);
    }
    else if (Menu_pointer == MENU_SPECTRUM_ZOOM)
    {
      //       if(encoder2_change < 0) spectrum_zoom--;
      //            else spectrum_zoom++;
      spectrum_zoom += (int)((float)encoder2_change / 4.0);
      //        Serial.println(encoder2_change);
      //        Serial.println((int)((float)encoder2_change / 4.0));
      if (spectrum_zoom > SPECTRUM_ZOOM_MAX) spectrum_zoom = SPECTRUM_ZOOM_MAX;
      //        if(spectrum_zoom == 4) spectrum_zoom = 9;
      //        if(spectrum_zoom == 8) spectrum_zoom = 3;
      if (spectrum_zoom < SPECTRUM_ZOOM_MIN) spectrum_zoom = SPECTRUM_ZOOM_MIN;
      Zoom_FFT_prep();
      //        Serial.print("ZOOM factor:  "); Serial.println(1<<spectrum_zoom);
      show_bandwidth ();
      FrequencyBarText();
      show_notch((int)notches[0], bands[current_band].mode);
    } // END Spectrum_zoom

    else if (Menu_pointer == MENU_IQ_AMPLITUDE)
    {
      //          K_dirty += (float32_t)encoder2_change / 1000.0;
      //          Serial.print("IQ Ampl corr factor:  "); Serial.println(K_dirty * 1000);
      //          Serial.print("encoder_change:  "); Serial.println(encoder2_change);

      IQ_amplitude_correction_factor += encoder2_change / 4000.0;
      //          Serial.print("IQ Ampl corr factor:  "); Serial.println(IQ_amplitude_correction_factor * 1000000);
      //          Serial.print("encoder_change:  "); Serial.println(encoder2_change);
    } // END IQadjust
    else if (Menu_pointer == MENU_SPECTRUM_BRIGHTNESS)
    {
      spectrum_brightness += encoder2_change / 4 * 10;
      if (spectrum_brightness > 255) spectrum_brightness = 255;
      if (spectrum_brightness < 10) spectrum_brightness = 10;
#if defined(BACKLIGHT_PIN)
      analogWrite(BACKLIGHT_PIN, spectrum_brightness);
#endif
    } //
    else if (Menu_pointer == MENU_SAMPLE_RATE)
    {
      //          if(encoder2_change < 0) SAMPLE_RATE--;
      //            else SAMPLE_RATE++;
#ifdef DEBUG
      Serial.println(encoder2_change);
      Serial.println((float)encoder2_change / 4.0);
#endif
      if (encoder2_change < -1)
      {
        SAMPLE_RATE -= 1;
      }
      else if (encoder2_change > 1)
      {
        SAMPLE_RATE += 1;
      }

      wait_flag = 1;
      set_samplerate ();
    }
    else if (Menu_pointer == MENU_LPF_SPECTRUM)
    {
      LPF_spectrum += encoder2_change / 400.0;
      if (LPF_spectrum < 0.00001) LPF_spectrum = 0.00001;
      if (LPF_spectrum > 1.0) LPF_spectrum = 1.0;
    } // END LPFSPECTRUM
    else if (Menu_pointer == MENU_SPECTRUM_OFFSET)   // added pixel offsets  <PUA>
    {
        bands[current_band].pixel_offset += (float)encoder2_change;
/*      offsetDisplayDB += 0.25 * displayScale[currentScale].offsetIncrement * (float)encoder2_change; // 4 changes per detent
      if (offsetDisplayDB > 100.0)       // This offset is in dB
        offsetDisplayDB = 100.0;
      else if (offsetDisplayDB < 0.0)
        offsetDisplayDB = 0;
      //      offsetPixels = displayScale[currentScale].baseOffset + (int16_t)(offsetDisplayDB*displayScale[currentScale].pixelsPerDB);
      bands[current_band].pixel_offset = displayScale[currentScale].baseOffset + (int16_t)(offsetDisplayDB * displayScale[currentScale].pixelsPerDB);
*/
      showSpectrumCorners();
    }
    else if (Menu_pointer == MENU_SPECTRUM_DISPLAY_SCALE)   // Redone to 1/2/5/10/20 steps  <PUA>
    {
      currentScale -= (long)((float)encoder2_change / 4.0);
      // wait_flag = 1;
      if (currentScale > 4)
        currentScale = 4;
      else if (currentScale < 0)
        currentScale = 0;
      //////
      //      offsetPixels = displayScale[currentScale].baseOffset + (int16_t)(offsetDisplayDB*displayScale[currentScale].pixelsPerDB);
      //bands[current_band].pixel_offset = displayScale[currentScale].baseOffset + (int16_t)(offsetDisplayDB * displayScale[currentScale].pixelsPerDB);
      ///////

      showSpectrumCorners();
    }
    else if (Menu_pointer == MENU_IQ_PHASE)
    {
      //          P_dirty += (float32_t)encoder2_change / 1000.0;
      //          Serial.print("IQ Phase corr factor:  "); Serial.println(P_dirty * 1000);
      //          Serial.print("encoder_change:  "); Serial.println(encoder2_change);
      IQ_phase_correction_factor = IQ_phase_correction_factor + (float32_t)encoder2_change / 4000.0;
      //          Serial.print("IQ Phase corr factor"); Serial.println(IQ_phase_correction_factor * 1000000);

    } // END IQadjust
    else if (Menu_pointer == MENU_CALIBRATION_FACTOR)
    {
      calibration_factor += encoder2_change * 25;
    } // END CALIBRATION_FACTOR
    else if (Menu_pointer == MENU_CALIBRATION_CONSTANT)
    {
      calibration_constant += encoder2_change * 25;
      si5351.set_correction(calibration_constant, SI5351_PLL_INPUT_XO);
      //  si5351.init(SI5351_CRYSTAL_LOAD_10PF, Si_5351_crystal, calibration_constant);
    } // END CALIBRATION_FACTOR
    else if (Menu_pointer == MENU_TIME_SET) {
      helpmin = minute(); helphour = hour();
      helpmin = helpmin + encoder2_change / 4;
      if (helpmin > 59) {
        helpmin = 0; helphour = helphour + 1;
      }
      if (helpmin < 0) {
        helpmin = 59; helphour = helphour - 1;
      }
      if (helphour < 0) helphour = 23;
      if (helphour > 23) helphour = 0;
      helpmonth = month(); helpyear = year(); helpday = day();
      setTime (helphour, helpmin, 0, helpday, helpmonth, helpyear);
      Teensy3Clock.set(now()); // set the RTC
    } // end TIMEADJUST
    else if (Menu_pointer == MENU_DATE_SET) {
      helpyear = year();
      helpmonth = month();
      helpday = day();
      helpday = helpday + encoder2_change / 4;
      if (helpday < 1) {
        helpday = 31;
        helpmonth = helpmonth - 1;
      }
      if (helpday > 31) {
        helpmonth = helpmonth + 1;
        helpday = 1;
      }
      if (helpmonth < 1) {
        helpmonth = 12;
        helpyear = helpyear - 1;
      }
      if (helpmonth > 12) {
        helpmonth = 1;
        helpyear = helpyear + 1;
      }
      helphour = hour(); helpmin = minute(); helpsec = second();
      setTime (helphour, helpmin, helpsec, helpday, helpmonth, helpyear);
      Teensy3Clock.set(now()); // set the RTC
      displayDate();
    } // end DATEADJUST


    show_menu();
    //        tune.write(0);
  } // end encoder2 was turned

  encoder3_pos = encoder3.read();
  if (encoder3_pos != last_encoder3_pos)
  {
    encoder3_change = (encoder3_pos - last_encoder3_pos);
    last_encoder3_pos = encoder3_pos;
    which_menu = 2;
    if (Menu2 == MENU_RF_GAIN)
    {
      if (auto_codec_gain == 1)
      {
        auto_codec_gain = 0;
        Menus[MENU_RF_GAIN].text2 = "  gain  ";
        //          Serial.println ("auto = 0");
      }
      bands[current_band].RFgain = bands[current_band].RFgain + encoder3_change / 4;
      if (bands[current_band].RFgain < 0)
      {
        auto_codec_gain = 1; //Serial.println ("auto = 1");
        Menus[MENU_RF_GAIN].text2 = " AUTO  ";
        bands[current_band].RFgain = 0;
      }
      if (bands[current_band].RFgain > 15)
      {
        bands[current_band].RFgain = 15;
      }
      sgtl5000_1.lineInLevel(bands[current_band].RFgain);
    }
    else if (Menu2 == MENU_VOLUME)
    {
      audio_volume = audio_volume + encoder3_change;
      if (audio_volume < 0) audio_volume = 0;
      else if (audio_volume > 100) audio_volume = 100;
      //      AudioNoInterrupts();
      sgtl5000_1.volume((float32_t)audio_volume / 100.0);

    }
    else if (Menu2 == MENU_RF_ATTENUATION)
    {
      RF_attenuation = RF_attenuation + encoder3_change / 4;
      if (RF_attenuation < 0) RF_attenuation = 0;
      else if (RF_attenuation > 31) RF_attenuation = 31;
      setAttenuator(RF_attenuation);
    }
    else if (Menu2 == MENU_SAM_ZETA)
    {
      zeta_help = zeta_help + (float32_t)encoder3_change / 4.0;
      if (zeta_help < 15) zeta_help = 15;
      else if (zeta_help > 99) zeta_help = 99;
      set_SAM_PLL();
    }
    else if (Menu2 == MENU_SAM_OMEGA)
    {
      omegaN = omegaN + (float32_t)encoder3_change * 10 / 4.0;
      if (omegaN < 20.0) omegaN = 20.0;
      else if (omegaN > 1000.0) omegaN = 1000.0;
      set_SAM_PLL();
    }
    else if (Menu2 == MENU_SAM_CATCH_BW)
    {
      pll_fmax = pll_fmax + (float32_t)encoder3_change * 100.0 / 4.0;
      if (pll_fmax < 200.0) pll_fmax = 200.0;
      else if (pll_fmax > 6000.0) pll_fmax = 6000.0;
      set_SAM_PLL();
    }
    else if (Menu2 == MENU_NOTCH_1)
    {
      notches[0] = notches[0] + (float32_t)encoder3_change * 10 / 4.0;
      if (notches[0] < -9900.0) //
      {
        notches[0] = -9900.0;
        //          notches_on[0] = 0;
        show_menu();
        return;
      }
      else if (notches[0] > 9900.0) notches[0] = 9900.0;
      //      notches_on[0] = 1;
      show_notch((int)notches[0], bands[current_band].mode);
      oldnotchF = notches[0];
    }
    else if (Menu2 == MENU_NOTCH_1_BW)
    {
      notches_BW[0] = notches_BW[0] + (float32_t)encoder3_change / 4;
      if (notches_BW[0] < 1)
      {
        notches_BW[0] = 1;
      }
      else if (notches_BW[0] > 300) notches_BW[0] = 300;
      show_notch((int)notches[0], bands[current_band].mode);
      oldnotchF = notches[0];
    }
    /*    else if(Menu2 == MENU_NOTCH_2)
        {
          notches[1] = notches[1] + (float32_t)encoder3_change * 10 / 4.0;
          if(notches[1] < 100.0)
          {
              notches[1] = 100.0;
              notches_on[1] = 0;
              show_menu();
              return;
          }
          else if(notches[1] > 9900.0) notches[1] = 9900.0;
          notches_on[1] = 1;
          show_notch((int)notches[0], bands[current_band].mode);
          oldnotchF = notches[1];
        }
        else if(Menu2 == MENU_NOTCH_2_BW)
        {
          notches_BW[1] = notches_BW[1] + (float32_t)encoder3_change / 4;
          if(notches_BW[1] < 2)
          {
              notches_BW[1] = 2;
          }
          else if(notches_BW[1] > 100) notches[1] = 100;
          show_notch((int)notches[0], bands[current_band].mode);
          oldnotchF = notches[0];
        }
    */
    else if (Menu2 == MENU_AGC_MODE)
    {
      AGC_mode = AGC_mode + (float32_t)encoder3_change / 4.0;
      if (AGC_mode > 5) AGC_mode = 5;
      else if (AGC_mode < 0) AGC_mode = 0;
      agc_switch_mode = 1;
      AGC_prep();
    }
    else if (Menu2 == MENU_AGC_THRESH)
    {
      bands[current_band].AGC_thresh = bands[current_band].AGC_thresh + encoder3_change / 4.0;
      if (bands[current_band].AGC_thresh < -20) bands[current_band].AGC_thresh = -20;
      else if (bands[current_band].AGC_thresh > 120) bands[current_band].AGC_thresh = 120;
      AGC_prep();
    }
    else if (Menu2 == MENU_AGC_DECAY)
    {
      agc_decay = agc_decay + encoder3_change * 100.0 / 4.0;
      if (agc_decay < 100) agc_decay = 100;
      else if (agc_decay > 5000) agc_decay = 5000;
      AGC_prep();
    }
    else if (Menu2 == MENU_AGC_SLOPE)
    {
      agc_slope = agc_slope + encoder3_change * 10.0 / 4.0;
      if (agc_slope < 0) agc_slope = 0;
      else if (agc_slope > 200) agc_slope = 200;
      AGC_prep();
    }
    else if (Menu2 == MENU_STEREO_FACTOR)
    {
      stereo_factor = stereo_factor + encoder3_change * 20.0 / 4.0;
      if (stereo_factor < 0.0) stereo_factor = 0.0;
      else if (stereo_factor > 1000.0) stereo_factor = 1000.0;
    }
    else if (Menu2 == MENU_BASS)
    {
      bass = bass + (float32_t)encoder3_change / 80.0;
      if (bass > 1.0) bass = 1.0;
      else if (bass < -1.0) bass = -1.0;
      sgtl5000_1.eqBands (bass, midbass, mid, midtreble, treble); // (float bass, etc.) in % -100 to +100
      //      sgtl5000_1.eqBands (bass, treble); // (float bass, float treble) in % -100 to +100
    }
    else if (Menu2 == MENU_MIDBASS)
    {
      midbass = midbass + (float32_t)encoder3_change / 80.0;
      if (midbass > 1.0) midbass = 1.0;
      else if (midbass < -1.0) midbass = -1.0;
      sgtl5000_1.eqBands (bass, midbass, mid, midtreble, treble); // (float bass, etc.) in % -100 to +100
    }
    else if (Menu2 == MENU_MID)
    {
      mid = mid + (float32_t)encoder3_change / 80.0;
      if (mid > 1.0) mid = 1.0;
      else if (mid < -1.0) mid = -1.0;
      sgtl5000_1.eqBands (bass, midbass, mid, midtreble, treble); // (float bass, etc.) in % -100 to +100
    }
    else if (Menu2 == MENU_MIDTREBLE)
    {
      midtreble = midtreble + (float32_t)encoder3_change / 80.0;
      if (midtreble > 1.0) midtreble = 1.0;
      else if (midtreble < -1.0) midtreble = -1.0;
      sgtl5000_1.eqBands (bass, midbass, mid, midtreble, treble); // (float bass, etc.) in % -100 to +100
    }
    else if (Menu2 == MENU_TREBLE)
    {
      treble = treble + (float32_t)encoder3_change / 80.0;
      if (treble > 1.0) treble = 1.0;
      else if (treble < -1.0) treble =  -1.0;
      sgtl5000_1.eqBands (bass, midbass, mid, midtreble, treble); // (float bass, etc.) in % -100 to +100
      //      sgtl5000_1.eqBands (bass, treble); // (float bass, float treble) in % -100 to +100
    }
    else if (Menu2 == MENU_SPECTRUM_DISPLAY_SCALE)
    {
      if (spectrum_display_scale < 100.0) spectrum_display_scale = spectrum_display_scale + (float32_t)encoder3_change / 4.0;
      else spectrum_display_scale = spectrum_display_scale + (float32_t)encoder3_change * 5.0;
      if (spectrum_display_scale > 2000.0) spectrum_display_scale = 2000.0;
      else if (spectrum_display_scale < 1.0) spectrum_display_scale =  1.0;
    }
    else if (Menu2 == MENU_BIT_NUMBER)
    {
      bitnumber = bitnumber + encoder3_change / 4.0;
      if (bitnumber > 16) bitnumber = 16;
      else if (bitnumber < 3) bitnumber = 3;
    }
    else if (Menu2 == MENU_ANR_TAPS)
    {
      ANR_taps = ANR_taps + encoder3_change / 4.0;
      if (ANR_taps < ANR_delay) ANR_taps = ANR_delay;
      if (ANR_taps < 16) ANR_taps = 16;
      else if (ANR_taps > 128) ANR_taps = 128;
    }
    else if (Menu2 == MENU_ANR_DELAY)
    {
      ANR_delay = ANR_delay + encoder3_change / 4.0;
      if (ANR_delay > ANR_taps) ANR_delay = ANR_taps;
      if (ANR_delay < 2) ANR_delay = 2;
      else if (ANR_delay > 128) ANR_delay = 128;
    }
    else if (Menu2 == MENU_ANR_MU)
    {
      if (encoder3_change > 0) ANR_two_mu *= 2.0;
      else ANR_two_mu /= 2.0;
      if (ANR_two_mu < 1.0e-07) ANR_two_mu = 1.0e-7;
      else if (ANR_two_mu > 8.192e-02) ANR_two_mu = 8.192e-02;
    }
    else if (Menu2 == MENU_ANR_GAMMA)
    {
      if (encoder3_change > 0) ANR_gamma *= 2.0;
      else ANR_gamma /= 2.0;
      //      ANR_two_mu = ANR_two_mu + encoder3_change / 4000000.0;
      if (ANR_gamma < 1.0e-03) ANR_gamma = 1.0e-3;
      else if (ANR_gamma > 8.192) ANR_gamma = 8.192;
    }
    else if (Menu2 == MENU_NB_THRESH)
    {
      NB_thresh = NB_thresh + (float32_t)encoder3_change / 40.0;
      if (NB_thresh > 20.0) NB_thresh = 20.0;
      else if (NB_thresh < 0.1) NB_thresh =  0.1;
    }
    else if (Menu2 == MENU_NB_TAPS)
    {
      NB_taps = NB_taps + encoder3_change / 4.0;
      if (NB_taps > 40) NB_taps = 40;
      else if (NB_taps < 6) NB_taps =  6;
    }
    else if (Menu2 == MENU_NB_IMPULSE_SAMPLES)
    {
      NB_impulse_samples = NB_impulse_samples + (float32_t)encoder3_change / 20.0;
      if (NB_impulse_samples > 41) NB_impulse_samples = 41;
      else if (NB_impulse_samples < 3) NB_impulse_samples =  3;
    }
    else if (Menu2 == MENU_F_LO_CUT)
    {
      if (abs(bands[current_band].FLoCut) < 500)
      {
        bands[current_band].FLoCut = bands[current_band].FLoCut + encoder3_change * 12.5;
      }
      else
      {
        bands[current_band].FLoCut = bands[current_band].FLoCut + encoder3_change * 25.0;
      }
      control_filter_f();
      filter_bandwidth();
      setfreq();
      show_frequency(bands[current_band].freq, 1);
    }
    /*    else if (Menu2 == MENU_NR_L)
        {
          NR_L_frames = NR_L_frames + encoder3_change / 4.0;
          if (NR_L_frames > 6) NR_L_frames = 6;
          else if (NR_L_frames < 1) NR_L_frames = 1;
        }
        else if (Menu2 == MENU_NR_N)
        {
          NR_N_frames = NR_N_frames + encoder3_change / 4.0;
          if (NR_N_frames > 40) NR_N_frames = 40;
          else if (NR_N_frames < 5) NR_N_frames = 5;
        } */
    else if (Menu2 == MENU_NR_PSI)
    {
      NR_PSI = NR_PSI + (float32_t)encoder3_change / 16;
      if (NR_PSI < 0.2) NR_PSI = 0.2;
      else if (NR_PSI > 20.0) NR_PSI = 20.0;
    }
    else if (Menu2 == MENU_NR_ALPHA)
    {
      NR_alpha = NR_alpha + (float32_t)encoder3_change / 800.0;
      if (NR_alpha < 0.7) NR_alpha = 0.7;
      else if (NR_alpha > 0.999) NR_alpha = 0.999;
      NR_onemalpha = (1.0 - NR_alpha);
    }
    else if (Menu2 == MENU_NR_BETA)
    {
      NR_beta = NR_beta + (float32_t)encoder3_change / 800.0;
      if (NR_beta < 0.1) NR_beta = 0.1;
      else if (NR_beta > 0.999) NR_beta = 0.999;
      NR_onemtwobeta = (1.0 - (2.0 * NR_beta));
      NR_onembeta = 1.0 - NR_beta;
    }
    else if (Menu2 == MENU_NR_KIM_K)
    {
      NR_KIM_K = NR_KIM_K + (float32_t)encoder3_change / 800.0;
      if (NR_KIM_K < 0.8) NR_KIM_K = 0.8;
      else if (NR_KIM_K > 1.0) NR_KIM_K = 1.0;
    }
    else if (Menu2 == MENU_LMS_NR_STRENGTH)
    {
      LMS_nr_strength = LMS_nr_strength + encoder3_change / 4.0;
      if (LMS_nr_strength < 0) LMS_nr_strength = 0;
      else if (LMS_nr_strength > 40) LMS_nr_strength = 40;
      Init_LMS_NR ();
    }
    /*    else if (Menu2 == MENU_NR_VAD_THRESH)
      {
      NR_VAD_thresh = NR_VAD_thresh + (float32_t)encoder3_change / 16.0;
      if (NR_VAD_thresh < 0.1) NR_VAD_thresh = 0.1;
      else if (NR_VAD_thresh > 1000.0) NR_VAD_thresh = 1000.0;
      } */

    show_menu();
    //    encoder3.write(0);

  }
}

#define DISPLAY_ANALOG_CLOCK 1
void displayClock()
{
  uint8_t hour10 = hour() / 10 % 10;
  uint8_t hour1 = hour() % 10;
  uint8_t minute10 = minute() / 10 % 10;
  uint8_t minute1 = minute() % 10;
  uint8_t second10 = second() / 10 % 10;
  uint8_t second1 = second() % 10;
  uint8_t time_pos_shift = 12; // distance between figures
  uint8_t dp = 7; // distance between ":" and figures

  /*  if (mesz != mesz_old && mesz >= 0) {
      tft.setTextColor(ILI9341_ORANGE);
      tft.setFont(Arial_16);
      tft.setCursor(pos_x_date, pos_y_date+20);
      tft.fillRect(pos_x_date, pos_y_date+20, 150-pos_x_date, 20, ILI9341_BLACK);
      tft.printf((mesz==0)?"(CET)":"(CEST)");
    }
  */

  if (!DISPLAY_ANALOG_CLOCK)
  {
    tft.setFont(Arial_14);
    tft.setTextColor(ILI9341_WHITE);

    // set up ":" for time display
    if (!timeflag) {
      tft.setCursor(pos_x_time + 2 * time_pos_shift, pos_y_time);
      tft.print(":");
      tft.setCursor(pos_x_time + 4 * time_pos_shift + dp, pos_y_time);
      tft.print(":");
      tft.setCursor(pos_x_time + 7 * time_pos_shift + 2 * dp, pos_y_time);
      //      tft.print("UTC");
    }

    if (hour10 != hour10_old || !timeflag) {
      tft.setCursor(pos_x_time, pos_y_time);
      tft.fillRect(pos_x_time, pos_y_time, time_pos_shift, time_pos_shift + 2, ILI9341_BLACK);
      if (hour10) tft.print(hour10);  // do not display, if zero
    }
    if (hour1 != hour1_old || !timeflag) {
      tft.setCursor(pos_x_time + time_pos_shift, pos_y_time);
      tft.fillRect(pos_x_time  + time_pos_shift, pos_y_time, time_pos_shift, time_pos_shift + 2, ILI9341_BLACK);
      tft.print(hour1);  // always display
    }
    if (minute1 != minute1_old || !timeflag) {
      tft.setCursor(pos_x_time + 3 * time_pos_shift + dp, pos_y_time);
      tft.fillRect(pos_x_time  + 3 * time_pos_shift + dp, pos_y_time, time_pos_shift, time_pos_shift + 2, ILI9341_BLACK);
      tft.print(minute1);  // always display
    }
    if (minute10 != minute10_old || !timeflag) {
      tft.setCursor(pos_x_time + 2 * time_pos_shift + dp, pos_y_time);
      tft.fillRect(pos_x_time  + 2 * time_pos_shift + dp, pos_y_time, time_pos_shift, time_pos_shift + 2, ILI9341_BLACK);
      tft.print(minute10);  // always display
    }
    if (second10 != second10_old || !timeflag) {
      tft.setCursor(pos_x_time + 4 * time_pos_shift + 2 * dp, pos_y_time);
      tft.fillRect(pos_x_time  + 4 * time_pos_shift + 2 * dp, pos_y_time, time_pos_shift, time_pos_shift + 2, ILI9341_BLACK);
      tft.print(second10);  // always display
    }
    if (second1 != second1_old || !timeflag) {
      tft.setCursor(pos_x_time + 5 * time_pos_shift + 2 * dp, pos_y_time);
      tft.fillRect(pos_x_time  + 5 * time_pos_shift + 2 * dp, pos_y_time, time_pos_shift, time_pos_shift + 2, ILI9341_BLACK);
      tft.print(second1);  // always display
    }
    tft.setFont(Arial_10);
  }
  else
  {
    // put code for analog clock here
    if (second1 != second1_old)
    {
      clock_draw_second(second());
    }
    if ((minute1 != minute1_old || !timeflag) || (second() == (minute() + 1)) || (second() == (minute() + 2)))
    {
      clock_draw_minute(minute());
      clock_draw_hour(hour(), minute());
    }
  }

  hour1_old = hour1;
  hour10_old = hour10;
  minute1_old = minute1;
  minute10_old = minute10;
  second1_old = second1;
  second10_old = second10;
  mesz_old = mesz;
  timeflag = 1;
} // end function displayTime

#define clock_circle_size 21

void Init_Display_Clock()
{
  // Draw Clockface
  for (int16_t i = 0; i < 2; i++)
  {
    tft.drawCircle(pos_x_a_time, pos_y_a_time, clock_circle_size - i, ILI9341_WHITE);
  }
  for (int16_t i = 0; i < 2; i++)
  {
    tft.drawCircle(pos_x_a_time, pos_y_a_time, i, ILI9341_WHITE);
  }

  // Draw a small mark for every hour
  for (int i = 0; i < 12; i++)
  {
    clock_draw_marks(i);
  }
}

void clock_draw_marks(int hour)
{
  float x1, y1, x2, y2;

  hour = hour * 30;
  hour = hour + 270;
  float COS = cosf(hour * 0.0175f);
  float SIN = sinf(hour * 0.0175f);
  x1 = (clock_circle_size - 2) * COS;
  y1 = (clock_circle_size - 2) * SIN;
  x2 = (clock_circle_size - 6) * COS;
  y2 = (clock_circle_size - 6) * SIN;

  tft.drawLine(x1 + pos_x_a_time, y1 + pos_y_a_time, x2 + pos_x_a_time, y2 + pos_y_a_time, ILI9341_WHITE);
}

/*
   drawLine(int16_t x0, int16_t y0,
  int16_t x1, int16_t y1, uint16_t color)
*/

void clock_draw_second(int s)
{
  static float x1 = 0;
  static float y1 = 0;
  static float x2 = 0;
  static float y2 = 0;

  s = s * 6;
  s = s + 270;

  {
    tft.drawLine(x1 + pos_x_a_time, y1 + pos_y_a_time, x2 + pos_x_a_time, y2 + pos_y_a_time, ILI9341_BLACK);
  }

  float COS = cosf(s * 0.0175f);
  float SIN = sinf(s * 0.0175f);

  x1 = (clock_circle_size - 7) * COS; // outer
  y1 = (clock_circle_size - 7) * SIN;
  x2 = 2 * COS; // inner
  y2 = 2 * SIN;

  //   if (!((s % 5) == 0))
  { // prevent overwriting hour marks
    tft.drawLine(x1 + pos_x_a_time, y1 + pos_y_a_time, x2 + pos_x_a_time, y2 + pos_y_a_time, ILI9341_RED);
  }
}

void clock_draw_minute (int m)
{
  static float x1 = 0;
  static float y1 = 0;
  static float x2 = 0;
  static float y2 = 0;

  m = m * 6;
  m = m + 270;

  tft.drawLine(x1 + pos_x_a_time, y1 + pos_y_a_time, x2 + pos_x_a_time, y2 + pos_y_a_time, ILI9341_BLACK);

  float COS = cosf(m * 0.0175f);
  float SIN = sinf(m * 0.0175f);

  x1 = (clock_circle_size - 6) * COS;
  y1 = (clock_circle_size - 6) * SIN;
  x2 = 2 * COS;
  y2 = 2 * SIN;
  // x3=10*cosf((m+8)*0.0175);
  // y3=10*sinf((m+8)*0.0175);
  // x4=10*cosf((m-8)*0.0175);
  // y4=10*sinf((m-8)*0.0175);

  tft.drawLine(x1 + pos_x_a_time, y1 + pos_y_a_time, x2 + pos_x_a_time, y2 + pos_y_a_time, ILI9341_WHITE);

  //  tft.drawLine(x1+pos_x_a_time, y1+pos_y_a_time, x3+pos_x_a_time, y3+pos_y_a_time, ILI9341_WHITE);
  //  tft.drawLine(x3+pos_x_a_time, y3+pos_y_a_time, x2+pos_x_a_time, y2+pos_y_a_time, ILI9341_WHITE);
  //  tft.drawLine(x2+pos_x_a_time, y2+pos_y_a_time, x4+pos_x_a_time, y4+pos_y_a_time, ILI9341_WHITE);
  //  tft.drawLine(x4+pos_x_a_time, y4+pos_y_a_time, x1+pos_x_a_time, y1+pos_y_a_time, ILI9341_WHITE);
}

void clock_draw_hour(int h, int m)
{
  static float x1 = 0;
  static float y1 = 0;
  static float x2 = 0;
  static float y2 = 0;
  static float x3 = 0;
  static float y3 = 0 ;
  static float x4 = 0;
  static float y4 = 0;

  h = (h * 30) + (m / 2);
  h = h + 270;

  tft.drawLine(x1 + pos_x_a_time, y1 + pos_y_a_time, x3 + pos_x_a_time, y3 + pos_y_a_time, ILI9341_BLACK);
  tft.drawLine(x3 + pos_x_a_time, y3 + pos_y_a_time, x2 + pos_x_a_time, y2 + pos_y_a_time, ILI9341_BLACK);
  tft.drawLine(x2 + pos_x_a_time, y2 + pos_y_a_time, x4 + pos_x_a_time, y4 + pos_y_a_time, ILI9341_BLACK);
  tft.drawLine(x4 + pos_x_a_time, y4 + pos_y_a_time, x1 + pos_x_a_time, y1 + pos_y_a_time, ILI9341_BLACK);

  float COS = cosf(h * 0.0175f);
  float SIN = sinf(h * 0.0175f);

  x1 = (clock_circle_size - 10) * COS;
  y1 = (clock_circle_size - 10) * SIN;
  x2 = 2 * COS;
  y2 = 2 * SIN;

  COS = cosf((h + 12) * 0.0175f);
  SIN = sinf((h + 12) * 0.0175f);

  x3 = 6 * COS;
  y3 = 6 * SIN;

  COS = cosf((h - 12) * 0.0175f);
  SIN = sinf((h - 12) * 0.0175f);

  x4 = 6 * COS;
  y4 = 6 * SIN;

  tft.drawLine(x1 + pos_x_a_time, y1 + pos_y_a_time, x3 + pos_x_a_time, y3 + pos_y_a_time, ILI9341_WHITE);
  tft.drawLine(x3 + pos_x_a_time, y3 + pos_y_a_time, x2 + pos_x_a_time, y2 + pos_y_a_time, ILI9341_WHITE);
  tft.drawLine(x2 + pos_x_a_time, y2 + pos_y_a_time, x4 + pos_x_a_time, y4 + pos_y_a_time, ILI9341_WHITE);
  tft.drawLine(x4 + pos_x_a_time, y4 + pos_y_a_time, x1 + pos_x_a_time, y1 + pos_y_a_time, ILI9341_WHITE);
}

void displayDate() {
  tft.fillRect(pos_x_date, pos_y_date, 320 - pos_x_date, 20, ILI9341_BLACK); // erase old string
  tft.setTextColor(ILI9341_ORANGE);
  tft.setFont(Arial_12);
  tft.setCursor(pos_x_date, pos_y_date);
  tft.printf("%s, %02d.%02d.%04d", Days[weekday() % 7], day(), month(), year());
} // end function displayDate

void set_SAM_PLL() {
  // DX adjustments: zeta = 0.15, omegaN = 100.0
  // very stable, but does not lock very fast
  // standard settings: zeta = 1.0, omegaN = 250.0
  // maybe user can choose between slow (DX), medium, fast SAM PLL
  // zeta / omegaN
  // DX = 0.2, 70
  // medium 0.6, 200
  // fast 1.2, 500
  //float32_t zeta = 0.8; // PLL step response: smaller, slower response 1.0 - 0.1
  //float32_t omegaN = 250.0; // PLL bandwidth 50.0 - 1000.0

  omega_min = TPI * (-pll_fmax) * DF / SR[SAMPLE_RATE].rate;
  omega_max = TPI * pll_fmax * DF / SR[SAMPLE_RATE].rate;
  zeta = (float32_t)zeta_help / 100.0;
  g1 = 1.0 - expf(-2.0 * omegaN * zeta * DF / SR[SAMPLE_RATE].rate);
  g2 = - g1 + 2.0 * (1 - expf(- omegaN * zeta * DF / SR[SAMPLE_RATE].rate) * cosf(omegaN * DF / SR[SAMPLE_RATE].rate * sqrtf(1.0 - zeta * zeta)));

  mtauR = expf(- DF / (SR[SAMPLE_RATE].rate * tauR));
  onem_mtauR = 1.0 - mtauR;
  mtauI = expf(- DF / (SR[SAMPLE_RATE].rate * tauI));
  onem_mtauI = 1.0 - mtauI;
}

#if defined(MP3)
void playFileMP3(const char *filename)
{
  trackchange = true; //auto track change is allowed.
  // Start playing the file.  This sketch continues to
  // run while the file plays.
  printTrack();
#if defined(EEPROM_h)
  EEPROM.write(eeprom_adress, track); //meanwhile write the track position to eeprom address 0
#endif
  //  Serial.println("After EEPROM.write");
  playMp3.play(filename);
  // Simply wait for the file to finish playing.
  while (playMp3.isPlaying()) {
    show_load();
    buttons();
    encoders();
    displayClock();
  }
}

void playFileAAC(const char *filename)
{
  trackchange = true; //auto track change is allowed.
  // Start playing the file.  This sketch continues to
  // run while the file plays.
  // print track no & trackname
  printTrack ();
#if defined(EEPROM_h)
  EEPROM.write(eeprom_adress, track); //meanwhile write the track position to eeprom address 0
#endif
  playAac.play(filename);
  // Simply wait for the file to finish playing.
  while (playAac.isPlaying()) {
    // update controls!
    show_load();
    buttons();
    encoders();
    displayClock();
  }
}

void nexttrack() {
#ifdef DEBUG
  Serial.println("Next track!");
#endif
  trackchange = false; // we are doing a track change here, so the auto trackchange will not skip another one.
  playMp3.stop();
  playAac.stop();
  track++;
  if (track >= tracknum) { // keeps in tracklist.
    track = 0;
  }
  tracklist[track].toCharArray(playthis, sizeof(tracklist[track])); //since we have to convert String to Char will do this
}

void prevtrack() {
#ifdef DEBUG
  Serial.println("Previous track! ");
#endif
  trackchange = false; // we are doing a track change here, so the auto trackchange will not skip another one.
  playMp3.stop();
  playAac.stop();
  track--;
  if (track < 0) { // keeps in tracklist.
    track = tracknum - 1;
  }
  tracklist[track].toCharArray(playthis, sizeof(tracklist[track])); //since we have to convert String to Char will do this
}

void pausetrack() {
  paused = !paused;
  playMp3.pause(paused);
  playAac.pause(paused);
}


void randomtrack() {
#ifdef DEBUG
  Serial.println("Random track!");
#endif
  trackchange = false; // we are doing a track change here, so the auto trackchange will not skip another one.
  if (trackext[track] == 1) playMp3.stop();
  if (trackext[track] == 2) playAac.stop();

  track = random(tracknum);

  tracklist[track].toCharArray(playthis, sizeof(tracklist[track])); //since we have to convert String to Char will do this
}

void printTrack () {
  tft.fillRect(0, 222, 320, 17, ILI9341_BLACK);
  tft.setCursor(0, 222);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.print("Track: ");
  tft.print (track);
  tft.print (" "); tft.print (playthis);
} //end printTrack

#endif //MP3

void show_load() {
  if (five_sec.check() == 1)
  {
#ifdef DEBUG
    Serial.print("Proc = ");
    Serial.print(AudioProcessorUsage());
    Serial.print(" (");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("),  Mem = ");
    Serial.print(AudioMemoryUsage());
    Serial.print(" (");
    Serial.print(AudioMemoryUsageMax());
    Serial.println(")");
#endif
    AudioProcessorUsageMaxReset();
    AudioMemoryUsageMaxReset();
  }
}

float32_t sign(float32_t x) {
  if (x < 0)
    return -1.0f;
  else if (x > 0)
    return 1.0f;
  else return 0.0f;
}

void Calculatedbm()
{
  // calculation of the signal level inside the filter bandwidth
  // taken from the spectrum display FFT
  // taking into account the analog gain before the ADC
  // analog gain is adjusted in steps of 1.5dB
  // bands[current_band].RFgain = 0 --> 0dB gain
  // bands[current_band].RFgain = 15 --> 22.5dB gain

  // spectrum display is generated from 256 samples based on 1024 samples of the FIR FFT . . .
  // could this cause errors in the calculation of the signal strength ?

  //  float32_t slope = 19.8; //
  float32_t slope = 10.0; //
  float32_t cons = -92; //
  float32_t Lbin, Ubin;
  float32_t bw_LSB = 0.0;
  float32_t bw_USB = 0.0;
  //            float64_t sum_db = 0.0; // FIXME: mabye this slows down the FPU, because the FPU does only process 32bit floats ???
  float32_t sum_db = 0.0; // FIXME: mabye this slows down the FPU, because the FPU does only process 32bit floats ???
  int posbin = 0;
  // bin_bandwidth = samplerate / 256bins
  float32_t bin_bandwidth = (float32_t) (SR[SAMPLE_RATE].rate / (256.0));
  // width of a 256 tap FFT bin @ 96ksps = 375Hz
  // we have to take into account the magnify mode
  // --> recalculation of bin_BW
  bin_bandwidth = bin_bandwidth / (1 << spectrum_zoom); // correct bin bandwidth is determined by the Zoom FFT display setting
  //            Serial.print("bin_bandwidth = "); Serial.println(bin_bandwidth);

  // in all magnify cases (2x up to 16x) the posbin is in the centre of the spectrum display
  if (spectrum_zoom != 0)
  {
    posbin = 128; // right in the middle!
  }
  else
  {
    posbin = 64;
  }

  //  determine Lbin and Ubin from ts.dmod_mode and FilterInfo.width
  //  = determine bandwith separately for lower and upper sideband
#if 0
  switch (bands[current_band].mode)
  {
    case DEMOD_LSB:
    case DEMOD_SAM_LSB:
      bw_USB = 0.0;
      bw_LSB = (float32_t)bands[current_band].bandwidthL;
      break;
    case DEMOD_USB:
    case DEMOD_SAM_USB:
      bw_LSB = 0.0;
      bw_USB = (float32_t)bands[current_band].bandwidthU;
      break;
    default:
      bw_LSB = (float32_t)bands[current_band].bandwidthL;
      bw_USB = (float32_t)bands[current_band].bandwidthU;
  }
#endif

  bw_LSB = bands[current_band].FLoCut;
  bw_USB = bands[current_band].FHiCut;
  // calculate upper and lower limit for determination of signal strength
  // = filter passband is between the lower bin Lbin and the upper bin Ubin
  Lbin = (float32_t)posbin + roundf(bw_LSB / bin_bandwidth); // bin on the lower/left side
  Ubin = (float32_t)posbin + roundf(bw_USB / bin_bandwidth); // bin on the upper/right side

  // take care of filter bandwidths that are larger than the displayed FFT bins
  if (Lbin < 0)
  {
    Lbin = 0;
  }
  if (Ubin > 255)
  {
    Ubin = 255;
  }
  //Serial.print("Lbin = "); Serial.println(Lbin);
  //Serial.print("Ubin = "); Serial.println(Ubin);
  if ((int)Lbin == (int)Ubin)
  {
    Ubin = 1.0 + Lbin;
  }
  // determine the sum of all the bin values in the passband
  for (int c = (int)Lbin; c <= (int)Ubin; c++)   // sum up all the values of all the bins in the passband
  {
    //                sum_db = sum_db + FFT_spec[c];
    sum_db = sum_db + FFT_spec_old[c];
  }

#ifdef USE_W7PUA
  if (sum_db > 0.0)
  {
#ifdef USE_LOG10FAST
    switch (display_dbm)
    {
      case DISPLAY_S_METER_DBM:
        dbm = dbm_calibration + bands[current_band].gainCorrection + (float32_t)RF_attenuation +
              slope * log10f_fast(sum_db) + cons - (float32_t)bands[current_band].RFgain * 1.5;
        dbmhz = 0;
        break;
      case DISPLAY_S_METER_DBMHZ:
        dbmhz = dbm - 10.0 * log10f_fast((float32_t)(((int)Ubin - (int)Lbin) * bin_BW));
        dbm = 0;
        break;
    }
#else
    switch (display_dbm)
    {
      case DISPLAY_S_METER_DBM:
        dbm = dbm_calibration + bands[current_band].gainCorrection + (float32_t)RF_attenuation +
              slope * log10f(sum_db) + cons - (float32_t)bands[current_band].RFgain * 1.5;
        dbmhz = 0;
        break;
      case DISPLAY_S_METER_DBMHZ:
        dbmhz = dbm - 10.0 * log10f((float32_t)(((int)Ubin - (int)Lbin) * bin_BW));
        dbm = 0;
        break;
    }
#endif
  }
#else
  if (sum_db > 0)
  {
#ifdef USE_LOG10FAST
    switch (display_dbm)
    {
      case DISPLAY_S_METER_DBM:
        dbm = dbm_calibration + (float32_t)RF_attenuation + slope * log10f_fast (sum_db) + cons - (float32_t)bands[current_band].RFgain * 1.5;
        dbmhz = 0;
        break;
      case DISPLAY_S_METER_DBMHZ:
        dbmhz = (float32_t)RF_attenuation +  - (float32_t)bands[current_band].RFgain * 1.5 + slope * log10f_fast (sum_db) -  10 * log10f_fast ((float32_t)(((int)Ubin - (int)Lbin) * bin_BW)) + cons;
        dbm = 0;
        break;
    }
#else
    switch (display_dbm)
    {
      case DISPLAY_S_METER_DBM:
        dbm = dbm_calibration + (float32_t)RF_attenuation + slope * log10f (sum_db) + cons - (float32_t)bands[current_band].RFgain * 1.5;
        dbmhz = 0;
        break;
      case DISPLAY_S_METER_DBMHZ:
        dbmhz = (float32_t)RF_attenuation +  - (float32_t)bands[current_band].RFgain * 1.5 + slope * log10f (sum_db) -  10 * log10f ((float32_t)(((int)Ubin - (int)Lbin) * bin_BW)) + cons;
        dbm = 0;
        break;
    }
#endif
  }
#endif
  else
  {
    dbm = -140.0;
    dbmhz = -165.0;
  }

  // lowpass IIR filter
  // Wheatley 2011: two averagers with two time constants
  // IIR filter with one element analog to 1st order RC filter
  // but uses two different time constants (ALPHA = 1 - e^(-T/Tau)) depending on
  // whether the signal is increasing (attack) or decreasing (decay)
  // m_AttackAlpha = 0.8647; //  ALPHA = 1 - e^(-T/Tau), T = 0.02s (because dbm routine is called every 20ms!)
  // Tau = 10ms = 0.01s attack time
  // m_DecayAlpha = 0.0392; // 500ms decay time
  //
  m_AttackAvedbm = (1.0 - m_AttackAlpha) * m_AttackAvedbm + m_AttackAlpha * dbm;
  m_DecayAvedbm = (1.0 - m_DecayAlpha) * m_DecayAvedbm + m_DecayAlpha * dbm;
  m_AttackAvedbmhz = (1.0 - m_AttackAlpha) * m_AttackAvedbmhz + m_AttackAlpha * dbmhz;
  m_DecayAvedbmhz = (1.0 - m_DecayAlpha) * m_DecayAvedbmhz + m_DecayAlpha * dbmhz;

  if (m_AttackAvedbm > m_DecayAvedbm)
  { // if attack average is larger then it must be an increasing signal
    m_AverageMagdbm = m_AttackAvedbm; // use attack average value for output
    m_DecayAvedbm = m_AttackAvedbm; // set decay average to attack average value for next time
  }
  else
  { // signal is decreasing, so use decay average value
    m_AverageMagdbm = m_DecayAvedbm;
  }

  if (m_AttackAvedbmhz > m_DecayAvedbmhz)
  { // if attack average is larger then it must be an increasing signal
    m_AverageMagdbmhz = m_AttackAvedbmhz; // use attack average value for output
    m_DecayAvedbmhz = m_AttackAvedbmhz; // set decay average to attack average value for next time
  }
  else
  { // signal is decreasing, so use decay average value
    m_AverageMagdbmhz = m_DecayAvedbmhz;
  }

  dbm = m_AverageMagdbm; // write average into variable for S-meter display
  dbmhz = m_AverageMagdbmhz; // write average into variable for S-meter display

}

void Display_dbm()
{
  //    static int dbm_old = (int)dbm;
  uint8_t display_something = 0;
  float32_t val_dbm = 0.0;
  const char* unit_label;
  float32_t dbuv;

  switch (display_dbm)
  {
    case DISPLAY_S_METER_DBM:
      display_something = 1;
      val_dbm = dbm;
      unit_label = "dBm   ";
      break;
    case DISPLAY_S_METER_DBMHZ:
      display_something = 0;
      val_dbm = dbmhz;
      unit_label = "dBm/Hz";
      break;
  }
  if ( abs(dbm - dbm_old) < 0.1) display_something = 0;

  if (display_something == 1)
  {
    /////////////////////////////////////////////////////////////////////////
    tft.fillRect(pos_x_dbm, pos_y_dbm, 100, 16, ILI9341_BLACK);
    // Added tenths of a dB and moved "dBm" right 10 oixels  <PUA>
    tft.setFont(Arial_14);
    tft.setCursor(pos_x_dbm, pos_y_dbm);
    tft.setTextColor(ILI9341_WHITE);
    tft.printf("%03.1f", val_dbm);
    tft.setFont(Arial_9);
    tft.setCursor(pos_x_dbm + 56, pos_y_dbm + 5);
    tft.setTextColor(ILI9341_GREEN);
    tft.print(unit_label);
    dbm_old = dbm;
    ///////////////////////////////////////////////////////////////////////////

    float32_t s = 9.0 + ((dbm + 73.0) / 6.0);
    if (s < 0.0) s = 0.0;
    if ( s > 9.0)
    {
      dbuv = dbm + 73.0;
      s = 9.0;
    }
    else dbuv = 0.0;
    int16_t pos_sxs_w = pos_x_smeter + s * s_w + 2;
    int16_t sxs_w = s * s_w + 2;
    int16_t nine_sw_minus = (9 * s_w + 1) - s * s_w;
    //            tft.drawFastHLine(pos_x_smeter, pos_y_smeter, s*s_w+1, ILI9341_BLUE);
    //            tft.drawFastHLine(pos_x_smeter+s*s_w+1, pos_y_smeter, nine_sw_minus, ILI9341_BLACK);

    tft.drawFastHLine(pos_x_smeter + 1, pos_y_smeter + 1, sxs_w, ILI9341_BLUE);
    tft.drawFastHLine(pos_sxs_w, pos_y_smeter + 1, nine_sw_minus, ILI9341_BLACK);
    tft.drawFastHLine(pos_x_smeter + 1, pos_y_smeter + 2, sxs_w, ILI9341_WHITE);
    tft.drawFastHLine(pos_sxs_w, pos_y_smeter + 2, nine_sw_minus, ILI9341_BLACK);
    tft.drawFastHLine(pos_x_smeter + 1, pos_y_smeter + 3, sxs_w, ILI9341_WHITE);
    tft.drawFastHLine(pos_sxs_w, pos_y_smeter + 3, nine_sw_minus, ILI9341_BLACK);
    tft.drawFastHLine(pos_x_smeter + 1, pos_y_smeter + 4, sxs_w, ILI9341_BLUE);
    tft.drawFastHLine(pos_sxs_w, pos_y_smeter + 4, nine_sw_minus, ILI9341_BLACK);
    //            tft.drawFastHLine(pos_x_smeter, pos_y_smeter+5, sxs_w, ILI9341_BLUE);
    //            tft.drawFastHLine(pos_x_smeter+s*s_w+1, pos_y_smeter+5, nine_sw_minus, ILI9341_BLACK);

    if (dbuv > 30) dbuv = 30;
    if (dbuv > 0)
    {
      int16_t dbuv_div_x = dbuv * s_w / 5 + 1;
      int16_t six_sw_minus = (6 * s_w + 1) - dbuv * s_w / 5 ;
      int16_t nine_sw_plus_x = pos_x_smeter + 9 * s_w + (dbuv / 5) * s_w + 1;
      int16_t nine_sw_plus = pos_x_smeter + 9 * s_w + 1;
      tft.drawFastHLine(nine_sw_plus, pos_y_smeter, dbuv_div_x, ILI9341_RED);
      tft.drawFastHLine(nine_sw_plus_x, pos_y_smeter, six_sw_minus, ILI9341_BLACK);
      tft.drawFastHLine(nine_sw_plus, pos_y_smeter + 1, dbuv_div_x, ILI9341_RED);
      tft.drawFastHLine(nine_sw_plus_x, pos_y_smeter + 1, six_sw_minus, ILI9341_BLACK);
      tft.drawFastHLine(nine_sw_plus, pos_y_smeter + 2, dbuv_div_x, ILI9341_RED);
      tft.drawFastHLine(nine_sw_plus_x, pos_y_smeter + 2, six_sw_minus, ILI9341_BLACK);
      tft.drawFastHLine(nine_sw_plus, pos_y_smeter + 3, dbuv_div_x, ILI9341_RED);
      tft.drawFastHLine(nine_sw_plus_x, pos_y_smeter + 3, six_sw_minus, ILI9341_BLACK);
      tft.drawFastHLine(nine_sw_plus, pos_y_smeter + 4, dbuv_div_x, ILI9341_RED);
      tft.drawFastHLine(nine_sw_plus_x, pos_y_smeter + 4, six_sw_minus, ILI9341_BLACK);
      tft.drawFastHLine(nine_sw_plus, pos_y_smeter + 5, dbuv_div_x, ILI9341_RED);
      tft.drawFastHLine(nine_sw_plus_x, pos_y_smeter + 5, six_sw_minus, ILI9341_BLACK);
    }
  }
  // AGC box
  if (agc_action == 1)
  {
    tft.setCursor(pos_x_dbm + 98, pos_y_dbm + 4);
    tft.setFont(Arial_11);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("AGC");
  }
  else
  {
    tft.fillRect(pos_x_dbm + 98, pos_y_dbm + 4, 100, 16, ILI9341_BLACK);
  }
}

#define CONFIG_VERSION "mr1"  //mdrhere ID of the E settings block, change if structure changes
#define CONFIG_START 0       //where to start the EEPROM data

struct config_t {
  unsigned long long calibration_factor;
  long calibration_constant;
  unsigned long long freq[NUM_BANDS];
  int mode[NUM_BANDS];
  int bwu[NUM_BANDS];
  int bwl[NUM_BANDS];
  int rfg[NUM_BANDS];
  int AGC_thresh[NUM_BANDS];
  int16_t pixel_offset[NUM_BANDS];
  int current_band;
  float32_t LPFcoeff;
  int audio_volume;
  int8_t AGC_mode;
  float32_t pll_fmax;
  float32_t omegaN;
  int zeta_help;
  uint8_t rate;
  float32_t bass;
  float32_t treble;
  int agc_thresh;
  int agc_decay;
  int agc_slope;
  uint8_t auto_IQ_correction;
  float32_t midbass;
  float32_t mid;
  float32_t midtreble;
  int8_t RF_attenuation;
  uint8_t show_spectrum_flag;
  float32_t stereo_factor;
  float32_t spectrum_display_scale;
  int32_t spectrum_zoom;
  uint8_t NR_use_X;
  //    uint8_t NR_L_frames;
  //    uint8_t NR_N_frames;
  float32_t NR_PSI;
  float32_t NR_alpha;
  float32_t NR_beta;
  float32_t offsetDisplayDB;
  uint16_t currentScale;
  char version_of_settings[4];  // validation string
  uint16_t crc;   // added when saving
} E;

void EEPROM_LOAD() { //mdrhere
  config_t E;
  if (loadFromEEPROM(&E) == true) {
    gEEPROM_current = true;
    //printConfig_t(&E);  //for debugging
    calibration_factor = E.calibration_factor;
    calibration_constant = E.calibration_constant;
    for (int i = 0; i < (NUM_BANDS); i++)
      bands[i].freq = E.freq[i];
    for (int i = 0; i < (NUM_BANDS); i++)
      bands[i].mode = E.mode[i];
    for (int i = 0; i < (NUM_BANDS); i++)
      bands[i].FHiCut = E.bwu[i];
    for (int i = 0; i < (NUM_BANDS); i++)
      bands[i].FLoCut = E.bwl[i];
    for (int i = 0; i < (NUM_BANDS); i++)
      bands[i].RFgain = E.rfg[i];
    for (int i = 0; i < (NUM_BANDS); i++)
      bands[i].AGC_thresh = E.AGC_thresh[i];
    for (int i = 0; i < (NUM_BANDS); i++)
      bands[i].pixel_offset = E.pixel_offset[i];
    current_band = E.current_band;
    //I_help = E.I_ampl;
    //Q_in_I_help = E.Q_in_I;
    //I_in_Q_help = E.I_in_Q;
    //Window_FFT = E.Window_FFT;
    LPF_spectrum = E.LPFcoeff;
    audio_volume = E.audio_volume;
    AGC_mode = E.AGC_mode;
    pll_fmax = E.pll_fmax;
    omegaN = E.omegaN;
    zeta_help = E.zeta_help;
    zeta = (float32_t) zeta_help / 100.0;
    SAMPLE_RATE = E.rate;
    bass = E.bass;
    treble = E.treble;
    agc_thresh = E.agc_thresh;
    agc_decay = E.agc_decay;
    agc_slope = E.agc_slope;
    auto_IQ_correction = E.auto_IQ_correction;
    midbass = E.midbass;
    mid = E.mid;
    midtreble = E.midtreble;
    RF_attenuation = E.RF_attenuation;
    show_spectrum_flag = E.show_spectrum_flag;
    stereo_factor = E.stereo_factor;
    spectrum_display_scale = E.spectrum_display_scale;
    spectrum_zoom = E.spectrum_zoom;
    NR_use_X = E.NR_use_X;
    //  NR_L_frames = E.NR_L_frames;
    //  NR_N_frames = E.NR_N_frames;
    NR_PSI = E.NR_PSI;
    NR_alpha = E.NR_alpha;
    NR_beta = E.NR_beta;
    offsetDisplayDB = E.offsetDisplayDB;
    currentScale = E.currentScale;
  } else {
    gEEPROM_current = false;
  }
}

void EEPROM_SAVE() {
  config_t E;
  E.calibration_factor = calibration_factor;
  E.current_band = current_band;
  E.calibration_constant = calibration_constant;
  for (int i = 0; i < (NUM_BANDS); i++)
    E.freq[i] = bands[i].freq;
  for (int i = 0; i < (NUM_BANDS); i++)
    E.mode[i] = bands[i].mode;
  for (int i = 0; i < (NUM_BANDS); i++)
    E.bwu[i] = bands[i].FHiCut;
  for (int i = 0; i < (NUM_BANDS); i++)
    E.bwl[i] = bands[i].FLoCut;
  for (int i = 0; i < (NUM_BANDS); i++)
    E.rfg[i] = bands[i].RFgain;
  for (int i = 0; i < (NUM_BANDS); i++)
    E.AGC_thresh[i] = bands[i].AGC_thresh;
  for (int i = 0; i < (NUM_BANDS); i++)
    E.pixel_offset[i] = bands[i].pixel_offset;
  //      E.I_ampl = I_help;
  //      E.Q_in_I = Q_in_I_help;
  //      E.I_in_Q = I_in_Q_help;
  //      E.Window_FFT = Window_FFT;
  //      E.LPFcoeff = LPFcoeff;
  E.LPFcoeff = LPF_spectrum;
  E.audio_volume = audio_volume;
  E.AGC_mode = AGC_mode;
  E.pll_fmax = pll_fmax;
  E.omegaN = omegaN;
  E.zeta_help = zeta_help;
  E.rate = SAMPLE_RATE;
  E.bass = bass;
  E.treble = treble;
  E.agc_thresh = agc_thresh;
  E.agc_decay = agc_decay;
  E.agc_slope = agc_slope;
  E.auto_IQ_correction = auto_IQ_correction;
  E.midbass = midbass;
  E.mid = mid;
  E.midtreble = midtreble;
  E.RF_attenuation = RF_attenuation;
  E.show_spectrum_flag = show_spectrum_flag;
  E.stereo_factor = stereo_factor;
  E.spectrum_display_scale = spectrum_display_scale;
  E.spectrum_zoom = spectrum_zoom;
  E.NR_use_X = NR_use_X;
  //  E.NR_L_frames = NR_L_frames;
  //  E.NR_N_frames = NR_N_frames;
  E.NR_PSI = NR_PSI;
  E.NR_alpha = NR_alpha;
  E.NR_beta = NR_beta;
  E.offsetDisplayDB = offsetDisplayDB;
  E.currentScale = currentScale;

  //eeprom_write_block (&E, 0, sizeof(E));
  char theversion[] = CONFIG_VERSION;
  for (int i = 0; i < 4; i++)
    E.version_of_settings[i] = theversion[i];
  E.crc = 0; //will be overwritten
  //printConfig_t(&E);  //for debugging
  saveInEEPROM(&E);
} // end void eeProm SAVE

boolean loadFromEEPROM(struct config_t *ls) {  //mdrhere
#if defined(EEPROM_h)
  char this_version[] = CONFIG_VERSION;
  unsigned char thechar = 0;
  uint8_t thecrc = 0;
  config_t ts, *ts_ptr;  //temp struct and ptr to hold the data
  ts_ptr = &ts;

  // To make sure there are settings, and they are YOURS! Load the settings and do the crc check first
  for (unsigned int t = 0; t < (sizeof(config_t) - 1); t++) {
    thechar = EEPROM.read(CONFIG_START + t);
    *((char*)ts_ptr + t) = thechar;
    thecrc = _crc_ibutton_update(thecrc, thechar);
  }
  if (thecrc == 0) { // have valid data
    //printConfig_t(ts_ptr);
    Serial.printf("Found EEPROM version %s", ts_ptr->version_of_settings);  //line continued after version
    if (ts.version_of_settings[3] == this_version[3] &&    // If the latest version
        ts.version_of_settings[2] == this_version[2] &&
        ts.version_of_settings[1] == this_version[1] &&
        ts.version_of_settings[0] == this_version[0] ) {
      for (int i = 0; i < (int)sizeof(config_t); i++) { //copy data to location passed in
        *((unsigned char*)ls + i) = *((unsigned char*)ts_ptr + i);
      }
      Serial.println(", loaded");
      return true;
    } else { // settings are old version
      Serial.printf(", not loaded, current version is %s\n", this_version);
      return false;
    }
  } else {
    Serial.println("Bad CRC, settings not loaded");
    return false;
  }
#else
  return false;
#endif
}

boolean saveInEEPROM(struct config_t *pd) {
#if defined(EEPROM_h)
  int byteswritten = 0;
  uint8_t thecrc = 0;
  boolean errors = false;
  unsigned int t;
  for (t = 0; t < (sizeof(config_t) - 2); t++) { // writes to EEPROM
    thecrc = _crc_ibutton_update(thecrc, *((unsigned char*)pd + t) );
    if ( EEPROM.read(CONFIG_START + t) != *((unsigned char*)pd + t) ) { //only if changed
      EEPROM.write(CONFIG_START + t, *((unsigned char*)pd + t));
      // and verifies the data
      if (EEPROM.read(CONFIG_START + t) != *((unsigned char*)pd + t))
      {
        errors = true; //error writing (or reading) exit
        break;
      } else {
        //Serial.print("EEPROM ");Serial.println(t);
        byteswritten += 1; //for debuggin
      }
    }
  }
  EEPROM.write(CONFIG_START + t, thecrc);   //write the crc to the end of the data
  if (EEPROM.read(CONFIG_START + t) != thecrc)  //and check it
    errors = true;
  if (errors == true) {
    Serial.println(" error writing to EEPROM");
  } else {
    Serial.printf("%d bytes saved to EEPROM version %s \n", byteswritten, CONFIG_VERSION);  //note: only changed written
  }
  return errors;
#else
  return false;
#endif
}

void printConfig_t(struct config_t *c) { //print some of the values for testing
  Serial.printf("%llu", c->calibration_factor);
  Serial.println(c->calibration_constant);
  Serial.println(c->current_band);
  Serial.println(c->LPFcoeff);
  Serial.println(c->audio_volume);
  Serial.println(c->AGC_mode);
  Serial.println(c->pll_fmax);
  Serial.println(c->omegaN);
  Serial.println(c->zeta_help);
  Serial.println(c->rate);
}

/*
  void set_freq_conv2(float32_t NCO_FREQ) {
  //  float32_t NCO_FREQ = AUDIO_SAMPLE_RATE_EXACT / 16; // + 20;
  float32_t NCO_INC = 2 * PI * NCO_FREQ / AUDIO_SAMPLE_RATE_EXACT;
  float32_t OSC_COS = cos (NCO_INC);
  float32_t OSC_SIN = sin (NCO_INC);
  float32_t Osc_Vect_Q = 1.0;
  float32_t Osc_Vect_I = 0.0;
  float32_t Osc_Gain = 0.0;
  float32_t Osc_Q = 0.0;
  float32_t Osc_I = 0.0;
  float32_t i_temp = 0.0;
  float32_t q_temp = 0.0;
  }
*/
/*
  void freq_conv2()
  {
  uint     i;
      for(i = 0; i < BUFFER_SIZE; i++) {
        // generate local oscillator on-the-fly:  This takes a lot of processor time!
        Osc_Q = (Osc_Vect_Q * OSC_COS) - (Osc_Vect_I * OSC_SIN);  // Q channel of oscillator
        Osc_I = (Osc_Vect_I * OSC_COS) + (Osc_Vect_Q * OSC_SIN);  // I channel of oscillator
        Osc_Gain = 1.95 - ((Osc_Vect_Q * Osc_Vect_Q) + (Osc_Vect_I * Osc_Vect_I));  // Amplitude control of oscillator
        // rotate vectors while maintaining constant oscillator amplitude
        Osc_Vect_Q = Osc_Gain * Osc_Q;
        Osc_Vect_I = Osc_Gain * Osc_I;
        //
        // do actual frequency conversion
        float_buffer_L[i] = (float_buffer_L_3[i] * Osc_Q) + (float_buffer_R_3[i] * Osc_I);  // multiply I/Q data by sine/cosine data to do translation
        float_buffer_R[i] = (float_buffer_R_3[i] * Osc_Q) - (float_buffer_L_3[i] * Osc_I);
        //
      }
  } // end freq_conv2()

*/


void reset_codec ()
{
  AudioNoInterrupts();
  sgtl5000_1.disable();
  delay(10);
  sgtl5000_1.enable();
  delay(10);
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.adcHighPassFilterDisable(); // does not help too much!
  sgtl5000_1.lineInLevel(bands[current_band].RFgain);
  sgtl5000_1.lineOutLevel(24);
  sgtl5000_1.audioPostProcessorEnable(); // enables the DAP chain of the codec post audio processing before the headphone out
  sgtl5000_1.eqSelect (3); // Tone Control
  sgtl5000_1.eqBands (bass, midbass, mid, midtreble, treble); // in % -100 to +100
  sgtl5000_1.enhanceBassEnable();
  sgtl5000_1.dacVolumeRamp();
  sgtl5000_1.volume((float32_t)audio_volume / 100.0); //
  twinpeaks_tested = 3;
  AudioInterrupts();

}

void setAttenuator(int value)
{
#if defined(ATT_LE)
  // bit-banging of the digital step attenuator chip PE4306
  // allows 0 to 31dB RF attenuation in 1dB steps
  // inspired by https://github.com/jefftranter/Arduino/blob/master/pe4306/pe4306.ino
  int level; // Holds level of DATA line when shifting

  if (value < 0) value = 0;
  if (value > 31) value = 31;

  digitalWrite(ATT_LE, LOW);
  digitalWrite(ATT_CLOCK, LOW);

  for (int bit = 5; bit >= 0; bit--) {
    if (bit == 0)
    {
      level = 0; // B0 has to be set to zero
    }
    else
    { // left shift of 1, because B0 has to be zero and value starts at B1
      // then right shift by the "bit"-variable value to write the specific bit
      // what does &0x01 do? --> sets the specific bit to a binary 1
      level = ((value << 1) >> bit) & 0x01; // Level is value of bit
    }

    digitalWrite(ATT_DATA, level); // Write data value
    digitalWrite(ATT_CLOCK, HIGH); // Toggle clock high and then low
    digitalWrite(ATT_CLOCK, LOW);
  }
  digitalWrite(ATT_LE, HIGH); // Toggle LE high to enable latch
  digitalWrite(ATT_LE, LOW);  // and then low again to hold it.
#endif
}

void show_analog_gain()
{
  static uint8_t RF_gain_old = 0;
  static uint8_t RF_att_old = 0;
  const uint16_t col = ILI9341_GREEN;
  // automatic RF gain indicated by different colors??
  if ((((bands[current_band].RFgain != RF_gain_old) || (RF_attenuation != RF_att_old)) && twinpeaks_tested == 1) || write_analog_gain)
  {
    tft.setCursor(pos_x_time - 40, pos_y_time + 26);
    tft.setFont(Arial_8);
    tft.setTextColor(ILI9341_BLACK);
    tft.printf("%02.1fdB -", (float)(RF_gain_old * 1.5));
    tft.setCursor(pos_x_time - 40, pos_y_time + 26);
    tft.setTextColor(col);
    tft.printf("%02.1fdB -", (float)(bands[current_band].RFgain * 1.5));
    tft.setCursor(pos_x_time, pos_y_time + 26);
    tft.setTextColor(ILI9341_BLACK);
    tft.printf(" %2ddB", RF_att_old);
    tft.setCursor(pos_x_time, pos_y_time + 26);
    tft.setTextColor(col);
    tft.printf(" %2ddB = ", RF_attenuation);
    tft.setCursor(pos_x_time + 40, pos_y_time + 24);
    tft.setFont(Arial_9);
    tft.setTextColor(ILI9341_BLACK);
    tft.printf("%02.1fdB", (float)(RF_gain_old * 1.5) - (float)RF_att_old);
    tft.setCursor(pos_x_time + 40, pos_y_time + 24);
    tft.setTextColor(ILI9341_WHITE);
    tft.printf("%02.1fdB", (float)(bands[current_band].RFgain * 1.5) - (float)RF_attenuation);
    RF_gain_old = bands[current_band].RFgain;
    RF_att_old = RF_attenuation;
    write_analog_gain = 0;
  }
}

void calc_notch_bins()
{
  float32_t spectrum_span = SR[SAMPLE_RATE].rate / 1000.0 / (float32_t)(1 << spectrum_zoom);
  const uint16_t sp_width = 256;
  float32_t magni = sp_width / spectrum_span;
  bin_BW = 0.0001220703125 * SR[SAMPLE_RATE].rate; // 1/8192 * sample rate --> 1024 point FFT AND decimation-by-8 = 8192
  // calculate notch centre bin for FFT1024
  bin = notches[0] / bin_BW;
  // calculate bins (* 2) for deletion of bins in the iFFT_buffer
  // set iFFT_buffer[notch_L] to iFFT_buffer[notch_R] to zero
  if (notches[0] > 0)
  {
    notch_L[0] = (int16_t)(2 * roundf((float32_t)bin - (notches_BW[0] / 2.0)));
    notch_R[0] = (int16_t)(2 * roundf((float32_t)bin + (notches_BW[0] / 2.0)));
  }
  else
  {
    notch_L[0] = (int16_t)(2 * roundf((float32_t) - bin - (notches_BW[0] / 2.0)));
    notch_R[0] = (int16_t)(2 * roundf((float32_t) - bin + (notches_BW[0] / 2.0)));
  }
  notch_pixel_L[0] = pos_centre_f + spectrum_x + 65 + magni * ((notches[0] - (float32_t)notches_BW[0] * bin_BW) / 1000.0);
  notch_pixel_R[0] = pos_centre_f + spectrum_x + 65 + magni * ((notches[0] + (float32_t)notches_BW[0] * bin_BW) / 1000.0);

  //  Serial.print("notches[0]"); Serial.println(notches[0]);
  //  Serial.print("notches_BW[0]"); Serial.println(notches_BW[0]);
  //  Serial.print("notch_pixel_L"); Serial.println(notch_pixel_L[0]);
  //  Serial.print("notch_pixel_R"); Serial.println(notch_pixel_R[0]);
  //  Serial.print("notch_L"); Serial.println(notch_L[0]);
  //  Serial.print("notch_R"); Serial.println(notch_R[0]);
}

void show_notch(int notchF, int MODE) {
  // pos_centre_f is the x position of the Rx centre
  // pos is the y position of the spectrum display
  // notch display should be at x = pos_centre_f +- notch frequency and y = 20
  //  LSB:

  // delete old notch rectangle
  tft.fillRect(notch_pixel_L[0] - 1, spectrum_y + 3, notch_pixel_R[0] - notch_pixel_L[0] + 2, spectrum_height - 3, ILI9341_BLACK);

  calc_notch_bins();
  float32_t spectrum_span = SR[SAMPLE_RATE].rate / 1000.0 / (float32_t)(1 << spectrum_zoom);
  pos_centre_f += spectrum_x + 65; // = pos_centre_f + 1;
  const uint16_t sp_width = 256;
  float32_t magni = sp_width / spectrum_span;

  if (notches_on[0])
  {
    // draw new notch rectangle with fancy margins
    tft.fillRect(notch_pixel_L[0], spectrum_y + 3, notch_pixel_R[0] - notch_pixel_L[0], spectrum_height - 3, ILI9341_NAVY);
    tft.drawFastVLine(notch_pixel_L[0] - 1, spectrum_y + 4, spectrum_height - 4, ILI9341_MAROON);
    tft.drawFastVLine(notch_pixel_R[0], spectrum_y + 4, spectrum_height - 4, ILI9341_MAROON);
  }
  /*
            // delete old indicator
            tft.drawFastVLine(pos_centre_f + 1 + sp_width/spectrum_span * oldnotchF / 1000, notchpos, notchL, ILI9341_BLACK);
            tft.drawFastVLine(pos_centre_f + sp_width/spectrum_span * oldnotchF / 1000, notchpos, notchL, ILI9341_BLACK);
            tft.drawFastVLine(pos_centre_f -1 + sp_width/spectrum_span * oldnotchF / 1000, notchpos, notchL, ILI9341_BLACK);
            tft.drawFastHLine(pos_centre_f -4 + sp_width/spectrum_span * oldnotchF / 1000, notchpos+notchL, 9, ILI9341_BLACK);
            tft.drawFastHLine(pos_centre_f -3 + sp_width/spectrum_span * oldnotchF / 1000, notchpos+notchL + 1, 7, ILI9341_BLACK);
            tft.drawFastHLine(pos_centre_f -2 + sp_width/spectrum_span * oldnotchF / 1000, notchpos+notchL + 2, 5, ILI9341_BLACK);
            tft.drawFastHLine(pos_centre_f -1 + sp_width/spectrum_span * oldnotchF / 1000, notchpos+notchL + 3, 3, ILI9341_BLACK);
            tft.drawFastVLine(pos_centre_f + sp_width/spectrum_span * oldnotchF / 1000, notchpos+notchL + 4, 2, ILI9341_BLACK);

            tft.drawFastVLine(pos_centre_f +1 - sp_width/spectrum_span * oldnotchF / 1000, notchpos, notchL, ILI9341_BLACK);
            tft.drawFastVLine(pos_centre_f - sp_width/spectrum_span * oldnotchF / 1000, notchpos, notchL, ILI9341_BLACK);
            tft.drawFastVLine(pos_centre_f -1 - sp_width/spectrum_span * oldnotchF / 1000, notchpos, notchL, ILI9341_BLACK);
            tft.drawFastHLine(pos_centre_f -4 - sp_width/spectrum_span * oldnotchF / 1000, notchpos+notchL, 9, ILI9341_BLACK);
            tft.drawFastHLine(pos_centre_f -3 - sp_width/spectrum_span * oldnotchF / 1000, notchpos+notchL + 1, 7, ILI9341_BLACK);
            tft.drawFastHLine(pos_centre_f -2 - sp_width/spectrum_span * oldnotchF / 1000, notchpos+notchL + 2, 5, ILI9341_BLACK);
            tft.drawFastHLine(pos_centre_f -1 - sp_width/spectrum_span * oldnotchF / 1000, notchpos+notchL + 3, 3, ILI9341_BLACK);
            tft.drawFastVLine(pos_centre_f - sp_width/spectrum_span * oldnotchF / 1000, notchpos+notchL + 4, 2, ILI9341_BLACK);
            // Show mid screen tune position
    //          tft.drawFastVLine(pos_centre_f - 1, 0,pos+1, ILI9341_RED); //WHITE);

        if (notchF >= 400 || notchF <= -400) {
            // draw new indicator according to mode
        switch (MODE)  {
            case DEMOD_LSB: //modeLSB:
            case DEMOD_SAM_LSB:
            tft.drawFastVLine(pos_centre_f + 1 - sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f - sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f -1 - sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastHLine(pos_centre_f -4 - sp_width/spectrum_span * notchF / 1000, notchpos+notchL, 9, notchColour);
            tft.drawFastHLine(pos_centre_f -3 - sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 1, 7, notchColour);
            tft.drawFastHLine(pos_centre_f -2 - sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 2, 5, notchColour);
            tft.drawFastHLine(pos_centre_f -1 - sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 3, 3, notchColour);
            tft.drawFastVLine(pos_centre_f - sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 4, 2, notchColour);
            break;
            case DEMOD_USB: //modeUSB:
            case DEMOD_SAM_USB:
            tft.drawFastVLine(pos_centre_f +1 + sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f + sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f -1 + sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastHLine(pos_centre_f -4 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL, 9, notchColour);
            tft.drawFastHLine(pos_centre_f -3 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 1, 7, notchColour);
            tft.drawFastHLine(pos_centre_f -2 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 2, 5, notchColour);
            tft.drawFastHLine(pos_centre_f -1 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 3, 3, notchColour);
            tft.drawFastVLine(pos_centre_f + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 4, 2, notchColour);
            break;
            case DEMOD_AM2: // modeAM:
            case DEMOD_AM_ME2:
            case DEMOD_SAM:
            case DEMOD_SAM_STEREO:
            tft.drawFastVLine(pos_centre_f + 1 + sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f + sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f - 1 + sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastHLine(pos_centre_f -4 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL, 9, notchColour);
            tft.drawFastHLine(pos_centre_f -3 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 1, 7, notchColour);
            tft.drawFastHLine(pos_centre_f -2 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 2, 5, notchColour);
            tft.drawFastHLine(pos_centre_f -1 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 3, 3, notchColour);
            tft.drawFastVLine(pos_centre_f + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 4, 2, notchColour);

            tft.drawFastVLine(pos_centre_f + 1 - sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f - sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f - 1 - sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastHLine(pos_centre_f -4 - sp_width/spectrum_span * notchF / 1000, notchpos+notchL, 9, notchColour);
            tft.drawFastHLine(pos_centre_f -3 - sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 1, 7, notchColour);
            tft.drawFastHLine(pos_centre_f -2 - sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 2, 5, notchColour);
            tft.drawFastHLine(pos_centre_f -1 - sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 3, 3, notchColour);
            tft.drawFastVLine(pos_centre_f - sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 4, 2, notchColour);
            break;
            case DEMOD_DSB: //modeDSB:
    //          case DEMOD_STEREO_AM: //modeStereoAM:
            if (notchF <=-400) {
            tft.drawFastVLine(pos_centre_f + 1 - sp_width/spectrum_span * notchF / -1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f - sp_width/spectrum_span * notchF / -1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f - 1 - sp_width/spectrum_span * notchF / -1000, notchpos, notchL, notchColour);
            tft.drawFastHLine(pos_centre_f -4 - sp_width/spectrum_span * notchF / -1000, notchpos+notchL, 9, notchColour);
            tft.drawFastHLine(pos_centre_f -3 - sp_width/spectrum_span * notchF / -1000, notchpos+notchL + 1, 7, notchColour);
            tft.drawFastHLine(pos_centre_f -2 - sp_width/spectrum_span * notchF / -1000, notchpos+notchL + 2, 5, notchColour);
            tft.drawFastHLine(pos_centre_f -1 - sp_width/spectrum_span * notchF / -1000, notchpos+notchL + 3, 3, notchColour);
            tft.drawFastVLine(pos_centre_f - sp_width/spectrum_span * notchF / -1000, notchpos+notchL + 4, 2, notchColour);
            }
            if (notchF >=400) {
            tft.drawFastVLine(pos_centre_f + 1 + sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f + sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastVLine(pos_centre_f - 1 + sp_width/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
            tft.drawFastHLine(pos_centre_f -4 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL, 9, notchColour);
            tft.drawFastHLine(pos_centre_f -3 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 1, 7, notchColour);
            tft.drawFastHLine(pos_centre_f -2 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 2, 5, notchColour);
            tft.drawFastHLine(pos_centre_f -1 + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 3, 3, notchColour);
            tft.drawFastVLine(pos_centre_f + sp_width/spectrum_span * notchF / 1000, notchpos+notchL + 4, 2, notchColour);
            }
            break;
        }
        }
  */
  oldnotchF = notchF;
  pos_centre_f -= spectrum_x + 65; // = pos_centre_f - 1;
} // end void show_notch

float deemphasis_wfm_ff (float* input, float* output, int input_size, int sample_rate, float last_output)
{
  /* taken from libcsdr
    typical time constant (tau) values:
    WFM transmission in USA: 75 us -> tau = 75e-6
    WFM transmission in EU:  50 us -> tau = 50e-6
    More info at: http://www.cliftonlaboratories.com/fm_receivers_and_de-emphasis.htm
    Simulate in octave: tau=75e-6; dt=1/48000; alpha = dt/(tau+dt); freqz([alpha],[1 -(1-alpha)])
  */
  //  if(is_nan(last_output)) last_output=0.0; //if last_output is NaN
  output[0] = deemp_alpha * input[0] + (onem_deemp_alpha) * last_output;
  for (int i = 1; i < input_size; i++) //@deemphasis_wfm_ff
    output[i] = deemp_alpha * input[i] + (onem_deemp_alpha) * output[i - 1]; //this is the simplest IIR LPF
  return output[input_size - 1];
}


// Michael Wild - very nicely implemented noise blanker !
// thanks a lot for putting this into the open source domain
// GPLv3
#define NB_FFT_SIZE FFT_length/2
void noiseblanker(float32_t* inputsamples, float32_t* outputsamples )
{

  float32_t* Energy = 0;



  alt_noise_blanking(inputsamples, NB_FFT_SIZE, Energy);

  for (unsigned k = 0; k < NB_FFT_SIZE;  k++)
  {
    outputsamples[k] = inputsamples[k];
  }

}


//alt noise blanking is trying to localize some impulse noise within the samples and after that
//trying to replace corrupted samples by linear predicted samples.
//therefore, first we calculate the lpc coefficients which represent the actual status of the
//speech or sound generating "instrument" (in case of speech this is an estimation of the current
//filter-function of the voice generating tract behind our lips :-) )
//after finding this function we inverse filter the actual samples by this function
//so we are eliminating the speech, but not the noise. Then we do a matched filtering an thereby detecting impulses
//After that we threshold the remaining samples by some
//level and so detecting impulse noise's positions within the current frame - if one (or more) impulses are there.
//finally some area around the impulse position will be replaced by predicted samples from both sides (forward and
//backward prediction)
//hopefully we have enough processor power left....
#define debug_alternate_NR
void alt_noise_blanking(float* insamp, int Nsam, float* E )
{
#define boundary_blank 14//14 // for first trials very large!!!!
#define impulse_length NB_impulse_samples // 7 // has to be odd!!!! 7 / 3 should be enough
#define PL             (impulse_length - 1) / 2 //6 // 3 has to be (impulse_length-1)/2 !!!!
  int order    =     NB_taps; //10 // lpc's order
  arm_fir_instance_f32 LPC;
  float32_t lpcs[order + 1]; // we reserve one more than "order" because of a leading "1"
  float32_t reverse_lpcs[order + 1]; //this takes the reversed order lpc coefficients
  float32_t firStateF32[NB_FFT_SIZE + order];
  float32_t tempsamp[NB_FFT_SIZE];
  float32_t sigma2; //taking the variance of the inpo
  float32_t lpc_power;
  float32_t impulse_threshold;
  int impulse_positions[20];  //we allow a maximum of 5 impulses per frame
  int search_pos = 0;
  int impulse_count = 0;
  //    static float32_t last_frame_end[order+PL]; //this takes the last samples from the previous frame to do the prediction within the boundaries
  static float32_t last_frame_end[80]; //this takes the last samples from the previous frame to do the prediction within the boundaries
#ifdef debug_alternate_NR
  static int frame_count = 0; //only used for the distortion insertion - can alter be deleted
  int dist_level = 0; //only used for the distortion insertion - can alter be deleted
#endif

  int nr_setting = 0;
  float32_t R[11];  // takes the autocorrelation results
  float32_t e, k, alfa;

  float32_t any[order + 1]; //some internal buffers for the levinson durben algorithm

  float32_t Rfw[impulse_length + order]; // takes the forward predicted audio restauration
  float32_t Rbw[impulse_length + order]; // takes the backward predicted audio restauration
  float32_t Wfw[impulse_length], Wbw[impulse_length]; // taking linear windows for the combination of fwd and bwd

  float32_t s;

#ifdef debug_alternate_NR  // generate test frames to test the noise blanker function
  // using the NR-setting (0..55) to select the test frame
  // 00 = noise blanker active on orig. audio; threshold factor=3
  // 01 = frame of vocal "a" undistorted
  // 02 .. 05 = frame of vocal "a" with different impulse distortion levels
  // 06 .. 09 = frame of vocal "a" with different impulse distortion levels
  //            noise blanker operating!!
  //************
  // 01..09 are now using the original received audio and applying a rythmic "click" distortion
  // 06..09 is detecting and removing the click by restoring the predicted audio!!!
  //************
  // 5 / 9 is the biggest "click" and it is slightly noticeable in the restored audio (9)
  // 10 = noise blanker active on orig. audio threshold factor=3
  // 11  = sinusoidal signal undistorted
  // 12 ..15 = sinusoidal signal with different impulse distortion levels
  // 16 ..19 = sinusoidal signal with different impulse distortion levels
  //            noise blanker operating!!
  // 20 ..50   noise blanker active on orig. audio; threshold factor varying between 3 and 0.26

  nr_setting = NB_test; //(int)ts.dsp_nr_strength;
  //*********************************from here just debug impulse / signal generation
  if ((nr_setting > 0) && (nr_setting < 10)) // we use the vocal "a" frame
  {
    //for (int i=0; i<128;i++)          // not using vocal "a" but the original signal
    //    insamp[i]=NR_test_samp[i];

    if ((frame_count > 19) && (nr_setting > 1))    // insert a distorting pulse
    {
      dist_level = nr_setting;
      if (dist_level > 5) dist_level = dist_level - 4; // distortion level is 1...5
      insamp[4] = insamp[4] + dist_level * 3000; // overlaying a short  distortion pulse +/-
      insamp[5] = insamp[5] - dist_level * 1000;
    }

  }

  if ((nr_setting > 10) && (nr_setting < 20)) // we use the sinus frame
  {
    for (int i = 0; i < 128; i++)
      //                insamp[i]=NR_test_sinus_samp[i];

      if ((frame_count > 19) && (nr_setting > 11))    // insert a distorting pulse
      {
        dist_level = nr_setting - 10;
        if (dist_level > 5) dist_level = dist_level - 4;
        insamp[24] = insamp[24] + dist_level * 1000; // overlaying a short  distortion pulse +/-
        insamp[25] = insamp[25] + dist_level * 500;
        insamp[26] = insamp[26] - dist_level * 200; // overlaying a short  distortion pulse +/-
        insamp[27] = insamp[27] - dist_level * 100;


      }



  }


  frame_count++;
  if (frame_count > 20) frame_count = 0;

#endif

  //*****************************end of debug impulse generation

  //  start of test timing zone

  for (int i = 0; i < impulse_length; i++) // generating 2 Windows for the combination of the 2 predictors
  { // will be a constant window later!
    Wbw[i] = 1.0 * i / (impulse_length - 1);
    Wfw[impulse_length - i - 1] = Wbw[i];
  }

  // calculate the autocorrelation of insamp (moving by max. of #order# samples)
  for (int i = 0; i < (order + 1); i++)
  {
    arm_dot_prod_f32(&insamp[0], &insamp[i], Nsam - i, &R[i]); // R is carrying the crosscorrelations
  }
  // end of autocorrelation

  //Serial.println("Noise Blanker working");

  //alternative levinson durben algorithm to calculate the lpc coefficients from the crosscorrelation

  R[0] = R[0] * (1.0 + 1.0e-9);

  lpcs[0] = 1;   //set lpc 0 to 1

  for (int i = 1; i < order + 1; i++)
    lpcs[i] = 0;                    // fill rest of array with zeros - could be done by memfill

  alfa = R[0];

  for (int m = 1; m <= order; m++)
  {
    s = 0.0;
    for (int u = 1; u < m; u++)
      s = s + lpcs[u] * R[m - u];

    k = -(R[m] + s) / alfa;

    for (int v = 1; v < m; v++)
      any[v] = lpcs[v] + k * lpcs[m - v];

    for (int w = 1; w < m; w++)
      lpcs[w] = any[w];

    lpcs[m] = k;
    alfa = alfa * (1 - k * k);
  }

  // end of levinson durben algorithm

  for (int o = 0; o < order + 1; o++ )           //store the reverse order coefficients separately
    reverse_lpcs[order - o] = lpcs[o];    // for the matched impulse filter

  arm_fir_init_f32(&LPC, order + 1, &reverse_lpcs[0], &firStateF32[0], NB_FFT_SIZE);                                   // we are using the same function as used in freedv
  arm_fir_f32(&LPC, insamp, tempsamp, Nsam); //do the inverse filtering to eliminate voice and enhance the impulses
  arm_fir_init_f32(&LPC, order + 1, &lpcs[0], &firStateF32[0], NB_FFT_SIZE);                                   // we are using the same function as used in freedv
  arm_fir_f32(&LPC, tempsamp, tempsamp, Nsam); // do a matched filtering to detect an impulse in our now voiceless signal

  arm_var_f32(tempsamp, NB_FFT_SIZE, &sigma2); //calculate sigma2 of the original signal ? or tempsignal
  arm_power_f32(lpcs, order, &lpc_power); // calculate the sum of the squares (the "power") of the lpc's

  impulse_threshold = NB_thresh * sqrtf(sigma2 * lpc_power);  //set a detection level (3 is not really a final setting)

  //if ((nr_setting > 20) && (nr_setting <51))
  //    impulse_threshold = impulse_threshold / (0.9 + (nr_setting-20.0)/10);  //scaling the threshold by 1 ... 0.26

  search_pos = order + PL; // lower boundary problem has been solved! - so here we start from 1 or 0?
  impulse_count = 0;

  do {        //going through the filtered samples to find an impulse larger than the threshold

    if ((tempsamp[search_pos] > impulse_threshold) || (tempsamp[search_pos] < (-impulse_threshold)))
    {
      impulse_positions[impulse_count] = search_pos - order; // save the impulse positions and correct it by the filter delay
      impulse_count++;
      search_pos += PL; //  set search_pos a bit away, cause we are already repairing this area later
      //  and the next impulse should not be that close
    }

    search_pos++;

  } while ((search_pos < NB_FFT_SIZE - boundary_blank) && (impulse_count < 20)); // avoid upper boundary

  //boundary handling has to be fixed later
  //as a result we now will not find any impulse in these areas

  // from here: reconstruction of the impulse-distorted audio part:

  // first we form the forward and backward prediction transfer functions from the lpcs
  // that is easy, as they are just the negated coefficients  without the leading "1"
  // we can do this in place of the lpcs, as they are not used here anymore and being recalculated in the next frame!

  arm_negate_f32(&lpcs[1], &lpcs[1], order);
  arm_negate_f32(&reverse_lpcs[0], &reverse_lpcs[0], order);

  for (int j = 0; j < impulse_count; j++)
  {
    for (int k = 0; k < order; k++) // we have to copy some samples from the original signal as
    { // basis for the reconstructions - could be done by memcopy

      if ((impulse_positions[j] - PL - order + k) < 0) // this solves the prediction problem at the left boundary
      {
        Rfw[k] = last_frame_end[impulse_positions[j] + k]; //take the sample from the last frame
      }
      else
      {
        Rfw[k] = insamp[impulse_positions[j] - PL - order + k]; //take the sample from this frame as we are away from the boundary
      }

      Rbw[impulse_length + k] = insamp[impulse_positions[j] + PL + k + 1];



    }     //bis hier alles ok

    for (int i = 0; i < impulse_length; i++) //now we calculate the forward and backward predictions
    {
      arm_dot_prod_f32(&reverse_lpcs[0], &Rfw[i], order, &Rfw[i + order]);
      arm_dot_prod_f32(&lpcs[1], &Rbw[impulse_length - i], order, &Rbw[impulse_length - i - 1]);

    }

    arm_mult_f32(&Wfw[0], &Rfw[order], &Rfw[order], impulse_length); // do the windowing, or better: weighing
    arm_mult_f32(&Wbw[0], &Rbw[0], &Rbw[0], impulse_length);

    //Serial.println("Noiseblanker 2");

#ifdef debug_alternate_NR
    // in debug mode do the restoration only in some cases
    if (((nr_setting > 0) && (nr_setting < 6)) || ((nr_setting > 10) && (nr_setting < 16)))
    {
      // just let the distortion pass at setting 1...5 and 11...15
      //    arm_add_f32(&Rfw[order],&Rbw[0],&insamp[impulse_positions[j]-PL],impulse_length);
    }
    else
    {
      //finally add the two weighted predictions and insert them into the original signal - thereby eliminating the distortion
      arm_add_f32(&Rfw[order], &Rbw[0], &insamp[impulse_positions[j] - PL], impulse_length);
    }
#else
    //finally add the two weighted predictions and insert them into the original signal - thereby eliminating the distortion
    arm_add_f32(&Rfw[order], &Rbw[0], &insamp[impulse_positions[j] - PL], impulse_length);

#endif
  }

  for (int p = 0; p < (order + PL); p++)
  {
    last_frame_end[p] = insamp[NB_FFT_SIZE - 1 - order - PL + p]; // store 13 samples from the current frame to use at the next frame
  }
  //end of test timing zone
}

void control_filter_f() {
  // low Fcut must never be larger than high Fcut and vice versa
  if (bands[current_band].FHiCut < bands[current_band].FLoCut) bands[current_band].FHiCut = bands[current_band].FLoCut;
  if (bands[current_band].FLoCut > bands[current_band].FHiCut) bands[current_band].FLoCut = bands[current_band].FHiCut;
  // calculate maximum possible FHiCut
  float32_t sam = (SR[SAMPLE_RATE].rate / (DF * 2.0)) - 100.0;
  // clamp FHicut and Flowcut to max / min values
  if (bands[current_band].FHiCut > (int)(sam))
  {
    bands[current_band].FHiCut = (int)sam;
  }
  else if (bands[current_band].FHiCut < -(int)(sam - 100.0))
  {
    bands[current_band].FHiCut = -(int)(sam - 100.0);
  }

  if (bands[current_band].FLoCut > (int)(sam - 100.0))
  {
    bands[current_band].FLoCut = (int)(sam - 100.0);
  }
  else if (bands[current_band].FLoCut < -(int)(sam))
  {
    bands[current_band].FLoCut = -(int)(sam);
  }

  //  if(old_demod_mode != -99)
  {
    switch (bands[current_band].mode)
    {
      case DEMOD_SAM_LSB:
      case DEMOD_SAM_USB:
      case DEMOD_SAM_STEREO:
      case DEMOD_IQ:
        bands[current_band].FLoCut = - bands[current_band].FHiCut;
        break;
      case DEMOD_LSB:
        if (bands[current_band].FHiCut > 0) bands[current_band].FHiCut = 0;
        break;
      case DEMOD_USB:
        if (bands[current_band].FLoCut < 0) bands[current_band].FLoCut = 0;
        break;
      case DEMOD_AM2:
      case DEMOD_AM_ME2:
      case DEMOD_SAM:
        if (bands[current_band].FLoCut > -100) bands[current_band].FLoCut = -100;
        if (bands[current_band].FHiCut < 100) bands[current_band].FHiCut = 100;
        break;
    }
  }
}

void set_dec_int_filters()
{
  //          control_filter_f(); // check, if filter bandwidth is within bounds
  //          filter_bandwidth(); // calculate new FIR & IIR coefficients according to the new sample rate
  //          show_bandwidth();
  //          set_SAM_PLL();
  /****************************************************************************************
     Recalculate decimation and interpolation FIR filters
  ****************************************************************************************/
  // new !
  // set decimation and interpolation filter BW according to the desired filter frequencies
  // determine largest bandwidth from FHiCut and FLoCut
  int filter_BW_highest = bands[current_band].FHiCut;
  if (filter_BW_highest < - bands[current_band].FLoCut)
  {
    filter_BW_highest = - bands[current_band].FLoCut;
  }
  LP_F_help = filter_BW_highest;

  if (LP_F_help > 10000) LP_F_help = 10000;
#ifdef DEBUG
  Serial.print("Alias frequ for decimation/interpolation"); Serial.println((LP_F_help));
#endif
  calc_FIR_coeffs (FIR_dec1_coeffs, n_dec1_taps, (float32_t)(LP_F_help), n_att, 0, 0.0, (float32_t)(SR[SAMPLE_RATE].rate));
  calc_FIR_coeffs (FIR_dec2_coeffs, n_dec2_taps, (float32_t)(LP_F_help), n_att, 0, 0.0, (float32_t)(SR[SAMPLE_RATE].rate / DF1));

  calc_FIR_coeffs (FIR_int1_coeffs, 48, (float32_t)(LP_F_help), n_att, 0, 0.0, (float32_t)(SR[SAMPLE_RATE].rate / DF1));
  calc_FIR_coeffs (FIR_int2_coeffs, 32, (float32_t)(LP_F_help), n_att, 0, 0.0, (float32_t)SR[SAMPLE_RATE].rate);
  bin_BW = 1.0 / (DF * FFT_length) * (float32_t)SR[SAMPLE_RATE].rate;
}

void spectral_noise_reduction_init()
{
  for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
  {
    NR_last_sample_buffer_L[bindx] = 0.1;
    NR_Hk_old[bindx] = 0.1; // old gain
    NR_Nest[bindx][0] = 0.01;
    NR_Nest[bindx][1] = 0.015;
    NR_Gts[bindx][1] = 0.1;
    NR_M[bindx] = 500.0;
    NR_E[bindx][0] = 0.1;
    NR_X[bindx][1] = 0.5;
    NR_SNR_post[bindx] = 2.0;
    NR_SNR_prio[bindx] = 1.0;
    NR_first_time = 2;
    NR_long_tone_gain[bindx] = 1.0;
  }
}

#if 0
void //SSB_AUTOTUNE_ccor(int n, float dat[], loat w[], float xr[], float xi[])
SSB_AUTOTUNE_ccor(int n, float32_t* dat, float32_t* FFT_buffer)

/*  "Complex Correlation" t->f->t function             */

/*  int   n;             Must be a positive power of 2 */
/*  float dat[];         Input (time) waveform section */
/*  float w[];           Data Window (raised cosine)   */
/*  float xr[], xi[];    Real & Imag data vectors      */
{
  // int idx;

  /*  Windowed data -> real vector, zeros -> imag vector */
  for (int idx = 0; idx < n; idx++)
  {

    FFT_buffer[]

    //    xr[idx] = dat[idx] * w[idx];
    //    xi[idx] = 0.0;
  }
  // forward FFT
  // FIXME
  //   fwfft(n,xr,xi);
  /*  Now in scrambled-index frequency domain            */
  /*  Form magnitudes at pos freqs, zeros elsewhere      */
  /*  Scrambled pos freq <=> even-numbered index         */
  // FIXME !!!
  // calculate magnitudes and put into real parts of the positive frequencies
  // set all imaginaries to zero
  // set all reals of negative frequencies to zero
  xr[0] = 0.0; /* DC is still zero index when scrambled */
  xi[0] = 0.0;
  //xr[1]=0.0;
  //xr[]
  xi[1] = 0.0;
  for (int idx = 2; idx < n; idx = idx + 2)
  {
    xr[idx]   = sqrtf(xr[idx] * xr[idx] + xi[idx] * xi[idx]);
    xi[idx]   = 0.0;
    //xr[idx+1] = 0.0;
    // we have to set the real parts of the negative frequencies to zero
    xr[]
    xi[idx + 1] = 0.0;
  }

  // inverse FFT
  // FIXME
  //   rvfft(n,xr,xi);
  /*  Now xr & xi are "complex correlation"  */
}
#endif


void SSB_AUTOTUNE_est(int n, float xr[], float xi[], float smpfrq,
                      float *ppeak, float *ppitch, float *pshift)
/*  Estimator of pitch and one (out of many possible) freq shift  */
/*  See "Cochannel" page B-21                                     */
/*  n;            Buffer size (power of 2)                        */
/*  xr[],xi[];    Real, imag buffers from ccor                    */
/*  smpfrq;       Sampling frequency                              */
/*  peak;         Peak power at speaker pitch                     */
/*  pitch;        Estimated speaker pitch                         */
/*  shift;        One possible frequency shift                    */
{
  float pi = 3.14159265358979;
  float lolim, hilim; /* low, high limits for speaker_pitch search range */
  int lidx, hidx; /* low, high index limits for tau search range */
  float temp;
  int idx, idpk; /* index and index_to_peak                     */
  float bsq, usq; /* below_square and upper_square               */
  float x; /* fractional spacing of interpolated peak, -1<=x<=1 */
  float tau; /* sampling_frequency/speaker_pitch                */
  float cf1, cf2; /* coefficients for parabolic interpolation    */
  float partr, parti; /* real, imag, parts of ccor peak          */
  float angl; /* Shift_angle in radians versus speaker pitch    */

  lolim = 50; /* Assumes speaker pitch >= 50 Hz */
  hilim = 250; /* Assumes speaker pitch <= 250 Hz */
  lidx = smpfrq / hilim; /* hi pitch is low time delta  */
  if (lidx < 4)lidx = 4;
  hidx = smpfrq / lolim;
  if (hidx > n / 2 - 2)hidx = n / 2 - 2;
  *ppeak = 0.;
  idpk = 4; /*  2-18-98  */
  for (int idx = lidx; idx <= hidx; idx++)
  {
    temp = xr[idx] * xr[idx] + xi[idx] * xi[idx];
    if (*ppeak < temp)
    {
      *ppeak = temp;
      idpk = idx;
    }
  }
  /* Find quadratic-interpolation peak */
  bsq = xr[idpk - 1] * xr[idpk - 1] + xi[idpk - 1] * xi[idpk - 1];
  usq = xr[idpk + 1] * xr[idpk + 1] + xi[idpk + 1] * xi[idpk + 1];
  x = 1.;
  if (*ppeak > usq)
    x = 0.;
  if (bsq >= *ppeak)
    x = -1.;
  if (x == 0.)
    x = 0.5 * (usq - bsq) / (2.* *ppeak - bsq - usq);
  tau = idpk + x;
  *ppitch = smpfrq / tau;
  /* Interpolate real and imag parts */
  cf1 = 0.5 * (xr[idpk + 1] - xr[idpk - 1]);
  cf2 = 0.5 * (xr[idpk - 1] + xr[idpk + 1]) - xr[idpk];
  partr = xr[idpk] + x * (cf1 + x * cf2);
  cf1 = 0.5 * (xi[idpk + 1] - xi[idpk - 1]);
  cf2 = 0.5 * (xi[idpk - 1] + xi[idpk + 1]) - xi[idpk];
  parti = xi[idpk] + x * (cf1 + x * cf2);
  *ppeak = partr * partr + parti * parti;
  /* calculate 4-quadrant arctangent (-pi/2 to 3pi/2) */
  if (partr > 0.)
    angl = atanf(parti / partr);
  if (partr == 0.)
  {
    if (parti >= 0.)
      angl = 0.5 * pi;
    else
      angl = -0.5 * pi;
  }
  if (partr < 0.)
    angl = pi - atanf(-parti / partr);
  *pshift = *ppitch * angl / (2.*pi);
}


void SSB_AUTOTUNE_srchist(int nbins, float bins[], float thresh,
                          int *pminwid, float *pctr)

/*  nbins   Number of histogram bins                 */
/*  bins    The histogram bins themselves            */
/*  thresh  Threshold for sum of bins                */
/*  minwid  Min width-1 of interval with sum >= thresh */
/*  ctr     Center of minwid bins interval           */
/*  Note:   Call using arguments &minwid and &ctr    */

{
  int lidx, hidx, oldlow, minlow;
  float sum;

  *pminwid = nbins;
  lidx = hidx = oldlow = minlow = 0;
  sum = bins[0]; /* 2-18-98 */

  while (hidx < nbins)
  {
    while (sum < thresh && hidx < nbins)
    { /*  Advance forward limit until sum>=thresh  */
      hidx++;
      if (hidx < nbins)
        sum = sum + bins[hidx];
    }
    while (sum >= thresh && lidx <= hidx && hidx < nbins)
    { /*  Advance rear limit until sum<thresh  */
      sum = sum - bins[lidx];
      oldlow = lidx;
      lidx++;
    }
    if (hidx < nbins)
    {
      if (hidx - oldlow < *pminwid)
      { /*  If intvl narrowest so far note it  */
        *pminwid = hidx - oldlow;
        minlow = oldlow;
      }
    }
  }
  *pctr = minlow + *pminwid * 0.5;
}


/*  Increment Histogram INCHIST.C  */

void SSB_AUTOTUNE_inchist(int nbins, float bins[],
                          float hpitch, float hshift, float incr)

/*  nbins   Number of histogram bins                */
/*  bins    The array of histogram bins             */
/*  hpitch  Est spkr pitch in num of bins spanned   */
/*  hshift  One poss freq shift in histogram units  */
/*  incr    Amount to increment each selected bin   */

/*  For lowf=min freq of bin 0 (lowf may be < 0),   */
/*  And hif=min freq of bin (nbins-1),              */
/*  hpitch = pitch*nbins/(hif-lowf) and             */
/*  hshift =(shift-lowf)*nbins/(hif-lowf).          */

{
  float fidx;  /*  Floating point histogram index  */
  int   idx;   /*  Integer histogram index         */

  if (hpitch >= 1.0  && 0.0 <= hshift && hshift < nbins && incr > 0.0)
  {
    fidx = hshift;
    while (fidx >= 0.0)
    {
      idx = fidx;
      bins[idx] = bins[idx] + incr;
      fidx = fidx - hpitch;
    }
    fidx = hpitch + hshift;
    while (fidx < nbins)
    {
      idx = fidx;
      bins[idx] = bins[idx] + incr;
      fidx = fidx + hpitch;
    }
  }
}


void spectral_noise_reduction (void)
/************************************************************************************************************

      Noise reduction with spectral subtraction rule
      based on Romanin et al. 2009 & Schmitt et al. 2002
      and MATLAB voicebox
      and Gerkmann & Hendriks 2002
      and Yao et al. 2016

   STAND: UHSDR github 14.1.2018
   ************************************************************************************************************/
{
  //const float32_t sqrtHann = {0,0.006147892,0.012295552,0.018442747,0.024589245,0.030734813,0.03687922,0.043022233,0.04916362,0.055303148,0.061440587,0.067575703,0.073708265,0.07983804,0.085964799,0.092088308,0.098208336,0.104324653,0.110437026,0.116545225,0.122649019,0.128748177,0.134842469,0.140931665,0.147015533,0.153093845,0.159166371,0.16523288,0.171293144,0.177346934,0.18339402,0.189434175,0.19546717,0.201492777,0.207510768,0.213520915,0.219522993,0.225516773,0.231502029,0.237478535,0.243446065,0.249404393,0.255353295,0.261292545,0.26722192,0.273141194,0.279050144,0.284948547,0.290836179,0.296712819,0.302578244,0.308432233,0.314274564,0.320105016,0.325923369,0.331729404,0.3375229,0.343303638,0.349071401,0.35482597,0.360567128,0.366294657,0.372008341,0.377707965,0.383393313,0.389064169,0.39472032,0.400361552,0.405987651,0.411598406,0.417193603,0.422773031,0.42833648,0.433883739,0.439414599,0.44492885,0.450426284,0.455906694,0.461369871,0.46681561,0.472243705,0.477653951,0.483046143,0.488420077,0.49377555,0.49911236,0.504430306,0.509729185,0.515008798,0.520268945,0.525509428,0.530730048,0.535930608,0.541110912,0.546270763,0.551409967,0.556528329,0.561625657,0.566701756,0.571756436,0.576789506,0.581800774,0.586790052,0.591757151,0.596701884,0.601624063,0.606523503,0.611400018,0.616253424,0.621083537,0.625890175,0.630673157,0.635432301,0.640167428,0.644878358,0.649564914,0.654226918,0.658864195,0.663476568,0.668063864,0.67262591,0.677162532,0.681673559,0.686158822,0.690618149,0.695051373,0.699458327,0.703838843,0.708192756,0.712519902,0.716820117,0.721093238,0.725339104,0.729557554,0.733748429,0.737911571,0.742046822,0.746154026,0.750233028,0.754283673,0.758305808,0.762299282,0.766263944,0.770199643,0.77410623,0.777983559,0.781831482,0.785649855,0.789438533,0.793197372,0.79692623,0.800624968,0.804293444,0.80793152,0.811539059,0.815115924,0.818661981,0.822177094,0.825661132,0.829113962,0.832535454,0.835925479,0.839283909,0.842610616,0.845905475,0.849168362,0.852399152,0.855597725,0.858763958,0.861897733,0.864998931,0.868067434,0.871103127,0.874105896,0.877075625,0.880012204,0.882915521,0.885785467,0.888621932,0.891424811,0.894193996,0.896929383,0.89963087,0.902298353,0.904931732,0.907530907,0.91009578,0.912626255,0.915122235,0.917583626,0.920010335,0.922402271,0.924759343,0.927081462,0.92936854,0.931620491,0.933837229,0.936018671,0.938164734,0.940275338,0.942350402,0.944389848,0.9463936,0.94836158,0.950293715,0.952189932,0.954050159,0.955874327,0.957662364,0.959414206,0.961129784,0.962809034,0.964451894,0.9660583,0.967628192,0.96916151,0.970658197,0.972118197,0.973541453,0.974927912,0.976277522,0.977590232,0.978865992,0.980104754,0.98130647,0.982471097,0.983598589,0.984688904,0.985742,0.986757839,0.987736381,0.98867759,0.98958143,0.990447867,0.991276868,0.992068401,0.992822438,0.993538949,0.994217907,0.994859287,0.995463064,0.996029215,0.99655772,0.997048558,0.997501711,0.997917161,0.998294893,0.998634892,0.998937146,0.999201643,0.999428374,0.999617329,0.999768502,0.999881887,0.999957479,0.999995275,0.999995275,0.999957479,0.999881887,0.999768502,0.999617329,0.999428374,0.999201643,0.998937146,0.998634892,0.998294893,0.997917161,0.997501711,0.997048558,0.99655772,0.996029215,0.995463064,0.994859287,0.994217907,0.993538949,0.992822438,0.992068401,0.991276868,0.990447867,0.98958143,0.98867759,0.987736381,0.986757839,0.985742,0.984688904,0.983598589,0.982471097,0.98130647,0.980104754,0.978865992,0.977590232,0.976277522,0.974927912,0.973541453,0.972118197,0.970658197,0.96916151,0.967628192,0.9660583,0.964451894,0.962809034,0.961129784,0.959414206,0.957662364,0.955874327,0.954050159,0.952189932,0.950293715,0.94836158,0.9463936,0.944389848,0.942350402,0.940275338,0.938164734,0.936018671,0.933837229,0.931620491,0.92936854,0.927081462,0.924759343,0.922402271,0.920010335,0.917583626,0.915122235,0.912626255,0.91009578,0.907530907,0.904931732,0.902298353,0.89963087,0.896929383,0.894193996,0.891424811,0.888621932,0.885785467,0.882915521,0.880012204,0.877075625,0.874105896,0.871103127,0.868067434,0.864998931,0.861897733,0.858763958,0.855597725,0.852399152,0.849168362,0.845905475,0.842610616,0.839283909,0.835925479,0.832535454,0.829113962,0.825661132,0.822177094,0.818661981,0.815115924,0.811539059,0.80793152,0.804293444,0.800624968,0.79692623,0.793197372,0.789438533,0.785649855,0.781831482,0.777983559,0.77410623,0.770199643,0.766263944,0.762299282,0.758305808,0.754283673,0.750233028,0.746154026,0.742046822,0.737911571,0.733748429,0.729557554,0.725339104,0.721093238,0.716820117,0.712519902,0.708192756,0.703838843,0.699458327,0.695051373,0.690618149,0.686158822,0.681673559,0.677162532,0.67262591,0.668063864,0.663476568,0.658864195,0.654226918,0.649564914,0.644878358,0.640167428,0.635432301,0.630673157,0.625890175,0.621083537,0.616253424,0.611400018,0.606523503,0.601624063,0.596701884,0.591757151,0.586790052,0.581800774,0.576789506,0.571756436,0.566701756,0.561625657,0.556528329,0.551409967,0.546270763,0.541110912,0.535930608,0.530730048,0.525509428,0.520268945,0.515008798,0.509729185,0.504430306,0.49911236,0.49377555,0.488420077,0.483046143,0.477653951,0.472243705,0.46681561,0.461369871,0.455906694,0.450426284,0.44492885,0.439414599,0.433883739,0.42833648,0.422773031,0.417193603,0.411598406,0.405987651,0.400361552,0.39472032,0.389064169,0.383393313,0.377707965,0.372008341,0.366294657,0.360567128,0.35482597,0.349071401,0.343303638,0.3375229,0.331729404,0.325923369,0.320105016,0.314274564,0.308432233,0.302578244,0.296712819,0.290836179,0.284948547,0.279050144,0.273141194,0.26722192,0.261292545,0.255353295,0.249404393,0.243446065,0.237478535,0.231502029,0.225516773,0.219522993,0.213520915,0.207510768,0.201492777,0.19546717,0.189434175,0.18339402,0.177346934,0.171293144,0.16523288,0.159166371,0.153093845,0.147015533,0.140931665,0.134842469,0.128748177,0.122649019,0.116545225,0.110437026,0.104324653,0.098208336,0.092088308,0.085964799,0.07983804,0.073708265,0.067575703,0.061440587,0.055303148,0.04916362,0.043022233,0.03687922,0.030734813,0.024589245,0.018442747,0.012295552,0.006147892,0};
  static uint8_t NR_init_counter = 0;
  uint8_t VAD_low = 0;
  uint8_t VAD_high = 127;
  float32_t lf_freq; // = (offset - width/2) / (12000 / NR_FFT_L); // bin BW is 46.9Hz [12000Hz / 256 bins] @96kHz
  float32_t uf_freq; //= (offset + width/2) / (12000 / NR_FFT_L);

  // TODO: calculate tinc from sample rate and decimation factor
  const float32_t tinc = 0.00533333; // frame time 5.3333ms
  const float32_t tax = 0.0239;  // noise output smoothing time constant = -tinc/ln(0.8)
  const float32_t tap = 0.05062;  // speech prob smoothing time constant = -tinc/ln(0.9) tinc = frame time (5.33ms)
  const float32_t psthr = 0.99; // threshold for smoothed speech probability [0.99]
  const float32_t pnsaf = 0.01; // noise probability safety value [0.01]
  const float32_t asnr = 20;  // active SNR in dB
  const float32_t psini = 0.5; // initial speech probability [0.5]
  const float32_t pspri = 0.5; // prior speech probability [0.5]
  const float32_t tavini = 0.064;
  static float32_t ax; //=0.8;       // ax=exp(-tinc/tax); % noise output smoothing factor
  static float32_t ap; //=0.9;        // ap=exp(-tinc/tap); % noise output smoothing factor
  static float32_t xih1; // = 31.6;
  //xih1=10^(asnr/10); % speech-present SNR
  //static float32_t xih1r; //=-0.969346; // xih1r=1/(1+xih1)-1;
  ax = expf(-tinc / tax);
  ap = expf(-tinc / tap);
  xih1 = powf(10, (float32_t)asnr / 10.0);
  static float32_t xih1r = 1.0 / (1.0 + xih1) - 1.0;
  static float32_t pfac = (1.0 / pspri - 1.0) * (1.0 + xih1);
  float32_t snr_prio_min = powf(10, - (float32_t)20 / 20.0);
  //static float32_t pfac; //=32.6;  // pfac=(1/pspri-1)*(1+xih1); % p(noise)/p(speech)
  static float32_t pslp[NR_FFT_L / 2];
  static float32_t xt[NR_FFT_L / 2];
  static float32_t xtr;
  static float32_t pre_power;
  static float32_t post_power;
  static float32_t power_ratio;
  static int16_t NN;
  const int16_t NR_width = 4;
  const float32_t power_threshold = 0.4;
  float32_t ph1y[NR_FFT_L / 2];
  static int NR_first_time_2 = 1;

  if (bands[current_band].FLoCut <= 0 && bands[current_band].FHiCut >= 0)
  {
    lf_freq = 0.0;
    uf_freq = fmax(-(float32_t)bands[current_band].FLoCut, (float32_t)bands[current_band].FHiCut);
  }
  else
  {
    if (bands[current_band].FLoCut > 0)
    {
      lf_freq = (float32_t)bands[current_band].FLoCut;
      uf_freq = (float32_t)bands[current_band].FHiCut;
    }
    else
    {
      uf_freq = -(float32_t)bands[current_band].FLoCut;
      lf_freq = -(float32_t)bands[current_band].FHiCut;
    }
  }
  // / rate DF SR[SAMPLE_RATE].rate/DF
  lf_freq /= ((SR[SAMPLE_RATE].rate / DF) / NR_FFT_L); // bin BW is 46.9Hz [12000Hz / 256 bins] @96kHz
  uf_freq /= ((SR[SAMPLE_RATE].rate / DF) / NR_FFT_L);

  // Frank DD4WH & Michael DL2FW, November 2017
  // NOISE REDUCTION BASED ON SPECTRAL SUBTRACTION
  // following Romanin et al. 2009 on the basis of Ephraim & Malah 1984 and Hu et al. 2001
  // detailed technical description of the implemented algorithm
  // can be found in our WIKI
  // https://github.com/df8oe/UHSDR/wiki/Noise-reduction
  //
  // half-overlapping input buffers (= overlap 50%)
  // sqrt von Hann window before FFT
  // sqrt von Hann window after inverse FFT
  // FFT256 - inverse FFT256
  // overlap-add

  // INITIALIZATION ONCE 1
  if (NR_first_time_2 == 1)
  { // TODO: properly initialize all the variables
    for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
    {
      NR_last_sample_buffer_L[bindx] = 0.0;
      NR_G[bindx] = 1.0;
      //xu[bindx] = 1.0;  //has to be replaced by other variable
      NR_Hk_old[bindx] = 1.0; // old gain or xu in development mode
      NR_Nest[bindx][0] = 0.0;
      NR_Nest[bindx][1] = 1.0;
      pslp[bindx] = 0.5;
    }
    NR_first_time_2 = 2; // we need to do some more a bit later down
  }



  for (int k = 0; k < 2; k++)
  {
    // NR_FFT_buffer is 512 floats big
    // interleaved r, i, r, i . . .
    // fill first half of FFT_buffer with last events audio samples
    for (int i = 0; i < NR_FFT_L / 2; i++)
    {
      NR_FFT_buffer[i * 2] = NR_last_sample_buffer_L[i]; // real
      NR_FFT_buffer[i * 2 + 1] = 0.0; // imaginary
    }
    // copy recent samples to last_sample_buffer for next time!
    for (int i = 0; i < NR_FFT_L  / 2; i++)
    {
      NR_last_sample_buffer_L [i] = float_buffer_L[i + k * (NR_FFT_L / 2)];
    }
    // now fill recent audio samples into second half of FFT_buffer
    for (int i = 0; i < NR_FFT_L / 2; i++)
    {
      NR_FFT_buffer[NR_FFT_L + i * 2] = float_buffer_L[i + k * (NR_FFT_L / 2)]; // real
      NR_FFT_buffer[NR_FFT_L + i * 2 + 1] = 0.0;
    }
    /////////////////////////////////
    // WINDOWING
#if 1
    // perform windowing on samples in the NR_FFT_buffer
    for (int idx = 0; idx < NR_FFT_L; idx++)
    { // sqrt Hann window
      //float32_t temp_sample = 0.5 * (float32_t)(1.0 - (cosf(PI * 2.0 * (float32_t)idx / (float32_t)((NR_FFT_L) - 1))));
      //NR_FFT_buffer[idx * 2] *= temp_sample;
      NR_FFT_buffer[idx * 2] *= sqrtHann[idx];
    }
#endif

    // NR_FFT
    // calculation is performed in-place the FFT_buffer [re, im, re, im, re, im . . .]
    arm_cfft_f32(NR_FFT, NR_FFT_buffer, 0, 1);

    //##########################################################################################################################################
    //##########################################################################################################################################
    //##########################################################################################################################################

    for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
    {
      // this is squared magnitude for the current frame
      NR_X[bindx][0] = (NR_FFT_buffer[bindx * 2] * NR_FFT_buffer[bindx * 2] + NR_FFT_buffer[bindx * 2 + 1] * NR_FFT_buffer[bindx * 2 + 1]);
    }

    if (NR_first_time_2 == 2)
    { // TODO: properly initialize all the variables
      for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
      {
        NR_Nest[bindx][0] = NR_Nest[bindx][0] + 0.05 * NR_X[bindx][0]; // we do it 20 times to average over 20 frames for app. 100ms only on NR_on/bandswitch/modeswitch,...
        xt[bindx] = psini * NR_Nest[bindx][0];
      }
      NR_init_counter++;
      if (NR_init_counter > 19)//average over 20 frames for app. 100ms
      {
        NR_init_counter = 0;
        NR_first_time_2 = 3;  // now we did all the necessary initialization to actually start the noise reduction
      }
    }

    if (NR_first_time_2 == 3)
    {

      //new noise estimate MMSE based!!!

      for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // 1. Step of NR - calculate the SNR's
      {
        ph1y[bindx] = 1.0 / (1.0 + pfac * expf(xih1r * NR_X[bindx][0] / xt[bindx]));
        pslp[bindx] = ap * pslp[bindx] + (1.0 - ap) * ph1y[bindx];

        if (pslp[bindx] > psthr)
        {
          ph1y[bindx] = 1.0 - pnsaf;
        }
        else
        {
          ph1y[bindx] = fmin(ph1y[bindx] , 1.0);
        }
        xtr = (1.0 - ph1y[bindx]) * NR_X[bindx][0] + ph1y[bindx] * xt[bindx];
        xt[bindx] = ax * xt[bindx] + (1.0 - ax) * xtr;
      }



      for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // 1. Step of NR - calculate the SNR's
      {
        NR_SNR_post[bindx] = fmax(fmin(NR_X[bindx][0] / xt[bindx], 1000.0), snr_prio_min); // limited to +30 /-15 dB, might be still too much of reduction, let's try it?

        NR_SNR_prio[bindx] = fmax(NR_alpha * NR_Hk_old[bindx] + (1.0 - NR_alpha) * fmax(NR_SNR_post[bindx] - 1.0, 0.0), 0.0);
      }

      VAD_low = (int)lf_freq;
      VAD_high = (int)uf_freq;
      if (VAD_low == VAD_high)
      {
        VAD_high++;
      }
      if (VAD_low < 1)
      {
        VAD_low = 1;
      }
      else if (VAD_low > NR_FFT_L / 2 - 2)
      {
        VAD_low = NR_FFT_L / 2 - 2;
      }
      if (VAD_high < 1)
      {
        VAD_high = 1;
      }
      else if (VAD_high > NR_FFT_L / 2)
      {
        VAD_high = NR_FFT_L / 2;
      }

      // 4    calculate v = SNRprio(n, bin[i]) / (SNRprio(n, bin[i]) + 1) * SNRpost(n, bin[i]) (eq. 12 of Schmitt et al. 2002, eq. 9 of Romanin et al. 2009)
      //      and calculate the HK's

      for (int bindx = VAD_low; bindx < VAD_high; bindx++) // maybe we should limit this to the signal containing bins (filtering!!)
      {
        float32_t v = NR_SNR_prio[bindx] * NR_SNR_post[bindx] / (1.0 + NR_SNR_prio[bindx]);

        NR_G[bindx] = 1.0 / NR_SNR_post[bindx] * sqrtf((0.7212 * v + v * v));

        NR_Hk_old[bindx] = NR_SNR_post[bindx] * NR_G[bindx] * NR_G[bindx]; //
      }

      // MUSICAL NOISE TREATMENT HERE, DL2FW

      // musical noise "artefact" reduction by dynamic averaging - depending on SNR ratio
      pre_power = 0.0;
      post_power = 0.0;
      for (int bindx = VAD_low; bindx < VAD_high; bindx++)
      {
        pre_power += NR_X[bindx][0];
        post_power += NR_G[bindx] * NR_G[bindx]  * NR_X[bindx][0];
      }

      power_ratio = post_power / pre_power;
      if (power_ratio > power_threshold)
      {
        power_ratio = 1.0;
        NN = 1;
      }
      else
      {
        NN = 1 + 2 * (int)(0.5 + NR_width * (1.0 - power_ratio / power_threshold));
      }

      for (int bindx = VAD_low + NN / 2; bindx < VAD_high - NN / 2; bindx++)
      {
        NR_Nest[bindx][0] = 0.0;
        for (int m = bindx - NN / 2; m <= bindx + NN / 2; m++)
        {
          NR_Nest[bindx][0] += NR_G[m];
        }
        NR_Nest[bindx][0] /= (float32_t)NN;
      }

      // and now the edges - only going NN steps forward and taking the average
      // lower edge
      for (int bindx = VAD_low; bindx < VAD_low + NN / 2; bindx++)
      {
        NR_Nest[bindx][0] = 0.0;
        for (int m = bindx; m < (bindx + NN); m++)
        {
          NR_Nest[bindx][0] += NR_G[m];
        }
        NR_Nest[bindx][0] /= (float32_t)NN;
      }

      // upper edge - only going NN steps backward and taking the average
      for (int bindx = VAD_high - NN; bindx < VAD_high; bindx++)
      {
        NR_Nest[bindx][0] = 0.0;
        for (int m = bindx; m > (bindx - NN); m--)
        {
          NR_Nest[bindx][0] += NR_G[m];
        }
        NR_Nest[bindx][0] /= (float32_t)NN;
      }

      // end of edge treatment

      for (int bindx = VAD_low + NN / 2; bindx < VAD_high - NN / 2; bindx++)
      {
        NR_G[bindx] = NR_Nest[bindx][0];
      }
      // end of musical noise reduction


    } //end of "if ts.nr_first_time == 3"


    //##########################################################################################################################################
    //##########################################################################################################################################
    //##########################################################################################################################################

#if 1
    // FINAL SPECTRAL WEIGHTING: Multiply current FFT results with NR_FFT_buffer for 128 bins with the 128 bin-specific gain factors G
    //              for(int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // try 128:
    for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // try 128:
    {
      NR_FFT_buffer[bindx * 2] = NR_FFT_buffer [bindx * 2] * NR_G[bindx] * NR_long_tone_gain[bindx]; // real part
      NR_FFT_buffer[bindx * 2 + 1] = NR_FFT_buffer [bindx * 2 + 1] * NR_G[bindx] * NR_long_tone_gain[bindx]; // imag part
      NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 2] = NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 2] * NR_G[bindx] * NR_long_tone_gain[bindx]; // real part conjugate symmetric
      NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 1] = NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 1] * NR_G[bindx] * NR_long_tone_gain[bindx]; // imag part conjugate symmetric
    }

#endif
    /*****************************************************************
       NOISE REDUCTION CODE ENDS HERE
     *****************************************************************/
    // very interesting!
    // if I leave the FFT_buffer as is and just multiply the 19 bins below with 0.1, the audio
    // is distorted a little bit !
    // To me, this is an indicator of a problem with windowing . . .
    //

#if 0
    for (int bindx = 1; bindx < 20; bindx++)
      // bins 2 to 29 attenuated
      // set real values to 0.1 of their original value
    {
      NR_FFT_buffer[bindx * 2] *= 0.1;
      //      NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 2] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
      NR_FFT_buffer[bindx * 2 + 1] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
      //      NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 1] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
    }
#endif

    // NR_iFFT
    // perform iFFT (in-place)
    arm_cfft_f32(NR_iFFT, NR_FFT_buffer, 1, 1);

    // perform windowing on samples in the NR_FFT_buffer
    for (int idx = 0; idx < NR_FFT_L; idx++)
    { // sqrt Hann window
      NR_FFT_buffer[idx * 2] *= sqrtHann[idx];
    }

    // do the overlap & add
    for (int i = 0; i < NR_FFT_L / 2; i++)
    { // take real part of first half of current iFFT result and add to 2nd half of last iFFT_result
      //              NR_output_audio_buffer[i + k * (NR_FFT_L / 2)] = NR_FFT_buffer[i * 2] + NR_last_iFFT_result[i];
      float_buffer_L[i + k * (NR_FFT_L / 2)] = NR_FFT_buffer[i * 2] + NR_last_iFFT_result[i];
      float_buffer_R[i + k * (NR_FFT_L / 2)] = float_buffer_L[i + k * (NR_FFT_L / 2)];
      // FIXME: take out scaling !
      //            in_buffer[i + k * (NR_FFT_L / 2)] *= 0.3;
    }
    for (int i = 0; i < NR_FFT_L / 2; i++)
    {
      NR_last_iFFT_result[i] = NR_FFT_buffer[NR_FFT_L + i * 2];
    }
    // end of "for" loop which repeats the FFT_iFFT_chain two times !!!
  }

  /*      for(int i = 0; i < NR_FFT_L; i++)
        {
            float_buffer_L [i] = NR_output_audio_buffer[i];  // * 1.6; // * 9.0; // * 5.0;
            float_buffer_R [i] = float_buffer_L [i];
        } */
} // end of Romanin algorithm


void LMS_NoiseReduction(int16_t blockSize, float32_t *nrbuffer)
{
  static ulong    lms1_inbuf = 0, lms1_outbuf = 0;

  arm_copy_f32(nrbuffer, &LMS_nr_delay[lms1_inbuf], blockSize);  // put new data into the delay buffer
  //
  arm_lms_norm_f32(&LMS_Norm_instance, nrbuffer, &LMS_nr_delay[lms1_outbuf], nrbuffer, LMS_errsig1, blockSize);  // do noise reduction
  //
  //
  lms1_inbuf += blockSize;  // bump input to the next location in our de-correlation buffer
  lms1_outbuf = lms1_inbuf + blockSize; // advance output to same distance ahead of input
  lms1_inbuf %= 512;
  lms1_outbuf %= 512;
}


void Init_LMS_NR ()
{
  // Initialize LMS (DSP Noise reduction) filter
  //
  uint16_t  calc_taps = 96;
  float32_t mu_calc;

  LMS_Norm_instance.numTaps = calc_taps;
  LMS_Norm_instance.pCoeffs = LMS_NormCoeff_f32;
  LMS_Norm_instance.pState = LMS_StateF32;

  // Calculate "mu" (convergence rate) from user "DSP Strength" setting.  This needs to be significantly de-linearized to
  // squeeze a wide range of adjustment (e.g. several magnitudes) into a fairly small numerical range.
  mu_calc = LMS_nr_strength;   // get user setting

  // New DSP NR "mu" calculation method as of 0.0.214
  mu_calc /= 2; // scale input value
  mu_calc += 2; // offset zero value
  mu_calc /= 10;  // convert from "bels" to "deci-bels"
  mu_calc = powf(10, mu_calc);  // convert to ratio
  mu_calc = 1 / mu_calc;    // invert to fraction
  LMS_Norm_instance.mu = mu_calc;

  arm_fill_f32(0.0, LMS_nr_delay, 512 + 256);
  arm_fill_f32(0.0, LMS_StateF32, 96 + 256);

  // use "canned" init to initialize the filter coefficients
  arm_lms_norm_init_f32(&LMS_Norm_instance, calc_taps, &LMS_NormCoeff_f32[0], &LMS_StateF32[0], mu_calc, 256);

}

/**
   Fast algorithm for log10

   This is a fast approximation to log2()
   Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;
   log10f is exactly log2(x)/log2(10.0f)
   Math_log10f_fast(x) =(log2f_approx(x)*0.3010299956639812f)

   @param X number want log10 for
   @return log10(x)
*/
float32_t log10f_fast(float32_t X) {
  float Y, F;
  int E;
  F = frexpf(fabsf(X), &E);
  Y = 1.23149591368684f;
  Y *= F;
  Y += -4.11852516267426f;
  Y *= F;
  Y += 6.02197014179219f;
  Y *= F;
  Y += -3.13396450166353f;
  Y += E;
  return (Y * 0.3010299956639812f);
}

// https://www.mikrocontroller.net/topic/atan2-funktion-mit-lookup-table-fuer-arm

/* ----------------------------------------------------------------------
  $Date:        19. May 2012
  $Revision:    V1.0.0

  Project:      CMSIS DSP Library
  Title:        arm_atan2_f32.c

  Description:  Fast atan2 calculation for floating-point values.

  Target Processor: Cortex-M4/Cortex-M3/Cortex-M0
  -------------------------------------------------------------------- */

//#include "arm_math.h"

#define TABLE_SIZE_64 64

/**
   @ingroup groupFastMath
*/

/**
   @defgroup atan2 arc tangent

   Computes the trigonometric atan2 function using a combination of table lookup
   and cubic interpolation.
   atan2(y, x) = atan( y / x )

   The lookup table only contains values for angles [0  pi/4],
   other values are calculated case sensitive depending on magnitude proportion
   and negativeness.

   The implementation is based on table lookup using 64 values together with cubic interpolation.
   The steps used are:
    -# Calculation of the nearest integer table index
    -# Fetch the four table values a, b, c, and d
    -# Compute the fractional portion (fract) of the table index.
    -# Calculation of wa, wb, wc, wd
    -# The final result equals <code>a*wa + b*wb + c*wc + d*wd</code>

   where
   <pre>
      a=Table[index-1];
      b=Table[index+0];
      c=Table[index+1];
      d=Table[index+2];
   </pre>
   and
   <pre>
      wa=-(1/6)*fract.^3 + (1/2)*fract.^2 - (1/3)*fract;
      wb=(1/2)*fract.^3 - fract.^2 - (1/2)*fract + 1;
      wc=-(1/2)*fract.^3+(1/2)*fract.^2+fract;
      wd=(1/6)*fract.^3 - (1/6)*fract;
   </pre>
*/

/**
   @addtogroup atan2
   @{
*/

/* pi value is  3.14159265358979 */


static const float32_t atanTable[68] = {
  -0.015623728620477f,
  0.000000000000000f,  // = 0 for in = 0.0
  0.015623728620477f,
  0.031239833430268f,
  0.046840712915970f,
  0.062418809995957f,
  0.077966633831542f,
  0.093476781158590f,
  0.108941956989866f,
  0.124354994546761f,
  0.139708874289164f,
  0.154996741923941f,
  0.170211925285474f,
  0.185347949995695f,
  0.200398553825879f,
  0.215357699697738f,
  0.230219587276844f,
  0.244978663126864f,
  0.259629629408258f,
  0.274167451119659f,
  0.288587361894077f,
  0.302884868374971f,
  0.317055753209147f,
  0.331096076704132f,
  0.345002177207105f,
  0.358770670270572f,
  0.372398446676754f,
  0.385882669398074f,
  0.399220769575253f,
  0.412410441597387f,
  0.425449637370042f,
  0.438336559857958f,
  0.451069655988523f,
  0.463647609000806f,
  0.476069330322761f,
  0.488333951056406f,
  0.500440813147294f,
  0.512389460310738f,
  0.524179628782913f,
  0.535811237960464f,
  0.547284380987437f,
  0.558599315343562f,
  0.569756453482978f,
  0.580756353567670f,
  0.591599710335111f,
  0.602287346134964f,
  0.612820202165241f,
  0.623199329934066f,
  0.633425882969145f,
  0.643501108793284f,
  0.653426341180762f,
  0.663202992706093f,
  0.672832547593763f,
  0.682316554874748f,
  0.691656621853200f,
  0.700854407884450f,
  0.709911618463525f,
  0.718829999621625f,
  0.727611332626511f,
  0.736257428981428f,
  0.744770125716075f,
  0.753151280962194f,
  0.761402769805578f,
  0.769526480405658f,
  0.777524310373348f,
  0.785398163397448f,  // = pi/4 for in = 1.0
  0.793149946109655f,
  0.800781565178043f
};


/**
   @brief  Fast approximation to the trigonometric atan2 function for floating-point data.
   @param[in] x input value, y input value.
   @return  atan2(y, x) = atan(y/x) as radians.
*/

float32_t arm_atan2_f32(float32_t y, float32_t x)
{
  float32_t atan2Val, fract, in;                 /* Temporary variables for input, output */
  uint32_t index;                                /* Index variable */
  uint32_t tableSize = (uint32_t) TABLE_SIZE_64; /* Initialise tablesize */
  float32_t wa, wb, wc, wd;                      /* Cubic interpolation coefficients */
  float32_t a, b, c, d;                          /* Four nearest output values */
  float32_t *tablePtr;                           /* Pointer to table */
  uint8_t flags = 0;                             /* flags providing information about input values:
                                                    Bit0 = 1 if |x| < |y|
                                                    Bit1 = 1 if x < 0
                                                    Bit2 = 1 if y < 0 */

  /* calculate magnitude of input values */
  if (x < 0.0f) {
    x = -x;
    flags |= 0x02;
  }

  if (y < 0.0f) {
    y = -y;
    flags |= 0x04;
  }

  /* calculate in value for LUT [0 1] */
  if (x < y) {
    in = x / y;
    flags |= 0x01;
  }
  else { /* x >= y */
    if (x > 0.0f)
      in = y / x;
    else /* both are 0.0 */
      in = 0.0; /* prevent division by 0 */
  }

  /* Calculation of index of the table */
  index = (uint32_t) (tableSize * in);

  /* fractional value calculation */
  fract = ((float32_t) tableSize * in) - (float32_t) index;

  /* Initialise table pointer */
  tablePtr = (float32_t *) & atanTable[index];

  /* Read four nearest values of output value from the sin table */
  a = *tablePtr++;
  b = *tablePtr++;
  c = *tablePtr++;
  d = *tablePtr++;

  /* Cubic interpolation process */
  wa = -(((0.166666667f) * (fract * (fract * fract))) +
         ((0.3333333333333f) * fract)) + ((0.5f) * (fract * fract));
  wb = (((0.5f) * (fract * (fract * fract))) -
        ((fract * fract) + ((0.5f) * fract))) + 1.0f;
  wc = (-((0.5f) * (fract * (fract * fract))) +
        ((0.5f) * (fract * fract))) + fract;
  wd = ((0.166666667f) * (fract * (fract * fract))) -
       ((0.166666667f) * fract);

  /* Calculate atan2 value */
  atan2Val = ((a * wa) + (b * wb)) + ((c * wc) + (d * wd));

  /* exchanged input values? */
  if (flags & 0x01)
    /* output = pi/2 - output */
    atan2Val = 1.5707963267949f - atan2Val;

  /* negative x input? Quadrant 2 or 3 */
  if (flags & 0x02)
    atan2Val = 3.14159265358979f - atan2Val;

  /* negative y input? Quadrant 3 or 4 */
  if (flags & 0x04)
    atan2Val = - atan2Val;

  /* Return the output value */
  return (atan2Val);
}

/**
   @} end of atan2 group
*/
