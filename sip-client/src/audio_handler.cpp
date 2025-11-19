/**
 * @file audio_handler.cpp
 * @brief Audio handling implementation
 */

#include "audio_handler.h"

// G.711 μ-law lookup tables (simplified implementation)
static const int16_t ulaw_table[256] = {
    -32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956,
    -23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764,
    -15996, -15484, -14972, -14460, -13948, -13436, -12924, -12412,
    -11900, -11388, -10876, -10364, -9852, -9340, -8828, -8316,
    -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
    -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
    -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
    -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
    -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
    -1372, -1308, -1244, -1180, -1116, -1052, -988, -924,
    -876, -844, -812, -780, -748, -716, -684, -652,
    -620, -588, -556, -524, -492, -460, -428, -396,
    -372, -356, -340, -324, -308, -292, -276, -260,
    -244, -228, -212, -196, -180, -164, -148, -132,
    -120, -112, -104, -96, -88, -80, -72, -64,
    -56, -48, -40, -32, -24, -16, -8, 0,
    32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
    23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
    15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
    11900, 11388, 10876, 10364, 9852, 9340, 8828, 8316,
    7932, 7676, 7420, 7164, 6908, 6652, 6396, 6140,
    5884, 5628, 5372, 5116, 4860, 4604, 4348, 4092,
    3900, 3772, 3644, 3516, 3388, 3260, 3132, 3004,
    2876, 2748, 2620, 2492, 2364, 2236, 2108, 1980,
    1884, 1820, 1756, 1692, 1628, 1564, 1500, 1436,
    1372, 1308, 1244, 1180, 1116, 1052, 988, 924,
    876, 844, 812, 780, 748, 716, 684, 652,
    620, 588, 556, 524, 492, 460, 428, 396,
    372, 356, 340, 324, 308, 292, 276, 260,
    244, 228, 212, 196, 180, 164, 148, 132,
    120, 112, 104, 96, 88, 80, 72, 64,
    56, 48, 40, 32, 24, 16, 8, 0
};

// G.711 μ-law decode
int16_t ulaw_decode(uint8_t ulaw) {
    return ulaw_table[ulaw];
}

// G.711 μ-law encode (simplified)
uint8_t ulaw_encode(int16_t pcm) {
    const uint16_t BIAS = 0x84;
    const uint16_t CLIP = 32635;
    
    uint8_t sign = (pcm < 0) ? 0x80 : 0x00;
    if (sign) pcm = -pcm;
    if (pcm > CLIP) pcm = CLIP;
    
    pcm += BIAS;
    uint8_t exponent = 7;
    for (uint8_t exp = 7; exp > 0; exp--) {
        if (pcm >= (256 << exp)) {
            exponent = exp;
            break;
        }
    }
    
    uint8_t mantissa = (pcm >> (exponent + 3)) & 0x0F;
    uint8_t ulaw = ~(sign | (exponent << 4) | mantissa);
    
    return ulaw;
}

// A-law codec (placeholder - similar to μ-law)
int16_t alaw_decode(uint8_t alaw) {
    // Simplified A-law decode
    return ulaw_decode(alaw); // For now, use μ-law
}

uint8_t alaw_encode(int16_t pcm) {
    // Simplified A-law encode
    return ulaw_encode(pcm); // For now, use μ-law
}

AudioHandler::AudioHandler() : 
    active(false),
    sequenceNumber(0),
    timestamp(0),
    ssrc(esp_random()),
    remoteRTPPort(0),
    localRTPPort(0) {
    memset(remoteIP, 0, sizeof(remoteIP));
}

AudioHandler::~AudioHandler() {
    stopCall();
}

bool AudioHandler::begin() {
    if (!initMicrophone()) {
        Serial.println("Failed to initialize microphone");
        return false;
    }
    
    if (!initSpeaker()) {
        Serial.println("Failed to initialize speaker");
        return false;
    }
    
    Serial.println("Audio subsystem initialized");
    return true;
}

bool AudioHandler::initMicrophone() {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
    // For newer ESP-IDF versions (4.4+), use the new I2S API
    // Note: This is a placeholder - the new API is more complex
    // For now, we'll skip I2S init on newer platforms and rely on M5Stack library
    Serial.println("I2S initialization skipped (using M5Stack library on newer ESP-IDF)");
    return true;
#else
    // Configure I2S for SPM1423 microphone (legacy API)
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = I2S_MIC_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_PIN_NO_CHANGE,
        .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_MIC_SERIAL_CLOCK
    };
    
    esp_err_t err = i2s_driver_install(I2S_MIC_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("Failed to install I2S driver: %d\n", err);
        return false;
    }
    
    err = i2s_set_pin(I2S_MIC_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("Failed to set I2S pins: %d\n", err);
        return false;
    }
    
    i2s_set_clk(I2S_MIC_PORT, I2S_MIC_SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    
    return true;
#endif
}

bool AudioHandler::initSpeaker() {
    // For M5StickCPlus2, we'll use the built-in buzzer/speaker
    // This is simplified - actual implementation would need proper DAC setup
    pinMode(BUZZ_PIN, OUTPUT);
    
    return true;
}

bool AudioHandler::startCall(const char* remoteIP, uint16_t remotePort, uint16_t localPort) {
    strncpy(this->remoteIP, remoteIP, sizeof(this->remoteIP) - 1);
    this->remoteRTPPort = remotePort;
    this->localRTPPort = localPort;
    
    // Start RTP socket
    if (!rtpSocket.begin(localPort)) {
        Serial.println("Failed to start RTP socket");
        return false;
    }
    
    // Reset RTP state
    sequenceNumber = esp_random() & 0xFFFF;
    timestamp = esp_random();
    
    active = true;
    
    Serial.printf("Audio call started: %s:%d (local port: %d)\n", remoteIP, remotePort, localPort);
    return true;
}

void AudioHandler::stopCall() {
    if (!active) return;
    
    active = false;
    rtpSocket.stop();
    
    Serial.println("Audio call stopped");
}

void AudioHandler::processAudio() {
    if (!active) return;
    
    // Capture audio from microphone
    captureAudio();
    
    // Play received audio to speaker
    playAudio();
}

void AudioHandler::captureAudio() {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
    // For newer ESP-IDF, audio capture would use the new I2S API
    // This is a placeholder - actual implementation would use i2s_channel_read
    // For now, we generate silence to avoid crashes
    memset(rtpPayload, 0, AUDIO_BUFFER_SIZE);
    
    // Encode silence to G.711 (0x7F for μ-law, 0x55 for A-law represents silence)
#if AUDIO_CODEC_MULAW
    memset(rtpPayload, 0x7F, AUDIO_BUFFER_SIZE);
#else
    memset(rtpPayload, 0x55, AUDIO_BUFFER_SIZE);
#endif
    
    // Send RTP packet with silence (prevents call from dropping)
    sendRTPPacket(rtpPayload, AUDIO_BUFFER_SIZE);
#else
    size_t bytesRead = 0;
    
    // Read from I2S microphone (legacy API)
    esp_err_t err = i2s_read(I2S_MIC_PORT, micBuffer, sizeof(micBuffer), &bytesRead, 100);
    
    if (err == ESP_OK && bytesRead > 0) {
        size_t samplesRead = bytesRead / sizeof(int16_t);
        
        // Encode to G.711
        for (size_t i = 0; i < samplesRead && i < AUDIO_BUFFER_SIZE; i++) {
#if AUDIO_CODEC_MULAW
            rtpPayload[i] = ulaw_encode(micBuffer[i]);
#else
            rtpPayload[i] = alaw_encode(micBuffer[i]);
#endif
        }
        
        // Send RTP packet
        sendRTPPacket(rtpPayload, samplesRead);
    }
#endif
}

void AudioHandler::playAudio() {
    // Receive RTP packet
    if (receiveRTPPacket()) {
        // Decode G.711 and play
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
#if AUDIO_CODEC_MULAW
            spkBuffer[i] = ulaw_decode(rtpPayload[i]);
#else
            spkBuffer[i] = alaw_decode(rtpPayload[i]);
#endif
        }
        
        // Play audio (simplified - actual implementation would use I2S DAC or PWM)
        // For now, we'll just use the buzzer for tone generation
        // This is a placeholder - real implementation would need proper audio output
    }
}

void AudioHandler::sendRTPPacket(const uint8_t* payload, size_t length) {
    uint8_t packet[512];
    RTPHeader* header = (RTPHeader*)packet;
    
    buildRTPHeader(header, length);
    
    // Copy payload after header
    memcpy(packet + sizeof(RTPHeader), payload, length);
    
    // Send packet
    rtpSocket.beginPacket(remoteIP, remoteRTPPort);
    rtpSocket.write(packet, sizeof(RTPHeader) + length);
    rtpSocket.endPacket();
    
    // Update state
    sequenceNumber++;
    timestamp += length;
    
#if DEBUG_AUDIO
    if (sequenceNumber % 50 == 0) {
        Serial.printf("Sent RTP packet: seq=%d, ts=%u, len=%d\n", 
                      sequenceNumber, timestamp, length);
    }
#endif
}

bool AudioHandler::receiveRTPPacket() {
    int packetSize = rtpSocket.parsePacket();
    if (packetSize <= 0) return false;
    
    uint8_t packet[512];
    int len = rtpSocket.read(packet, sizeof(packet));
    
    if (len < sizeof(RTPHeader)) return false;
    
    RTPHeader header;
    if (!parseRTPHeader(packet, &header)) return false;
    
    // Copy payload
    int payloadLen = len - sizeof(RTPHeader);
    if (payloadLen > 0 && payloadLen <= AUDIO_BUFFER_SIZE) {
        memcpy(rtpPayload, packet + sizeof(RTPHeader), payloadLen);
        
#if DEBUG_AUDIO
        static uint16_t lastSeq = 0;
        if (header.sequence % 50 == 0) {
            Serial.printf("Received RTP packet: seq=%d, ts=%u, len=%d\n", 
                          ntohs(header.sequence), ntohl(header.timestamp), payloadLen);
        }
        lastSeq = ntohs(header.sequence);
#endif
        
        return true;
    }
    
    return false;
}

void AudioHandler::buildRTPHeader(RTPHeader* header, size_t payloadLength) {
    header->vpxcc = 0x80;  // V=2, P=0, X=0, CC=0
    header->mpt = RTP_PAYLOAD_TYPE;  // M=0, PT=0 (PCMU) or 8 (PCMA)
    header->sequence = htons(sequenceNumber);
    header->timestamp = htonl(timestamp);
    header->ssrc = htonl(ssrc);
}

bool AudioHandler::parseRTPHeader(const uint8_t* packet, RTPHeader* header) {
    memcpy(header, packet, sizeof(RTPHeader));
    
    // Verify RTP version (should be 2)
    uint8_t version = (header->vpxcc >> 6) & 0x03;
    if (version != 2) {
        Serial.printf("Invalid RTP version: %d\n", version);
        return false;
    }
    
    return true;
}
