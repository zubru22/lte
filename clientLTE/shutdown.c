#ifndef SHUTDOWN_H
#include "shutdown.h"
#endif

extern FILE* log_file;

bool check_for_shutdown(int socketfd, s_message* message)
{
    if (message->message_type == enb_off)
    {
        add_logf(log_file, LOG_INFO, "eNodeB is OFF, shutting down.");
        
        if (close(socketfd) == 0)
            add_logf(log_file, LOG_INFO, "Socket closed correctly.");
        else
            add_logf(log_file, LOG_WARNING, "Socket did not close correctly!");
        
        return true;
    }
    return false;
}