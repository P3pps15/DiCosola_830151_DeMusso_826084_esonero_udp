/*
 * protocol.h
 *
 * Shared header file for UDP client and server
 * Contains protocol definitions, data structures, constants and function prototypes
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>

/*
 * ============================================================================
 * PROTOCOL CONSTANTS
 * ============================================================================
 */

#define SERVER_PORT 56700
#define BUFFER_SIZE 512
#define MAX_CITY_LENGTH 64

/*
 * ============================================================================
 * PROTOCOL DATA STRUCTURES
 * ============================================================================
 */

// Weather request and response structures
struct request {
    char type;      // 't'=temperatura, 'h'=umidità, 'w'=vento, 'p'=pressione
    char city[64];  // nome città (null-terminated)
};

struct response {
    unsigned int status;  // 0=successo, 1=città non trovata, 2=richiesta invalida
    char type;            // eco del tipo richiesto
    float value;          // dato meteo generato
};

/*
 * ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================
 */

// Client argument parsing
int ParseClientArguments(int argc, char *argv[], char **server, int *port, char **request);

// Request validation
int ValidateRequest(const char *requestStr, char *type, char *city);
int ValidateCityLength(const char *city);
int HasTabCharacters(const char *str);

// Socket creation
int CreateUDPSocket(void);

// DNS resolution
int ResolveServerAddress(const char *server, int port, struct sockaddr_in *serverAddr, char *hostname, int hostnameSize);
int GetHostnameFromAddress(const struct sockaddr_in *addr, char *hostname, int hostnameSize);

// Serialization/Deserialization
int SerializeRequest(const struct request *req, char *buffer, int bufferSize);
int DeserializeRequest(const char *buffer, int bufferSize, struct request *req);
int SerializeResponse(const char *buffer, int bufferSize, struct response *resp);
int DeserializeResponse(const char *buffer, int bufferSize, struct response *resp);

// Output formatting
void FormatCityName(char *city);
void PrintResponse(const struct response *resp, const char *hostname, const char *ip, const char *city);


#endif /* PROTOCOL_H_ */
