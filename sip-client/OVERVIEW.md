# SIP Client - Project Overview

## Introduction

This is a complete Session Initiation Protocol (SIP) client implementation for the M5Stack StickC Plus2 ESP32-based device. It enables full-duplex VoIP (Voice over IP) calls with real-time audio streaming.

## Architecture

### System Components

```
┌─────────────────────────────────────────────────────────┐
│                    M5Stack StickC Plus2                 │
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │              │  │              │  │              │ │
│  │  UI Handler  │  │  SIP Client  │  │    Audio     │ │
│  │              │  │              │  │   Handler    │ │
│  │  - Display   │  │  - REGISTER  │  │  - I2S Mic   │ │
│  │  - Buttons   │  │  - INVITE    │  │  - Speaker   │ │
│  │  - Status    │  │  - BYE       │  │  - G.711     │ │
│  │              │  │  - Response  │  │  - RTP       │ │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘ │
│         │                 │                 │         │
│         └─────────────────┴─────────────────┘         │
│                           │                           │
│                      ┌────┴────┐                      │
│                      │  WiFi   │                      │
│                      └─────────┘                      │
└──────────────────────────┬──────────────────────────────┘
                           │
                           │ UDP 5060 (SIP)
                           │ UDP 10000+ (RTP)
                           │
                    ┌──────┴──────┐
                    │ SIP Server  │
                    │  (Asterisk/ │
                    │ FreeSWITCH) │
                    └─────────────┘
```

### Communication Flow

#### Call Establishment

```
M5Stack                    SIP Server              Remote Phone
   │                            │                        │
   │ 1. REGISTER               │                        │
   │──────────────────────────>│                        │
   │                            │                        │
   │ 2. 200 OK                 │                        │
   │<──────────────────────────│                        │
   │                            │                        │
   │ 3. INVITE                 │                        │
   │──────────────────────────>│                        │
   │                            │ 4. INVITE             │
   │                            │──────────────────────>│
   │                            │                        │
   │                            │ 5. 180 Ringing        │
   │ 6. 180 Ringing            │<──────────────────────│
   │<──────────────────────────│                        │
   │                            │                        │
   │                            │ 7. 200 OK             │
   │ 8. 200 OK                 │<──────────────────────│
   │<──────────────────────────│                        │
   │                            │                        │
   │ 9. ACK                    │                        │
   │──────────────────────────>│ 10. ACK               │
   │                            │──────────────────────>│
   │                            │                        │
   │<======== RTP Audio Stream ========================>│
   │                            │                        │
   │ 11. BYE                   │                        │
   │──────────────────────────>│ 12. BYE               │
   │                            │──────────────────────>│
   │                            │                        │
   │                            │ 13. 200 OK            │
   │ 14. 200 OK                │<──────────────────────│
   │<──────────────────────────│                        │
```

#### Audio Pipeline

```
Outgoing Audio:
┌─────────────┐    ┌──────────┐    ┌──────────┐    ┌─────────┐
│ Microphone  │───>│ I2S Read │───>│ G.711    │───>│   RTP   │
│  SPM1423    │    │  8kHz    │    │ Encode   │    │ Packet  │
└─────────────┘    └──────────┘    └──────────┘    └────┬────┘
                                                         │
                                                         v
                                                    ┌─────────┐
                                                    │   UDP   │
                                                    │  Send   │
                                                    └─────────┘

Incoming Audio:
┌─────────────┐    ┌──────────┐    ┌──────────┐    ┌─────────┐
│   Speaker   │<───│ Speaker  │<───│ G.711    │<───│   RTP   │
│  (Buzzer)   │    │  Output  │    │ Decode   │    │ Packet  │
└─────────────┘    └──────────┘    └──────────┘    └────┬────┘
                                                         ^
                                                         │
                                                    ┌─────────┐
                                                    │   UDP   │
                                                    │ Receive │
                                                    └─────────┘
```

## File Structure

```
sip-client/
│
├── platformio.ini              # Build configuration
│   └── Defines board, libraries, build flags
│
├── Documentation
│   ├── README.md               # Main documentation
│   ├── QUICKSTART.md           # 5-minute setup guide
│   ├── SETUP_GUIDE.md          # Detailed setup for different servers
│   └── OVERVIEW.md             # This file
│
├── Configuration
│   ├── include/config.h        # User configuration (not in git)
│   └── include/config.example.h # Example configuration
│
├── Headers (include/)
│   ├── sip_client.h            # SIP protocol handler
│   ├── audio_handler.h         # Audio processing and RTP
│   └── ui_handler.h            # Display and button handling
│
├── Implementation (src/)
│   ├── main.cpp                # Application entry point
│   ├── sip_client.cpp          # SIP protocol implementation
│   ├── audio_handler.cpp       # Audio and codec implementation
│   └── ui_handler.cpp          # UI implementation
│
└── Tools
    └── validate.py             # Code validation script
```

## Key Features

### 1. SIP Protocol Implementation

**Supported Methods:**
- REGISTER - Registration with SIP server
- INVITE - Call initiation
- ACK - Call acknowledgment
- BYE - Call termination
- Response handling (100, 180, 200, 401, etc.)

**Features:**
- Automatic re-registration
- Basic authentication support
- SDP (Session Description Protocol) parsing
- Call state management

### 2. Audio System

**Input:**
- SPM1423 I2S PDM Microphone
- 8kHz sampling rate
- 16-bit samples
- Mono channel

**Output:**
- Built-in buzzer/speaker
- GPIO-based output
- PWM generation (simplified)

**Codec:**
- G.711 μ-law (North America standard)
- G.711 A-law (International standard)
- 8000 Hz sample rate
- ~64 kbps bitrate

**Transport:**
- RTP (Real-time Transport Protocol)
- 20ms packet size (160 samples)
- Sequence numbering
- Timestamp management

### 3. User Interface

**Display:**
- ST7789 135x240 TFT LCD
- Status indicators
- Call information
- Duration timer

**Controls:**
- Button A (GPIO 37): Call/Answer
- Button B (GPIO 39): Hang up
- Power Button (GPIO 35): Power management

**Status Display:**
- WiFi connection state
- SIP registration state
- Call state (Idle, Calling, Ringing, In Call)
- Call duration
- Visual indicators

### 4. Network Layer

**WiFi:**
- WPA2-PSK authentication
- Auto-reconnect
- Connection monitoring

**UDP Sockets:**
- SIP signaling (port 5060)
- RTP audio (ports 10000-20000)
- Non-blocking I/O

## Hardware Pin Mapping

| Function | GPIO | Details |
|----------|------|---------|
| **Display** | | |
| TFT CS | 5 | Chip Select |
| TFT DC | 14 | Data/Command |
| TFT RST | 12 | Reset |
| TFT BL | 27 | Backlight |
| TFT MOSI | 15 | SPI MOSI |
| TFT CLK | 13 | SPI Clock |
| **Audio** | | |
| Mic WS | 34 | I2S Word Select |
| Mic CLK | 0 | I2S Clock |
| Speaker | 2 | PWM Output |
| **Buttons** | | |
| Button A | 37 | Call/Answer |
| Button B | 39 | Hang up |
| Button PWR | 35 | Power |
| **I2C** | | |
| SDA | 32 | Grove SDA |
| SCL | 33 | Grove SCL |
| **Other** | | |
| Battery | 38 | ADC for battery voltage |

## Configuration Options

### Essential Settings

| Setting | Description | Example |
|---------|-------------|---------|
| WIFI_SSID | WiFi network name | "MyHomeWiFi" |
| WIFI_PASSWORD | WiFi password | "password123" |
| SIP_SERVER_IP | SIP server address | "192.168.1.100" |
| SIP_USER | SIP username | "1001" |
| SIP_PASSWORD | SIP password | "secret" |
| SIP_DOMAIN | SIP domain | "192.168.1.100" |

### Audio Settings

| Setting | Description | Default |
|---------|-------------|---------|
| AUDIO_CODEC_MULAW | Codec selection | 1 (μ-law) |
| AUDIO_SAMPLE_RATE | Sample rate | 8000 Hz |
| AUDIO_BUFFER_SIZE | Buffer size | 160 samples |
| SPEAKER_VOLUME | Output volume | 50 (0-100) |

### Debug Settings

| Setting | Description | Default |
|---------|-------------|---------|
| DEBUG_ENABLED | Enable debug | 1 |
| DEBUG_SIP | SIP messages | 1 |
| DEBUG_AUDIO | Audio debug | 1 |
| DEBUG_NETWORK | Network debug | 1 |

## Performance Characteristics

### Resource Usage

| Resource | Usage |
|----------|-------|
| Flash Memory | ~800 KB |
| RAM | ~100 KB |
| CPU (idle) | ~5% |
| CPU (in call) | ~40% |

### Network Bandwidth

| Direction | Bandwidth |
|-----------|-----------|
| Upstream (mic) | ~64 kbps |
| Downstream (speaker) | ~64 kbps |
| Total | ~128 kbps |

### Latency

| Component | Latency |
|-----------|---------|
| Audio capture | 20 ms |
| Encoding | 1-2 ms |
| Network (local) | 10-30 ms |
| Decoding | 1-2 ms |
| Audio output | 20 ms |
| **Total (typical)** | **50-75 ms** |

## Dependencies

### PlatformIO Libraries

| Library | Purpose |
|---------|---------|
| M5StickCPlus2 | Hardware abstraction |
| TFT_eSPI | Display driver |
| WiFi | Network connectivity |
| WiFiUdp | UDP sockets |
| ArduinoJson | JSON parsing (optional) |

### System Requirements

- ESP32 Arduino Framework 2.0+
- PlatformIO Core 6.0+
- Python 3.7+ (for validation)

## Testing Strategy

### Unit Testing

Currently manual testing:
1. WiFi connection test
2. SIP registration test
3. Call establishment test
4. Audio loop test
5. Button response test

### Integration Testing

1. End-to-end call test
2. Echo test (verifies full audio path)
3. Multiple call test
4. Reconnection test

### Recommended Test Equipment

- Another SIP phone (hardware or software)
- SIP server with logging enabled
- Network packet analyzer (Wireshark)
- Audio recording equipment

## Known Limitations

### Current Version

1. **Single call only** - No call waiting
2. **No DTMF** - Tone generation not implemented
3. **Basic auth only** - No digest authentication
4. **No NAT traversal** - No STUN/TURN support
5. **Limited codecs** - Only G.711
6. **Speaker quality** - Built-in speaker has limitations

### Workarounds

1. **Better audio** - Use external speaker via Grove port
2. **NAT issues** - Use port forwarding or local SIP server
3. **Codec support** - Use G.711 on SIP server

## Future Enhancements

### Planned Features

- [ ] DTMF tone generation
- [ ] Multiple call support
- [ ] Call history
- [ ] Contact list
- [ ] SIP digest authentication
- [ ] STUN/TURN support
- [ ] Additional codecs (G.722, Opus)
- [ ] Bluetooth headset support
- [ ] SD card call recording
- [ ] Web configuration interface

### Nice to Have

- [ ] Call forwarding
- [ ] Conference calling
- [ ] Voicemail integration
- [ ] Presence/BLF support
- [ ] Encrypted SIP (TLS)
- [ ] SRTP for audio encryption

## Contributing

### Code Style

- Follow Arduino coding conventions
- Use descriptive variable names
- Comment complex sections
- Keep functions focused and small

### Testing Requirements

- Test WiFi connectivity
- Test SIP registration
- Test call establishment
- Test audio quality
- Document any issues

## License

This project is provided as-is for educational and personal use.

## Credits

### Hardware
- M5Stack for the StickC Plus2 platform

### Protocols
- SIP: RFC 3261
- RTP: RFC 3550
- SDP: RFC 4566
- G.711: ITU-T G.711

### Inspiration
- Various open-source SIP implementations
- ESP32 audio projects
- VoIP community

## Version History

### v1.0 (Current)
- Initial release
- Basic SIP client functionality
- G.711 audio codec
- RTP audio streaming
- UI with button controls
- Serial command interface

---

For questions or support, please refer to the README.md and SETUP_GUIDE.md files.
