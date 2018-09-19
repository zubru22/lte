#ifndef MESSAGE_H
#define MESSAGE_H
#include <sys/types.h>
#include <time.h>

#define BUFFER_SIZE 16

typedef struct prach_preamble {
    int16_t ra_rnti; // 16bit integer sent to server - first 2 bits are PreambleIndex
} preamble;

typedef struct prach_response {
    int8_t rapid; // Get 2 first bits from ra_rnti and send it back to client
    time_t unix_epoch_timestamp;
} response;

typedef enum type_of_message {
    random_access_request, 
    random_access_response, 
    rrc_request, 
    rrc_setup, 
    rrc_complete, 
    ue_battery_low, 
    ue_battery_high, 
    ping, 
    pong, 
    ue_off, 
    enb_off, 
    measurement_control_request, 
    measurement_report,
    data_start,
    data,
    data_end
} e_message_type;

typedef enum establishment_cause {EMERGENCY, HIGH_PRIORITY_ACCESS, MT_ACCESS, MO_SIGNALING, MO_DATA} e_ec;

typedef struct stmsi {
    u_int8_t mme_code;
    u_int32_t mtmsi;
} s_stmsi;

typedef struct rrc_req {
    int8_t c_rnti;
    s_stmsi ue_identity;
    e_ec establishment_cause;
} rrc;

typedef enum CyclicPrefixUL {
  Normal,
  Abnormal
} e_CyclicPrefix;

typedef enum Hopping {
  Off,
  On
} e_Hopping;

typedef struct rrc_config {
    int NULRB;
    int NSubframe;
    int NCellID;
    int RNTI;
    e_CyclicPrefix cyclic_prefix;
    e_Hopping hopping;
    int SegGroup;
    int CyclicShift;
    int ShortEnd;
} rrc_config;

typedef struct rrc_setup_complete {
    int16_t mcc;
    int16_t mnc;
} rrc_setup_complete;

typedef enum _s_event {a1, a2, a3, a4, def} s_event; // def means 'default event' - it means no event whatsoever
                                                         // and it can occur right after ue has been turned on
                                                         // or between the rest of events

typedef union message_value {
        preamble message_preamble;
        response message_response;
        rrc message_request;
        rrc_config rrc_response;
        rrc_setup_complete message_complete;
        size_t size_of_file;
        unsigned char buffer[BUFFER_SIZE];
        s_event events;
} u_message_value;

typedef struct message {
    e_message_type message_type;
    u_message_value message_value;
} s_message;
#endif
