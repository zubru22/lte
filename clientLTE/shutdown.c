#ifndef SHUTDOWN_H
#include "shutdown.h"
#endif

static char client_log_filename[] = "../logs/client.log";

bool check_for_shutdown(int socketfd, s_message* message)
{
    if (message->message_type == enb_off)
    {
        add_logf(client_log_filename, LOG_INFO, "eNodeB is OFF, shutting down.");
        
        if (close(socketfd) == 0)
            add_logf(client_log_filename, LOG_INFO, "Socket closed correctly.");
        else
            add_logf(client_log_filename, LOG_WARNING, "Socket did not close correctly!");
        
        return true;
    }
    return false;
}