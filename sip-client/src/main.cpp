/**
 * @file main.cpp
 * @brief Main application file for SIP Client
 * 
 * M5Stack StickC Plus2 SIP Client Application
 * 
 * This application provides a complete SIP client implementation
 * for making and receiving VoIP calls using the M5Stack StickCPlus2.
 * 
 * Hardware Features:
 * - Display: ST7789 135x240 TFT
 * - Microphone: SPM1423 I2S
 * - Speaker: Built-in buzzer/speaker
 * - Buttons: A (GPIO37), B (GPIO39), Power (GPIO35)
 * - WiFi: ESP32 built-in
 * 
 * @author SIP Client Project
 * @date 2025
 */

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "sip_client.h"
#include "audio_handler.h"
#include "ui_handler.h"

// Global objects
SIPClient sipClient;
AudioHandler audioHandler;
UIHandler ui;

// Application state
enum AppState {
    APP_INIT,
    APP_CONNECTING_WIFI,
    APP_WIFI_CONNECTED,
    APP_REGISTERING_SIP,
    APP_READY,
    APP_IN_CALL,
    APP_ERROR
};

AppState appState = APP_INIT;
unsigned long lastUpdate = 0;
bool autoRegister = true;

// WiFi connection with timeout
bool connectWiFi() {
    Serial.println("Connecting to WiFi...");
    ui.updateStatus(UI_CONNECTING_WIFI);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_CONNECT_TIMEOUT) {
            Serial.println("WiFi connection timeout");
            return false;
        }
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    ui.displayWiFiStatus(true, WiFi.localIP().toString().c_str());
    
    return true;
}

void handleCallButton() {
    if (!ui.isButtonAPressed()) return;
    
    switch (appState) {
        case APP_READY:
            // Make a call to default URI
            Serial.println("Making call...");
            ui.updateStatus(UI_CALLING, DEFAULT_CALL_URI);
            
            if (sipClient.makeCall(DEFAULT_CALL_URI)) {
                appState = APP_IN_CALL;
            } else {
                ui.updateError("Call failed");
            }
            break;
            
        case APP_IN_CALL:
            // Mute/unmute (not implemented yet)
            Serial.println("Mute/unmute toggle");
            break;
            
        default:
            break;
    }
}

void handleHangupButton() {
    if (!ui.isButtonBPressed()) return;
    
    switch (appState) {
        case APP_IN_CALL:
            // Hang up the call
            Serial.println("Hanging up...");
            sipClient.hangUp();
            audioHandler.stopCall();
            appState = APP_READY;
            ui.updateStatus(UI_REGISTERED);
            break;
            
        default:
            break;
    }
}

void updateSIPState() {
    // Update SIP client
    sipClient.update();
    
    // Sync UI with SIP state
    SIPState sipState = sipClient.getState();
    
    switch (sipState) {
        case SIP_IDLE:
            if (appState == APP_WIFI_CONNECTED && autoRegister) {
                appState = APP_REGISTERING_SIP;
                ui.updateStatus(UI_REGISTERING);
                sipClient.registerWithServer();
            }
            break;
            
        case SIP_REGISTERED:
            if (appState != APP_READY && appState != APP_IN_CALL) {
                appState = APP_READY;
                ui.updateStatus(UI_REGISTERED);
                Serial.println("Ready to make/receive calls");
            }
            break;
            
        case SIP_CALLING:
            ui.updateStatus(UI_CALLING);
            break;
            
        case SIP_RINGING:
            ui.updateStatus(UI_RINGING);
            break;
            
        case SIP_IN_CALL:
            if (appState != APP_IN_CALL) {
                appState = APP_IN_CALL;
                ui.updateStatus(UI_IN_CALL);
                
                // Start audio session
                const char* remoteIP = sipClient.getRemoteIP();
                uint16_t remotePort = sipClient.getRemoteRTPPort();
                uint16_t localPort = sipClient.getRTPPort();
                
                if (strlen(remoteIP) > 0 && remotePort > 0) {
                    audioHandler.startCall(remoteIP, remotePort, localPort);
                    Serial.printf("Audio session started: %s:%d <-> :%d\n", 
                                  remoteIP, remotePort, localPort);
                }
            }
            break;
            
        case SIP_ERROR:
            ui.updateStatus(UI_ERROR, "SIP Error");
            appState = APP_ERROR;
            break;
            
        default:
            break;
    }
}

void updateCallStatus() {
    if (appState == APP_IN_CALL) {
        ui.displayCallStatus("Active Call");
    }
}

void setup() {
    // Initialize serial
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("  M5Stack StickC Plus2 SIP Client");
    Serial.println("========================================");
    Serial.println();
    
    // Print configuration
    Serial.println("Configuration:");
    Serial.printf("  WiFi SSID: %s\n", WIFI_SSID);
    Serial.printf("  SIP Server: %s:%d\n", SIP_SERVER_IP, SIP_SERVER_PORT);
    Serial.printf("  SIP User: %s@%s\n", SIP_USER, SIP_DOMAIN);
    Serial.printf("  Default Call URI: %s\n", DEFAULT_CALL_URI);
    Serial.printf("  Audio Codec: G.711 %s\n", AUDIO_CODEC_MULAW ? "μ-law" : "A-law");
    Serial.println();
    
    // Initialize UI
    if (!ui.begin()) {
        Serial.println("Failed to initialize UI!");
        while (1) delay(1000);
    }
    
    // Initialize audio handler
    if (!audioHandler.begin()) {
        Serial.println("Failed to initialize audio!");
        ui.updateError("Audio init failed");
        // Continue anyway - audio might work later
    }
    
    // Connect to WiFi
    appState = APP_CONNECTING_WIFI;
    if (!connectWiFi()) {
        Serial.println("Failed to connect to WiFi!");
        ui.updateStatus(UI_ERROR, "WiFi failed");
        appState = APP_ERROR;
        return;
    }
    
    appState = APP_WIFI_CONNECTED;
    delay(1000);
    
    // Initialize SIP client
    if (!sipClient.begin()) {
        Serial.println("Failed to initialize SIP client!");
        ui.updateStatus(UI_ERROR, "SIP init failed");
        appState = APP_ERROR;
        return;
    }
    
    Serial.println("SIP Client ready!");
    Serial.println();
    Serial.println("Controls:");
    Serial.println("  Button A: Make call / Answer");
    Serial.println("  Button B: Hang up");
    Serial.println();
    Serial.println("Serial commands:");
    Serial.println("  CALL <sip-uri> - Make a call");
    Serial.println("  HANGUP - End current call");
    Serial.println("  STATUS - Show current status");
    Serial.println("  REGISTER - Re-register with SIP server");
    Serial.println();
    
    lastUpdate = millis();
}

void loop() {
    // Update UI (check buttons)
    ui.checkButtons();
    
    // Handle button presses
    handleCallButton();
    handleHangupButton();
    
    // Update SIP state
    updateSIPState();
    
    // Process audio if in call
    if (appState == APP_IN_CALL && audioHandler.isActive()) {
        audioHandler.processAudio();
    }
    
    // Periodic UI updates
    if (millis() - lastUpdate > 1000) {
        updateCallStatus();
        lastUpdate = millis();
    }
    
    // Handle serial commands
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        cmd.toUpperCase();
        
        if (cmd.startsWith("CALL ")) {
            String uri = cmd.substring(5);
            uri.trim();
            Serial.printf("Making call to: %s\n", uri.c_str());
            sipClient.makeCall(uri.c_str());
        } else if (cmd == "HANGUP") {
            Serial.println("Hanging up...");
            sipClient.hangUp();
            audioHandler.stopCall();
        } else if (cmd == "STATUS") {
            Serial.println("\nStatus:");
            Serial.printf("  App State: %d\n", appState);
            Serial.printf("  SIP State: %s\n", sipClient.getStateString());
            Serial.printf("  WiFi: %s (IP: %s)\n", 
                          WiFi.isConnected() ? "Connected" : "Disconnected",
                          WiFi.localIP().toString().c_str());
            Serial.printf("  Audio: %s\n", audioHandler.isActive() ? "Active" : "Inactive");
            Serial.println();
        } else if (cmd == "REGISTER") {
            Serial.println("Registering with SIP server...");
            sipClient.registerWithServer();
        } else if (cmd == "HELP") {
            Serial.println("\nAvailable commands:");
            Serial.println("  CALL <sip-uri> - Make a call");
            Serial.println("  HANGUP - End current call");
            Serial.println("  STATUS - Show current status");
            Serial.println("  REGISTER - Re-register with SIP server");
            Serial.println("  HELP - Show this help");
            Serial.println();
        } else if (cmd.length() > 0) {
            Serial.printf("Unknown command: %s (type HELP for commands)\n", cmd.c_str());
        }
    }
    
    // Small delay to prevent watchdog issues
    delay(10);
}
