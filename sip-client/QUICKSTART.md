# Quick Start Guide

Get your M5Stack StickC Plus2 SIP client up and running in 5 minutes!

## Prerequisites

- [ ] M5Stack StickC Plus2 device
- [ ] USB-C cable
- [ ] Computer with PlatformIO installed
- [ ] WiFi network (2.4GHz)
- [ ] SIP server or account

## 5-Minute Setup

### Step 1: Download the Project (30 seconds)

```bash
git clone <repository-url>
cd sip-client
```

### Step 2: Configure (1 minute)

Copy and edit the configuration file:

```bash
cp include/config.example.h include/config.h
nano include/config.h  # or use your favorite editor
```

**Minimum required changes:**

```cpp
// WiFi Settings
#define WIFI_SSID     "YourWiFiName"      // ← Change this
#define WIFI_PASSWORD "YourWiFiPassword"  // ← Change this

// SIP Settings
#define SIP_SERVER_IP    "192.168.1.100"  // ← Your SIP server IP
#define SIP_USER         "1001"            // ← Your extension
#define SIP_PASSWORD     "yourpassword"    // ← Your SIP password
#define DEFAULT_CALL_URI "sip:1002@192.168.1.100"  // ← Number to call
```

### Step 3: Build and Upload (3 minutes)

```bash
# Build the firmware
pio run

# Upload to M5Stack
pio run --target upload

# Watch the logs (optional)
pio device monitor
```

### Step 4: Test (30 seconds)

1. **Power on** the device
2. **Wait** for "Registered" status on screen
3. **Press Button A** to make a call
4. **Press Button B** to hang up

## Quick Reference Card

### Device Layout

```
┌─────────────────────┐
│  ┌───────────────┐  │  ← Display (240x135)
│  │               │  │
│  │   SIP Client  │  │
│  │               │  │
│  └───────────────┘  │
│                     │
│   [A]  [PWR]  [B]   │  ← Buttons
│                     │
│   🎤 Microphone     │  ← Built-in mic
│   🔊 Speaker        │  ← Built-in speaker
└─────────────────────┘
```

### Button Functions

| Button | Idle State | During Call |
|--------|------------|-------------|
| **A** (GPIO 37) | Make call | Mute/Unmute* |
| **B** (GPIO 39) | Menu* | Hang up |
| **PWR** (GPIO 35) | Power on/off | - |

*Feature coming soon

### Display Status

| Message | Meaning |
|---------|---------|
| "Disconnected" | No WiFi connection |
| "Connecting..." | Connecting to WiFi |
| "Registering" | Registering with SIP server |
| "Ready" | Ready to make/receive calls |
| "Calling..." | Dialing outgoing call |
| "Ringing" | Incoming call or call ringing |
| "In Call" | Active call in progress |

### Serial Commands

Connect via serial (115200 baud) and use these commands:

```
CALL sip:1002@192.168.1.100    Make a call
HANGUP                         End current call
STATUS                         Show system status
REGISTER                       Re-register with SIP server
HELP                          Show command help
```

## Common Configurations

### Home Asterisk Server

```cpp
#define SIP_SERVER_IP    "192.168.1.100"
#define SIP_USER         "1001"
#define SIP_PASSWORD     "password123"
#define SIP_DOMAIN       "192.168.1.100"
#define DEFAULT_CALL_URI "sip:1002@192.168.1.100"
```

### Cloud SIP Provider

```cpp
#define SIP_SERVER_IP    "sip.yourprovider.com"
#define SIP_USER         "yourphone@yourprovider.com"
#define SIP_PASSWORD     "yourpassword"
#define SIP_DOMAIN       "yourprovider.com"
#define DEFAULT_CALL_URI "sip:+15551234567@yourprovider.com"
```

## Troubleshooting Quick Fixes

### Problem: Won't connect to WiFi
```
Solution: Check SSID/password, ensure 2.4GHz network
```

### Problem: Won't register with SIP server
```
Solution: Verify server IP, check credentials, ensure server is running
```

### Problem: Call connects but no audio
```
Solution: Check firewall allows UDP ports 10000-20000
```

### Problem: Can't hear caller
```
Solution: Increase volume in config, check speaker connection
```

## Testing Your Setup

### Test 1: Echo Test

Configure to call an echo extension:

```cpp
#define DEFAULT_CALL_URI "sip:1000@192.168.1.100"  // Echo extension
```

Press Button A. You should hear your own voice with a small delay.

### Test 2: Two-Way Call

1. Set up two SIP accounts (1001 and 1002)
2. Configure M5Stack with account 1001
3. Configure a softphone with account 1002
4. Call from M5Stack to softphone
5. Test both directions

## Next Steps

Once basic calling works:

1. **Read** the full [README.md](README.md) for features
2. **Check** [SETUP_GUIDE.md](SETUP_GUIDE.md) for advanced setup
3. **Customize** audio settings in `config.h`
4. **Explore** serial commands for advanced control
5. **Monitor** debug output for troubleshooting

## Getting Help

1. Enable debug output in `config.h`:
   ```cpp
   #define DEBUG_ENABLED 1
   #define DEBUG_SIP     1
   #define DEBUG_AUDIO   1
   ```

2. Connect to serial monitor:
   ```bash
   pio device monitor
   ```

3. Check output for error messages

4. Review logs on your SIP server

## Important Notes

⚠️ **WiFi Requirements**
- Must be 2.4GHz network (ESP32 doesn't support 5GHz)
- WPA2-PSK security recommended

⚠️ **Audio Quality**
- Built-in speaker has limited quality
- External speaker recommended for better audio
- Ensure good WiFi signal strength

⚠️ **SIP Server**
- Must be accessible from device's network
- Firewall must allow UDP 5060 and RTP ports
- Use G.711 codec (PCMU or PCMA)

## Specifications

| Feature | Specification |
|---------|--------------|
| **WiFi** | 802.11 b/g/n (2.4GHz) |
| **SIP** | RFC 3261 compliant |
| **Audio Codec** | G.711 μ-law/A-law |
| **Sample Rate** | 8000 Hz |
| **Protocol** | RTP (RFC 3550) |
| **Latency** | ~40-100ms typical |

## Default Ports

| Service | Port | Protocol |
|---------|------|----------|
| SIP signaling | 5060 | UDP |
| RTP audio | 10000-20000 | UDP |

---

**That's it! You're ready to make VoIP calls with your M5Stack StickC Plus2!** 📞

For more details, see:
- [README.md](README.md) - Complete feature documentation
- [SETUP_GUIDE.md](SETUP_GUIDE.md) - Detailed setup instructions
- [config.h](include/config.h) - All configuration options
