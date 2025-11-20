/**
 * @file config.example.h
 * @brief Example configuration file for SIP Client
 * 
 * Copy this file to config.h and modify the values according to your setup.
 * 
 * cp include/config.example.h include/config.h
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// WiFi Configuration
// ============================================================================

// WiFi credentials - Change these to match your network
#define WIFI_SSID     "MyHomeNetwork"
#define WIFI_PASSWORD "MyWiFiPassword123"

// WiFi connection timeout (milliseconds)
#define WIFI_CONNECT_TIMEOUT 15000

// ============================================================================
// SIP Server Configuration
// ============================================================================

// Example 1: Using a local Asterisk server
#define SIP_SERVER_IP    "192.168.1.100"    // Your SIP server IP or domain
#define SIP_SERVER_PORT  5060                // Default SIP port

// Example 2: Using a cloud SIP provider (uncomment and modify)
// #define SIP_SERVER_IP    "sip.yourprovider.com"
// #define SIP_SERVER_PORT  5060

// SIP account credentials
#define SIP_USER         "1001"              // Your SIP username/extension
#define SIP_PASSWORD     "secretpassword"    // Your SIP password
#define SIP_DOMAIN       "192.168.1.100"     // Your SIP domain (can be IP or domain)

// Example: Cloud provider setup
// #define SIP_USER         "yourphone@yourprovider.com"
// #define SIP_PASSWORD     "yourpassword"
// #define SIP_DOMAIN       "yourprovider.com"

// SIP URI to call (can be changed at runtime via serial)
#define DEFAULT_CALL_URI "sip:1002@192.168.1.100"  // Default number to call

// Example: Calling external numbers via SIP provider
// #define DEFAULT_CALL_URI "sip:+15551234567@yourprovider.com"

// SIP session timers (seconds)
#define SIP_REGISTER_INTERVAL 3600   // Re-registration interval
#define SIP_SESSION_TIMEOUT   120    // Session timeout

// ============================================================================
// Audio Configuration
// ============================================================================

// Audio codec (G.711 μ-law/A-law)
// μ-law (1) is standard in North America
// A-law (0) is standard in most other countries
#define AUDIO_CODEC_MULAW  1  // 1 = μ-law (US), 0 = A-law (International)

// Audio sample rate (Hz) - G.711 standard is 8000 Hz
#define AUDIO_SAMPLE_RATE  8000

// Audio buffer sizes (samples)
#define AUDIO_BUFFER_SIZE  160  // 20ms @ 8kHz (typical for VoIP)

// RTP configuration
#define RTP_PAYLOAD_TYPE   0    // 0 = PCMU (μ-law), 8 = PCMA (A-law)
#define RTP_PACKET_SIZE    160  // Matches audio buffer size

// I2S Microphone Configuration (SPM1423)
#define I2S_MIC_PORT          I2S_NUM_0
#define I2S_MIC_SAMPLE_RATE   8000
#define I2S_MIC_BITS          16
#define I2S_MIC_CHANNELS      1

// Buzzer Configuration (M5StickCPlus2 only has buzzer, not speaker)
// Note: The M5StickCPlus2 does NOT have a speaker (AW88298)
// It only has a simple buzzer on GPIO 2 for tone generation
// For actual audio playback during calls, you need to connect an external speaker
// via the Grove port using an I2S DAC or analog amplifier
#define BUZZER_ENABLED        1   // Enable buzzer for call indicators and alerts

// ============================================================================
// UI Configuration
// ============================================================================

// Display settings
#define SCREEN_WIDTH   240
#define SCREEN_HEIGHT  135

// UI colors (RGB565 format)
#define COLOR_BG       0x0000  // Black
#define COLOR_TEXT     0xFFFF  // White
#define COLOR_STATUS   0x07E0  // Green
#define COLOR_ERROR    0xF800  // Red
#define COLOR_WARNING  0xFFE0  // Yellow
#define COLOR_CALLING  0x07FF  // Cyan

// Button debounce time (milliseconds)
#define BUTTON_DEBOUNCE_MS 50

// ============================================================================
// Network Configuration
// ============================================================================

// Local SIP port
#define LOCAL_SIP_PORT 5060

// RTP port range (for audio streaming)
#define RTP_PORT_MIN 10000
#define RTP_PORT_MAX 20000

// Network timeout (milliseconds)
#define NETWORK_TIMEOUT 5000

// ============================================================================
// Debug Configuration
// ============================================================================

// Enable debug output via Serial
#define DEBUG_ENABLED     1

// Debug levels (set to 1 to enable)
#define DEBUG_SIP         1  // SIP protocol messages
#define DEBUG_AUDIO       1  // Audio processing
#define DEBUG_NETWORK     1  // Network operations
#define DEBUG_UI          1  // UI events

#endif // CONFIG_H
