/**
 * @file audio_handler.h
 * @brief Audio handling for SIP calls using I2S microphone and buzzer
 * 
 * This module handles:
 * - I2S microphone input (SPM1423)
 * - Buzzer output (GPIO 2 - tone generation only, no full audio playback)
 * - G.711 codec (μ-law/A-law)
 * - RTP packet transmission and reception
 */

#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H

#include <Arduino.h>

// I2S driver compatibility for different ESP32 variants
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
    // For newer ESP-IDF versions (ESP32-S3, etc.)
    #include <driver/i2s_std.h>
    #include <driver/i2s_pdm.h>
#else
    // For older ESP-IDF versions (ESP32, ESP32-S2)
    #include <driver/i2s.h>
#endif

#include <WiFiUdp.h>
#include "config.h"

// G.711 codec functions
int16_t ulaw_decode(uint8_t ulaw);
uint8_t ulaw_encode(int16_t pcm);
int16_t alaw_decode(uint8_t alaw);
uint8_t alaw_encode(int16_t pcm);

// RTP header structure
struct RTPHeader {
    uint8_t vpxcc;      // V(2), P(1), X(1), CC(4)
    uint8_t mpt;        // M(1), PT(7)
    uint16_t sequence;
    uint32_t timestamp;
    uint32_t ssrc;
};

class AudioHandler {
public:
    AudioHandler();
    ~AudioHandler();
    
    // Initialize audio subsystem
    bool begin();
    
    // Start/stop audio streaming for a call
    bool startCall(const char* remoteIP, uint16_t remotePort, uint16_t localPort);
    void stopCall();
    
    // Audio processing (call in loop during active call)
    void processAudio();
    
    // Status
    bool isActive() { return active; }
    
private:
    bool active;
    WiFiUDP rtpSocket;
    
    // RTP session info
    char remoteIP[64];
    uint16_t remoteRTPPort;
    uint16_t localRTPPort;
    
    // RTP state
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint32_t ssrc;
    
    // Audio buffers
    int16_t micBuffer[AUDIO_BUFFER_SIZE];
    int16_t spkBuffer[AUDIO_BUFFER_SIZE];
    uint8_t rtpPayload[AUDIO_BUFFER_SIZE];
    
    // I2S configuration
    bool initMicrophone();
    bool initBuzzer();
    
    // Audio processing
    void captureAudio();
    void playAudio();
    
    // RTP functions
    void sendRTPPacket(const uint8_t* payload, size_t length);
    bool receiveRTPPacket();
    void buildRTPHeader(RTPHeader* header, size_t payloadLength);
    bool parseRTPHeader(const uint8_t* packet, RTPHeader* header);
};

#endif // AUDIO_HANDLER_H
