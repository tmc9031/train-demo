/*
 * daemon.h
 *
 *  Created on: 2011-7-29
 *      Author: tmc
 */

#ifndef DAEMON_H_
#define DAEMON_H_


void (*set_signal(int signo,void(*func)(int)))(int);
void sig_routine(int signo);
void make_daemon();

#endif /* DAEMON_H_ */
