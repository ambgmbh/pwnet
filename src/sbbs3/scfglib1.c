/* Synchronet configuration library routines */

/****************************************************************************
 * @format.tab-size 4		(Plain Text/Source Code File Header)			*
 * @format.use-tabs true	(see http://www.synchro.net/ptsc_hdr.html)		*
 *																			*
 * Copyright Rob Swindell - http://www.synchro.net/copyright.html			*
 *																			*
 * This program is free software; you can redistribute it and/or			*
 * modify it under the terms of the GNU General Public License				*
 * as published by the Free Software Foundation; either version 2			*
 * of the License, or (at your option) any later version.					*
 * See the GNU General Public License for more details: gpl.txt or			*
 * http://www.fsf.org/copyleft/gpl.html										*
 *																			*
 * For Synchronet coding style and modification guidelines, see				*
 * http://www.synchro.net/source.html										*
 *																			*
 * Note: If this box doesn't appear square, then you need to fix your tabs.	*
 ****************************************************************************/

#include "scfglib.h"
#include "load_cfg.h"
#include "nopen.h"
#include "ars_defs.h"
#include "findstr.h"
#include "ini_file.h"

BOOL allocerr(char* error, size_t maxerrlen, const char* fname, const char *item, size_t size)
{
	safe_snprintf(error, maxerrlen, "%s: allocating %u bytes of memory for %s"
		,fname, (uint)size, item);
	return(FALSE);
}

/****************************************************************************/
/* Reads in node .ini and initializes the associated variables				*/
/****************************************************************************/
BOOL read_node_cfg(scfg_t* cfg, char* error, size_t maxerrlen)
{
	char	path[MAX_PATH+1];
	char	errstr[256];
	FILE*	fp;
	str_list_t	ini;
	char	value[INI_MAX_VALUE_LEN];
	const char* section = ROOT_SECTION;

	const char* fname = "node.ini";
	SAFEPRINTF2(path,"%s%s",cfg->node_dir,fname);
	if((fp = fnopen(NULL, path, O_RDONLY)) == NULL) {
		safe_snprintf(error, maxerrlen, "%d (%s) opening %s",errno,safe_strerror(errno, errstr, sizeof(errstr)),path);
		return FALSE;
	}
	ini = iniReadFile(fp);
	fclose(fp);

	SAFECOPY(cfg->node_phone, iniGetString(ini, section, "phone", "", value));
	SAFECOPY(cfg->node_daily, iniGetString(ini, section, "daily", "", value));
	SAFECOPY(cfg->text_dir, iniGetString(ini, section, "text_dir", "../text/", value));
	SAFECOPY(cfg->temp_dir, iniGetString(ini, section, "temp_dir", "temp", value));
	SAFECOPY(cfg->node_arstr, iniGetString(ini, section, "ars", "", value));
	arstr(NULL, cfg->node_arstr, cfg, cfg->node_ar);

	cfg->node_misc = iniGetLongInt(ini, section, "settings", 0);
	cfg->node_valuser = iniGetShortInt(ini, section, "valuser", 0);
	cfg->node_sem_check = iniGetShortInt(ini, section, "sem_check", 60);
	cfg->node_stat_check = iniGetShortInt(ini, section, "stat_check", 10);
	cfg->sec_warn = iniGetShortInt(ini, section, "sec_warn", 180);
	cfg->sec_hangup = iniGetShortInt(ini, section, "sec_hangup", 300);
	cfg->node_erruser = iniGetShortInt(ini, section, "erruser", 300);
	cfg->node_errlevel = (uchar)iniGetShortInt(ini, section, "errlevel", 0);

	iniFreeStringList(ini);

	return TRUE;
}

/****************************************************************************/
/* Reads in main.ini and initializes the associated variables				*/
/****************************************************************************/
BOOL read_main_cfg(scfg_t* cfg, char* error, size_t maxerrlen)
{
	BOOL	result = FALSE;
	char	path[MAX_PATH+1];
	char	errstr[256];
	FILE*	fp;
	str_list_t	ini = NULL;
	char	value[INI_MAX_VALUE_LEN];
	const char* section = ROOT_SECTION;

	const char* fname = "main.ini";
	SAFEPRINTF2(path,"%s%s",cfg->ctrl_dir,fname);
	if((fp = fnopen(NULL, path, O_RDONLY)) == NULL) {
		safe_snprintf(error, maxerrlen, "%d (%s) opening %s",errno,safe_strerror(errno, errstr, sizeof(errstr)),path);
	} else {
		ini = iniReadFile(fp);
		fclose(fp);
		result = TRUE;
	}

	SAFECOPY(cfg->sys_name, iniGetString(ini, section, "name", "", value));
	SAFECOPY(cfg->sys_op, iniGetString(ini, section, "operator", "", value));
	SAFECOPY(cfg->sys_pass, iniGetString(ini, section, "password", "", value));
	SAFECOPY(cfg->sys_id, iniGetString(ini, section, "qwk_id", "", value));
	SAFECOPY(cfg->sys_guru, iniGetString(ini, section, "guru", "", value));
	SAFECOPY(cfg->sys_location, iniGetString(ini, section, "location", "", value));
	SAFECOPY(cfg->sys_phonefmt, iniGetString(ini, section, "phonefmt", "", value));
	SAFECOPY(cfg->sys_chat_arstr, iniGetString(ini, section, "chat_ars", "", value));
	arstr(NULL, cfg->sys_chat_arstr, cfg, cfg->sys_chat_ar);

	cfg->sys_timezone = iniGetShortInt(ini, section, "timezone", 0);
	cfg->sys_misc = iniGetLongInt(ini, section, "settings", 0);
	cfg->sys_pwdays = iniGetInteger(ini, section, "pwdays", 0);
	cfg->sys_deldays = iniGetInteger(ini, section, "deldays", 0);
	cfg->sys_exp_warn = iniGetInteger(ini, section, "exp_warn", 0);
	cfg->sys_autodel = iniGetInteger(ini, section, "autodel", 0);
	cfg->cdt_min_value =  iniGetBytes(ini, section, "cdt_min_value", 1, 0);
	cfg->max_minutes = iniGetInteger(ini, section, "max_minutes", 0);
	cfg->cdt_per_dollar = iniGetBytes(ini, section, "cdt_per_dollar", 1, 0);
	cfg->guest_msgscan_init = iniGetInteger(ini, section, "guest_msgscan_init", 0);
	cfg->min_pwlen = iniGetInteger(ini, section, "min_pwlen", 0);
	if(cfg->min_pwlen < MIN_PASS_LEN)
		cfg->min_pwlen = MIN_PASS_LEN;
	if(cfg->min_pwlen > LEN_PASS)
		cfg->min_pwlen = LEN_PASS;
	cfg->max_log_size = iniGetLongInt(ini, section, "max_log_size", 0);
	cfg->max_logs_kept = iniGetInteger(ini, section, "max_logs_kept", 0);
	cfg->ctrlkey_passthru = iniGetInteger(ini, section, "ctrlkey_passthru", 0);

	cfg->user_backup_level = iniGetInteger(ini, section, "user_backup_level", 5);
	cfg->mail_backup_level = iniGetInteger(ini, section, "mail_backup_level", 5);
	cfg->new_install = iniGetBool(ini, section, "new_install", FALSE);

	// fixed events
	SAFECOPY(cfg->sys_logon, iniGetString(ini, section, "logon", "", value));
	SAFECOPY(cfg->sys_logout, iniGetString(ini, section, "logout", "", value));
	SAFECOPY(cfg->sys_daily, iniGetString(ini, section, "daily", "", value));

	str_list_t node_dirs = iniGetKeyList(ini, "node_dir");
	cfg->sys_nodes = strListCount(node_dirs);
	for(size_t i=0; i<cfg->sys_nodes; i++) {
		SAFECOPY(cfg->node_path[i], node_dirs[i]);
#if defined(__unix__)
		strlwr(cfg->node_path[i]);
#endif
	}
	strListFree(&node_dirs);

	cfg->sys_lastnode = iniGetInteger(ini, section, "lastnode", cfg->sys_nodes);

	section = "dir";
	SAFECOPY(cfg->data_dir, iniGetString(ini, section, "data", "../data/", value));
	SAFECOPY(cfg->exec_dir, iniGetString(ini, section, "exec", "../exec/", value));
	SAFECOPY(cfg->mods_dir, iniGetString(ini, section, "mods", "../mods/", value));
	SAFECOPY(cfg->logs_dir, iniGetString(ini, section, "logs", cfg->data_dir, value));

	/*********************/
	/* New User Settings */
	/*********************/
	section = "newuser";
	cfg->uq = iniGetLongInt(ini, section, "questions", 0);

	SAFECOPY(cfg->new_genders, iniGetString(ini, section, "gender_options", "MFX", value));
	SAFECOPY(cfg->new_pass, iniGetString(ini, section, "password", "", value));
	SAFECOPY(cfg->new_magic, iniGetString(ini, section, "magic_word", "", value));
	SAFECOPY(cfg->new_sif, iniGetString(ini, section, "sif", "", value));
	SAFECOPY(cfg->new_sof, iniGetString(ini, section, "sof", cfg->new_sif, value));
	cfg->new_prot = *iniGetString(ini, section, "download_protocol", " ", value);
	char new_shell[LEN_CODE + 1];
	SAFECOPY(new_shell, iniGetString(ini, section, "shell", "default", value));
	SAFECOPY(cfg->new_xedit, iniGetString(ini, section, "editor", "", value));

	cfg->new_level = iniGetInteger(ini, section, "level", 0);
	cfg->new_flags1 = iniGetLongInt(ini, section, "flags1", 0);
	cfg->new_flags2 = iniGetLongInt(ini, section, "flags2", 0);
	cfg->new_flags3 = iniGetLongInt(ini, section, "flags3", 0);
	cfg->new_flags4 = iniGetLongInt(ini, section, "flags4", 0);
	cfg->new_exempt = iniGetLongInt(ini, section, "exemptions", 0);
	cfg->new_rest = iniGetLongInt(ini, section, "restrictions", 0);
	cfg->new_cdt = iniGetBytes(ini, section, "credits", 1, 0);
	cfg->new_min = iniGetLongInt(ini, section, "minutes", 0);
	cfg->new_expire = iniGetInteger(ini, section, "expire", 0);
	cfg->new_misc = iniGetLongInt(ini, section, "settings", 0);
	cfg->new_msgscan_init = iniGetInteger(ini, section, "msgscan_init", 0);

	/*************************/
	/* Expired User Settings */
	/*************************/
	section = "expired";
	cfg->expired_level = iniGetInteger(ini, section, "level", 0);
	cfg->expired_flags1 = iniGetLongInt(ini, section, "flags1", 0);
	cfg->expired_flags2 = iniGetLongInt(ini, section, "flags2", 0);
	cfg->expired_flags3 = iniGetLongInt(ini, section, "flags3", 0);
	cfg->expired_flags4 = iniGetLongInt(ini, section, "flags4", 0);
	cfg->expired_exempt = iniGetLongInt(ini, section, "exemptions", 0);
	cfg->expired_rest = iniGetLongInt(ini, section, "restrictions", 0);

	/***********/
	/* Modules */
	/***********/
	section = "module";
	SAFECOPY(cfg->logon_mod, iniGetString(ini, section, "logon", "logon", value));
	SAFECOPY(cfg->logoff_mod, iniGetString(ini, section, "logoff", "", value));
	SAFECOPY(cfg->newuser_mod, iniGetString(ini, section, "newuser", "newuser", value));
	SAFECOPY(cfg->login_mod, iniGetString(ini, section, "login", "login", value));
	SAFECOPY(cfg->logout_mod, iniGetString(ini, section, "logout", "", value));
	SAFECOPY(cfg->sync_mod, iniGetString(ini, section, "sync", "", value));
	SAFECOPY(cfg->expire_mod, iniGetString(ini, section, "expire", "", value));
	SAFECOPY(cfg->readmail_mod, iniGetString(ini, section, "readmail", "", value));
	SAFECOPY(cfg->scanposts_mod, iniGetString(ini, section, "scanposts", "", value));
	SAFECOPY(cfg->scansubs_mod, iniGetString(ini, section, "scansubs", "", value));
	SAFECOPY(cfg->listmsgs_mod, iniGetString(ini, section, "listmsgs", "", value));
	SAFECOPY(cfg->textsec_mod, iniGetString(ini, section, "text_sec", "text_sec", value));
	SAFECOPY(cfg->automsg_mod, iniGetString(ini, section, "automsg", "automsg", value));
	SAFECOPY(cfg->xtrnsec_mod, iniGetString(ini, section, "xtrnsec", "xtrn_sec", value));
	SAFECOPY(cfg->nodelist_mod, iniGetString(ini, section, "nodelist", "nodelist", value));
	SAFECOPY(cfg->whosonline_mod, iniGetString(ini, section, "whosonline", "nodelist -active", value));
	SAFECOPY(cfg->privatemsg_mod, iniGetString(ini, section, "privatemsg", "privatemsg", value));
	SAFECOPY(cfg->logonlist_mod, iniGetString(ini, section, "logonlist", "logonlist", value));
	SAFECOPY(cfg->prextrn_mod, iniGetString(ini, section, "prextrn", "prextrn", value));
	SAFECOPY(cfg->postxtrn_mod, iniGetString(ini, section, "postxtrn", "postxtrn", value));
	SAFECOPY(cfg->tempxfer_mod, iniGetString(ini, section, "tempxfer", "tempxfer", value));

	/*******************/
	/* Validation Sets */
	/*******************/

	for(uint i=0; i<10; i++) {
		char str[128];
		SAFEPRINTF(str, "valset:%u", i);
		section = str;
		cfg->val_level[i] = iniGetInteger(ini, section, "level", 0);
		cfg->val_expire[i] = iniGetInteger(ini, section, "expire", 0);
		cfg->val_flags1[i] = iniGetLongInt(ini, section, "flags1", 0);
		cfg->val_flags2[i] = iniGetLongInt(ini, section, "flags2", 0);
		cfg->val_flags3[i] = iniGetLongInt(ini, section, "flags3", 0);
		cfg->val_flags4[i] = iniGetLongInt(ini, section, "flags4", 0);
		cfg->val_cdt[i] = iniGetBytes(ini, section, "credits", 1, 0);
		cfg->val_exempt[i] = iniGetLongInt(ini, section, "exemptions", 0);
		cfg->val_rest[i] = iniGetLongInt(ini, section, "restrictions", 0);
	}

	/***************************/
	/* Security Level Settings */
	/***************************/

	for(uint i=0; i<100; i++) {
		char str[128];
		SAFEPRINTF(str, "level:%u", i);
		section = str;
		cfg->level_timeperday[i] = iniGetInteger(ini, section, "timeperday", 0);
		cfg->level_timepercall[i] = iniGetInteger(ini, section, "timepercall", 0);
		cfg->level_callsperday[i] = iniGetInteger(ini, section, "callsperday", 0);
		cfg->level_linespermsg[i] = iniGetInteger(ini, section, "linespermsg", 0);
		cfg->level_postsperday[i] = iniGetInteger(ini, section, "postsperday", 0);
		cfg->level_emailperday[i] = iniGetInteger(ini, section, "emailperday", 0);
		cfg->level_misc[i] = iniGetLongInt(ini, section, "settings", 0);
		cfg->level_expireto[i] = iniGetInteger(ini, section, "expireto", 0);
		cfg->level_freecdtperday[i] = iniGetBytes(ini, section, "freecdtperday", 1, 0);
	}

	str_list_t shell_list = iniGetSectionList(ini, "shell:");
	cfg->total_shells = strListCount(shell_list);
	#ifdef SBBS
	if(!cfg->total_shells) {
		safe_snprintf(error, maxerrlen,"At least one command shell must be configured.");
		return(FALSE);
	}
	#endif

	if(cfg->total_shells) {
		if((cfg->shell=(shell_t **)malloc(sizeof(shell_t *)*cfg->total_shells))==NULL)
			return allocerr(error, maxerrlen, fname, "shells", sizeof(shell_t *)*cfg->total_shells);
	} else
		cfg->shell=NULL;

	cfg->new_shell = 0;
	for(uint i=0; i<cfg->total_shells; i++) {
		if((cfg->shell[i]=(shell_t *)malloc(sizeof(shell_t)))==NULL)
			return allocerr(error, maxerrlen, fname, "shell", sizeof(shell_t));
		memset(cfg->shell[i],0,sizeof(shell_t));

		section = shell_list[i];
		SAFECOPY(cfg->shell[i]->code, section + 5);
		SAFECOPY(cfg->shell[i]->name, iniGetString(ini, section, "name", section + 5, value));
		SAFECOPY(cfg->shell[i]->arstr, iniGetString(ini, section, "ars", "", value));
		arstr(NULL,cfg->shell[i]->arstr,cfg,cfg->shell[i]->ar);
		cfg->shell[i]->misc = iniGetLongInt(ini, section, "settings", 0);
		if(stricmp(cfg->shell[i]->code, new_shell) == 0)
			cfg->new_shell = i;
	}

	iniFreeStringList(shell_list);
	iniFreeStringList(ini);

	return result;
}

/****************************************************************************/
/* Reads in msgs.ini and initializes the associated variables				*/
/****************************************************************************/
BOOL read_msgs_cfg(scfg_t* cfg, char* error, size_t maxerrlen)
{
	char	path[MAX_PATH+1];
	char	errstr[256];
	FILE*	fp;
	str_list_t	ini;
	char	value[INI_MAX_VALUE_LEN];
	const char* section = ROOT_SECTION;

	const char* fname = "msgs.ini";
	SAFEPRINTF2(path,"%s%s",cfg->ctrl_dir,fname);
	if((fp = fnopen(NULL, path, O_RDONLY)) == NULL) {
		safe_snprintf(error, maxerrlen, "%d (%s) opening %s",errno,safe_strerror(errno, errstr, sizeof(errstr)),path);
		return FALSE;
	}
	ini = iniReadFile(fp);
	fclose(fp);

	/*************************/
	/* General Message Stuff */
	/*************************/
	cfg->smb_retry_time = iniGetInteger(ini, section, "smb_retry_time", 30);

	/* QWK stuff */
	section = "QWK";
	cfg->msg_misc = iniGetLongInt(ini, section, "settings", 0xffff0000);
	cfg->max_qwkmsgs = iniGetInteger(ini, section, "max_msgs", 0);
	cfg->max_qwkmsgage = iniGetInteger(ini, section, "max_age", 0);
	SAFECOPY(cfg->qnet_tagline, iniGetString(ini, section, "default_tagline", "", value));
	SAFECOPY(cfg->preqwk_arstr, iniGetString(ini, section, "prepack_ars", "", value));
	arstr(NULL, cfg->preqwk_arstr, cfg, cfg->preqwk_ar);

	/* E-Mail stuff */
	section = "mail";
	cfg->mail_maxcrcs = iniGetInteger(ini, section, "max_crcs", 0);
	cfg->mail_maxage = iniGetInteger(ini, section, "max_age", 0);
	cfg->max_spamage = iniGetInteger(ini, section, "max_spam_age", 0);

	/******************/
	/* Message Groups */
	/******************/

	str_list_t grp_list = iniGetSectionList(ini, "grp:");
	cfg->total_grps = strListCount(grp_list);

	if(cfg->total_grps) {
		if((cfg->grp=(grp_t **)malloc(sizeof(grp_t *)*cfg->total_grps))==NULL)
			return allocerr(error, maxerrlen, fname, "groups", sizeof(grp_t *)*cfg->total_grps);
	} else
		cfg->grp=NULL;

	for(uint i=0; i<cfg->total_grps; i++) {

		section = grp_list[i];
		if((cfg->grp[i]=(grp_t *)malloc(sizeof(grp_t)))==NULL)
			return allocerr(error, maxerrlen, fname, "group", sizeof(grp_t));
		memset(cfg->grp[i],0,sizeof(grp_t));
		SAFECOPY(cfg->grp[i]->sname, section + 4);
		SAFECOPY(cfg->grp[i]->lname, iniGetString(ini, section, "description", section + 4, value));
		SAFECOPY(cfg->grp[i]->code_prefix, iniGetString(ini, section, "code_prefix", "", value));
		SAFECOPY(cfg->grp[i]->arstr, iniGetString(ini, section, "ars", "", value));
		arstr(NULL, cfg->grp[i]->arstr, cfg, cfg->grp[i]->ar);
		cfg->grp[i]->sort = iniGetInteger(ini, section, "sort", 0);
	}
	strListFree(&grp_list);

	/**********************/
	/* Message Sub-boards */
	/**********************/

	str_list_t sub_list = iniGetSectionList(ini, "sub:");
	cfg->total_subs = strListCount(sub_list);

	if(cfg->total_subs) {
		if((cfg->sub=(sub_t **)malloc(sizeof(sub_t *)*cfg->total_subs))==NULL)
			return allocerr(error, maxerrlen, fname, "subs", sizeof(sub_t *)*cfg->total_subs);
	} else
		cfg->sub=NULL;

	cfg->total_subs = 0;
	for(uint i=0; sub_list[i] != NULL; i++) {

		char group[INI_MAX_VALUE_LEN];
		section = sub_list[i];
		SAFECOPY(group, section + 4);
		char* p = strchr(group, ':');
		if(p == NULL)
			continue;
		*p = '\0';
		char* code = p + 1;
		int grpnum = getgrpnum_from_name(cfg, group);
		if(!is_valid_grpnum(cfg, grpnum))
			continue;
		if((cfg->sub[i]=(sub_t *)malloc(sizeof(sub_t)))==NULL)
			return allocerr(error, maxerrlen, fname, "sub", sizeof(sub_t));
		memset(cfg->sub[i],0,sizeof(sub_t));
		SAFECOPY(cfg->sub[i]->code_suffix, code);

		cfg->sub[i]->subnum = i;
		cfg->sub[i]->grp = grpnum;
		SAFECOPY(cfg->sub[i]->lname, iniGetString(ini, section, "description", code, value));
		SAFECOPY(cfg->sub[i]->sname, iniGetString(ini, section, "name", code, value));
		SAFECOPY(cfg->sub[i]->qwkname, iniGetString(ini, section, "qwk_name", code, value));
		SAFECOPY(cfg->sub[i]->data_dir, iniGetString(ini, section, "data_dir", "", value));

		SAFECOPY(cfg->sub[i]->arstr, iniGetString(ini, section, "ars", "", value));
		SAFECOPY(cfg->sub[i]->read_arstr, iniGetString(ini, section, "read_ars", "", value));
		SAFECOPY(cfg->sub[i]->post_arstr, iniGetString(ini, section, "post_ars", "", value));
		SAFECOPY(cfg->sub[i]->op_arstr, iniGetString(ini, section, "operator_ars", "", value));
		SAFECOPY(cfg->sub[i]->mod_arstr, iniGetString(ini, section, "moderated_ars", "", value));

		arstr(NULL, cfg->sub[i]->arstr, cfg, cfg->sub[i]->ar);
		arstr(NULL, cfg->sub[i]->read_arstr, cfg, cfg->sub[i]->read_ar);
		arstr(NULL, cfg->sub[i]->post_arstr, cfg, cfg->sub[i]->post_ar);
		arstr(NULL, cfg->sub[i]->op_arstr, cfg, cfg->sub[i]->op_ar);
		arstr(NULL, cfg->sub[i]->mod_arstr, cfg,cfg->sub[i]->mod_ar);

		cfg->sub[i]->misc = iniGetLongInt(ini, section, "settings", 0);
		if((cfg->sub[i]->misc&(SUB_FIDO|SUB_INET)) && !(cfg->sub[i]->misc&SUB_QNET))
			cfg->sub[i]->misc|=SUB_NOVOTING;

		SAFECOPY(cfg->sub[i]->tagline, iniGetString(ini, section, "qwknet_tagline", "", value));
		SAFECOPY(cfg->sub[i]->origline, iniGetString(ini, section, "fidonet_origin", "", value));
		SAFECOPY(cfg->sub[i]->post_sem, iniGetString(ini, section, "post_sem", "", value));
		SAFECOPY(cfg->sub[i]->newsgroup, iniGetString(ini, section, "newsgroup", "", value));
		SAFECOPY(cfg->sub[i]->area_tag, iniGetString(ini, section, "area_tag", "", value));

		cfg->sub[i]->faddr = smb_atofaddr(NULL, iniGetString(ini, section, "fidonet_addr", "", value));
		cfg->sub[i]->maxmsgs = iniGetInteger(ini, section, "max_msgs", 0);
		cfg->sub[i]->maxcrcs = iniGetInteger(ini, section, "max_crcs", 0);
		cfg->sub[i]->maxage = iniGetInteger(ini, section, "max_age", 0);
		cfg->sub[i]->ptridx = iniGetInteger(ini, section, "ptridx", 0);
#ifdef SBBS
		for(j=0;j<i;j++)
			if(cfg->sub[i]->ptridx==cfg->sub[j]->ptridx) {
				safe_snprintf(error, maxerrlen,"%s: Duplicate pointer index for subs %s and %s"
					,fname
					,cfg->sub[i]->code_suffix,cfg->sub[j]->code_suffix);
				return(FALSE);
			}
#endif


		cfg->sub[i]->qwkconf = iniGetShortInt(ini, section, "qwk_conf", 0);
		cfg->sub[i]->pmode = iniGetLongInt(ini, section, "print_mode", 0);
		cfg->sub[i]->n_pmode = iniGetLongInt(ini, section, "print_mode_neg", 0);
		++cfg->total_subs;
	}
	strListFree(&sub_list);

	/***********/
	/* FidoNet */
	/***********/
	section = "fidonet";
	str_list_t faddr_list = iniGetStringList(ini, section, "addr_list", ",", "");
	cfg->total_faddrs = strListCount(faddr_list);

	if(cfg->total_faddrs) {
		if((cfg->faddr=(faddr_t *)malloc(sizeof(faddr_t)*cfg->total_faddrs))==NULL)
			return allocerr(error, maxerrlen, fname, "fido_addrs", sizeof(faddr_t)*cfg->total_faddrs);
	} else
		cfg->faddr=NULL;

	for(uint i=0;i<cfg->total_faddrs;i++)
		cfg->faddr[i] = smb_atofaddr(NULL, faddr_list[i]);
	strListFree(&faddr_list);

	// Sanity-check each sub's FidoNet-style address
	for(uint i = 0; i < cfg->total_subs; i++)
		cfg->sub[i]->faddr = *nearest_sysfaddr(cfg, &cfg->sub[i]->faddr);

	SAFECOPY(cfg->origline, iniGetString(ini, section, "default_origin", "", value));
	SAFECOPY(cfg->netmail_sem, iniGetString(ini, section, "netmail_sem", "", value));
	SAFECOPY(cfg->echomail_sem, iniGetString(ini, section, "echomail_sem", "", value));
	SAFECOPY(cfg->netmail_dir, iniGetString(ini, section, "netmail_dir", "", value));
	SAFECOPY(cfg->fidofile_dir, iniGetString(ini, section, "file_dir", "", value));
	cfg->netmail_misc = iniGetLongInt(ini, section, "netmail_settings", 0);
	cfg->netmail_cost = iniGetLongInt(ini, section, "netmail_cost", 0);

	/**********/
	/* QWKnet */
	/**********/
	str_list_t qhub_list = iniGetSectionList(ini, "qhub:");
	cfg->total_qhubs = strListCount(qhub_list);

	if(cfg->total_qhubs) {
		if((cfg->qhub=(qhub_t **)malloc(sizeof(qhub_t *)*cfg->total_qhubs))==NULL)
			return allocerr(error, maxerrlen, fname, "qhubs", sizeof(qhub_t*)*cfg->total_qhubs);
	} else
		cfg->qhub=NULL;

	cfg->total_qhubs = 0;
	for(uint i=0; qhub_list[i] != NULL; i++) {
		section = qhub_list[i];
		if((cfg->qhub[i]=(qhub_t *)malloc(sizeof(qhub_t)))==NULL)
			return allocerr(error, maxerrlen, fname, "qhub", sizeof(qhub_t));
		memset(cfg->qhub[i],0,sizeof(qhub_t));

		SAFECOPY(cfg->qhub[i]->id, section + 5);
		cfg->qhub[i]->time = iniGetShortInt(ini, section, "time", 0);
		cfg->qhub[i]->freq = iniGetShortInt(ini, section, "freq", 0);
		cfg->qhub[i]->days = iniGetShortInt(ini, section, "days", 0);
		cfg->qhub[i]->node = iniGetShortInt(ini, section, "node", 0);
		SAFECOPY(cfg->qhub[i]->call, iniGetString(ini, section, "call", "", value));
		SAFECOPY(cfg->qhub[i]->pack, iniGetString(ini, section, "pack", "", value));
		SAFECOPY(cfg->qhub[i]->unpack, iniGetString(ini, section, "unpack", "", value));
		SAFECOPY(cfg->qhub[i]->fmt, iniGetString(ini, section, "format", "zip", value));
		cfg->qhub[i]->misc = iniGetLongInt(ini, section, "settings", 0);

		char str[128];
		SAFEPRINTF(str, "qhubsub:%s:", cfg->qhub[i]->id);
		str_list_t qsub_list = iniGetSectionList(ini, str);
		uint k = strListCount(qsub_list);
		if(k) {
			if((cfg->qhub[i]->sub=(sub_t**)malloc(sizeof(sub_t*)*k))==NULL)
				return allocerr(error, maxerrlen, fname, "qhub sub", sizeof(sub_t)*k);
			if((cfg->qhub[i]->conf=(ushort *)malloc(sizeof(ushort)*k))==NULL)
				return allocerr(error, maxerrlen, fname, "qhub conf", sizeof(ushort)*k);
			if((cfg->qhub[i]->mode=(char *)malloc(sizeof(char)*k))==NULL)
				return allocerr(error, maxerrlen, fname, "qhub mode", sizeof(uchar)*k);
		}

		for(uint j=0;j<k;j++) {
			uint16_t	confnum;
			uint16_t	subnum;
			char		subcode[LEN_EXTCODE + 1];
			uint8_t		mode;
			confnum = atoi(qsub_list[j] + strlen(str));
			SAFECOPY(subcode, iniGetString(ini, qsub_list[j], "sub", "", value));
			subnum = getsubnum(cfg, subcode);
			mode = iniGetLongInt(ini, qsub_list[i], "mode", 0);
			if(subnum < cfg->total_subs) {
				cfg->sub[subnum]->misc |= SUB_QNET;
				cfg->qhub[i]->sub[cfg->qhub[i]->subs]	= cfg->sub[subnum];
				cfg->qhub[i]->mode[cfg->qhub[i]->subs]	= mode;
				cfg->qhub[i]->conf[cfg->qhub[i]->subs]	= confnum;
				cfg->qhub[i]->subs++;
			}
		}
		strListFree(&qsub_list);
		++cfg->total_qhubs;
	}
	strListFree(&qhub_list);

	/************/
	/* Internet */
	/************/
	section = "Internet";
	SAFECOPY(cfg->sys_inetaddr, iniGetString(ini, section, "addr", "", value));
	SAFECOPY(cfg->inetmail_sem, iniGetString(ini, section, "netmail_sem", "", value));
	SAFECOPY(cfg->smtpmail_sem, iniGetString(ini, section, "smtp_sem", "", value));
	cfg->inetmail_misc = iniGetLongInt(ini, section, "settings", 0);
	cfg->inetmail_cost = iniGetLongInt(ini, section, "cost", 0);

	iniFreeStringList(ini);

	return TRUE;
}

void free_node_cfg(scfg_t* cfg)
{
	if(cfg->mdm_result!=NULL) {
		FREE_AND_NULL(cfg->mdm_result);
	}
}

void free_main_cfg(scfg_t* cfg)
{
	int i;

	if(cfg->shell!=NULL) {
		for(i=0;i<cfg->total_shells;i++) {
			FREE_AND_NULL(cfg->shell[i]);
		}
		FREE_AND_NULL(cfg->shell);
	}
	cfg->total_shells=0;
}

void free_msgs_cfg(scfg_t* cfg)
{
	int i;

	if(cfg->grp!=NULL) {
		for(i=0;i<cfg->total_grps;i++) {
			FREE_AND_NULL(cfg->grp[i]);
		}
		FREE_AND_NULL(cfg->grp);
	}
	cfg->total_grps=0;

	if(cfg->sub!=NULL) {
		for(i=0;i<cfg->total_subs;i++) {
			FREE_AND_NULL(cfg->sub[i]);
		}
		FREE_AND_NULL(cfg->sub);
	}
	cfg->total_subs=0;

	FREE_AND_NULL(cfg->faddr);
	cfg->total_faddrs=0;

	if(cfg->qhub!=NULL) {
		for(i=0;i<cfg->total_qhubs;i++) {
			FREE_AND_NULL(cfg->qhub[i]->mode);
			FREE_AND_NULL(cfg->qhub[i]->conf);
			FREE_AND_NULL(cfg->qhub[i]->sub);
			FREE_AND_NULL(cfg->qhub[i]);
		}
		FREE_AND_NULL(cfg->qhub);
	}
	cfg->total_qhubs=0;

	if(cfg->phub!=NULL) {
		for(i=0;i<cfg->total_phubs;i++) {
			FREE_AND_NULL(cfg->phub[i]);
		}
		FREE_AND_NULL(cfg->phub);
	}
	cfg->total_phubs=0;
}

/************************************************************/
/* Create data and sub-dirs off data if not already created */
/************************************************************/
void make_data_dirs(scfg_t* cfg)
{
	char	str[MAX_PATH+1];

	md(cfg->data_dir);
	SAFEPRINTF(str,"%ssubs",cfg->data_dir);
	md(str);
	SAFEPRINTF(str,"%sdirs",cfg->data_dir);
	md(str);
	SAFEPRINTF(str,"%stext",cfg->data_dir);
	md(str);
	SAFEPRINTF(str,"%smsgs",cfg->data_dir);
	md(str);
	SAFEPRINTF(str,"%suser",cfg->data_dir);
	md(str);
	SAFEPRINTF(str,"%sqnet",cfg->data_dir);
	md(str);
	SAFEPRINTF(str,"%sfile",cfg->data_dir);
	md(str);

	md(cfg->logs_dir);
	SAFEPRINTF(str,"%slogs",cfg->logs_dir);
	md(str);

	if(cfg->mods_dir[0])
		md(cfg->mods_dir);

	for(int i = 0; i < cfg->total_dirs; i++) {
		md(cfg->dir[i]->data_dir);
		if(cfg->dir[i]->misc & DIR_FCHK) 
			md(cfg->dir[i]->path);
	}
}

int getdirnum(scfg_t* cfg, const char* code)
{
	char fullcode[LEN_EXTCODE + 1];
	size_t i;

	if(code == NULL || *code == '\0')
		return -1;

	for(i=0;i<cfg->total_dirs;i++) {
		if(cfg->dir[i]->code[0] == '\0' && cfg->dir[i]->lib < cfg->total_libs ) {
			SAFEPRINTF2(fullcode, "%s%s", cfg->lib[cfg->dir[i]->lib]->code_prefix, cfg->dir[i]->code_suffix);
			if(stricmp(fullcode, code) == 0)
				return i;
		} else {
			if(stricmp(cfg->dir[i]->code, code)==0)
				return i;
		}
	}
	return -1;
}

int getlibnum(scfg_t* cfg, const char* code)
{
	int i = getdirnum(cfg, code);

	if(i >= 0)
		return cfg->dir[i]->lib;
	return i;
}

int getsubnum(scfg_t* cfg, const char* code)
{
	char fullcode[LEN_EXTCODE + 1];
	size_t i;

	if(code == NULL || *code == '\0')
		return -1;

	for(i=0;i<cfg->total_subs;i++) {
		if(cfg->sub[i]->code[0] == '\0' && cfg->sub[i]->grp < cfg->total_grps ) {
			SAFEPRINTF2(fullcode, "%s%s", cfg->grp[cfg->sub[i]->grp]->code_prefix, cfg->sub[i]->code_suffix);
			if(stricmp(fullcode, code) == 0)
				return i;
		} else {
			if(stricmp(cfg->sub[i]->code,code) == 0)
				return i;
		}
	}
	return -1;
}

int getgrpnum(scfg_t* cfg, const char* code)
{
	int i = getdirnum(cfg, code);

	if(i >= 0)
		return cfg->sub[i]->grp;
	return i;
}

int getgrpnum_from_name(scfg_t* cfg, const char* name)
{
	int i;

	for(i = 0; i < cfg->total_grps; i++) {
		if(stricmp(cfg->grp[i]->sname, name) == 0)
			break;
	}
	return i;
}

int getlibnum_from_name(scfg_t* cfg, const char* name)
{
	int i;

	for(i = 0; i < cfg->total_libs; i++) {
		if(stricmp(cfg->lib[i]->sname, name) == 0)
			break;
	}
	return i;
}

int getxtrnsec(scfg_t* cfg, const char* code)
{
	int i;

	for(i = 0; i < cfg->total_xtrnsecs; i++) {
		if(stricmp(cfg->xtrnsec[i]->code, code) == 0)
			break;
	}
	return i;
}

int getgurunum(scfg_t* cfg, const char* code)
{
	int i;

	for(i = 0; i < cfg->total_gurus; i++) {
		if(stricmp(cfg->guru[i]->code, code) == 0)
			break;
	}
	return i;
}

int getchatactset(scfg_t* cfg, const char* name)
{
	int i;

	for(i = 0; i < cfg->total_actsets; i++) {
		if(stricmp(cfg->actset[i]->name, name) == 0)
			break;
	}
	return i;
}

BOOL is_valid_dirnum(scfg_t* cfg, int dirnum)
{
	return (dirnum >= 0) && (cfg != NULL) && (dirnum < cfg->total_dirs);
}

BOOL is_valid_libnum(scfg_t* cfg, int libnum)
{
	return (libnum >= 0) && (cfg != NULL) && (libnum < cfg->total_libs);
}

BOOL is_valid_subnum(scfg_t* cfg, int subnum)
{
	return (subnum >= 0) && (cfg != NULL) && (subnum < cfg->total_subs);
}

BOOL is_valid_grpnum(scfg_t* cfg, int grpnum)
{
	return (grpnum >= 0) && (cfg != NULL) && (grpnum < cfg->total_grps);
}

BOOL is_valid_xtrnsec(scfg_t* cfg, int secnum)
{
	return (secnum >= 0) && (cfg != NULL) && (secnum < cfg->total_xtrnsecs);
}

faddr_t* nearest_sysfaddr(scfg_t* cfg, faddr_t* addr)
{
	uint i;

	if(cfg->total_faddrs <= 0)
		return addr;

	for(i=0; i < cfg->total_faddrs; i++)
		if(memcmp(addr, &cfg->faddr[i], sizeof(*addr)) == 0)
			return &cfg->faddr[i];
	for(i=0; i < cfg->total_faddrs; i++)
		if(addr->zone == cfg->faddr[i].zone
			&& addr->net == cfg->faddr[i].net
			&& addr->node == cfg->faddr[i].node)
			return &cfg->faddr[i];
	for(i=0; i < cfg->total_faddrs; i++)
		if(addr->zone == cfg->faddr[i].zone
			&& addr->net == cfg->faddr[i].net)
			return &cfg->faddr[i];
	for(i=0; i < cfg->total_faddrs; i++)
		if(addr->zone == cfg->faddr[i].zone)
			return &cfg->faddr[i];
	return &cfg->faddr[0];
}

/****************************************************************************/
/* Searches the file <name>.can in the TEXT directory for matches			*/
/* Returns TRUE if found in list, FALSE if not.								*/
/****************************************************************************/
BOOL trashcan(scfg_t* cfg, const char* insearchof, const char* name)
{
	char fname[MAX_PATH+1];

	return(findstr(insearchof,trashcan_fname(cfg,name,fname,sizeof(fname))));
}

/****************************************************************************/
char* trashcan_fname(scfg_t* cfg, const char* name, char* fname, size_t maxlen)
{
	safe_snprintf(fname,maxlen,"%s%s.can",cfg->text_dir,name);
	return fname;
}

/****************************************************************************/
str_list_t trashcan_list(scfg_t* cfg, const char* name)
{
	char	fname[MAX_PATH+1];

	return findstr_list(trashcan_fname(cfg, name, fname, sizeof(fname)));
}

char* sub_newsgroup_name(scfg_t* cfg, sub_t* sub, char* str, size_t size)
{
	memset(str, 0, size);
	if(sub->newsgroup[0])
		strncpy(str, sub->newsgroup, size - 1);
	else {
		snprintf(str, size - 1, "%s.%s", cfg->grp[sub->grp]->sname, sub->sname);
		/*
		 * From RFC5536:
		 * newsgroup-name  =  component *( "." component )
		 * component       =  1*component-char
		 * component-char  =  ALPHA / DIGIT / "+" / "-" / "_"
		 */
		if (str[0] == '.')
			str[0] = '_';
		size_t c;
		for(c = 0; str[c] != 0; c++) {
			/* Legal characters */
			if ((str[c] >= 'A' && str[c] <= 'Z')
					|| (str[c] >= 'a' && str[c] <= 'z')
					|| (str[c] >= '0' && str[c] <= '9')
					|| str[c] == '+'
					|| str[c] == '-'
					|| str[c] == '_'
					|| str[c] == '.')
				continue;
			str[c] = '_';
		}
		c--;
		if (str[c] == '.')
			str[c] = '_';
	}
	return str;
}

char* sub_area_tag(scfg_t* cfg, sub_t* sub, char* str, size_t size)
{
	char* p;

	memset(str, 0, size);
	if(sub->area_tag[0])
		strncpy(str, sub->area_tag, size - 1);
	else if(sub->newsgroup[0])
		strncpy(str, sub->newsgroup, size - 1);
	else {
		strncpy(str, sub->sname, size - 1);
		REPLACE_CHARS(str, ' ', '_', p);
	}
	strupr(str);
	return str;
}

char* dir_area_tag(scfg_t* cfg, dir_t* dir, char* str, size_t size)
{
	char* p;

	memset(str, 0, size);
	if(dir->area_tag[0])
		strncpy(str, dir->area_tag, size - 1);
	else {
		strncpy(str, dir->sname, size - 1);
		REPLACE_CHARS(str, ' ', '_', p);
	}
	strupr(str);
	return str;
}
