#ifndef MESSAGE_H
#define MESSAGE_H
#include <sys/types.h>
#include <time.h>

typedef struct prach_preamble {
    int16_t ra_rnti; // 16bit integer sent to server - first 2 bits are PreambleIndex
} preamble;
typedef struct prach_response {
    int8_t rapid; // Get 2 first bits from ra_rnti and send it back to client
    time_t unix_epoch_timestamp;
} response;

typedef enum type_of_message {RA_RNTI, C_RNTI} e_message_type;

typedef union message_value {
        preamble message_preamble;
        response message_response;
} u_message_value;

typedef struct message {
    e_message_type message_type;
    u_message_value message_value;
} s_message;
#endif
