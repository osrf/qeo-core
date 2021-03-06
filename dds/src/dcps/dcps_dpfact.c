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

/* dcps_dpfact.c -- DomainParticipantFactory methods. */

#include <stdio.h>
#include "log.h"
#include "prof.h"
#include "ctrace.h"
#include "config.h"
#include "dds/dds_dcps.h"
#include "dds_data.h"
#include "dds.h"
#include "rtps.h"
#include "domain.h"
#include "disc.h"
#include "dcps_priv.h"
#include "dcps_builtin.h"
#include "dcps_dpfact.h"
#include "error.h"
#ifdef DDS_SECURITY
#include "security.h"
#endif

static unsigned nparticipants;
static int autoenable_created_entities = 1;

DDS_DomainParticipantQos dcps_def_participant_qos = {
/* TopicData */
	{ DDS_SEQ_INITIALIZER (unsigned char) },
/* EntityFactory */
	{ 1 }
};

DDS_DomainParticipant DDS_DomainParticipantFactory_create_participant (
			DDS_DomainId_t			    domain,
			const DDS_DomainParticipantQos	    *qos,
			const DDS_DomainParticipantListener *listener,
			DDS_StatusMask			    mask)
{
	Domain_t			*dp;
#ifdef DDS_SECURITY
	unsigned			secure;
	uint32_t			sec_caps;
	PermissionsHandle_t		permissions;
	unsigned char			buffer [128];
	size_t				length;
#endif

	if (!nparticipants) {
#ifdef CTRACE_USED
		log_fct_str [DCPS_ID] = dds_fct_str;
#endif
		dds_init ();
	}

	ctrc_begind (DCPS_ID, DCPS_DPF_C_PART, &domain, sizeof (domain));
	ctrc_contd (&qos, sizeof (qos));
	ctrc_contd (&listener, sizeof (listener));
	ctrc_contd (&mask, sizeof (mask));
	ctrc_endd ();

	prof_start (dcps_create_part);

	if (qos == DDS_PARTICIPANT_QOS_DEFAULT)
		qos = &dcps_def_participant_qos;
	else if (!qos_valid_participant_qos (qos))
		return (NULL);

#ifdef DDS_SECURITY

	/* Check if security policy allows us to join the domain! */
	permissions = validate_local_permissions (domain, local_identity);
	if (check_create_participant (permissions, qos, &secure)) {
		log_printf (DCPS_ID, 0, "DDS: create_participant(%u): secure domain - access denied!\r\n", domain);
		return (NULL);
	}
	else if (secure) {
		log_printf (DCPS_ID, 0, "DDS: create_participant(%u): secure domain - access granted!\r\n", domain);
		sec_caps = get_domain_security (domain);
	}
	else
		sec_caps = 0;
#endif
	dp = domain_create (domain);
	if (!dp)
		return (NULL);

	dp->autoenable = qos->entity_factory.autoenable_created_entities;

#ifdef DDS_SECURITY
	dp->security = secure;
	dp->participant.p_permissions = permissions;
	dp->participant.p_sec_caps = sec_caps;
	dp->participant.p_sec_locs = NULL;
	dp->identity = dp->ptoken = NULL;
	if (secure) {
		length = sizeof (buffer);
		if (get_identity_token (local_identity, buffer, &length))
			warn_printf ("DDS: can't derive identity token!");
		else {
			dp->identity = str_new ((char *) buffer, length, length, 0);
			if (!dp->identity)
				warn_printf ("DDS: out-of-memory for identity token!");
		}
		length = sizeof (buffer);
		if (get_permissions_token (permissions, buffer, &length))
			warn_printf ("DDS: can't derive permissions token!");
		else {
			dp->ptoken = str_new ((char *) buffer, length, length, 0);
			if (!dp->ptoken)
				warn_printf ("DDS: out-of-memory for permissions token!");
		}
	}
#endif
#ifdef DDS_FORWARD
	dp->participant.p_forward = config_get_number (DC_Forward, 0);;
#endif
	dp->participant.p_user_data = qos_octets2str (&qos->user_data.value);
	if (dds_entity_name)
		dp->participant.p_entity_name = str_new_cstr (dds_entity_name);
	else
		dp->participant.p_entity_name = NULL;
	if (listener)
		dp->listener = *listener;
	else
		memset (&dp->listener, 0, sizeof (dp->listener));
	dp->mask = mask;
	dp->def_topic_qos = qos_def_topic_qos;
	dp->def_publisher_qos = qos_def_publisher_qos;
	dp->def_subscriber_qos = qos_def_subscriber_qos;
	
	nparticipants++;

	lock_release (dp->lock);

	if (autoenable_created_entities)
		DDS_DomainParticipant_enable (dp);

	prof_stop (dcps_create_part, 1);
	return (dp);
}

DDS_ReturnCode_t DDS_DomainParticipantFactory_delete_participant (DDS_DomainParticipant dp)
{
	Condition_t			*cp;
	DDS_ReturnCode_t		ret;

	ctrc_printd (DCPS_ID, DCPS_DPF_D_PART, &dp, sizeof (dp));
	prof_start (dcps_delete_part);
	if (!domain_ptr (dp, 0, &ret))
		return (ret);

	domain_close (dp->index);
        lock_take (dp->lock);

	if (dp->publishers.head || dp->subscribers.head) {
		lock_release (dp->lock);
		return (DDS_RETCODE_PRECONDITION_NOT_MET);
	}

#ifdef DCPS_BUILTIN_READERS
	dcps_delete_builtin_readers (dp);
#endif
#ifdef RTPS_USED

	if ((dp->participant.p_flags & EF_ENABLED) != 0 && rtps_used)

		/* Delete RTPS-specific participant data (automatically deletes
		   Discovery participant data as well). */
		rtps_participant_delete (dp);
#endif

	/* Delete Status Condition if it was created. */
	if (dp->condition) {
		cp = (Condition_t *) dp->condition;
		if (cp->deferred)
			dds_defer_waitset_undo (dp, dp->condition);
		dcps_delete_status_condition ((StatusCondition_t *) dp->condition);
		dp->condition = NULL;
	}

#ifdef DDS_SECURITY
	if (dp->identity)
		str_unref (dp->identity);
	if (dp->ptoken)
		str_unref (dp->ptoken);
#endif

	/* Delete participant QoS data. */
	if (dp->participant.p_user_data)
		str_unref (dp->participant.p_user_data);

	/* Delete entity name. */
	if (dp->participant.p_entity_name)
		str_unref (dp->participant.p_entity_name);

	/* Remove domain from list of valid domains. */
	domain_detach (dp);

	/* Remove relays from domain. */
	if (dp->nr_relays)
		xfree (dp->relays);

	/* Finally delete the generic participant. */
	domain_delete (dp);

	prof_stop (dcps_delete_part, 1);

	if (!--nparticipants) {
		dds_final ();
#ifdef PROFILE
		prof_list();
#endif
	}

	return (DDS_RETCODE_OK);
}

DDS_DomainParticipant DDS_DomainParticipantFactory_lookup_participant (DDS_DomainId_t domain)
{
	DDS_DomainParticipant	p;

	ctrc_printd (DCPS_ID, DCPS_DPF_L_PART, &domain, sizeof (domain));
	dds_lock_domains ();
	p = domain_lookup (domain);
	dds_unlock_domains ();
	return (p);
}

DDS_ReturnCode_t DDS_DomainParticipantFactory_get_default_participant_qos (DDS_DomainParticipantQos *qos)
{
	ctrc_printd (DCPS_ID, DCPS_DPF_G_DP_QOS, &qos, sizeof (qos));
	if (qos)
		*qos = dcps_def_participant_qos;

	return (qos ? DDS_RETCODE_OK : DDS_RETCODE_BAD_PARAMETER);
}

DDS_ReturnCode_t DDS_DomainParticipantFactory_set_default_participant_qos (DDS_DomainParticipantQos *qos)
{
	ctrc_printd (DCPS_ID, DCPS_DPF_S_DP_QOS, &qos, sizeof (qos));
	if (qos == DDS_PARTICIPANT_QOS_DEFAULT)
		dcps_def_participant_qos = qos_def_participant_qos;
	else if (qos_valid_participant_qos (qos))
		dcps_def_participant_qos = *qos;
	else
		return (DDS_RETCODE_INCONSISTENT_POLICY);

	return (DDS_RETCODE_OK);
}

DDS_ReturnCode_t DDS_DomainParticipantFactory_get_qos (DDS_DomainParticipantFactoryQos *qos)
{
	ctrc_printd (DCPS_ID, DCPS_DPF_G_QOS, &qos, sizeof (qos));
	if (!qos)
		return (DDS_RETCODE_BAD_PARAMETER);

	qos->entity_factory.autoenable_created_entities = autoenable_created_entities;
	return (DDS_RETCODE_OK);
}

DDS_ReturnCode_t DDS_DomainParticipantFactory_set_qos (DDS_DomainParticipantFactoryQos *qos)
{
	ctrc_printd (DCPS_ID, DCPS_DPF_S_QOS, &qos, sizeof (qos));
	if (!qos)
		return (DDS_RETCODE_BAD_PARAMETER);

	autoenable_created_entities = qos->entity_factory.autoenable_created_entities;
	return (DDS_RETCODE_OK);
}


