#include <WiFi.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <SPIFFSEditor.h>
#include "WifiManager.h"
#include "giessomat.h"

extern WifiManager wifiManager;
extern uint16_t sensorval;
extern struct config_item configuration;

// SKETCH BEGIN
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

String HTMLfoot() {
	return "</body></html>";
}

String HTMLhead(String title) {
	String html = "<html><head><title>" + title + "</title>";
	html += "<link rel='stylesheet' type='text/css' href='default.css' />";
	html += "</head><body>";
	return html;
}

String wifiscanner_form() {
	String reply = F("<form action='/connect' method='get'>");
	reply += F("<table id='tab1'><TH><TH>Access Points<TH>Empfang");

	int n = WiFi.scanNetworks();
	if (n == 0)
		reply += F("Keine Access Points gefunden");
	else {
		for (int i = 0; i < n; ++i) {
			reply += F("<TR><TD>");
			reply += "<input type='radio' name='ssid' value='" + WiFi.SSID(i) + "'";
			if(i == 0)
				reply += " checked";
			reply += "><TD>";
			reply += WiFi.SSID(i);
			reply += "<TD>";
			reply += "<progress max='255' value='";
			reply += (uint8_t)WiFi.RSSI(i);
			reply += "'></progress>";
		}
	}

	reply += F("</table><br>");
	reply += F("Passwort: ");
	reply += F("<input type='text' name='password'>");
	reply += F("<input type='submit' value='Verbinden'>");
	return reply;
}

String handle_wifiscanner() {
	String reply = HTMLhead("Wifi");
	reply += wifiscanner_form();
	reply += HTMLfoot();
	return reply;
}


String handle_wificonnect(AsyncWebServerRequest *request) {
	String reply = HTMLhead("Wifi");

	String ssid, passwd;
	int i;
	int params = request->params();
	for(i=0;i<params;i++) {
		AsyncWebParameter* p = request->getParam(i);
		if(p->name() == "ssid")
			ssid = p->value();
		if(p->name() == "password")
			passwd = p->value();
	}
	if(wifiManager.connect(ssid, passwd))
	{
		String ip = WiFi.localIP().toString();
		reply += "Verbunden mit " + ssid + " mit IP Adresse: " + ip + "<br>";
		reply += "<a href='http://" + ip + "/disconnectAP'>";
		reply += "Mit IP " + ip + " verbinden und Access Point ausschalten</a>";
	}
	else
	{
		reply += wifiscanner_form();
		reply += "<br><br>Verbindung zu " + ssid + " fehlgeschlagen !";
	}

	reply += HTMLfoot();
	return reply;
}

String handle_giesstab(){
	String reply = HTMLhead("Giess-o-mat Werte");
	reply += F("<table id='tab1'>");
	reply += F("<TR><TD>Sensor</TD><TD>");
	reply += String(sensorval);
	reply += F("</TD></TR>");
	reply += F("<TR><TD>Version</TD><TD>");
	reply += String(configuration.version);
	reply += F("</TD></TR>");
	reply += F("<TR><TD>Schaltschwelle trocken</TD><TD>");
	reply += String(configuration.threashold_dry);
	reply += F("</TD></TR>");
	reply += F("<TR><TD>Schaltschwelle nass</TD><TD>");
	reply += String(configuration.threashold_wet);
	reply += F("</TD></TR>");
	reply += F("<TR><TD>Einschaltzeit</TD><TD>");
	reply += String(configuration.seconds_on);
	reply += F("</TD></TR>");
	reply += F("<TR><TD>Ausschaltzeit</TD><TD>");
	reply += String(configuration.minutes_off);
	reply += F("</TD></TR>");
	reply += F("</table><br>");
	reply += F("<a href='/giesstab'>aktualisieren</a>");
	reply += HTMLfoot();
	return reply;
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client,
		AwsEventType type, void * arg, uint8_t *data, size_t len) {
	if (type == WS_EVT_CONNECT) {
		Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
		client->printf("Hello Client %u :)", client->id());
		client->ping();
	} else if (type == WS_EVT_DISCONNECT) {
		Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(),
				client->id());
	} else if (type == WS_EVT_ERROR) {
		Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(),
				*((uint16_t*) arg), (char*) data);
	} else if (type == WS_EVT_PONG) {
		Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(),
				len, (len) ? (char*) data : "");
	} else if (type == WS_EVT_DATA) {
		AwsFrameInfo * info = (AwsFrameInfo*) arg;
		String msg = "";
		if (info->final && info->index == 0 && info->len == len) {
			//the whole message is in a single frame and we got all of it's data
			Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(),
					client->id(), (info->opcode == WS_TEXT) ? "text" : "binary",
					info->len);

			if (info->opcode == WS_TEXT) {
				for (size_t i = 0; i < info->len; i++) {
					msg += (char) data[i];
				}
			} else {
				char buff[3];
				for (size_t i = 0; i < info->len; i++) {
					sprintf(buff, "%02x ", (uint8_t) data[i]);
					msg += buff;
				}
			}
			Serial.printf("%s\n", msg.c_str());

			if (info->opcode == WS_TEXT)
				client->text("I got your text message");
			else
				client->binary("I got your binary message");
		} else {
			//message is comprised of multiple frames or the frame is split into multiple packets
			if (info->index == 0) {
				if (info->num == 0)
					Serial.printf("ws[%s][%u] %s-message start\n",
							server->url(), client->id(),
							(info->message_opcode == WS_TEXT) ?
									"text" : "binary");
				Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n",
						server->url(), client->id(), info->num, info->len);
			}

			Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ",
					server->url(), client->id(), info->num,
					(info->message_opcode == WS_TEXT) ? "text" : "binary",
					info->index, info->index + len);

			if (info->opcode == WS_TEXT) {
				for (size_t i = 0; i < info->len; i++) {
					msg += (char) data[i];
				}
			} else {
				char buff[3];
				for (size_t i = 0; i < info->len; i++) {
					sprintf(buff, "%02x ", (uint8_t) data[i]);
					msg += buff;
				}
			}
			Serial.printf("%s\n", msg.c_str());

			if ((info->index + len) == info->len) {
				Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(),
						client->id(), info->num, info->len);
				if (info->final) {
					Serial.printf("ws[%s][%u] %s-message end\n", server->url(),
							client->id(),
							(info->message_opcode == WS_TEXT) ?
									"text" : "binary");
					if (info->message_opcode == WS_TEXT)
						client->text("I got your text message");
					else
						client->binary("I got your binary message");
				}
			}
		}
	}
}

const char * hostName = "giess-o-mat";
const char* http_username = "admin";
const char* http_password = "admin";

void setup_webserver() {
	Serial.setDebugOutput(true);

	//Send OTA events to the browser
	ArduinoOTA.onStart([]() {events.send("Update Start", "ota");});
	ArduinoOTA.onEnd([]() {events.send("Update End", "ota");});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		char p[32];
		sprintf(p, "Progress: %u%%\n", (progress/(total/100)));
		events.send(p, "ota");
	});
	ArduinoOTA.onError(
			[](ota_error_t error) {
				if(error == OTA_AUTH_ERROR) events.send("Auth Failed", "ota");
				else if(error == OTA_BEGIN_ERROR) events.send("Begin Failed", "ota");
				else if(error == OTA_CONNECT_ERROR) events.send("Connect Failed", "ota");
				else if(error == OTA_RECEIVE_ERROR) events.send("Recieve Failed", "ota");
				else if(error == OTA_END_ERROR) events.send("End Failed", "ota");
			});
	ArduinoOTA.setHostname(hostName);
	ArduinoOTA.begin();

	//SPIFFS.begin();

	ws.onEvent(onWsEvent);
	server.addHandler(&ws);

	events.onConnect([](AsyncEventSourceClient *client) {
		client->send("hello!",NULL,millis(),1000);
	});
	server.addHandler(&events);

	server.addHandler(new SPIFFSEditor(SPIFFS, http_username, http_password));

	server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", String(ESP.getFreeHeap()));
	});

	server.on("/wifiscan", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/html", handle_wifiscanner());
	});

	server.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/html", handle_wificonnect(request));
	});

	server.on("/disconnectAP", HTTP_GET, [](AsyncWebServerRequest *request) {
		wifiManager.disconnectAP();
		server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");
	});

	server.on("/giesstab", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/html", handle_giesstab());
	});

	server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

	server.onNotFound(
			[](AsyncWebServerRequest *request) {
				Serial.printf("NOT_FOUND: ");
				if(request->method() == HTTP_GET)
				Serial.printf("GET");
				else if(request->method() == HTTP_POST)
				Serial.printf("POST");
				else if(request->method() == HTTP_DELETE)
				Serial.printf("DELETE");
				else if(request->method() == HTTP_PUT)
				Serial.printf("PUT");
				else if(request->method() == HTTP_PATCH)
				Serial.printf("PATCH");
				else if(request->method() == HTTP_HEAD)
				Serial.printf("HEAD");
				else if(request->method() == HTTP_OPTIONS)
				Serial.printf("OPTIONS");
				else
				Serial.printf("UNKNOWN");
				Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

				if(request->contentLength()) {
					Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
					Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
				}

				int headers = request->headers();
				int i;
				for(i=0;i<headers;i++) {
					AsyncWebHeader* h = request->getHeader(i);
					Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
				}

				int params = request->params();
				for(i=0;i<params;i++) {
					AsyncWebParameter* p = request->getParam(i);
					if(p->isFile()) {
						Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
					} else if(p->isPost()) {
						Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
					} else {
						Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
					}
				}

				request->send(404);
			});
	server.onFileUpload(
			[](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
				if(!index)
				Serial.printf("UploadStart: %s\n", filename.c_str());
				Serial.printf("%s", (const char*)data);
				if(final)
				Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
			});
	server.onRequestBody(
			[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
				if(!index)
				Serial.printf("BodyStart: %u\n", total);
				Serial.printf("%s", (const char*)data);
				if(index + len == total)
				Serial.printf("BodyEnd: %u\n", total);
			});
	server.begin();
}

void loop_webserver() {
	ArduinoOTA.handle();
}
