// This functionality is about to create Random Access Preamble (RA-RNTI) and wait for Random Access Response (C-RNTI) from eNB
#include "random_access.h"
#include <errno.h>

// This function generates and returns ra_rnti for prach preamble and modifies original structure
int generate_ra_rnti(void) {
    const char max_t_id = 10;
    const char max_f_id = 6;
    return (1 + (rand() % max_t_id) + 10 * (rand() % max_f_id));
}

// This function sends prach preamble to eNodeB. Function returns -1 on error and 0 on success
int send_prach_preamble(int sockfd, json_t* json_message, int (*ra_rnti_generator_func)(void)) {
    char *json_str_outgoing;

    // We need to set message type to random_access_request
    json_object_set(json_message, "message_type", json_integer(random_access_request));
    // We need to fill preamble value (ra_rnti)
    json_object_set(json_message, "ra_rnti", json_integer(ra_rnti_generator_func()));
    // Convert json object to string, 0 means no formating
    json_str_outgoing = json_dumps(json_message, 0);
    
    // Length of json string to be send
    size_t json_str_len = strlen(json_str_outgoing);
    
    // Send length of json string
    write(sockfd, &json_str_len, json_str_len);
    // Send json string and store amount of bytes written to the file descriptor
    size_t written = write(sockfd, json_str_outgoing, json_str_len);
    printf("String: %s\n", json_str_outgoing);
    free(json_str_outgoing);
    assert(json_str_len == written);

    return 0;
}

// This function receives response from eNodeB. Function returns -1 on error #1, 1 on error #2 and 0 on success
int receive_prach_response(int socketfd, json_t *json_message) {
    int number_of_bytes_read, message_type_received;
    size_t json_received_len;
    char *json_received;
    json_t *json_obj_received;

    // Read json length from socket
    read(socketfd, (size_t*) &json_received_len, sizeof(json_received_len));

    // Allocate memory for json string
    json_received = (char *) calloc(json_received_len, sizeof(char));
    // Read json string
    do {
        number_of_bytes_read = read(socketfd, json_received, json_received_len);
    } while (number_of_bytes_read != json_received_len);
    // Decode json string to json object
    json_obj_received = json_loads(json_received, 0, 0);
    printf("String: %s\n", json_received);
    for (int i = 0; i < json_received_len; i++)
        printf("%c\n", json_received[i]);
    free(json_received);

    // Check if whole string was read
    assert(number_of_bytes_read == json_received_len);
    // Exctract message type value from json
    json_unpack(json_object_get(json_obj_received, "message_type"), "i", &message_type_received);

    if(random_access_response == message_type_received) {
        int8_t value_received;
        int8_t value_expected;

        json_unpack(json_object_get(json_obj_received, "rapid"), "i", &value_received);
        json_unpack(json_object_get(json_message, "ra_rnti"), "i", &value_expected);
        value_expected = (value_expected & 0b1100000000000000) >> 8;
        json_object_set(json_message, "c_rnti", json_integer(value_expected & 0b11000000));
        if (value_received == value_expected)
            return 0;
    }
    return 1;
}