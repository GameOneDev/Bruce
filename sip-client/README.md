# SIP Client for M5Stack StickC Plus2

A complete Session Initiation Protocol (SIP) client implementation for the M5Stack StickC Plus2, enabling VoIP calls with real-time audio streaming.

## Features

### ✅ Network Connectivity
- WiFi connection with configurable SSID and password
- Automatic reconnection on connection loss
- Connection status display on LCD

### ✅ SIP Protocol Support
- SIP REGISTER - Registration with SIP server
- SIP INVITE - Outgoing call initiation
- SIP response handling - Incoming call support
- SIP BYE - Call termination
- Basic SIP authentication support
- Real-time SIP message debugging

### ✅ Audio Handling
- **Input**: I2S microphone (SPM1423) with 8kHz sampling
- **Output**: Built-in buzzer (tone generation only - NOT a speaker)
- **Codec**: G.711 μ-law/A-law compression
- **Protocol**: RTP (Real-time Transport Protocol) for audio streaming
- Low-latency real-time audio processing
- **Note**: Audio playback requires external speaker (buzzer cannot play voice audio)

### ✅ User Interface
- Status display showing:
  - WiFi connection status
  - SIP registration status
  - Call status (Idle, Calling, Ringing, In Call)
  - Call duration timer
- Button controls:
  - **Button A**: Make call / Answer incoming call
  - **Button B**: Hang up / Reject call
  - **Power Button**: Device power control

### ✅ Serial Commands
Interactive serial console for advanced control:
- `CALL <sip-uri>` - Make a call to any SIP URI
- `HANGUP` - End current call
- `STATUS` - Display system status
- `REGISTER` - Re-register with SIP server
- `HELP` - Show command help

## Hardware Requirements

### Required Hardware
- **M5Stack StickC Plus2** (ESP32-PICO with built-in peripherals)
  - Display: ST7789 135x240 TFT LCD
  - Microphone: SPM1423 I2S PDM Microphone
  - Buzzer: GPIO 2 (tone generation only - **NOT a speaker**)
  - Buttons: A, B, and Power buttons
  - Battery: Built-in rechargeable battery

### Optional Hardware (Recommended)
- **External speaker** - **REQUIRED for audio playback** (can be connected via Grove port with I2S DAC or analog amplifier)
  - The M5StickCPlus2 only has a buzzer, which cannot play voice audio
  - For two-way voice calls, an external speaker is necessary
- External antenna for better WiFi range

## Software Requirements

- **PlatformIO** - Build system and IDE
- **Arduino Framework** for ESP32
- Compatible SIP server (Asterisk, FreeSWITCH, or any standard SIP server)

## Installation

### 1. Clone or Download

Download this project or clone the repository:
```bash
git clone <repository-url>
cd sip-client
```

### 2. Configure Settings

Copy the example configuration file and edit it with your credentials:

```bash
cp include/config.example.h include/config.h
```

**Note:** The `config.h` file is excluded from git to protect your credentials. Always use `config.example.h` as the template.

Then edit `include/config.h` to set your credentials:

```cpp
// WiFi Configuration
#define WIFI_SSID     "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// SIP Server Configuration
#define SIP_SERVER_IP    "192.168.1.100"    // Your SIP server IP
#define SIP_SERVER_PORT  5060                // SIP port (usually 5060)

// SIP Account Credentials
#define SIP_USER         "1001"              // Your extension/username
#define SIP_PASSWORD     "yourpassword"      // Your SIP password
#define SIP_DOMAIN       "yourdomain.com"    // Your SIP domain

// Default number to call
#define DEFAULT_CALL_URI "sip:1002@yourdomain.com"
```

### 3. Build and Upload

Using PlatformIO CLI:
```bash
# Build the project
pio run

# Upload to M5Stack StickC Plus2
pio run --target upload

# Monitor serial output
pio device monitor
```

Using PlatformIO IDE (VS Code):
1. Open the project folder in VS Code
2. Click the PlatformIO icon in the sidebar
3. Click "Build" to compile
4. Click "Upload" to flash to device
5. Click "Serial Monitor" to view output

## Usage

### First Time Setup

1. **Power on the device** - Press and hold the power button
2. **Wait for WiFi connection** - Device will automatically connect to configured WiFi
3. **Automatic SIP registration** - Device registers with SIP server automatically
4. **Ready state** - Display shows "Ready" when registered

### Making a Call

**Method 1: Using Button A**
- Press Button A to call the default number (configured in DEFAULT_CALL_URI)

**Method 2: Using Serial Command**
```
CALL sip:1002@yourdomain.com
```

### Receiving a Call

1. When an incoming call arrives, the display shows "Ringing"
2. Press **Button A** to answer the call
3. Press **Button B** to reject the call

### During a Call

- **Audio Input**: Speak into the built-in microphone
- **Audio Output**: **Requires external speaker** (buzzer cannot play voice audio)
  - The M5StickCPlus2's buzzer can only generate tones, not play voice
  - Connect an external speaker via Grove port for audio playback
- **Hang up**: Press Button B to end the call
- **Call duration**: Displayed on screen in MM:SS format

### Ending a Call

- Press **Button B** to hang up
- Or use serial command: `HANGUP`

## Configuration Options

### Audio Configuration

In `include/config.h`, you can adjust audio settings:

```cpp
// Choose codec (μ-law for US, A-law for international)
#define AUDIO_CODEC_MULAW  1  // 1 = μ-law, 0 = A-law

// Audio sample rate (8000 Hz is standard for VoIP)
#define AUDIO_SAMPLE_RATE  8000

// Audio buffer size (20ms @ 8kHz)
#define AUDIO_BUFFER_SIZE  160
```

### Network Configuration

```cpp
// WiFi connection timeout
#define WIFI_CONNECT_TIMEOUT 15000  // 15 seconds

// SIP re-registration interval
#define SIP_REGISTER_INTERVAL 3600  // 1 hour

// Local SIP and RTP ports
#define LOCAL_SIP_PORT 5060
#define RTP_PORT_MIN 10000
#define RTP_PORT_MAX 20000
```

### UI Configuration

```cpp
// Display colors (RGB565 format)
#define COLOR_BG       0x0000  // Black
#define COLOR_TEXT     0xFFFF  // White
#define COLOR_STATUS   0x07E0  // Green
#define COLOR_ERROR    0xF800  // Red

// Button debounce time
#define BUTTON_DEBOUNCE_MS 50
```

### Debug Configuration

Enable detailed debug output:

```cpp
#define DEBUG_ENABLED     1
#define DEBUG_SIP         1  // SIP protocol messages
#define DEBUG_AUDIO       1  // Audio processing
#define DEBUG_NETWORK     1  // Network operations
#define DEBUG_UI          1  // UI events
```

## SIP Server Setup

### Using Asterisk

Example `extensions.conf`:
```ini
[default]
exten => 1001,1,Dial(SIP/1001)
exten => 1002,1,Dial(SIP/1002)
```

Example `sip.conf`:
```ini
[1001]
type=friend
secret=password
host=dynamic
context=default
```

### Using FreeSWITCH

Create a user in the directory with appropriate permissions for registration and calling.

### Port Forwarding

If your SIP server is on a different network, ensure these ports are forwarded:
- UDP 5060 (SIP signaling)
- UDP 10000-20000 (RTP audio)

## Troubleshooting

### WiFi Connection Issues

**Problem**: Device cannot connect to WiFi
- **Solution**: Verify SSID and password in `config.h`
- **Solution**: Check WiFi signal strength
- **Solution**: Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)

### SIP Registration Fails

**Problem**: "Registering" status doesn't change to "Registered"
- **Solution**: Verify SIP server IP and port
- **Solution**: Check SIP credentials (username, password, domain)
- **Solution**: Ensure SIP server is accessible from device's network
- **Solution**: Check firewall settings on SIP server

### No Audio During Call

**Problem**: Call connects but no audio
- **Solution**: **Connect external speaker** - M5StickCPlus2 only has a buzzer (cannot play voice audio)
- **Solution**: Verify RTP ports are not blocked by firewall
- **Solution**: Check microphone is working (test with other M5Stack apps)
- **Solution**: For audio playback, connect I2S DAC or analog amplifier to Grove port

### Call Doesn't Connect

**Problem**: Call shows "Calling" but never connects
- **Solution**: Verify the destination SIP URI is correct
- **Solution**: Check that destination is registered on SIP server
- **Solution**: Review SIP server logs for errors

### Serial Monitor Shows Errors

Enable debug output and check for specific error messages:
```
pio device monitor
```

Common errors:
- `Failed to start SIP socket` - Port already in use, restart device
- `WiFi connection timeout` - Check WiFi credentials
- `Failed to initialize microphone` - I2S configuration issue

## Technical Details

### Audio Pipeline

```
Microphone (SPM1423) 
    → I2S Input (8kHz, 16-bit mono)
    → G.711 Encoding (μ-law/A-law)
    → RTP Packetization
    → UDP Transmission
```

```
UDP Reception
    → RTP Depacketization
    → G.711 Decoding
    → Audio Buffer
    → External Speaker Output (buzzer cannot play audio)
```

### SIP Call Flow

```
1. REGISTER → SIP Server
2. 200 OK ← SIP Server
   [Device is now registered]

3. INVITE → Destination
4. 100 Trying ← SIP Server
5. 180 Ringing ← Destination
6. 200 OK ← Destination
7. ACK → Destination
   [Call is established, RTP audio flows]

8. BYE → Destination (when hanging up)
9. 200 OK ← Destination
   [Call ended]
```

### Hardware Pin Mapping

| Function | GPIO | Description |
|----------|------|-------------|
| Display CS | 5 | TFT Chip Select |
| Display DC | 14 | TFT Data/Command |
| Display RST | 12 | TFT Reset |
| Display BL | 27 | Backlight Control |
| Display MOSI | 15 | SPI MOSI |
| Display CLK | 13 | SPI Clock |
| Mic WS | 34 | I2S Word Select (L/R) |
| Mic CLK | 0 | I2S Serial Clock |
| Buzzer | 2 | Buzzer Output (tone only - NOT a speaker) |
| Button A | 37 | Call/Answer Button |
| Button B | 39 | Hangup Button |
| Button PWR | 35 | Power Button |
| I2C SDA | 32 | Grove SDA |
| I2C SCL | 33 | Grove SCL |
| Battery ADC | 38 | Battery Voltage |

## Known Limitations

1. **No Built-in Audio Playback**: The M5StickCPlus2 **does NOT have a speaker**. It only has a buzzer on GPIO 2 that can generate simple tones. **For audio playback during calls, you MUST connect an external speaker** via the Grove port using an I2S DAC or analog amplifier.
2. **Codec Support**: Currently only G.711 μ-law/A-law is supported. More codecs (G.722, Opus) could be added.
3. **DTMF**: DTMF tone generation is not yet implemented.
4. **Multiple Calls**: Only one call at a time is supported.
5. **Authentication**: Basic authentication support only (no digest authentication).
6. **NAT Traversal**: No STUN/TURN support for complex NAT scenarios.
7. **ESP-IDF Compatibility**: I2S microphone capture uses legacy ESP-IDF API. On newer ESP32 variants (ESP32-S3, etc.) with ESP-IDF 4.4+, audio capture is disabled and RTP packets contain silence. For full audio support on these platforms, the I2S code needs to be updated to use the new I2S driver API.

## Future Enhancements

- [ ] DTMF tone generation for interactive menus
- [ ] Contact list stored in flash memory
- [ ] Call history and logs
- [ ] Additional codec support (G.722, Opus)
- [ ] SIP digest authentication
- [ ] STUN/TURN support for NAT traversal
- [ ] Conference calling
- [ ] Call recording to SD card
- [ ] Web interface for configuration
- [ ] Bluetooth headset support

## License

This project is provided as-is for educational and personal use. Modify and distribute as needed.

## Support

For issues, questions, or contributions:
1. Check the troubleshooting section above
2. Review debug output with `DEBUG_ENABLED` set to 1
3. Consult the code comments for implementation details

## Credits

- M5Stack for the excellent StickC Plus2 hardware
- ESP32 Arduino framework
- SIP/RTP protocol implementations based on RFC 3261 and RFC 3550

## Disclaimer

This is a basic SIP client implementation intended for educational purposes and simple VoIP applications. It may not be suitable for production environments without additional security and reliability enhancements.
