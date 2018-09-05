#ifndef MESSAGE_H
#define MESSAGE_H
#include <sys/types.h>

typedef struct prach_preamble {
    int16_t ra_rnti; // 16bit integer sent to server - first 2 bits are PreambleIndex
} preamble;
typedef struct prach_response {
    int8_t rapid; // Get 2 first bits from ra_rnti and send it back to client
} response;

typedef struct rrc_req {
    int ue_identity;
    e_ec establishment_cause;
} rrc;

typedef enum type_of_message {RA_RNTI, C_RNTI, RRC_REQ} e_message_type;
typedef enum establishment_cause {EMERGENCY, HIGH_PRIORITY_ACCESS, MT_ACCESS, MO-SIGNALING, MO-DATA} e_ec;

typedef union message_value {
        preamble message_preamble;
        response message_response;
        rrc message_request;
} u_message_value;

typedef struct message {
    e_message_type message_type;
    u_message_value message_value;
} s_message;
#endif