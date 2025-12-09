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

#define DEFAULT_SERVER_NAME   "localhost"
#define DEFAULT_SERVER_PORT   56700

#define MAX_CITY_LEN          64
#define REQ_WIRE_SIZE         (1 + MAX_CITY_LEN) /* type + city */
#define RESP_WIRE_SIZE        (sizeof(uint32_t) + sizeof(char) + sizeof(float))

/*
 * ============================================================================
 * PROTOCOL DATA STRUCTURES
 * ============================================================================
 */

/* Richiesta client: invariata rispetto al primo esonero */
struct request {
    char type;                       /* 't','h','w','p' */
    char city[MAX_CITY_LEN];         /* nome città, null-terminated */
};

/* Risposta server: invariata rispetto al primo esonero */
struct response {
    unsigned int status;             /* 0=ok,1=city not found,2=invalid req */
    char type;                       /* eco del tipo richiesto */
    float value;                     /* dato meteo */
};

/*
* ============================================================================
* FUNCTION PROTOTYPES
* ============================================================================
*/

/* Parsing stringa di -r "type city" in struct request */
int parse_request_string(const char *arg, struct request *req);

/* Serializza struct request in buffer per sendto() */
int serialize_request(const struct request *req, char *buffer, int bufsize);

/* Deserializza buffer ricevuto in struct response */
int deserialize_response(const char *buffer, int len, struct response *resp);

/* Stampa risultato (puoi modificare i printf dentro a tuo piacere) */
void print_response_message(const char *server_name,
                            const char *server_ip,
                            const struct response *resp,
                            const char *city);


#endif /* PROTOCOL_H_ */
