/**
 * @file ui_handler.h
 * @brief User interface handler for SIP client
 * 
 * Manages:
 * - Display updates showing call status
 * - Button input handling
 * - Status indicators
 */

#ifndef UI_HANDLER_H
#define UI_HANDLER_H

#include <M5StickCPlus2.h>
#include "config.h"

// UI states
enum UIState {
    UI_DISCONNECTED,
    UI_CONNECTING_WIFI,
    UI_CONNECTED_WIFI,
    UI_REGISTERING,
    UI_REGISTERED,
    UI_CALLING,
    UI_RINGING,
    UI_IN_CALL,
    UI_ERROR
};

class UIHandler {
public:
    UIHandler();
    
    // Initialize display and buttons
    bool begin();
    
    // Update display
    void updateStatus(UIState state, const char* message = nullptr);
    void updateCallInfo(const char* callInfo);
    void updateError(const char* error);
    
    // Display specific information
    void displayWiFiStatus(bool connected, const char* ip = nullptr);
    void displaySIPStatus(const char* status);
    void displayCallStatus(const char* status, const char* duration = nullptr);
    
    // Button handling
    void checkButtons();
    bool isButtonAPressed();  // Call/Answer button
    bool isButtonBPressed();  // Hang up button
    
    // Visual feedback
    void showCallIndicator(bool active);
    
private:
    UIState currentState;
    unsigned long lastUpdate;
    unsigned long callStartTime;
    
    // Button states
    bool btnAPressed;
    bool btnBPressed;
    bool btnPwrPressed;
    unsigned long btnALastPress;
    unsigned long btnBLastPress;
    
    // Display helpers
    void clearScreen();
    void drawHeader(const char* title);
    void drawStatusBar();
    void drawCenteredText(const char* text, int y, uint16_t color = COLOR_TEXT);
    String formatCallDuration(unsigned long seconds);
};

#endif // UI_HANDLER_H
