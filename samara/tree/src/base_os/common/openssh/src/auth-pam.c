/*-
 * Copyright (c) 2002 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed for the FreeBSD Project by ThinkSec AS and
 * NAI Labs, the Security Research Division of Network Associates, Inc.
 * under DARPA/SPAWAR contract N66001-01-C-8035 ("CBOSS"), as part of the
 * DARPA CHATS research program.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Based on $FreeBSD: src/crypto/openssh/auth2-pam-freebsd.c,v 1.11 2003/03/31 13:48:18 des Exp $ */
#include "includes.h"
RCSID("$Id: auth-pam.c,v 1.7 2013/03/04 02:44:42 et Exp $");

#ifdef USE_PAM
#if defined(HAVE_SECURITY_PAM_APPL_H)
#include <security/pam_appl.h>
#elif defined (HAVE_PAM_PAM_APPL_H)
#include <pam/pam_appl.h>
#endif

#include "auth.h"
#include "auth-pam.h"
#include "buffer.h"
#include "bufaux.h"
#include "canohost.h"
#include "log.h"
#include "monitor_wrap.h"
#include "msg.h"
#include "packet.h"
#include "readpass.h"
#include "servconf.h"
#include "ssh2.h"
#include "xmalloc.h"
#include "auth-options.h"

/* TMS */
#include "customer.h"

/*
 * NOTE: this is shared with 
 * src/base_os/linux_el/el5/pam/pam_unix/src/pam_unix_sess.c
 * (with whom we share no header files)
 */
#define PAM_TRUSTED_AUTH_INFO_SET_STR "set"

extern ServerOptions options;
extern Buffer loginmsg;
extern int compat20;
extern u_int utmp_len;

#ifdef USE_POSIX_THREADS
#include <pthread.h>
/*
 * Avoid namespace clash when *not* using pthreads for systems *with*
 * pthreads, which unconditionally define pthread_t via sys/types.h
 * (e.g. Linux)
 */
typedef pthread_t sp_pthread_t;
#else
typedef pid_t sp_pthread_t;
#endif

struct pam_ctxt {
	sp_pthread_t	 pam_thread;
	int		 pam_psock;
	int		 pam_csock;
	int		 pam_done;
};

static void sshpam_free_ctx(void *);
static struct pam_ctxt *cleanup_ctxt;

#ifndef USE_POSIX_THREADS
/*
 * Simulate threads with processes.
 */

static int sshpam_thread_status = -1;
static mysig_t sshpam_oldsig;

static void 
sshpam_sigchld_handler(int sig)
{
        signal(SIGCHLD, SIG_DFL);
	if (cleanup_ctxt == NULL)
		return;	/* handler called after PAM cleanup, shouldn't happen */
        if (waitpid(cleanup_ctxt->pam_thread, &sshpam_thread_status, WNOHANG)
            <= 0) {
            /* PAM thread has not exitted, privsep slave must have */
            kill(cleanup_ctxt->pam_thread, SIGTERM);
            if (waitpid(cleanup_ctxt->pam_thread, &sshpam_thread_status, 0)
                <= 0)
                return; /* could not wait */
	}
	if (WIFSIGNALED(sshpam_thread_status) &&
	    WTERMSIG(sshpam_thread_status) == SIGTERM)
		return;	/* terminated by pthread_cancel */
	if (!WIFEXITED(sshpam_thread_status))
		fatal("PAM: authentication thread exited unexpectedly");
	if (WEXITSTATUS(sshpam_thread_status) != 0)
		fatal("PAM: authentication thread exited uncleanly");
}

static void
pthread_exit(void *value __unused)
{
	_exit(0);
}

static int
pthread_create(sp_pthread_t *thread, const void *attr __unused,
    void *(*thread_start)(void *), void *arg)
{
	pid_t pid;
        struct pam_ctxt *ctx = arg;

	sshpam_thread_status = -1;
	switch ((pid = fork())) {
	case -1:
		error("fork(): %s", strerror(errno));
		return (-1);
	case 0:
                close(ctx->pam_psock);
                ctx->pam_psock = -1;
		thread_start(arg);
		_exit(1);
	default:
		*thread = pid;
                close(ctx->pam_csock);
                ctx->pam_csock = -1;
		sshpam_oldsig = signal(SIGCHLD, sshpam_sigchld_handler);
		return (0);
	}
}

static int
pthread_cancel(sp_pthread_t thread)
{
	signal(SIGCHLD, sshpam_oldsig);
	return (kill(thread, SIGTERM));
}

static int
pthread_join(sp_pthread_t thread, void **value __unused)
{
	int status;

	if (sshpam_thread_status != -1)
		return (sshpam_thread_status);
	signal(SIGCHLD, sshpam_oldsig);
	waitpid(thread, &status, 0);
	return (status);
}
#endif


static pam_handle_t *sshpam_handle = NULL;
static int sshpam_err = 0;
static int sshpam_authenticated = 0;
static int sshpam_session_open = 0;
static int sshpam_cred_established = 0;
static int sshpam_account_status = -1;
static char **sshpam_env = NULL;
static Authctxt *sshpam_authctxt = NULL;

/* Some PAM implementations don't implement this */
#ifndef HAVE_PAM_GETENVLIST
static char **
pam_getenvlist(pam_handle_t *pamh)
{
	/*
	 * XXX - If necessary, we can still support envrionment passing
	 * for platforms without pam_getenvlist by searching for known
	 * env vars (e.g. KRB5CCNAME) from the PAM environment.
	 */
	 return NULL;
}
#endif

void
pam_password_change_required(int reqd)
{
	debug3("%s %d", __func__, reqd);
	if (sshpam_authctxt == NULL)
		fatal("%s: PAM authctxt not initialized", __func__);
	sshpam_authctxt->force_pwchange = reqd;
	if (reqd) {
		no_port_forwarding_flag |= 2;
		no_agent_forwarding_flag |= 2;
		no_x11_forwarding_flag |= 2;
	} else {
		no_port_forwarding_flag &= ~2;
		no_agent_forwarding_flag &= ~2;
		no_x11_forwarding_flag &= ~2;
	}
}

/* Import regular and PAM environment from subprocess */
static void
import_environments(Buffer *b)
{
	char *env;
	u_int i, num_env;
	int err;

	debug3("PAM: %s entering", __func__);

#ifndef USE_POSIX_THREADS
	/* Import variables set by do_pam_account */
	sshpam_account_status = buffer_get_int(b);
	pam_password_change_required(buffer_get_int(b));

	/* Import environment from subprocess */
	num_env = buffer_get_int(b);
	sshpam_env = xmalloc((num_env + 1) * sizeof(*sshpam_env));
	debug3("PAM: num env strings %d", num_env);
	for(i = 0; i < num_env; i++)
		sshpam_env[i] = buffer_get_string(b, NULL);

	sshpam_env[num_env] = NULL;

	/* Import PAM environment from subprocess */
	num_env = buffer_get_int(b);
	debug("PAM: num PAM env strings %d", num_env);
	for(i = 0; i < num_env; i++) {
		env = buffer_get_string(b, NULL);

#ifdef HAVE_PAM_PUTENV
		/* Errors are not fatal here */
		if ((err = pam_putenv(sshpam_handle, env)) != PAM_SUCCESS) {
			error("PAM: pam_putenv: %s",
			    pam_strerror(sshpam_handle, sshpam_err));
		}
#endif
	}
#endif
}

/*
 * Conversation function for authentication thread.
 */
static int
sshpam_thread_conv(int n, const struct pam_message **msg,
    struct pam_response **resp, void *data)
{
	Buffer buffer;
	struct pam_ctxt *ctxt;
	struct pam_response *reply;
	int i;

	debug3("PAM: %s entering, %d messages", __func__, n);
	*resp = NULL;

	ctxt = data;
	if (n <= 0 || n > PAM_MAX_NUM_MSG)
		return (PAM_CONV_ERR);

	if ((reply = malloc(n * sizeof(*reply))) == NULL)
		return (PAM_CONV_ERR);
	memset(reply, 0, n * sizeof(*reply));

	buffer_init(&buffer);
	for (i = 0; i < n; ++i) {
		switch (PAM_MSG_MEMBER(msg, i, msg_style)) {
		case PAM_PROMPT_ECHO_OFF:
			buffer_put_cstring(&buffer,
			    PAM_MSG_MEMBER(msg, i, msg));
			if (ssh_msg_send(ctxt->pam_csock,
			    PAM_MSG_MEMBER(msg, i, msg_style), &buffer) == -1)
				goto fail;
			if (ssh_msg_recv(ctxt->pam_csock, &buffer) == -1)
				goto fail;
			if (buffer_get_char(&buffer) != PAM_AUTHTOK)
				goto fail;
			reply[i].resp = buffer_get_string(&buffer, NULL);
			break;
		case PAM_PROMPT_ECHO_ON:
			buffer_put_cstring(&buffer,
			    PAM_MSG_MEMBER(msg, i, msg));
			if (ssh_msg_send(ctxt->pam_csock,
			    PAM_MSG_MEMBER(msg, i, msg_style), &buffer) == -1)
				goto fail;
			if (ssh_msg_recv(ctxt->pam_csock, &buffer) == -1)
				goto fail;
			if (buffer_get_char(&buffer) != PAM_AUTHTOK)
				goto fail;
			reply[i].resp = buffer_get_string(&buffer, NULL);
			break;
		case PAM_ERROR_MSG:
			buffer_put_cstring(&buffer,
			    PAM_MSG_MEMBER(msg, i, msg));
			if (ssh_msg_send(ctxt->pam_csock,
			    PAM_MSG_MEMBER(msg, i, msg_style), &buffer) == -1)
				goto fail;
			break;
		case PAM_TEXT_INFO:
			buffer_put_cstring(&buffer,
			    PAM_MSG_MEMBER(msg, i, msg));
			if (ssh_msg_send(ctxt->pam_csock,
			    PAM_MSG_MEMBER(msg, i, msg_style), &buffer) == -1)
				goto fail;
			break;
		default:
			goto fail;
		}
		buffer_clear(&buffer);
	}
	buffer_free(&buffer);
	*resp = reply;
	return (PAM_SUCCESS);

 fail:
	for(i = 0; i < n; i++) {
		if (reply[i].resp != NULL)
			xfree(reply[i].resp);
	}
	xfree(reply);
	buffer_free(&buffer);
	return (PAM_CONV_ERR);
}

/*
 * Authentication thread.
 */
static void *
sshpam_thread(void *ctxtp)
{
	struct pam_ctxt *ctxt = ctxtp;
	Buffer buffer;
	struct pam_conv sshpam_conv;
#ifndef USE_POSIX_THREADS
	extern char **environ;
	char **env_from_pam;
	u_int i;
	const char *pam_user, *finaluser = NULL;
        int retval;
        char ubuf[1024];

	pam_get_item(sshpam_handle, PAM_USER, (const void **)&pam_user);
	setproctitle("%s [pam]", pam_user);
	environ[0] = NULL;
#endif

	sshpam_conv.conv = sshpam_thread_conv;
	sshpam_conv.appdata_ptr = ctxt;

	if (sshpam_authctxt == NULL)
		fatal("%s: PAM authctxt not initialized", __func__);

	buffer_init(&buffer);
	sshpam_err = pam_set_item(sshpam_handle, PAM_CONV,
	    (const void *)&sshpam_conv);
	if (sshpam_err != PAM_SUCCESS)
		goto auth_fail;
	sshpam_err = pam_authenticate(sshpam_handle, 0);
	if (sshpam_err != PAM_SUCCESS)
		goto auth_fail;

	if (compat20) {
		if (!do_pam_account())
			goto auth_fail;
		if (sshpam_authctxt->force_pwchange) {
			sshpam_err = pam_chauthtok(sshpam_handle,
			    PAM_CHANGE_EXPIRED_AUTHTOK);
			if (sshpam_err != PAM_SUCCESS)
				goto auth_fail;
			pam_password_change_required(0);
		}
	}

        /* 
         * Set our SSH_PAM_USER.  This is to expose the final,
         * authenticated username to the rest of sshd.
         *
         * We always set SSH_PAM_USER, but will only end up using it if
         * there's not a valid local account.  This goes along with our
         * "faked" pw data scheme for supporting template (mapped) users.
         *
         * Note that in the case of template (mapped) users,
         * SSH_PAM_USER will have the same value that the TMS PAM
         * modules will set in AUTH_USER, which we no longer use here.
         * 
         */

        retval = pam_get_item(sshpam_handle, PAM_USER, 
                              (const void **)&finaluser);
        if (retval == PAM_SUCCESS && finaluser && *finaluser) {
            ubuf[0] = '\0';
            snprintf(ubuf, sizeof(ubuf), "SSH_PAM_USER=%s", finaluser);
            sshpam_err = pam_putenv(sshpam_handle, ubuf);
            if (sshpam_err != PAM_SUCCESS) {
                goto auth_fail;
            }
        }

	buffer_put_cstring(&buffer, "OK");

#ifndef USE_POSIX_THREADS
	/* Export variables set by do_pam_account */
	buffer_put_int(&buffer, sshpam_account_status);
	buffer_put_int(&buffer, sshpam_authctxt->force_pwchange);

	/* Export any environment strings set in child */
	for(i = 0; environ[i] != NULL; i++)
		; /* Count */
	buffer_put_int(&buffer, i);
	for(i = 0; environ[i] != NULL; i++)
		buffer_put_cstring(&buffer, environ[i]);

	/* Export any environment strings set by PAM in child */
	env_from_pam = pam_getenvlist(sshpam_handle);
	for(i = 0; env_from_pam != NULL && env_from_pam[i] != NULL; i++)
		; /* Count */
	buffer_put_int(&buffer, i);
	for(i = 0; env_from_pam != NULL && env_from_pam[i] != NULL; i++)
		buffer_put_cstring(&buffer, env_from_pam[i]);
#endif /* USE_POSIX_THREADS */

	/* XXX - can't do much about an error here */
	ssh_msg_send(ctxt->pam_csock, sshpam_err, &buffer);
	buffer_free(&buffer);
	pthread_exit(NULL);

 auth_fail:
	buffer_put_cstring(&buffer,
	    pam_strerror(sshpam_handle, sshpam_err));
	/* XXX - can't do much about an error here */
	ssh_msg_send(ctxt->pam_csock, PAM_AUTH_ERR, &buffer);
	buffer_free(&buffer);
	pthread_exit(NULL);

	return (NULL); /* Avoid warning for non-pthread case */
}

void
sshpam_thread_cleanup(void)
{
	struct pam_ctxt *ctxt = cleanup_ctxt;

	debug3("PAM: %s entering", __func__);
	if (ctxt != NULL && ctxt->pam_thread != 0) {
		pthread_cancel(ctxt->pam_thread);
		pthread_join(ctxt->pam_thread, NULL);
		close(ctxt->pam_psock);
		close(ctxt->pam_csock);
		memset(ctxt, 0, sizeof(*ctxt));
		cleanup_ctxt = NULL;
	}
}

static int
sshpam_null_conv(int n, const struct pam_message **msg,
    struct pam_response **resp, void *data)
{
	debug3("PAM: %s entering, %d messages", __func__, n);
	return (PAM_CONV_ERR);
}

static struct pam_conv null_conv = { sshpam_null_conv, NULL };

void
sshpam_cleanup(void)
{
	debug("PAM: cleanup");
	if (sshpam_handle == NULL || (use_privsep && !mm_is_monitor()))
		return;
	pam_set_item(sshpam_handle, PAM_CONV, (const void *)&null_conv);
	if (sshpam_cred_established) {
		pam_setcred(sshpam_handle, PAM_DELETE_CRED);
		sshpam_cred_established = 0;
	}
	if (sshpam_session_open) {
		pam_close_session(sshpam_handle, PAM_SILENT);
		sshpam_session_open = 0;
	}
	sshpam_authenticated = 0;
	pam_end(sshpam_handle, sshpam_err);
	sshpam_handle = NULL;
}

static int
sshpam_init(Authctxt *authctxt)
{
	extern char *__progname;
	const char *pam_rhost, *pam_user, *user = authctxt->user;

	if (sshpam_handle != NULL) {
		/* We already have a PAM context; check if the user matches */
		sshpam_err = pam_get_item(sshpam_handle,
		    PAM_USER, (const void **)&pam_user);
		if (sshpam_err == PAM_SUCCESS && strcmp(user, pam_user) == 0)
			return (0);
		pam_end(sshpam_handle, sshpam_err);
		sshpam_handle = NULL;
	}
	debug("PAM: initializing for \"%s\"", user);
	sshpam_err =
	    pam_start(SSHD_PAM_SERVICE, user, &null_conv, &sshpam_handle);
	sshpam_authctxt = authctxt;

	if (sshpam_err != PAM_SUCCESS) {
		pam_end(sshpam_handle, sshpam_err);
		sshpam_handle = NULL;
		return (-1);
	}
	pam_rhost = get_remote_name_or_ip(utmp_len, options.use_dns);
	debug("PAM: setting PAM_RHOST to \"%s\"", pam_rhost);
	sshpam_err = pam_set_item(sshpam_handle, PAM_RHOST, pam_rhost);
	if (sshpam_err != PAM_SUCCESS) {
		pam_end(sshpam_handle, sshpam_err);
		sshpam_handle = NULL;
		return (-1);
	}
#ifdef PAM_TTY_KLUDGE
	/*
	 * Some silly PAM modules (e.g. pam_time) require a TTY to operate.
	 * sshd doesn't set the tty until too late in the auth process and
	 * may not even set one (for tty-less connections)
	 */
	debug("PAM: setting PAM_TTY to \"ssh\"");
	sshpam_err = pam_set_item(sshpam_handle, PAM_TTY, "ssh");
	if (sshpam_err != PAM_SUCCESS) {
		pam_end(sshpam_handle, sshpam_err);
		sshpam_handle = NULL;
		return (-1);
	}
#endif
	return (0);
}

static void *
sshpam_init_ctx(Authctxt *authctxt)
{
	struct pam_ctxt *ctxt;
	int socks[2];

	debug3("PAM: %s entering", __func__);
	/* Refuse to start if we don't have PAM enabled */
	if (!options.use_pam)
		return NULL;

	/* Initialize PAM */
	if (sshpam_init(authctxt) == -1) {
		error("PAM: initialization failed");
		return (NULL);
	}

	ctxt = xmalloc(sizeof *ctxt);
	memset(ctxt, 0, sizeof(*ctxt));

	/* Start the authentication thread */
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, socks) == -1) {
		error("PAM: failed create sockets: %s", strerror(errno));
		xfree(ctxt);
		return (NULL);
	}
	ctxt->pam_psock = socks[0];
	ctxt->pam_csock = socks[1];
	if (pthread_create(&ctxt->pam_thread, NULL, sshpam_thread, ctxt) == -1) {
		error("PAM: failed to start authentication thread: %s",
		    strerror(errno));
		close(socks[0]);
		close(socks[1]);
		xfree(ctxt);
		return (NULL);
	}
	cleanup_ctxt = ctxt;
	return (ctxt);
}

static int
sshpam_query(void *ctx, char **name, char **info,
    u_int *num, char ***prompts, u_int **echo_on)
{
	Buffer buffer;
	struct pam_ctxt *ctxt = ctx;
	size_t plen;
	u_char type;
	char *msg;
	size_t len;
        const char *auth_user = NULL;

	debug3("PAM: %s entering", __func__);
	buffer_init(&buffer);
	*name = xstrdup("");
	*info = xstrdup("");
	*prompts = xmalloc(sizeof(char *));
	**prompts = NULL;
	plen = 0;
	*echo_on = xmalloc(sizeof(u_int));
	while (ssh_msg_recv(ctxt->pam_psock, &buffer) == 0) {
		type = buffer_get_char(&buffer);
		msg = buffer_get_string(&buffer, NULL);
		switch (type) {
		case PAM_PROMPT_ECHO_ON:
		case PAM_PROMPT_ECHO_OFF:
			*num = 1;
			len = plen + strlen(msg) + 1;
			**prompts = xrealloc(**prompts, len);
			plen += snprintf(**prompts + plen, len, "%s", msg);
			**echo_on = (type == PAM_PROMPT_ECHO_ON);
			xfree(msg);
			return (0);
		case PAM_ERROR_MSG:
		case PAM_TEXT_INFO:
			/* accumulate messages */
			len = plen + strlen(msg) + 2;
			**prompts = xrealloc(**prompts, len);
			plen += snprintf(**prompts + plen, len, "%s\n", msg);
			xfree(msg);
			break;
		case PAM_SUCCESS:
		case PAM_AUTH_ERR:
			if (**prompts != NULL) {
				/* drain any accumulated messages */
				debug("PAM: %s", **prompts);
				buffer_append(&loginmsg, **prompts,
				    strlen(**prompts));
				xfree(**prompts);
				**prompts = NULL;
			}
			if (type == PAM_SUCCESS) {
				import_environments(&buffer);
				*num = 0;
				**echo_on = 0;
				ctxt->pam_done = 1;
				xfree(msg);
                                
                                /* 
                                 * If we faked the pw data, now we have to
                                 * get it right.  The PAM module (in another
                                 * process in the privsep case) has
                                 * authenticated us.  However, we may have
                                 * ended up with a different authenticated
                                 * user name than the user name we started
                                 * with.  To deal with this, we query the
                                 * PAM environment variable SSH_PAM_USER, which 
                                 * we unconditionally set to what PAM_USER had.
                                 * Then we'll update the authctxt's concept
                                 * of the user name, and lookup the other
                                 * fields to fill in the pw information.
                                 */
                                if (!sshpam_authctxt->valid) {

                                        debug2("Doing SSH_PAM_USER fixup");
                                        auth_user = pam_getenv(sshpam_handle,
                                                               "SSH_PAM_USER");
                                        if (!auth_user) {
                                                logit("No SSH_PAM_USER found!");
                                                return(-1);
                                        }

                                        debug2("auth_user = %s", auth_user);
                                        
                                        if (pam_set_item(sshpam_handle, PAM_USER, 
                                                         auth_user) != PAM_SUCCESS) {
                                            error("PAM: failed to set PAM_USER: %s",
                                                  pam_strerror(sshpam_handle, sshpam_err));
                                            return(-1);
                                        }
                                        
                                        /* Verify that the user is valid */
                                        if ((sshpam_authctxt->pw = 
                                             getpwnamallow(auth_user))
                                            != NULL) {
                                                sshpam_authctxt->valid = 1;
                                        }
                                        else {
                                                logit("No local user %s found", 
                                                      auth_user);
                                                return(-1);
                                        }
                                }
                                if (pam_putenv(sshpam_handle, "SSH_PAM_USER")
                                    != PAM_SUCCESS) {
                                    error("PAM: pam_putenv: %s",
                                          pam_strerror(sshpam_handle, sshpam_err));
                                }
                                return (0);
                        }
                        if (sshpam_authctxt->valid || aaa_log_unknown_usernames_flag) {
                            error("PAM: %s for %s%.100s from %.100s", msg,
                                  sshpam_authctxt->valid ? "" : "unknown user ",
                                  sshpam_authctxt->user,
                                  get_remote_name_or_ip(utmp_len, options.use_dns));
                        }
                        else {
                            debug("PAM: %s for %s%.100s from %.100s", msg,
                                  sshpam_authctxt->valid ? "" : "unknown user ",
                                  sshpam_authctxt->user,
                                  get_remote_name_or_ip(utmp_len, options.use_dns));
                            error("PAM: %s for unknown user from %.100s", msg,
                                  get_remote_name_or_ip(utmp_len, options.use_dns));
                        }
			/* FALLTHROUGH */
		default:
			*num = 0;
			**echo_on = 0;
			xfree(msg);
			ctxt->pam_done = -1;
			return (-1);
		}
	}
	return (-1);
}

/* XXX - see also comment in auth-chall.c:verify_response */
static int
sshpam_respond(void *ctx, u_int num, char **resp)
{
	Buffer buffer;
	struct pam_ctxt *ctxt = ctx;
        const char *auth_user;

	debug2("PAM: %s entering, %d responses", __func__, num);
	switch (ctxt->pam_done) {
	case 1:
                pam_get_item(sshpam_handle, PAM_USER, 
                             (const void **)&auth_user);
                debug2("auth_user = %s", auth_user);

                if (sshpam_authctxt->user) {
                        xfree(sshpam_authctxt->user);
                }
                sshpam_authctxt->user = xstrdup(auth_user);

		sshpam_authenticated = 1;
		return (0);
	case 0:
		break;
	default:
		return (-1);
	}
	if (num != 1) {
		error("PAM: expected one response, got %u", num);
		return (-1);
	}
	buffer_init(&buffer);
	buffer_put_cstring(&buffer, *resp);
	if (ssh_msg_send(ctxt->pam_psock, PAM_AUTHTOK, &buffer) == -1) {
		buffer_free(&buffer);
		return (-1);
	}
	buffer_free(&buffer);
	return (1);
}

static void
sshpam_free_ctx(void *ctxtp)
{
	struct pam_ctxt *ctxt = ctxtp;

	debug3("PAM: %s entering", __func__);
	sshpam_thread_cleanup();
	xfree(ctxt);
	/*
	 * We don't call sshpam_cleanup() here because we may need the PAM
	 * handle at a later stage, e.g. when setting up a session.  It's
	 * still on the cleanup list, so pam_end() *will* be called before
	 * the server process terminates.
	 */
}

KbdintDevice sshpam_device = {
	"pam",
	sshpam_init_ctx,
	sshpam_query,
	sshpam_respond,
	sshpam_free_ctx
};

KbdintDevice mm_sshpam_device = {
	"pam",
	mm_sshpam_init_ctx,
	mm_sshpam_query,
	mm_sshpam_respond,
	mm_sshpam_free_ctx
};

/*
 * This replaces auth-pam.c
 */
void
start_pam(Authctxt *authctxt)
{
	if (!options.use_pam)
		fatal("PAM: initialisation requested when UsePAM=no");

	if (sshpam_init(authctxt) == -1)
		fatal("PAM: initialisation failed");
}

void
finish_pam(void)
{
	sshpam_cleanup();
}

u_int
do_pam_account(void)
{
	if (sshpam_account_status != -1)
		return (sshpam_account_status);

	sshpam_err = pam_acct_mgmt(sshpam_handle, 0);
	debug3("PAM: %s pam_acct_mgmt = %d", __func__, sshpam_err);
	
	if (sshpam_err != PAM_SUCCESS && sshpam_err != PAM_NEW_AUTHTOK_REQD) {
		sshpam_account_status = 0;
		return (sshpam_account_status);
	}

	if (sshpam_err == PAM_NEW_AUTHTOK_REQD)
		pam_password_change_required(1);

	sshpam_account_status = 1;
	return (sshpam_account_status);
}

void
do_pam_set_tty(const char *tty)
{
	if (tty != NULL) {
		debug("PAM: setting PAM_TTY to \"%s\"", tty);
		sshpam_err = pam_set_item(sshpam_handle, PAM_TTY, tty);
		if (sshpam_err != PAM_SUCCESS)
			fatal("PAM: failed to set PAM_TTY: %s",
			    pam_strerror(sshpam_handle, sshpam_err));
	}
}

void
do_pam_setcred(int init)
{
	sshpam_err = pam_set_item(sshpam_handle, PAM_CONV,
	    (const void *)&null_conv);
	if (sshpam_err != PAM_SUCCESS)
		fatal("PAM: failed to set PAM_CONV: %s",
		    pam_strerror(sshpam_handle, sshpam_err));
	if (init) {
		debug("PAM: establishing credentials");
		sshpam_err = pam_setcred(sshpam_handle, PAM_ESTABLISH_CRED);
	} else {
		debug("PAM: reinitializing credentials");
		sshpam_err = pam_setcred(sshpam_handle, PAM_REINITIALIZE_CRED);
	}
	if (sshpam_err == PAM_SUCCESS) {
		sshpam_cred_established = 1;
		return;
	}
	if (sshpam_authenticated)
		fatal("PAM: pam_setcred(): %s",
		    pam_strerror(sshpam_handle, sshpam_err));
	else
		debug("PAM: pam_setcred(): %s",
		    pam_strerror(sshpam_handle, sshpam_err));
}

static int
pam_tty_conv(int n, const struct pam_message **msg,
    struct pam_response **resp, void *data)
{
	char input[PAM_MAX_MSG_SIZE];
	struct pam_response *reply;
	int i;

	debug3("PAM: %s called with %d messages", __func__, n);

	*resp = NULL;

	if (n <= 0 || n > PAM_MAX_NUM_MSG || !isatty(STDIN_FILENO))
		return (PAM_CONV_ERR);

	if ((reply = malloc(n * sizeof(*reply))) == NULL)
		return (PAM_CONV_ERR);
	memset(reply, 0, n * sizeof(*reply));

	for (i = 0; i < n; ++i) {
		switch (PAM_MSG_MEMBER(msg, i, msg_style)) {
		case PAM_PROMPT_ECHO_OFF:
			reply[i].resp =
			    read_passphrase(PAM_MSG_MEMBER(msg, i, msg),
			    RP_ALLOW_STDIN);
			reply[i].resp_retcode = PAM_SUCCESS;
			break;
		case PAM_PROMPT_ECHO_ON:
			fprintf(stderr, "%s\n", PAM_MSG_MEMBER(msg, i, msg));
			fgets(input, sizeof input, stdin);
			reply[i].resp = xstrdup(input);
			reply[i].resp_retcode = PAM_SUCCESS;
			break;
		case PAM_ERROR_MSG:
		case PAM_TEXT_INFO:
			fprintf(stderr, "%s\n", PAM_MSG_MEMBER(msg, i, msg));
			reply[i].resp_retcode = PAM_SUCCESS;
			break;
		default:
			goto fail;
		}
	}
	*resp = reply;
	return (PAM_SUCCESS);

 fail:
	for(i = 0; i < n; i++) {
		if (reply[i].resp != NULL)
			xfree(reply[i].resp);
	}
	xfree(reply);
	return (PAM_CONV_ERR);
}

static struct pam_conv tty_conv = { pam_tty_conv, NULL };

/*
 * XXX this should be done in the authentication phase, but ssh1 doesn't
 * support that
 */
void
do_pam_chauthtok(void)
{
	if (use_privsep)
		fatal("Password expired (unable to change with privsep)");
	sshpam_err = pam_set_item(sshpam_handle, PAM_CONV,
	    (const void *)&tty_conv);
	if (sshpam_err != PAM_SUCCESS)
		fatal("PAM: failed to set PAM_CONV: %s",
		    pam_strerror(sshpam_handle, sshpam_err));
	debug("PAM: changing password");
	sshpam_err = pam_chauthtok(sshpam_handle, PAM_CHANGE_EXPIRED_AUTHTOK);
	if (sshpam_err != PAM_SUCCESS)
		fatal("PAM: pam_chauthtok(): %s",
		    pam_strerror(sshpam_handle, sshpam_err));
}

static int
pam_store_conv(int n, const struct pam_message **msg,
    struct pam_response **resp, void *data)
{
	struct pam_response *reply;
	int i;
	size_t len;

	debug3("PAM: %s called with %d messages", __func__, n);
	*resp = NULL;

	if (n <= 0 || n > PAM_MAX_NUM_MSG)
		return (PAM_CONV_ERR);

	if ((reply = malloc(n * sizeof(*reply))) == NULL)
		return (PAM_CONV_ERR);
	memset(reply, 0, n * sizeof(*reply));

	for (i = 0; i < n; ++i) {
		switch (PAM_MSG_MEMBER(msg, i, msg_style)) {
		case PAM_ERROR_MSG:
		case PAM_TEXT_INFO:
			len = strlen(PAM_MSG_MEMBER(msg, i, msg));
			buffer_append(&loginmsg, PAM_MSG_MEMBER(msg, i, msg), len);
			buffer_append(&loginmsg, "\n", 1 );
			reply[i].resp_retcode = PAM_SUCCESS;
			break;
		default:
			goto fail;
		}
	}
	*resp = reply;
	return (PAM_SUCCESS);

 fail:
	for(i = 0; i < n; i++) {
		if (reply[i].resp != NULL)
			xfree(reply[i].resp);
	}
	xfree(reply);
	return (PAM_CONV_ERR);
}

static struct pam_conv store_conv = { pam_store_conv, NULL };

void
do_pam_session(void)
{
	const char *tai = NULL;    

	debug3("PAM: opening session");
	sshpam_err = pam_set_item(sshpam_handle, PAM_CONV,
	    (const void *)&store_conv);
	if (sshpam_err != PAM_SUCCESS)
		fatal("PAM: failed to set PAM_CONV: %s",
		    pam_strerror(sshpam_handle, sshpam_err));

        /*
         * In cahoots with our modified pam_unix, make sure
         * TRUSTED_AUTH_INFO gets set.  If we called pam_authenticate()
         * it would be set by the authenticate step of one of the PAM
         * modules.  But if we did our own authentication (either 
         * PasswordAuthentication, or ssh authorized keys), it will not
         * have gotten set.  pam_unix can set it for us, but we have to
         * signal it to do so, since it wants some indication that it's
         * in a trustworthy situation.
         *
         * The way we signal it is by setting TRUSTED_AUTH_INFO ourselves
         * to a special string that pam_unix will see.  This string is
         * defined as PAM_TRUSTED_AUTH_INFO_SET_STR.
         *
         * Of course, anyone (running as uid 0) could set this.  This is
         * not a security mechanism per se; more a way of preventing us
         * from setting TRUSTED_AUTH_INFO in unanticipated cases like
         * where a customer adds a new component.
         */
        tai = pam_getenv(sshpam_handle, "TRUSTED_AUTH_INFO");
        if (tai && strlen(tai)) {
            /*
             * It's already set, leave it alone.
             */
        }
        else {
            /*
             * Signal pam_unix.
             */
            do_pam_putenv("TRUSTED_AUTH_INFO", PAM_TRUSTED_AUTH_INFO_SET_STR);
        }

	sshpam_err = pam_open_session(sshpam_handle, 0);
	if (sshpam_err != PAM_SUCCESS)
		fatal("PAM: pam_open_session(): %s",
		    pam_strerror(sshpam_handle, sshpam_err));
	sshpam_session_open = 1;
}

/*
 * Set a PAM environment string. We need to do this so that the session
 * modules can handle things like Kerberos/GSI credentials that appear
 * during the ssh authentication process.
 */
int
do_pam_putenv(char *name, char *value)
{
	int ret = 1;
#ifdef HAVE_PAM_PUTENV
	char *compound;
	size_t len;

	len = strlen(name) + strlen(value) + 2;
	compound = xmalloc(len);

	snprintf(compound, len, "%s=%s", name, value);
	ret = pam_putenv(sshpam_handle, compound);
	xfree(compound);
#endif

	return (ret);
}

char **
fetch_pam_child_environment(void)
{
	return sshpam_env;
}

char **
fetch_pam_environment(void)
{
	return (pam_getenvlist(sshpam_handle));
}

void
free_pam_environment(char **env)
{
	char **envp;

	if (env == NULL)
		return;

	for (envp = env; *envp; envp++)
		xfree(*envp);
	xfree(env);
}

#endif /* USE_PAM */
