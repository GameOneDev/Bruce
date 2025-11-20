/**
 * @file sip_client.h
 * @brief Lightweight SIP client implementation for ESP32
 * 
 * This is a simplified SIP client that supports:
 * - SIP REGISTER (registration with SIP server)
 * - SIP INVITE (outgoing calls)
 * - SIP response handling (incoming calls)
 * - SIP BYE (call termination)
 */

#ifndef SIP_CLIENT_H
#define SIP_CLIENT_H

#include <WiFi.h>
#include <WiFiUdp.h>
#include "config.h"

// SIP message types
enum SIPMessageType {
    SIP_REGISTER,
    SIP_INVITE,
    SIP_ACK,
    SIP_BYE,
    SIP_CANCEL,
    SIP_OPTIONS,
    SIP_RESPONSE
};

// SIP client states
enum SIPState {
    SIP_IDLE,
    SIP_REGISTERING,
    SIP_REGISTERED,
    SIP_CALLING,
    SIP_RINGING,
    SIP_IN_CALL,
    SIP_TERMINATING,
    SIP_ERROR
};

// SIP response codes
#define SIP_TRYING              100
#define SIP_RINGING             180
#define SIP_OK                  200
#define SIP_UNAUTHORIZED        401
#define SIP_REQUEST_TIMEOUT     408
#define SIP_BUSY_HERE           486

class SIPClient {
public:
    SIPClient();
    ~SIPClient();
    
    // Initialize SIP client
    bool begin();
    
    // SIP operations
    bool registerWithServer();
    bool makeCall(const char* sipUri);
    bool answerCall();
    bool hangUp();
    
    // State management
    void update();  // Call this in loop() to handle incoming messages
    SIPState getState() { return currentState; }
    const char* getStateString();
    
    // Audio RTP session
    uint16_t getRTPPort() { return rtpPort; }
    const char* getRemoteIP() { return remoteIP; }
    uint16_t getRemoteRTPPort() { return remoteRTPPort; }
    
private:
    WiFiUDP sipSocket;
    SIPState currentState;
    
    // SIP transaction identifiers
    String callId;
    String fromTag;
    String toTag;
    String branch;
    uint32_t cseq;
    
    // RTP session info
    uint16_t rtpPort;
    char remoteIP[64];
    uint16_t remoteRTPPort;
    
    // Internal methods
    bool sendSIPMessage(SIPMessageType type, const char* recipient = nullptr);
    void handleIncomingMessage();
    void parseResponse(const char* message, int length);
    void parseInvite(const char* message, int length);
    
    // SIP message builders
    String buildRegisterMessage();
    String buildInviteMessage(const char* sipUri);
    String buildAckMessage();
    String buildByeMessage();
    
    // Helper functions
    String generateCallId();
    String generateBranch();
    String generateTag();
    void parseSDPForRTP(const char* sdp);
    String buildSDP();
    
    // Authentication (basic)
    String realm;
    String nonce;
    bool needsAuth;
};

#endif // SIP_CLIENT_H
