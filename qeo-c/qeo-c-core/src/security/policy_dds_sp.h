/*
 * Copyright (c) 2014 - Qeo LLC
 *
 * The source code form of this Qeo Open Source Project component is subject
 * to the terms of the Clear BSD license.
 *
 * You can redistribute it and/or modify it under the terms of the Clear BSD
 * License (http://directory.fsf.org/wiki/License:ClearBSD). See LICENSE file
 * for more details.
 *
 * The Qeo Open Source Project also includes third party Open Source Software.
 * See LICENSE file for more details.
 */

                 
#ifndef POLICY_DDS_SP_H_
#define POLICY_DDS_SP_H_
/*#######################################################################
#                       HEADER (INCLUDE) SECTION                        #
########################################################################*/                                                                           
#include <qeo/error.h>
#include <stdbool.h>
 
/*#######################################################################
#                           TYPES SECTION                               #
########################################################################*/  
typedef struct {
    bool read;
    bool write;
    bool blacklist;
}policy_dds_sp_perms_t;

/*#######################################################################
#                   PUBLIC FUNCTION DECLARATION                         #
########################################################################*/
/* The functions below are directly mapped to DDS security plugin functions */

qeo_retcode_t policy_dds_sp_init(void); 
void policy_dds_sp_destroy(void); 
qeo_retcode_t policy_dds_sp_flush(void);
void policy_dds_sp_update_start(void);
void policy_dds_sp_update_done(void);
qeo_retcode_t policy_dds_sp_add_domain(unsigned int domain_id);
qeo_retcode_t policy_dds_sp_add_participant(uintptr_t *cookie, const char *participant_id);
qeo_retcode_t policy_dds_sp_add_partition(uintptr_t cookie, const char *name, policy_dds_sp_perms_t *perms);
 
#endif
