/*
 * main.c
 *
 * UDP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a UDP client
 * portable across Windows, Linux, and macOS.
 */

#if defined WIN32
#include <winsock2.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

#define NO_ERROR 0

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

/* ======================== FUNZIONI DI SUPPORTO ======================== */

static void usage(const char *progname) {
    printf("Uso: %s [-s server] [-p port] -r \"type city\"\n", progname);
}

/* Parsing -r "type city" */
int parse_request_string(const char *arg, struct request *req) {
    if (arg == NULL || req == NULL) {
        return -1;
    }

    /* Copia in buffer modificabile */
    char buffer[256];
    size_t len = strlen(arg);
    if (len >= sizeof(buffer)) {
        return -1;
    }
    strcpy(buffer, arg);

    /* vietiamo caratteri di tabulazione */
    for (size_t i = 0; i < len; ++i) {
        if (buffer[i] == '\t') {
            return -1;
        }
    }

    /* primo token = type, il resto = city */
    char *space = strchr(buffer, ' ');
    if (space == NULL) {
        /* manca città */
        return -1;
    }

    *space = '\0';
    char *type_str = buffer;
    char *city_str = space + 1;

    /* ignora spazi iniziali nella città */
    while (*city_str == ' ') {
        city_str++;
    }

    if (type_str[0] == '\0' || type_str[1] != '\0') {
        /* primo token non è singolo carattere */
        return -1;
    }

    if (*city_str == '\0') {
        /* città vuota */
        return -1;
    }

    /* verifica lunghezza città (max 63 char + '\0') */
    size_t city_len = strlen(city_str);
    if (city_len >= MAX_CITY_LEN) {
        return -1;
    }

    req->type = type_str[0];
    strncpy(req->city, city_str, MAX_CITY_LEN);
    req->city[MAX_CITY_LEN - 1] = '\0';

    return 0;
}

/* Serializza request: type (1 byte) + city (64 byte) */
int serialize_request(const struct request *req, char *buffer, int bufsize) {
    if (req == NULL || buffer == NULL) {
        return -1;
    }
    if (bufsize < REQ_WIRE_SIZE) {
        return -1;
    }

    int offset = 0;

    /* type */
    buffer[offset] = req->type;
    offset += 1;

    /* city: copiamo sempre MAX_CITY_LEN byte, includendo '\0' e padding */
    memset(buffer + offset, 0, MAX_CITY_LEN);
    size_t city_len = strlen(req->city);
    if (city_len > MAX_CITY_LEN - 1) {
        city_len = MAX_CITY_LEN - 1;
    }
    memcpy(buffer + offset, req->city, city_len);
    offset += MAX_CITY_LEN;

    return offset; /* deve essere REQ_WIRE_SIZE */
}

/* Deserializza response: status(uint32_t) + type(char) + value(float) */
int deserialize_response(const char *buffer, int len, struct response *resp) {
    if (buffer == NULL || resp == NULL) {
        return -1;
    }
    if (len < (int)RESP_WIRE_SIZE) {
        return -1;
    }

    int offset = 0;

    /* status */
    uint32_t net_status;
    memcpy(&net_status, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    resp->status = ntohl(net_status);

    /* type */
    resp->type = buffer[offset];
    offset += sizeof(char);

    /* value (float tramite uint32_t) */
    uint32_t temp;
    memcpy(&temp, buffer + offset, sizeof(uint32_t));
    temp = ntohl(temp);
    float f;
    memcpy(&f, &temp, sizeof(float));
    resp->value = f;

    return 0;
}

/* Stampa messaggio di risposta (puoi modificare i testi come vuoi) */
void print_response_message(const char *server_name,
                            const char *server_ip,
                            const struct response *resp,
                            const char *city) {

    if (!server_name) server_name = "(unknown)";
    if (!server_ip) server_ip = "(unknown)";
    if (!resp) return;

    if (resp->status == 0) {
        /* successo */
        switch (resp->type) {
            case 't':
                printf("OK dal server %s (ip %s). %s: Temperatura = %.1f\n",
                       server_name, server_ip, city, resp->value);
                break;
            case 'h':
                printf("OK dal server %s (ip %s). %s: Umidita' = %.1f\n",
                       server_name, server_ip, city, resp->value);
                break;
            case 'w':
                printf("OK dal server %s (ip %s). %s: Vento = %.1f\n",
                       server_name, server_ip, city, resp->value);
                break;
            case 'p':
                printf("OK dal server %s (ip %s). %s: Pressione = %.1f\n",
                       server_name, server_ip, city, resp->value);
                break;
            default:
                printf("OK dal server %s (ip %s). %s: Tipo sconosciuto\n",
                       server_name, server_ip, city);
                break;
        }
    } else if (resp->status == 1) {
        printf("Risposta dal server %s (ip %s). Citta' non disponibile\n",
               server_name, server_ip);
    } else if (resp->status == 2) {
        printf("Risposta dal server %s (ip %s). Richiesta non valida\n",
               server_name, server_ip);
    } else {
        printf("Risposta dal server %s (ip %s). Errore sconosciuto (%u)\n",
               server_name, server_ip, resp->status);
    }
}



int main(int argc, char *argv[]) {

	// TODO: Implement client logic

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	int my_socket;

	// TODO: Create UDP socket

	// TODO: Configure server address

	// TODO: Implement UDP communication logic

	// TODO: Close socket
	// closesocket(my_socket);

	printf("Client terminated.\n");

	clearwinsock();
	return 0;
} // main end
