# SIP Client Setup Guide

This guide provides detailed instructions for setting up the SIP client with various SIP servers and configurations.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Setting Up Asterisk](#setting-up-asterisk)
3. [Setting Up FreeSWITCH](#setting-up-freeswitch)
4. [Using Cloud SIP Providers](#using-cloud-sip-providers)
5. [Network Configuration](#network-configuration)
6. [Troubleshooting](#troubleshooting)

## Quick Start

### Minimum Requirements

- M5Stack StickC Plus2 device
- WiFi network (2.4GHz)
- SIP server (local or cloud-based)
- SIP account credentials

### Basic Configuration

1. **Copy the example configuration:**
   ```bash
   cp include/config.example.h include/config.h
   ```

2. **Edit WiFi settings** in `include/config.h`:
   ```cpp
   #define WIFI_SSID     "YourNetworkName"
   #define WIFI_PASSWORD "YourNetworkPassword"
   ```

3. **Edit SIP settings** in `include/config.h`:
   ```cpp
   #define SIP_SERVER_IP    "192.168.1.100"  // Your SIP server
   #define SIP_USER         "1001"            // Your extension
   #define SIP_PASSWORD     "yourpassword"    // Your password
   #define SIP_DOMAIN       "192.168.1.100"   // Same as server
   ```

4. **Build and upload:**
   ```bash
   pio run --target upload
   ```

## Setting Up Asterisk

Asterisk is a popular open-source PBX (Private Branch Exchange) system.

### Installing Asterisk

#### On Ubuntu/Debian:
```bash
sudo apt update
sudo apt install asterisk
```

#### On Raspberry Pi:
```bash
sudo apt update
sudo apt install asterisk
```

### Configuring Asterisk

1. **Edit SIP configuration** (`/etc/asterisk/sip.conf`):

```ini
[general]
context=default
allowguest=no
allowoverlap=no
bindport=5060
bindaddr=0.0.0.0
srvlookup=no
disallow=all
allow=ulaw
allow=alaw

; SIP user for M5Stack device
[1001]
type=friend
secret=password123
host=dynamic
context=internal
directmedia=no
disallow=all
allow=ulaw
allow=alaw
canreinvite=no

; Another user to call
[1002]
type=friend
secret=password123
host=dynamic
context=internal
directmedia=no
disallow=all
allow=ulaw
allow=alaw
canreinvite=no
```

2. **Edit dialplan** (`/etc/asterisk/extensions.conf`):

```ini
[internal]
; Allow calls between extensions
exten => 1001,1,Dial(SIP/1001,20)
exten => 1001,n,Hangup()

exten => 1002,1,Dial(SIP/1002,20)
exten => 1002,n,Hangup()

; Echo test
exten => 1000,1,Answer()
exten => 1000,n,Echo()
exten => 1000,n,Hangup()
```

3. **Restart Asterisk:**
```bash
sudo systemctl restart asterisk
```

4. **Check status:**
```bash
sudo asterisk -rvvv
# In the Asterisk console:
sip show peers
sip show users
```

### M5Stack Configuration for Asterisk

In your `include/config.h`:

```cpp
#define WIFI_SSID     "YourWiFi"
#define WIFI_PASSWORD "YourPassword"

#define SIP_SERVER_IP    "192.168.1.100"  // IP of your Asterisk server
#define SIP_SERVER_PORT  5060
#define SIP_USER         "1001"
#define SIP_PASSWORD     "password123"
#define SIP_DOMAIN       "192.168.1.100"
#define DEFAULT_CALL_URI "sip:1002@192.168.1.100"  // Call extension 1002
```

## Setting Up FreeSWITCH

FreeSWITCH is another powerful open-source telephony platform.

### Installing FreeSWITCH

#### On Ubuntu/Debian:
```bash
wget -O - https://files.freeswitch.org/repo/deb/debian-release/fsstretch-archive-keyring.asc | apt-key add -
echo "deb http://files.freeswitch.org/repo/deb/debian-release/ `lsb_release -sc` main" > /etc/apt/sources.list.d/freeswitch.list
apt-get update
apt-get install freeswitch-meta-vanilla
```

### Configuring FreeSWITCH

1. **Create a user** in `/etc/freeswitch/directory/default/1001.xml`:

```xml
<include>
  <user id="1001">
    <params>
      <param name="password" value="password123"/>
      <param name="vm-password" value="1001"/>
    </params>
    <variables>
      <variable name="toll_allow" value="domestic,international,local"/>
      <variable name="accountcode" value="1001"/>
      <variable name="user_context" value="default"/>
      <variable name="effective_caller_id_name" value="M5Stack 1001"/>
      <variable name="effective_caller_id_number" value="1001"/>
    </variables>
  </user>
</include>
```

2. **Restart FreeSWITCH:**
```bash
systemctl restart freeswitch
```

3. **Check status:**
```bash
fs_cli
# In the FreeSWITCH console:
sofia status
```

### M5Stack Configuration for FreeSWITCH

In your `include/config.h`:

```cpp
#define SIP_SERVER_IP    "192.168.1.100"  // IP of your FreeSWITCH server
#define SIP_SERVER_PORT  5060
#define SIP_USER         "1001"
#define SIP_PASSWORD     "password123"
#define SIP_DOMAIN       "192.168.1.100"
#define DEFAULT_CALL_URI "sip:1002@192.168.1.100"
```

## Using Cloud SIP Providers

### Example: VoIP.ms

1. **Sign up** for an account at VoIP.ms or similar provider

2. **Get your SIP credentials** from the provider's dashboard

3. **Configure** in `include/config.h`:

```cpp
#define SIP_SERVER_IP    "atlanta.voip.ms"  // Or your provider's SIP server
#define SIP_SERVER_PORT  5060
#define SIP_USER         "123456_yoursubaccount"
#define SIP_PASSWORD     "yourpassword"
#define SIP_DOMAIN       "atlanta.voip.ms"
#define DEFAULT_CALL_URI "sip:+15551234567@atlanta.voip.ms"  // External number
```

### Example: Twilio

1. **Create a Twilio account** and get SIP credentials

2. **Configure** in `include/config.h`:

```cpp
#define SIP_SERVER_IP    "youraccount.sip.us1.twilio.com"
#define SIP_SERVER_PORT  5060
#define SIP_USER         "yoursipcredentials"
#define SIP_PASSWORD     "yourpassword"
#define SIP_DOMAIN       "youraccount.sip.us1.twilio.com"
```

## Network Configuration

### Firewall Settings

#### For SIP Server (Asterisk/FreeSWITCH)

Allow incoming connections on:
- **UDP 5060** - SIP signaling
- **UDP 10000-20000** - RTP audio streams

Ubuntu/Debian firewall:
```bash
sudo ufw allow 5060/udp
sudo ufw allow 10000:20000/udp
```

#### Router Port Forwarding

If your SIP server is behind a router:
1. Forward UDP port 5060 to your server's IP
2. Forward UDP ports 10000-20000 to your server's IP

### NAT Traversal

For complex NAT scenarios, you may need STUN server configuration:

```cpp
// Future enhancement - not yet implemented
#define STUN_SERVER "stun.l.google.com"
#define STUN_PORT   19302
```

## Advanced Configuration

### Testing with Echo Test

Configure an echo test extension in Asterisk:

```ini
; In extensions.conf
exten => 1000,1,Answer()
exten => 1000,n,Echo()
exten => 1000,n,Hangup()
```

Then in your M5Stack:
```cpp
#define DEFAULT_CALL_URI "sip:1000@192.168.1.100"
```

Press button A to call the echo test. You should hear your own voice with a slight delay.

### Audio Quality Tuning

Adjust audio settings in `config.h`:

```cpp
// For better quality (but higher bandwidth)
#define AUDIO_BUFFER_SIZE  160  // 20ms frames

// For lower latency (but more packets)
#define AUDIO_BUFFER_SIZE  80   // 10ms frames

// Codec selection
#define AUDIO_CODEC_MULAW  1  // North America
// or
#define AUDIO_CODEC_MULAW  0  // International (A-law)
```

### Multiple Extensions

To support calling multiple numbers, use serial commands:

```
CALL sip:1001@192.168.1.100
CALL sip:1002@192.168.1.100
CALL sip:+15551234567@voipprovider.com
```

## Testing Your Setup

### 1. Test WiFi Connection

Power on the device and check the serial output:
```
Connecting to WiFi...
WiFi connected!
IP address: 192.168.1.50
```

### 2. Test SIP Registration

Check the serial output:
```
SIP Client initialized
Registering with SIP server...
Received SIP response: 200
Successfully registered with SIP server
Ready to make/receive calls
```

### 3. Test Echo Call

Use the echo test extension (1000) to verify audio:
1. Set `DEFAULT_CALL_URI` to `sip:1000@yourserver`
2. Press button A to call
3. Speak into the microphone
4. You should hear your voice back

### 4. Test Two-Way Call

1. Set up two SIP accounts (1001 and 1002)
2. Configure M5Stack with account 1001
3. Configure another SIP client (softphone) with account 1002
4. Call from M5Stack to the softphone
5. Test both directions of audio

## Common Issues and Solutions

### Issue: Cannot Register with SIP Server

**Check:**
- SIP server IP is correct and reachable
- Credentials are correct
- Firewall allows UDP port 5060
- SIP server is running: `sudo systemctl status asterisk`

**Solution:**
```bash
# Verify network connectivity
ping 192.168.1.100

# Check SIP server logs
sudo tail -f /var/log/asterisk/messages

# Enable debug in config.h
#define DEBUG_SIP 1
```

### Issue: Call Connects But No Audio

**Check:**
- RTP ports (10000-20000) are open
- Audio codec matches between server and client
- Network bandwidth is sufficient

**Solution:**
- Verify RTP ports in Asterisk logs
- Check codec configuration:
  ```ini
  ; In sip.conf
  disallow=all
  allow=ulaw
  allow=alaw
  ```

### Issue: One-Way Audio

**Possible causes:**
- NAT/firewall blocking RTP in one direction
- Asymmetric routing

**Solution:**
- Configure `directmedia=no` in Asterisk sip.conf
- Use STUN server (future enhancement)
- Check firewall rules on both sides

### Issue: Poor Audio Quality

**Solutions:**
- Reduce WiFi distance to router
- Check network bandwidth and latency
- Adjust audio buffer size:
  ```cpp
  #define AUDIO_BUFFER_SIZE  160  // Try different values
  ```

## SIP Server Monitoring

### Asterisk

```bash
# Connect to Asterisk console
sudo asterisk -rvvv

# Show registered peers
sip show peers

# Show active calls
core show channels

# Show recent messages
sip set debug on
```

### FreeSWITCH

```bash
# Connect to FreeSWITCH console
fs_cli

# Show registrations
sofia status profile internal reg

# Show active calls
show calls

# Enable SIP debug
sofia loglevel all 9
```

## Performance Optimization

### For Best Audio Quality

1. Use a good WiFi connection (close to router)
2. Minimize network traffic on the WiFi
3. Use μ-law codec in North America
4. Set audio buffer to 20ms (160 samples)

### For Best Reliability

1. Enable debug logging initially
2. Test with echo extension first
3. Gradually increase complexity
4. Monitor SIP server logs

## Next Steps

After basic setup works:
1. Test with different SIP accounts
2. Try calling external numbers (if using cloud provider)
3. Experiment with audio settings
4. Set up call logging
5. Configure contact list (future enhancement)

## Support

For issues not covered here:
1. Check the main README.md
2. Review debug output (set all DEBUG_* to 1)
3. Check SIP server logs
4. Verify network connectivity and firewall rules
