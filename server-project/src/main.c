/*
 * main.c
 *
 * UDP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a UDP server
 * portable across Windows, Linux, and macOS.
 */

#if defined(_WIN32) || defined(WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <ctype.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <signal.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "protocol.h"

#define NO_ERROR 0

// Global socket for cleanup on signal
static int g_serverSocket = -1;

void clearwinsock() {
#if defined(_WIN32) || defined(WIN32)
	WSACleanup();
#endif
}

// Signal handler for cleanup
void signalHandler(int sig) {
	if (g_serverSocket != -1) {
		closesocket(g_serverSocket);
	}
	clearwinsock();
	exit(0);
}

// Parse server command line arguments
int ParseServerArguments(int argc, char *argv[], int *port) {
	*port = SERVER_PORT; // default
	
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-p") == 0) {
			if (i + 1 < argc) {
				*port = atoi(argv[i + 1]);
				if (*port <= 0 || *port > 65535) {
					fprintf(stderr, "Invalid port number\n");
					return -1;
				}
				i++;
			} else {
				fprintf(stderr, "Missing port number after -p\n");
				return -1;
			}
		}
	}
	return 0;
}

// Create UDP socket
int CreateUDPSocket(void) {
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
#if defined(_WIN32) || defined(WIN32)
		fprintf(stderr, "Error creating socket: %d\n", WSAGetLastError());
#else
		perror("Error creating socket");
#endif
		return -1;
	}
	return sock;
}

// Validate request type
int ValidateRequestType(char type) {
	return (type == 't' || type == 'h' || type == 'w' || type == 'p');
}

// Case-insensitive string comparison
int CaseInsensitiveCompare(const char *s1, const char *s2) {
	while (*s1 && *s2) {
		if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
			return 1;
		}
		s1++;
		s2++;
	}
	return (*s1 != *s2);
}

// Check if city has invalid characters (tabs, special chars)
int HasInvalidCharacters(const char *city) {
	for (int i = 0; city[i] != '\0'; i++) {
		if (city[i] == '\t') {
			return 1;
		}
		// Check for special characters (non-alphanumeric, non-space, non-apostrophe, non-hyphen)
		if (!isalnum((unsigned char)city[i]) && 
		    city[i] != ' ' && 
		    city[i] != '\'' && 
		    city[i] != '-') {
			return 1;
		}
	}
	return 0;
}

// Check if city is supported (case-insensitive)
int IsCitySupported(const char *city) {
	const char *supportedCities[] = {
		"Bari", "Roma", "Milano", "Napoli", "Torino",
		"Palermo", "Genova", "Bologna", "Firenze", "Venezia"
	};
	int numCities = 10;
	
	for (int i = 0; i < numCities; i++) {
		if (CaseInsensitiveCompare(city, supportedCities[i]) == 0) {
			return 1;
		}
	}
	return 0;
}

// Validate city name
int ValidateCity(const char *city) {
	if (HasInvalidCharacters(city)) {
		return 0;
	}
	return IsCitySupported(city);
}

// Get temperature (-10.0 to 40.0 °C)
float GetTemperature(void) {
	return ((float)rand() / RAND_MAX) * 50.0f - 10.0f;
}

// Get humidity (20.0 to 100.0%)
float GetHumidity(void) {
	return ((float)rand() / RAND_MAX) * 80.0f + 20.0f;
}

// Get wind speed (0.0 to 100.0 km/h)
float GetWind(void) {
	return ((float)rand() / RAND_MAX) * 100.0f;
}

// Get pressure (950.0 to 1050.0 hPa)
float GetPressure(void) {
	return ((float)rand() / RAND_MAX) * 100.0f + 950.0f;
}

// Format city name (first letter uppercase, rest lowercase)
void FormatCityName(char *city) {
	if (city == NULL || city[0] == '\0') {
		return;
	}
	
	// First letter uppercase
	city[0] = toupper((unsigned char)city[0]);
	
	// Rest lowercase
	for (int i = 1; city[i] != '\0'; i++) {
		city[i] = tolower((unsigned char)city[i]);
	}
}

// Get hostname from address (reverse DNS lookup)
int GetHostnameFromAddress(const struct sockaddr_in *addr, char *hostname, int hostnameSize) {
	char ipStr[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &addr->sin_addr, ipStr, INET_ADDRSTRLEN) == NULL) {
		return -1;
	}
	
#if defined(_WIN32) || defined(WIN32)
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];
	if (getnameinfo((struct sockaddr *)addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, serv, NI_MAXSERV, 0) != 0) {
		strncpy(hostname, ipStr, hostnameSize - 1);
		hostname[hostnameSize - 1] = '\0';
		return 0;
	}
	strncpy(hostname, host, hostnameSize - 1);
	hostname[hostnameSize - 1] = '\0';
#else
	if (getnameinfo((struct sockaddr *)addr, sizeof(struct sockaddr_in), hostname, hostnameSize, NULL, 0, 0) != 0) {
		strncpy(hostname, ipStr, hostnameSize - 1);
		hostname[hostnameSize - 1] = '\0';
		return 0;
	}
#endif
	return 0;
}

// Deserialize request from buffer
int DeserializeRequest(const char *buffer, int bufferSize, struct request *req) {
	if (buffer == NULL || req == NULL || bufferSize < (int)(sizeof(char) + 1)) {
		return -1;
	}
	
	int offset = 0;
	
	// Deserialize type (1 byte)
	req->type = buffer[offset];
	offset += sizeof(char);
	
	// Deserialize city (up to 64 bytes including null terminator)
	int cityLen = 0;
	while (offset < bufferSize && cityLen < MAX_CITY_LENGTH - 1 && buffer[offset] != '\0') {
		req->city[cityLen++] = buffer[offset++];
	}
	req->city[cityLen] = '\0';
	
	return 0;
}

// Serialize response to buffer
int SerializeResponse(const struct response *resp, char *buffer, int bufferSize) {
	if (resp == NULL || buffer == NULL) {
		return -1;
	}
	
	int requiredSize = sizeof(uint32_t) + sizeof(char) + sizeof(float);
	if (bufferSize < requiredSize) {
		return -1;
	}
	
	int offset = 0;
	
	// Serialize status (uint32_t) with network byte order
	uint32_t netStatus = htonl(resp->status);
	memcpy(buffer + offset, &netStatus, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	
	// Serialize type (1 byte, no conversion needed)
	buffer[offset] = resp->type;
	offset += sizeof(char);
	
	// Serialize value (float) with network byte order
	uint32_t temp;
	memcpy(&temp, &resp->value, sizeof(float));
	temp = htonl(temp);
	memcpy(buffer + offset, &temp, sizeof(float));
	offset += sizeof(float);
	
	return offset;
}

int main(int argc, char *argv[]) {
	int port;
	struct sockaddr_in serverAddr, clientAddr;
	char buffer[BUFFER_SIZE];
	struct request req;
	struct response resp;
	socklen_t clientAddrLen;
	int bytesReceived, bytesSent;
	char clientHostname[NI_MAXHOST];
	char clientIP[INET_ADDRSTRLEN];
	
	// Initialize random seed
	srand((unsigned int)time(NULL));
	
	// Parse arguments
	if (ParseServerArguments(argc, argv, &port) != 0) {
		return 1;
	}

#if defined(_WIN32) || defined(WIN32)
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 1;
	}
#endif

	// Setup signal handler
#if !defined(_WIN32) && !defined(WIN32)
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
#endif

	// Create UDP socket
	int my_socket = CreateUDPSocket();
	if (my_socket < 0) {
		clearwinsock();
		return 1;
	}
	g_serverSocket = my_socket;

	// Configure server address
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons((unsigned short)port);

	// Bind socket
	if (bind(my_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
#if defined(_WIN32) || defined(WIN32)
		fprintf(stderr, "Error binding socket: %d\n", WSAGetLastError());
#else
		perror("Error binding socket");
#endif
		closesocket(my_socket);
		clearwinsock();
		return 1;
	}

	printf("Server listening on port %d\n", port);

	// UDP datagram reception loop
	while (1) {
		clientAddrLen = sizeof(clientAddr);
		memset(buffer, 0, BUFFER_SIZE);
		
		// Receive request
		bytesReceived = recvfrom(my_socket, buffer, BUFFER_SIZE - 1, 0,
		                         (struct sockaddr *)&clientAddr, &clientAddrLen);
		
		if (bytesReceived < 0) {
#if defined(_WIN32) || defined(WIN32)
			int error = WSAGetLastError();
			if (error != WSAECONNRESET) {
				fprintf(stderr, "Error receiving data: %d\n", error);
			}
#else
			perror("Error receiving data");
#endif
			continue;
		}
		
		// Get client IP address
		if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN) == NULL) {
			strcpy(clientIP, "unknown");
		}
		
		// Get client hostname (reverse DNS lookup)
		if (GetHostnameFromAddress(&clientAddr, clientHostname, NI_MAXHOST) != 0) {
			strncpy(clientHostname, clientIP, NI_MAXHOST - 1);
			clientHostname[NI_MAXHOST - 1] = '\0';
		}
		
		// Deserialize request
		memset(&req, 0, sizeof(req));
		if (DeserializeRequest(buffer, bytesReceived, &req) != 0) {
			// Invalid request format, send error response
			resp.status = 2;
			resp.type = '?';
			resp.value = 0.0f;
		} else {
			// Validate request type
			if (!ValidateRequestType(req.type)) {
				resp.status = 2;
				resp.type = req.type;
				resp.value = 0.0f;
			} else if (!ValidateCity(req.city)) {
				// City not found or invalid characters
				if (HasInvalidCharacters(req.city)) {
					resp.status = 2;
				} else {
					resp.status = 1;
				}
				resp.type = req.type;
				resp.value = 0.0f;
			} else {
				// Valid request, generate weather data
				resp.status = 0;
				resp.type = req.type;
				
				switch (req.type) {
					case 't':
						resp.value = GetTemperature();
						break;
					case 'h':
						resp.value = GetHumidity();
						break;
					case 'w':
						resp.value = GetWind();
						break;
					case 'p':
						resp.value = GetPressure();
						break;
					default:
						resp.status = 2;
						resp.value = 0.0f;
						break;
				}
			}
		}
		
		// Log request
		printf("Richiesta ricevuta da %s (ip %s): type='%c', city='%s'\n",
		       clientHostname, clientIP, req.type, req.city);
		
		// Serialize and send response
		int respSize = SerializeResponse(&resp, buffer, BUFFER_SIZE);
		if (respSize > 0) {
			bytesSent = sendto(my_socket, buffer, respSize, 0,
			                   (struct sockaddr *)&clientAddr, clientAddrLen);
			if (bytesSent < 0) {
#if defined(_WIN32) || defined(WIN32)
				fprintf(stderr, "Error sending response: %d\n", WSAGetLastError());
#else
				perror("Error sending response");
#endif
			}
		}
	}

	printf("Server terminated.\n");

	closesocket(my_socket);
	clearwinsock();
	return 0;
} // main end
