/**
 * @file sip_client.cpp
 * @brief SIP client implementation
 */

#include "sip_client.h"
#include <Arduino.h>

SIPClient::SIPClient() : 
    currentState(SIP_IDLE),
    cseq(1),
    rtpPort(0),
    remoteRTPPort(0),
    needsAuth(false) {
    memset(remoteIP, 0, sizeof(remoteIP));
}

SIPClient::~SIPClient() {
    sipSocket.stop();
}

bool SIPClient::begin() {
    // Initialize UDP socket for SIP
    if (!sipSocket.begin(LOCAL_SIP_PORT)) {
        Serial.println("Failed to start SIP socket");
        return false;
    }
    
    // Generate initial identifiers
    callId = generateCallId();
    fromTag = generateTag();
    
    Serial.println("SIP Client initialized");
    return true;
}

bool SIPClient::registerWithServer() {
    if (currentState != SIP_IDLE && currentState != SIP_REGISTERED) {
        Serial.println("Cannot register in current state");
        return false;
    }
    
    currentState = SIP_REGISTERING;
    return sendSIPMessage(SIP_REGISTER);
}

bool SIPClient::makeCall(const char* sipUri) {
    if (currentState != SIP_REGISTERED) {
        Serial.println("Must be registered to make calls");
        return false;
    }
    
    currentState = SIP_CALLING;
    cseq++;
    
    // Generate new call-specific identifiers
    callId = generateCallId();
    fromTag = generateTag();
    toTag = "";
    
    // Allocate RTP port
    rtpPort = RTP_PORT_MIN + (esp_random() % (RTP_PORT_MAX - RTP_PORT_MIN));
    
    return sendSIPMessage(SIP_INVITE, sipUri);
}

bool SIPClient::answerCall() {
    if (currentState != SIP_RINGING) {
        Serial.println("No incoming call to answer");
        return false;
    }
    
    currentState = SIP_IN_CALL;
    
    // Send 200 OK response
    // This is simplified - in a real implementation, we'd send a proper response
    
    return true;
}

bool SIPClient::hangUp() {
    if (currentState != SIP_IN_CALL && currentState != SIP_CALLING) {
        Serial.println("No active call to hang up");
        return false;
    }
    
    currentState = SIP_TERMINATING;
    cseq++;
    
    return sendSIPMessage(SIP_BYE);
}

void SIPClient::update() {
    // Check for incoming SIP messages
    int packetSize = sipSocket.parsePacket();
    if (packetSize > 0) {
        char buffer[2048];
        int len = sipSocket.read(buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            
#if DEBUG_SIP
            Serial.println("=== Received SIP Message ===");
            Serial.println(buffer);
            Serial.println("============================");
#endif
            
            handleIncomingMessage();
        }
    }
}

const char* SIPClient::getStateString() {
    switch (currentState) {
        case SIP_IDLE: return "Idle";
        case SIP_REGISTERING: return "Registering";
        case SIP_REGISTERED: return "Registered";
        case SIP_CALLING: return "Calling";
        case SIP_RINGING: return "Ringing";
        case SIP_IN_CALL: return "In Call";
        case SIP_TERMINATING: return "Terminating";
        case SIP_ERROR: return "Error";
        default: return "Unknown";
    }
}

bool SIPClient::sendSIPMessage(SIPMessageType type, const char* recipient) {
    String message;
    
    switch (type) {
        case SIP_REGISTER:
            message = buildRegisterMessage();
            break;
        case SIP_INVITE:
            message = buildInviteMessage(recipient);
            break;
        case SIP_ACK:
            message = buildAckMessage();
            break;
        case SIP_BYE:
            message = buildByeMessage();
            break;
        default:
            return false;
    }
    
#if DEBUG_SIP
    Serial.println("=== Sending SIP Message ===");
    Serial.println(message);
    Serial.println("===========================");
#endif
    
    sipSocket.beginPacket(SIP_SERVER_IP, SIP_SERVER_PORT);
    sipSocket.print(message);
    return sipSocket.endPacket();
}

void SIPClient::handleIncomingMessage() {
    // This is a simplified handler
    // In a real implementation, we'd parse the message properly
    
    char buffer[2048];
    int len = sipSocket.available();
    if (len > 0 && len < sizeof(buffer)) {
        sipSocket.read(buffer, len);
        buffer[len] = '\0';
        
        // Check if it's a response or request
        if (strncmp(buffer, "SIP/2.0", 7) == 0) {
            parseResponse(buffer, len);
        } else if (strstr(buffer, "INVITE") != nullptr) {
            parseInvite(buffer, len);
        }
    }
}

void SIPClient::parseResponse(const char* message, int length) {
    // Extract response code
    int responseCode = 0;
    if (sscanf(message, "SIP/2.0 %d", &responseCode) == 1) {
        Serial.printf("Received SIP response: %d\n", responseCode);
        
        switch (responseCode) {
            case SIP_TRYING:
            case SIP_RINGING:
                if (currentState == SIP_CALLING) {
                    currentState = SIP_RINGING;
                }
                break;
                
            case SIP_OK:
                if (currentState == SIP_REGISTERING) {
                    currentState = SIP_REGISTERED;
                    Serial.println("Successfully registered with SIP server");
                } else if (currentState == SIP_CALLING || currentState == SIP_RINGING) {
                    currentState = SIP_IN_CALL;
                    // Parse SDP to get remote RTP info
                    parseSDPForRTP(message);
                    // Send ACK
                    sendSIPMessage(SIP_ACK);
                } else if (currentState == SIP_TERMINATING) {
                    currentState = SIP_IDLE;
                }
                break;
                
            case SIP_UNAUTHORIZED:
                // Handle authentication challenge
                needsAuth = true;
                // Extract realm and nonce from WWW-Authenticate header
                // This is simplified - real implementation would parse properly
                break;
                
            default:
                Serial.printf("Unhandled response code: %d\n", responseCode);
                break;
        }
    }
}

void SIPClient::parseInvite(const char* message, int length) {
    // Incoming call
    if (currentState == SIP_REGISTERED) {
        currentState = SIP_RINGING;
        Serial.println("Incoming call!");
        
        // Parse caller info and SDP
        parseSDPForRTP(message);
    }
}

String SIPClient::buildRegisterMessage() {
    branch = generateBranch();
    
    String msg = "REGISTER sip:" + String(SIP_DOMAIN) + " SIP/2.0\r\n";
    msg += "Via: SIP/2.0/UDP " + WiFi.localIP().toString() + ":" + String(LOCAL_SIP_PORT);
    msg += ";branch=" + branch + "\r\n";
    msg += "From: <sip:" + String(SIP_USER) + "@" + String(SIP_DOMAIN) + ">;tag=" + fromTag + "\r\n";
    msg += "To: <sip:" + String(SIP_USER) + "@" + String(SIP_DOMAIN) + ">\r\n";
    msg += "Call-ID: " + callId + "\r\n";
    msg += "CSeq: " + String(cseq) + " REGISTER\r\n";
    msg += "Contact: <sip:" + String(SIP_USER) + "@" + WiFi.localIP().toString() + ":" + String(LOCAL_SIP_PORT) + ">\r\n";
    msg += "Expires: " + String(SIP_REGISTER_INTERVAL) + "\r\n";
    msg += "User-Agent: M5StickCPlus2-SIP/1.0\r\n";
    msg += "Content-Length: 0\r\n";
    msg += "\r\n";
    
    return msg;
}

String SIPClient::buildInviteMessage(const char* sipUri) {
    branch = generateBranch();
    String sdp = buildSDP();
    
    String msg = "INVITE " + String(sipUri) + " SIP/2.0\r\n";
    msg += "Via: SIP/2.0/UDP " + WiFi.localIP().toString() + ":" + String(LOCAL_SIP_PORT);
    msg += ";branch=" + branch + "\r\n";
    msg += "From: <sip:" + String(SIP_USER) + "@" + String(SIP_DOMAIN) + ">;tag=" + fromTag + "\r\n";
    msg += "To: <" + String(sipUri) + ">\r\n";
    msg += "Call-ID: " + callId + "\r\n";
    msg += "CSeq: " + String(cseq) + " INVITE\r\n";
    msg += "Contact: <sip:" + String(SIP_USER) + "@" + WiFi.localIP().toString() + ":" + String(LOCAL_SIP_PORT) + ">\r\n";
    msg += "Content-Type: application/sdp\r\n";
    msg += "Content-Length: " + String(sdp.length()) + "\r\n";
    msg += "User-Agent: M5StickCPlus2-SIP/1.0\r\n";
    msg += "\r\n";
    msg += sdp;
    
    return msg;
}

String SIPClient::buildAckMessage() {
    String msg = "ACK sip:" + String(SIP_USER) + "@" + String(SIP_DOMAIN) + " SIP/2.0\r\n";
    msg += "Via: SIP/2.0/UDP " + WiFi.localIP().toString() + ":" + String(LOCAL_SIP_PORT);
    msg += ";branch=" + branch + "\r\n";
    msg += "From: <sip:" + String(SIP_USER) + "@" + String(SIP_DOMAIN) + ">;tag=" + fromTag + "\r\n";
    msg += "To: <sip:" + String(SIP_USER) + "@" + String(SIP_DOMAIN) + ">;tag=" + toTag + "\r\n";
    msg += "Call-ID: " + callId + "\r\n";
    msg += "CSeq: " + String(cseq) + " ACK\r\n";
    msg += "Content-Length: 0\r\n";
    msg += "\r\n";
    
    return msg;
}

String SIPClient::buildByeMessage() {
    branch = generateBranch();
    
    String msg = "BYE sip:" + String(SIP_USER) + "@" + String(SIP_DOMAIN) + " SIP/2.0\r\n";
    msg += "Via: SIP/2.0/UDP " + WiFi.localIP().toString() + ":" + String(LOCAL_SIP_PORT);
    msg += ";branch=" + branch + "\r\n";
    msg += "From: <sip:" + String(SIP_USER) + "@" + String(SIP_DOMAIN) + ">;tag=" + fromTag + "\r\n";
    msg += "To: <sip:" + String(SIP_USER) + "@" + String(SIP_DOMAIN) + ">;tag=" + toTag + "\r\n";
    msg += "Call-ID: " + callId + "\r\n";
    msg += "CSeq: " + String(cseq) + " BYE\r\n";
    msg += "Content-Length: 0\r\n";
    msg += "\r\n";
    
    return msg;
}

String SIPClient::generateCallId() {
    return String(esp_random(), HEX) + "@" + WiFi.localIP().toString();
}

String SIPClient::generateBranch() {
    return "z9hG4bK-" + String(esp_random(), HEX);
}

String SIPClient::generateTag() {
    return String(esp_random(), HEX);
}

void SIPClient::parseSDPForRTP(const char* sdp) {
    // Look for media description (m=audio)
    const char* mLine = strstr(sdp, "m=audio");
    if (mLine) {
        int port = 0;
        if (sscanf(mLine, "m=audio %d", &port) == 1) {
            remoteRTPPort = port;
        }
    }
    
    // Look for connection information (c=IN IP4)
    const char* cLine = strstr(sdp, "c=IN IP4");
    if (cLine) {
        char ip[64];
        if (sscanf(cLine, "c=IN IP4 %s", ip) == 1) {
            strncpy(remoteIP, ip, sizeof(remoteIP) - 1);
        }
    }
    
    Serial.printf("Remote RTP endpoint: %s:%d\n", remoteIP, remoteRTPPort);
}

String SIPClient::buildSDP() {
    String sdp = "v=0\r\n";
    sdp += "o=- " + String(esp_random()) + " " + String(esp_random()) + " IN IP4 " + WiFi.localIP().toString() + "\r\n";
    sdp += "s=M5StickCPlus2 SIP Call\r\n";
    sdp += "c=IN IP4 " + WiFi.localIP().toString() + "\r\n";
    sdp += "t=0 0\r\n";
    sdp += "m=audio " + String(rtpPort) + " RTP/AVP " + String(RTP_PAYLOAD_TYPE) + "\r\n";
    
#if AUDIO_CODEC_MULAW
    sdp += "a=rtpmap:0 PCMU/8000\r\n";
#else
    sdp += "a=rtpmap:8 PCMA/8000\r\n";
#endif
    
    sdp += "a=sendrecv\r\n";
    
    return sdp;
}
