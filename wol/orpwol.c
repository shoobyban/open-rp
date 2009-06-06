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
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "config.h"

#define ORP_MAX_THREADS		256

extern int errno;

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

	while (!tinfo->terminate) {
		fprintf(stderr, "[%08x]: locking...\n", tinfo->id);
		pthread_mutex_lock(&tinfo->mutex);
		fprintf(stderr, "[%08x]: waiting...\n", tinfo->id);
		tinfo->state = STATE_WAIT;
		pthread_cond_wait(&tinfo->cond, &tinfo->mutex);
		fprintf(stderr, "[%08x]: busy: %s\n", tinfo->id,
			inet_ntoa(tinfo->client.sin_addr));
		tinfo->state = STATE_BUSY;

		int sd;
		if ((sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			fprintf(stderr, "[%08x]: socket: %s\n", tinfo->id, strerror(errno));
		}

		struct sockaddr_in sin;
		memset(&sin, 0, sizeof(struct sockaddr_in));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(sd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) < 0) {
			fprintf(stderr, "[%08x]: bind: %s\n", tinfo->id, strerror(errno));
		}

		tinfo->client.sin_port = htons(ORP_PORT);
		if (sendto(sd, tinfo->pkt, ORP_WOLPKT_LEN, 0,
			(struct sockaddr *)&tinfo->client, sizeof(struct sockaddr_in)) != ORP_WOLPKT_LEN) {
			fprintf(stderr, "[%08x]: sendto: %s\n", tinfo->id, strerror(errno));
		}
		close(sd);

		fprintf(stderr, "[%08x]: unlocking...\n", tinfo->id);
		pthread_mutex_unlock(&tinfo->mutex);
	}

	fprintf(stderr, "[%08x]: terminate...\n", tinfo->id);
	return NULL;
}

int main(int argc, char *argv[])
{
	int i;
	pthread_attr_t attr;
	struct thread_info *tinfo;

	int sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sd < 0) {
		fprintf(stderr, "socket: %s\n", strerror(errno));
		return -1;
	}

	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(ORP_PORT);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) < 0) {
		fprintf(stderr, "bind: %s\n", strerror(errno));
		return -1;
	}

	if (pthread_attr_init(&attr) != 0) {
		fprintf(stderr, "pthread_attr_init: %s\n", strerror(errno));
		return -1;
	}

	tinfo = calloc(ORP_MAX_THREADS, sizeof(struct thread_info));
	if (!tinfo) {
		fprintf(stderr, "calloc: %s\n", strerror(errno));
		return -1;
	}

	for (i = 0; i < ORP_MAX_THREADS; i++) {
		if (pthread_cond_init(&tinfo[i].cond, NULL)) {
			fprintf(stderr, "pthread_cond_init: %s\n", strerror(errno));
			return -1;
		}
		if (pthread_mutex_init(&tinfo[i].mutex, NULL)) {
			fprintf(stderr, "pthread_mutex_init: %s\n", strerror(errno));
			return -1;
		}
		if (pthread_create(&tinfo[i].id,
			&attr, &wol_reflect, &tinfo[i]) != 0) {
			fprintf(stderr, "pthread_create: %s\n", strerror(errno));
			return -1;
		}
	}

	if (pthread_attr_destroy(&attr) != 0) {
		fprintf(stderr, "pthread_attr_destroy: %s\n", strerror(errno));
		return -1;
	}

	while (1) {
		int c = 0;
		for (i = 0; i < ORP_MAX_THREADS; i++) {
			if (tinfo[i].state == STATE_WAIT) c++;
		}
		if (c == ORP_MAX_THREADS) break;
	}

	fprintf(stderr, "ready.\n");

	while (1) {
		ssize_t plen;
		socklen_t slen = sizeof(struct sockaddr_in);
		struct sockaddr_in client;
		memset(&client, 0, sizeof(struct sockaddr_in));
		unsigned char pkt[ORP_WOLPKT_LEN];
		if ((plen = recvfrom(sd, pkt, ORP_WOLPKT_LEN, 0,
			(struct sockaddr *)&client, &slen)) < 0) {
			fprintf(stderr, "recvfrom: %s\n", strerror(errno));
			return -1;
		}
		if (plen != ORP_WOLPKT_LEN) continue;
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
#if 0
	// TODO: Not implemented... being lazy.
	fprintf(stderr, "shutdown...\n");

	for (i = 0; i < ORP_MAX_THREADS; i++) {
		pthread_mutex_lock(&tinfo[i].mutex);
		tinfo[i].terminate = 1;
		pthread_cond_signal(&tinfo[i].cond);
		pthread_mutex_unlock(&tinfo[i].mutex);
		pthread_join(tinfo[i].id, NULL);
	}
#endif
	return 0;
}

// vi: ts=4
