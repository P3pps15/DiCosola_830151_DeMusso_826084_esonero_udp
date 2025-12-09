/*
 * main.c
 *
 * UDP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a UDP client
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
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

#define NO_ERROR 0

void clearwinsock() {
#if defined(_WIN32) || defined(WIN32)
	WSACleanup();
#endif
}

// Parse client command line arguments
int ParseClientArguments(int argc, char *argv[], char **server, int *port, char **request) {
	*server = "localhost"; // default
	*port = SERVER_PORT; // default
	*request = NULL;
	
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-s") == 0) {
			if (i + 1 < argc) {
				*server = argv[i + 1];
				i++;
			} else {
				fprintf(stderr, "Missing server address after -s\n");
				return -1;
			}
		} else if (strcmp(argv[i], "-p") == 0) {
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
		} else if (strcmp(argv[i], "-r") == 0) {
			if (i + 1 < argc) {
				*request = argv[i + 1];
				i++;
			} else {
				fprintf(stderr, "Missing request after -r\n");
				return -1;
			}
		}
	}
	
	if (*request == NULL) {
		fprintf(stderr, "Missing required argument -r\n");
		return -1;
	}
	
	return 0;
}

// Check if string contains tab characters
int HasTabCharacters(const char *str) {
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] == '\t') {
			return 1;
		}
	}
	return 0;
}

// Validate request string and extract type and city
int ValidateRequest(const char *requestStr, char *type, char *city) {
	if (requestStr == NULL || type == NULL || city == NULL) {
		return -1;
	}
	
	// Check for tab characters
	if (HasTabCharacters(requestStr)) {
		fprintf(stderr, "Request contains tab characters\n");
		return -1;
	}
	
	// Skip leading spaces
	int start = 0;
	while (requestStr[start] == ' ') {
		start++;
	}
	
	if (requestStr[start] == '\0') {
		fprintf(stderr, "Invalid request format\n");
		return -1;
	}
	
	// Extract first token (should be single character)
	*type = requestStr[start];
	
	// Check if first token is more than one character
	if (requestStr[start + 1] != ' ' && requestStr[start + 1] != '\0') {
		fprintf(stderr, "Request type must be a single character\n");
		return -1;
	}
	
	// Find start of city name (skip spaces after type)
	int cityStart = start + 1;
	while (requestStr[cityStart] == ' ') {
		cityStart++;
	}
	
	if (requestStr[cityStart] == '\0') {
		fprintf(stderr, "Missing city name\n");
		return -1;
	}
	
	// Extract city name
	int cityLen = 0;
	int i = cityStart;
	while (requestStr[i] != '\0' && cityLen < MAX_CITY_LENGTH - 1) {
		city[cityLen++] = requestStr[i++];
	}
	city[cityLen] = '\0';
	
	return 0;
}

// Validate city length
int ValidateCityLength(const char *city) {
	int len = 0;
	while (city[len] != '\0') {
		len++;
	}
	if (len > 63) {
		fprintf(stderr, "City name too long (maximum 63 characters)\n");
		return -1;
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

// Get hostname from address (reverse DNS lookup)
int GetHostnameFromAddress(const struct sockaddr_in *addr, char *hostname, int hostnameSize) {
	char ipStr[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &addr->sin_addr, ipStr, INET_ADDRSTRLEN) == NULL) {
		return -1;
	}
	
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	if (inet_pton(AF_INET, ipStr, &sa.sin_addr) <= 0) {
		return -1;
	}
	
#if defined(_WIN32) || defined(WIN32)
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];
	if (getnameinfo((struct sockaddr *)&sa, sizeof(sa), host, NI_MAXHOST, serv, NI_MAXSERV, 0) != 0) {
		strncpy(hostname, ipStr, hostnameSize - 1);
		hostname[hostnameSize - 1] = '\0';
		return 0;
	}
	strncpy(hostname, host, hostnameSize - 1);
	hostname[hostnameSize - 1] = '\0';
#else
	if (getnameinfo((struct sockaddr *)&sa, sizeof(sa), hostname, hostnameSize, NULL, 0, 0) != 0) {
		strncpy(hostname, ipStr, hostnameSize - 1);
		hostname[hostnameSize - 1] = '\0';
		return 0;
	}
#endif
	return 0;
}

// Resolve server address and get hostname
int ResolveServerAddress(const char *server, int port, struct sockaddr_in *serverAddr, char *hostname, int hostnameSize) {
	struct addrinfo hints, *result, *rp;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	
	int ret = getaddrinfo(server, NULL, &hints, &result);
	if (ret != 0) {
#if defined(_WIN32) || defined(WIN32)
		fprintf(stderr, "Error resolving server address\n");
#else
		fprintf(stderr, "Error resolving server address: %s\n", gai_strerror(ret));
#endif
		return -1;
	}
	
	// Find first valid address
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_family == AF_INET) {
			memcpy(serverAddr, rp->ai_addr, rp->ai_addrlen);
			((struct sockaddr_in *)serverAddr)->sin_port = htons((unsigned short)port);
			break;
		}
	}
	
	if (rp == NULL) {
		freeaddrinfo(result);
		return -1;
	}
	
	// Get hostname from resolved address
	if (GetHostnameFromAddress((struct sockaddr_in *)serverAddr, hostname, hostnameSize) != 0) {
		// Fallback to server parameter if reverse lookup fails
		strncpy(hostname, server, hostnameSize - 1);
		hostname[hostnameSize - 1] = '\0';
	}
	
	freeaddrinfo(result);
	return 0;
}

// Serialize request to buffer
int SerializeRequest(const struct request *req, char *buffer, int bufferSize) {
	if (req == NULL || buffer == NULL) {
		return -1;
	}
	
	int cityLen = (int)strlen(req->city);
	int requiredSize = sizeof(char) + cityLen + 1; // type + city + null terminator
	if (bufferSize < requiredSize) {
		return -1;
	}
	
	int offset = 0;
	
	// Serialize type (1 byte)
	buffer[offset] = req->type;
	offset += sizeof(char);
	
	// Serialize city (null-terminated string)
	memcpy(buffer + offset, req->city, cityLen + 1);
	offset += cityLen + 1;
	
	return offset;
}

// Deserialize response from buffer
int DeserializeResponse(const char *buffer, int bufferSize, struct response *resp) {
	if (buffer == NULL || resp == NULL) {
		return -1;
	}
	
	int requiredSize = sizeof(uint32_t) + sizeof(char) + sizeof(float);
	if (bufferSize < requiredSize) {
		return -1;
	}
	
	int offset = 0;
	
	// Deserialize status (uint32_t) with network byte order
	uint32_t netStatus;
	memcpy(&netStatus, buffer + offset, sizeof(uint32_t));
	resp->status = ntohl(netStatus);
	offset += sizeof(uint32_t);
	
	// Deserialize type (1 byte)
	resp->type = buffer[offset];
	offset += sizeof(char);
	
	// Deserialize value (float) with network byte order
	uint32_t temp;
	memcpy(&temp, buffer + offset, sizeof(float));
	temp = ntohl(temp);
	memcpy(&resp->value, &temp, sizeof(float));
	offset += sizeof(float);
	
	return 0;
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

// Print response with proper formatting
void PrintResponse(const struct response *resp, const char *hostname, const char *ip, const char *city) {
	if (resp == NULL || hostname == NULL || ip == NULL) {
		return;
	}
	
	printf("Ricevuto risultato dal server %s (ip %s). ", hostname, ip);
	
	if (resp->status == 0) {
		// Success - format city name
		char formattedCity[MAX_CITY_LENGTH];
		strncpy(formattedCity, city, MAX_CITY_LENGTH - 1);
		formattedCity[MAX_CITY_LENGTH - 1] = '\0';
		FormatCityName(formattedCity);
		
		switch (resp->type) {
			case 't':
				printf("%s: Temperatura = %.1f°C\n", formattedCity, resp->value);
				break;
			case 'h':
				printf("%s: Umidità = %.1f%%\n", formattedCity, resp->value);
				break;
			case 'w':
				printf("%s: Vento = %.1f km/h\n", formattedCity, resp->value);
				break;
			case 'p':
				printf("%s: Pressione = %.1f hPa\n", formattedCity, resp->value);
				break;
			default:
				printf("Città non disponibile\n");
				break;
		}
	} else if (resp->status == 1) {
		printf("Città non disponibile\n");
	} else if (resp->status == 2) {
		printf("Richiesta non valida\n");
	} else {
		printf("Errore sconosciuto\n");
	}
}

int main(int argc, char *argv[]) {
	char *server;
	int port;
	char *requestStr;
	char type;
	char city[MAX_CITY_LENGTH];
	struct request req;
	struct response resp;
	struct sockaddr_in serverAddr;
	char buffer[BUFFER_SIZE];
	int bytesSent, bytesReceived;
	char serverHostname[NI_MAXHOST];
	char serverIP[INET_ADDRSTRLEN];
	
	// Parse arguments
	if (ParseClientArguments(argc, argv, &server, &port, &requestStr) != 0) {
		return 1;
	}
	
	// Validate and parse request
	if (ValidateRequest(requestStr, &type, city) != 0) {
		return 1;
	}
	
	// Validate city length
	if (ValidateCityLength(city) != 0) {
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

	// Create UDP socket
	int my_socket = CreateUDPSocket();
	if (my_socket < 0) {
		clearwinsock();
		return 1;
	}

	// Resolve server address and get hostname/IP
	if (ResolveServerAddress(server, port, &serverAddr, serverHostname, NI_MAXHOST) != 0) {
		closesocket(my_socket);
		clearwinsock();
		return 1;
	}
	
	// Get server IP address
	if (inet_ntop(AF_INET, &serverAddr.sin_addr, serverIP, INET_ADDRSTRLEN) == NULL) {
		strcpy(serverIP, "unknown");
	}

	// Prepare request
	memset(&req, 0, sizeof(req));
	req.type = type;
	strncpy(req.city, city, MAX_CITY_LENGTH - 1);
	req.city[MAX_CITY_LENGTH - 1] = '\0';

	// Serialize request
	int reqSize = SerializeRequest(&req, buffer, BUFFER_SIZE);
	if (reqSize < 0) {
		closesocket(my_socket);
		clearwinsock();
		return 1;
	}

	// Send request
	bytesSent = sendto(my_socket, buffer, reqSize, 0,
	                   (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if (bytesSent < 0) {
#if defined(_WIN32) || defined(WIN32)
		fprintf(stderr, "Error sending request: %d\n", WSAGetLastError());
#else
		perror("Error sending request");
#endif
		closesocket(my_socket);
		clearwinsock();
		return 1;
	}

	// Receive response
	socklen_t serverAddrLen = sizeof(serverAddr);
	memset(buffer, 0, BUFFER_SIZE);
	bytesReceived = recvfrom(my_socket, buffer, BUFFER_SIZE, 0,
	                         (struct sockaddr *)&serverAddr, &serverAddrLen);
	
	if (bytesReceived < 0) {
#if defined(_WIN32) || defined(WIN32)
		fprintf(stderr, "Error receiving response: %d\n", WSAGetLastError());
#else
		perror("Error receiving response");
#endif
		closesocket(my_socket);
		clearwinsock();
		return 1;
	}

	// Deserialize response
	memset(&resp, 0, sizeof(resp));
	if (DeserializeResponse(buffer, bytesReceived, &resp) != 0) {
		fprintf(stderr, "Error deserializing response\n");
		closesocket(my_socket);
		clearwinsock();
		return 1;
	}

	// Print response
	PrintResponse(&resp, serverHostname, serverIP, city);

	// Close socket
	closesocket(my_socket);

	printf("Client terminated.\n");

	clearwinsock();
	return 0;
} // main end
