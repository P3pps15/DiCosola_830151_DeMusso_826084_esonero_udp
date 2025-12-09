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

// Server argument parsing
int ParseServerArguments(int argc, char *argv[], int *port);

// Socket creation
int CreateUDPSocket(void);

// Request validation
int ValidateRequestType(char type);
int ValidateCity(const char *city);
int IsCitySupported(const char *city);
int HasInvalidCharacters(const char *city);

// Weather data generation
float GetTemperature(void);
float GetHumidity(void);
float GetWind(void);
float GetPressure(void);

// Serialization/Deserialization
int SerializeRequest(const struct request *req, char *buffer, int bufferSize);
int DeserializeRequest(const char *buffer, int bufferSize, struct request *req);
int SerializeResponse(const struct response *resp, char *buffer, int bufferSize);
int DeserializeResponse(const char *buffer, int bufferSize, struct response *resp);

// DNS and network utilities
int GetHostnameFromAddress(const struct sockaddr_in *addr, char *hostname, int hostnameSize);

// City name formatting
void FormatCityName(char *city);


#endif /* PROTOCOL_H_ */
