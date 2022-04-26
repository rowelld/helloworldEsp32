/**
 * \file            lwgsm_opts_template.h
 * \brief           Template config file
 */

/*
 * Copyright (c) 2020 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwGSM - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.0
 */
#ifndef COMPONENTS_LWGSM_SRC_INCLUDE_LWGSM_LWGSM_OPTS
#define COMPONENTS_LWGSM_SRC_INCLUDE_LWGSM_LWGSM_OPTS

/* Rename this file to "lwgsm_opts.h" for your application */
#define LWGSM_CFG_DBG                           LWGSM_DBG_ON
#define LWGSM_CFG_DBG_TYPES_ON                  LWGSM_DBG_TYPE_TRACE | LWGSM_DBG_TYPE_STATE
#define LWGSM_CFG_DBG_MQTT                      LWGSM_DBG_OFF
#define LWGSM_CFG_DBG_INIT                      LWGSM_DBG_ON


#define LWGSM_CFG_DBG_OUT(fmt, ...)             do { extern int printf( const char * format, ... ); printf(fmt, ## __VA_ARGS__); } while (0)

#define LWGSM_CFG_NETWORK                       1
#define LWGSM_CFG_NETCONN                       1
// #define LWGSM_CFG_MEM_CUSTOM                    1

#define LWGSM_CFG_SMS                           1
#define LWGSM_CFG_CALL                          1
#define LWGSM_CFG_CONN                          1

/*
 * Open "include/lwgsm/lwgsm_opt.h" and
 * copy & replace here settings you want to change values
 */

#endif /* COMPONENTS_LWGSM_SRC_INCLUDE_LWGSM_LWGSM_OPTS */
