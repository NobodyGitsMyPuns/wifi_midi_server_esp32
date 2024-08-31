#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "/Users/jesselopez/Documents/Arduino/wifi_midi_server/midi_wifi_server_esp32/midi_wifi_server_esp/config.h"

WebServer server(80);

// Function to sanitize the filename
String sanitizeFilename(const String& filename) {
    String sanitized = filename;

    // Remove /midi/ prefix if it exists
    if (sanitized.startsWith("/midi/")) {
        sanitized = sanitized.substring(6); // Remove the /midi/ part
    }

    // Replace spaces and %20 with underscores
    sanitized.replace(" ", "_");
    sanitized.replace("%20", "_");

    // Remove any extra .mid extensions
    if (sanitized.endsWith(".mid.mid")) {
        sanitized = sanitized.substring(0, sanitized.length() - 4); // Remove the extra .mid
    }

    return sanitized;
}

void setup() {
    Serial.begin(115200);

    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    // Print available space
    Serial.printf("Available space: %lu bytes\n", LittleFS.totalBytes() - LittleFS.usedBytes());

    // Connect to Wi-Fi
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    // Route to handle file download
    server.on("/downloadSignedMidi", HTTP_POST, []() {
        if (server.hasArg("url") && server.hasArg("filename")) {
            String url = server.arg("url");
            String originalFilename = server.arg("filename");
            String sanitizedFilename = sanitizeFilename(originalFilename);

            Serial.printf("Signed URL: %s\n", url.c_str());
            Serial.printf("Downloading from URL: %s\n", url.c_str());
            Serial.printf("Saving as: %s\n", sanitizedFilename.c_str());

            WiFiClientSecure client;
            client.setInsecure();  // Disable SSL/TTLS verification
            HTTPClient http;
            http.begin(client, url);
            int httpCode = http.GET();

            if (httpCode == HTTP_CODE_OK) {
                File file = LittleFS.open(sanitizedFilename, FILE_WRITE);
                if (!file) {
                    Serial.println("Failed to create file.");
                    server.send(500, "text/plain", "Failed to create file.");
                    return;
                }

                WiFiClient *stream = http.getStreamPtr();
                int totalBytesWritten = 0;
                uint8_t buffer[512];
                int bytesRead;

                while ((bytesRead = stream->read(buffer, sizeof(buffer))) > 0) {
                    // Print the data to the Serial monitor for debugging
                    Serial.print("Data received: ");
                    for (int i = 0; i < bytesRead; i++) {
                        Serial.print((char)buffer[i]);
                    }
                    Serial.println();

                    int bytesWritten = file.write(buffer, bytesRead);
                    totalBytesWritten += bytesWritten;
                    Serial.printf("Read %d bytes, wrote %d bytes\n", bytesRead, bytesWritten);
                }
                file.close();

                if (totalBytesWritten > 0) {
                    Serial.println("File downloaded and saved successfully.");
                    server.send(200, "text/plain", "File downloaded and saved successfully.");
                } else {
                    Serial.println("Failed to write data to file.");
                    server.send(500, "text/plain", "Failed to write data to file.");
                }
            } else {
                Serial.printf("Failed to download file, error: %d\n", httpCode);
                server.send(500, "text/plain", "Failed to download file.");
            }
            http.end();
        } else {
            server.send(400, "text/plain", "URL or filename missing.");
        }
    });

    // Start server
    server.begin();
}

void loop() {
    server.handleClient();  // Handle incoming client requests
}
