//////////////////////////////////////////////////////////////////////////////
//
// Open Remote Play
// http://ps3-hacks.com
//
//////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but 
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ORP_PSP
typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;
#include "config.h"

extern int errno;

struct client_stats_t
{
	struct timeval tv;
	struct in_addr in;
	struct client_stats_t *next;
	struct client_stats_t *prev;
};

static struct client_stats_t *client_stats = NULL;

static int client_update(struct in_addr *in)
{
	struct client_stats_t *stats = client_stats;
	struct timeval tv;
	gettimeofday(&tv, NULL);

	while (stats) {
		if (tv.tv_sec - stats->tv.tv_sec > ORP_MAX_REQUEST) {
			if (stats->prev == NULL) {
				client_stats = stats->next;
				if (stats->next) stats->next->prev = NULL;
			} else {
				stats->prev->next = stats->next;
				if (stats->next) stats->next->prev = stats->prev;
			}

			free(stats);
			stats = client_stats;
			continue;
		}
		if (!memcmp(&stats->in, in, sizeof(struct in_addr))) {
			if (tv.tv_sec - stats->tv.tv_sec < ORP_MAX_REQUEST) {
				memcpy(&stats->tv, &tv, sizeof(struct timeval));
				syslog(LOG_DAEMON | LOG_WARNING,
					"throttle: %s", inet_ntoa(*in));
				return -1;
			} else {
				memcpy(&stats->tv, &tv, sizeof(struct timeval));
				return 0;
			}
		}

		stats = stats->next;
	}

	stats = malloc(sizeof(struct client_stats_t));
	if (!stats) return -1;

	memcpy(&stats->tv, &tv, sizeof(struct timeval));
	memcpy(&stats->in, in, sizeof(struct in_addr));
	stats->prev = NULL;
	stats->next = client_stats;
	if (client_stats) client_stats->prev = stats;
	client_stats = stats;

	return 0;
}

static void client_dump(void)
{
	struct client_stats_t *stats = client_stats;
	struct timeval tv;
	gettimeofday(&tv, NULL);

	while (stats) {
		syslog(LOG_DAEMON | LOG_DEBUG, "%s: %d", inet_ntoa(stats->in),
			tv.tv_sec - stats->tv.tv_sec);
		stats = stats->next;
	}
}

enum thread_state
{
	STATE_INIT,
	STATE_WAIT,
	STATE_BUSY
};

struct thread_info
{
	pthread_t id;
	struct sockaddr_in client;
	unsigned char pkt[ORP_WOLPKT_LEN];
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	int terminate;
	enum thread_state state;
};

static void *wol_reflect(void *arg)
{
	struct thread_info *tinfo = (struct thread_info *)arg;
	unsigned char magic[ORP_MAC_LEN];
	memset(magic, 0xFF, ORP_MAC_LEN);

	while (!tinfo->terminate) {
		pthread_mutex_lock(&tinfo->mutex);
		tinfo->state = STATE_WAIT;
		pthread_cond_wait(&tinfo->cond, &tinfo->mutex);
		if (tinfo->terminate) {
			pthread_mutex_unlock(&tinfo->mutex);
			break;
		}
		tinfo->state = STATE_BUSY;

		if (memcmp(tinfo->pkt, magic, ORP_MAC_LEN)) {
			pthread_mutex_unlock(&tinfo->mutex);
			continue;
		}

		syslog(LOG_DAEMON | LOG_INFO, "[%08x]: %s: %02x:%02x:%02x:%02x:%02x:%02x",
			tinfo->id, inet_ntoa(tinfo->client.sin_addr),
			tinfo->pkt[ORP_MAC_LEN + 0], tinfo->pkt[ORP_MAC_LEN + 1],
			tinfo->pkt[ORP_MAC_LEN + 2], tinfo->pkt[ORP_MAC_LEN + 3],
			tinfo->pkt[ORP_MAC_LEN + 4], tinfo->pkt[ORP_MAC_LEN + 5]);

		int sd;
		if ((sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			syslog(LOG_DAEMON | LOG_ERR, "[%08x]: socket: %s", tinfo->id, strerror(errno));
			pthread_mutex_unlock(&tinfo->mutex);
			continue;
		}

		struct sockaddr_in sin;
		memset(&sin, 0, sizeof(struct sockaddr_in));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(sd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) < 0) {
			syslog(LOG_DAEMON | LOG_ERR, "[%08x]: bind: %s", tinfo->id, strerror(errno));
			close(sd);
			pthread_mutex_unlock(&tinfo->mutex);
			continue;
		}

		tinfo->client.sin_port = htons(ORP_PORT);
		if (sendto(sd, tinfo->pkt, ORP_WOLPKT_LEN, 0,
			(struct sockaddr *)&tinfo->client, sizeof(struct sockaddr_in)) != ORP_WOLPKT_LEN) {
			syslog(LOG_DAEMON | LOG_ERR, "[%08x]: sendto: %s", tinfo->id, strerror(errno));
		}

		close(sd);
		pthread_mutex_unlock(&tinfo->mutex);
	}

	return NULL;
}

static int last_signal = 0;
static void sighandler(int sig) { last_signal = sig; }

int main(int argc, char *argv[])
{
#ifndef ORP_DEBUG
	daemon(0, 0);
	openlog("orpwol", LOG_PID, LOG_DAEMON);
#else
	openlog("orpwol", LOG_PERROR, LOG_DAEMON);
#endif
	int sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sd < 0) {
		syslog(LOG_DAEMON | LOG_ERR, "socket: %s", strerror(errno));
		return -1;
	}

	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(ORP_PORT);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) < 0) {
		syslog(LOG_DAEMON | LOG_ERR, "bind: %s", strerror(errno));
		return -1;
	}

	pthread_attr_t attr;
	if (pthread_attr_init(&attr) != 0) {
		syslog(LOG_DAEMON | LOG_ERR, "pthread_attr_init: %s", strerror(errno));
		return -1;
	}
	if (pthread_attr_setstacksize(&attr, ORP_STACK_SIZE) != 0) {
		syslog(LOG_DAEMON | LOG_ERR, "pthread_attr_setstacksize(0x%08x): %s",
			ORP_STACK_SIZE, strerror(errno));
		return -1;
	}

	struct thread_info *tinfo;
	tinfo = calloc(ORP_MAX_THREADS, sizeof(struct thread_info));
	if (!tinfo) {
		syslog(LOG_DAEMON | LOG_ERR, "calloc: %s", strerror(errno));
		return -1;
	}

	int i;
	for (i = 0; i < ORP_MAX_THREADS; i++) {
		if (pthread_cond_init(&tinfo[i].cond, NULL)) {
			syslog(LOG_DAEMON | LOG_ERR, "pthread_cond_init: %s", strerror(errno));
			return -1;
		}
		if (pthread_mutex_init(&tinfo[i].mutex, NULL)) {
			syslog(LOG_DAEMON | LOG_ERR, "pthread_mutex_init: %s", strerror(errno));
			return -1;
		}
		if (pthread_create(&tinfo[i].id,
			&attr, &wol_reflect, &tinfo[i]) != 0) {
			syslog(LOG_DAEMON | LOG_ERR, "pthread_create: %s", strerror(errno));
			return -1;
		}
	}

	if (pthread_attr_destroy(&attr) != 0) {
		syslog(LOG_DAEMON | LOG_ERR, "pthread_attr_destroy: %s", strerror(errno));
		return -1;
	}

	while (1) {
		int c = 0;
		for (i = 0; i < ORP_MAX_THREADS; i++) {
			if (tinfo[i].state == STATE_WAIT) c++;
		}
		if (c == ORP_MAX_THREADS) break;
	}

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGHUP, sighandler);

	syslog(LOG_DAEMON | LOG_INFO, "ready.");

	fd_set fds;
	struct timeval tv;
	ssize_t plen;
	struct sockaddr_in client;
	unsigned char pkt[ORP_WOLPKT_LEN];
	while (!last_signal) {
		FD_ZERO(&fds);
		FD_SET(sd, &fds);
		tv.tv_sec = 0; tv.tv_usec = 1500;
		if (select(sd + 1, &fds, NULL, NULL, &tv) != 1) continue;

		socklen_t slen = sizeof(struct sockaddr_in);
		if ((plen = recvfrom(sd, pkt, ORP_WOLPKT_LEN, 0,
			(struct sockaddr *)&client, &slen)) < 0) {
			syslog(LOG_DAEMON | LOG_ERR, "recvfrom: %s", strerror(errno));
			return -1;
		}

		if (plen != ORP_WOLPKT_LEN) continue;
		if (client_update(&client.sin_addr) != 0) continue;

		for (i = 0; i < ORP_MAX_THREADS; i++) {
			if (tinfo[i].state != STATE_WAIT) continue;
			pthread_mutex_lock(&tinfo[i].mutex);
			memcpy(&tinfo[i].client, &client, sizeof(struct sockaddr_in));
			memcpy(tinfo[i].pkt, pkt, ORP_WOLPKT_LEN);
			pthread_cond_signal(&tinfo[i].cond);
			pthread_mutex_unlock(&tinfo[i].mutex);
			break;
		}
	}

	syslog(LOG_DAEMON | LOG_INFO, "shutdown...");

	for (i = 0; i < ORP_MAX_THREADS; i++) {
		pthread_mutex_lock(&tinfo[i].mutex);
		tinfo[i].terminate = 1;
		pthread_cond_signal(&tinfo[i].cond);
		pthread_mutex_unlock(&tinfo[i].mutex);
		pthread_join(tinfo[i].id, NULL);
		pthread_cond_destroy(&tinfo[i].cond);
		pthread_mutex_destroy(&tinfo[i].mutex);
	}

	close(sd);
	free(tinfo);
	closelog();

	return 0;
}

// vi: ts=4
