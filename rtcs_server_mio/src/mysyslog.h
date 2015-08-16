/*
 * mysyslog.h
 *
 *  Created on: 2011-7-31
 *      Author: tmc
 */

#ifndef MYSYSLOG_H_
#define MYSYSLOG_H_

void my_openlog();
void Error_log(const char *fmt,...);
void Warning_log(const char *fmt,...);
void Info_log(const char *fmt,...);

#endif /* MYSYSLOG_H_ */
