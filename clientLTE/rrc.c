/* This functionality is about sending RRC Connection Request to eNodeB, 
* wait for RCC Connection Setup from RRC and finally send back
* the RRC Connection Complete message */
#include "rrc.h"

// This function generates ue_identity random key
void generate_ue_identity(s_stmsi* ue_identity)
{
    ue_identity->mme_code = rand() % (int)pow(2, 8);
    ue_identity->mtmsi = rand() % (int)pow(2, 32);
}

