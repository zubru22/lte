#ifndef MESSAGE_H
#define MESSAGE_H
#include <sys/types.h>

typedef struct prach_preamble {
    int16_t ra_rnti;
} preamble;
typedef struct prach_response {

} response;

typedef enum type_of_message {RA_RNTI, C_RNTI} e_message_type;

struct message {
    e_message_type message_type;
    union message_value {
        preamble message_preamble;
        response message_response;
    };
};
#endif