#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "config.h"  // Include the config.h file

WebServer server(80);  // Create a web server object that listens on port 80

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

    // Route to handle file upload with binary data
    server.on("/upload", HTTP_POST, []() {
        if (server.hasArg("filename")) {
            String filename = sanitizeFilename("/" + server.arg("filename"));

            Serial.printf("Attempting to create file: %s\n", filename.c_str());

            // Write the binary data to the file
            size_t len = server.arg("plain").length();
            File file = LittleFS.open(filename, FILE_WRITE);
            if (file) {
                file.write((uint8_t*)server.arg("plain").c_str(), len);
                file.flush();  // Ensure data is written to storage
                file.close();

                Serial.println("File written successfully");

                // Send a response
                String response = "File Uploaded: " + filename;
                server.send(200, "text/plain", response);
            } else {
                Serial.println("Failed to create file.");
                server.send(500, "text/plain", "Failed to create file for writing");
            }
        } else {
            server.send(400, "text/plain", "Filename missing");
        }
    });

    // Route to handle file listing
    server.on("/files", HTTP_GET, []() {
        String fileList = "";
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            fileList += String(file.name()) + "\n";
            file = root.openNextFile();
        }
        server.send(200, "text/plain", fileList);
    });

    // Route to handle file deletion
    server.on("/delete", HTTP_DELETE, []() {
        if (server.hasArg("name")) {
            String filename = "/" + server.arg("name");
            if (LittleFS.remove(filename)) {
                Serial.println("File deleted successfully");

                // Send a response
                String response = "File Deleted";
                server.send(200, "text/plain", response);
            } else {
                server.send(404, "text/plain", "File Not Found");
            }
        } else {
            server.send(400, "text/plain", "Name parameter missing");
        }
    });

    // Route to handle IP check
    server.on("/check-ip", HTTP_GET, []() {
        server.send(200, "text/plain", "ESP32 is reachable");
        Serial.println("Received Check-ip!");
    });

    // Start server
    server.begin();
}

void loop() {
    server.handleClient();  // Handle incoming client requests
}
