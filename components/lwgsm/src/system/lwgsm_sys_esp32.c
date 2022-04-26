/**
 * \file            lwgsm_sys_template.c
 * \brief           System dependant functions
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
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.0
 */
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "system/lwgsm_sys.h"

static SemaphoreHandle_t sys_mutex;

/**
 * \brief           Init system dependant parameters
 *
 * After this function is called,
 * all other system functions must be fully ready.
 *
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_init(void) {
    lwgsm_sys_mutex_create(&sys_mutex);
    return 1;
}

/**
 * \brief           Get current time in units of milliseconds
 * \return          Current time in units of milliseconds
 */
uint32_t lwgsm_sys_now(void) { return xTaskGetTickCount(); }

/**
 * \brief           Protect middleware core
 *
 * Stack protection must support recursive mode.
 * This function may be called multiple times,
 * even if access has been granted before.
 *
 * \note            Most operating systems support recursive mutexes.
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_protect(void) {
    lwgsm_sys_mutex_lock(&sys_mutex);
    return 1;
}

/**
 * \brief           Unprotect middleware core
 *
 * This function must follow number of calls of \ref lwgsm_sys_protect
 * and unlock access only when counter reached back zero.
 *
 * \note            Most operating systems support recursive mutexes.
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_unprotect(void) {
    lwgsm_sys_mutex_unlock(&sys_mutex);
    return 1;
}

/**
 * \brief           Create new recursive mutex
 * \note            Recursive mutex has to be created as it may be locked
 * multiple times before unlocked \param[out]      p: Pointer to mutex structure
 * to allocate \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_mutex_create(lwgsm_sys_mutex_t* p) {
    *p = xSemaphoreCreateRecursiveMutex();
    return *p != NULL;
}

/**
 * \brief           Delete recursive mutex from system
 * \param[in]       p: Pointer to mutex structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_mutex_delete(lwgsm_sys_mutex_t* p) {
    vSemaphoreDelete(*p);
    return pdPASS;
}

/**
 * \brief           Lock recursive mutex, wait forever to lock
 * \param[in]       p: Pointer to mutex structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_mutex_lock(lwgsm_sys_mutex_t* p) {
    return xSemaphoreTakeRecursive(*p, portMAX_DELAY) == pdPASS;
}

/**
 * \brief           Unlock recursive mutex
 * \param[in]       p: Pointer to mutex structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_mutex_unlock(lwgsm_sys_mutex_t* p) {
    return xSemaphoreGiveRecursive(*p) == pdPASS; /* Release mutex */
}

/**
 * \brief           Check if mutex structure is valid system
 * \param[in]       p: Pointer to mutex structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_mutex_isvalid(lwgsm_sys_mutex_t* p) {
    return p != NULL && *p != NULL;
}

/**
 * \brief           Set recursive mutex structure as invalid
 * \param[in]       p: Pointer to mutex structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_mutex_invalid(lwgsm_sys_mutex_t* p) {
    *p = LWGSM_SYS_MUTEX_NULL;
    return 1;
}

/**
 * \brief           Create a new binary semaphore and set initial state
 * \note            Semaphore may only have `1` token available
 * \param[out]      p: Pointer to semaphore structure to fill with result
 * \param[in]       cnt: Count indicating default semaphore state:
 *                     `0`: Take semaphore token immediately
 *                     `1`: Keep token available
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_sem_create(lwgsm_sys_sem_t* p, uint8_t cnt) {
    *p = xSemaphoreCreateBinary(); /* Create semaphore with one token */

    if (*p && cnt) {        /* We have valid entry */
        xSemaphoreGive(*p); /* UnLock semaphore */
    }
    return !!*p;
}

/**
 * \brief           Delete binary semaphore
 * \param[in]       p: Pointer to semaphore structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_sem_delete(lwgsm_sys_sem_t* p) {
    vSemaphoreDelete(*p);
    return pdPASS; /* Delete semaphore */
}

/**
 * \brief           Wait for semaphore to be available
 * \param[in]       p: Pointer to semaphore structure
 * \param[in]       timeout: Timeout to wait in milliseconds. When `0` is
 * applied, wait forever \return          Number of milliseconds waited for
 * semaphore to become available or \ref LWGSM_SYS_TIMEOUT if not available
 * within given time
 */
uint32_t lwgsm_sys_sem_wait(lwgsm_sys_sem_t* p, uint32_t timeout) {
   uint32_t tick = xTaskGetTickCount();          /* Get start tick time */
    return (xSemaphoreTake(*p, !timeout ? portMAX_DELAY : pdMS_TO_TICKS(timeout)) == pdPASS) ? pdMS_TO_TICKS(xTaskGetTickCount() - tick) : LWGSM_SYS_TIMEOUT;    /* Wait for semaphore with specific time */
}

/**
 * \brief           Release semaphore
 * \param[in]       p: Pointer to semaphore structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_sem_release(lwgsm_sys_sem_t* p) {
    return xSemaphoreGive(*p) == pdPASS;      /* Release semaphore */
}

/**
 * \brief           Check if semaphore is valid
 * \param[in]       p: Pointer to semaphore structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_sem_isvalid(lwgsm_sys_sem_t* p) {
    return p != NULL && *p != NULL;             /* Check if valid */
}

/**
 * \brief           Invalid semaphore
 * \param[in]       p: Pointer to semaphore structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_sem_invalid(lwgsm_sys_sem_t* p) {
    *p = LWGSM_SYS_SEM_NULL;
    return 1;
}

/**
 * \brief           Create a new message queue with entry type of `void *`
 * \param[out]      b: Pointer to message queue structure
 * \param[in]       size: Number of entries for message queue to hold
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_mbox_create(lwgsm_sys_mbox_t* b, size_t size) {
    *b = xQueueCreate(size, sizeof(void*)); /* Create message box */
    return !!*b;
}

/**
 * \brief           Delete message queue
 * \param[in]       b: Pointer to message queue structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_mbox_delete(lwgsm_sys_mbox_t* b) {
    if (uxQueueMessagesWaiting(*b)) {              
        return 0;                               
    }
    vQueueDelete(*b);
    return  pdPASS;      
}

/**
 * \brief           Put a new entry to message queue and wait until memory
 * available \param[in]       b: Pointer to message queue structure \param[in]
 * m: Pointer to entry to insert to message queue \return          Time in units
 * of milliseconds needed to put a message to queue
 */
uint32_t lwgsm_sys_mbox_put(lwgsm_sys_mbox_t* b, void* m) {    
    uint32_t tick = xTaskGetTickCount();          /* Get start time */
    return xQueueSendToBack(*b, &m, portMAX_DELAY) == pdPASS ? pdMS_TO_TICKS(xTaskGetTickCount() - tick) : LWGSM_SYS_TIMEOUT; /* Put new message with forever timeout */
}

/**
 * \brief           Get a new entry from message queue with timeout
 * \param[in]       b: Pointer to message queue structure
 * \param[in]       m: Pointer to pointer to result to save value from message
 * queue to \param[in]       timeout: Maximal timeout to wait for new message.
 * When `0` is applied, wait for unlimited time \return          Time in units
 * of milliseconds needed to put a message to queue or \ref LWGSM_SYS_TIMEOUT if
 * it was not successful
 */
uint32_t lwgsm_sys_mbox_get(lwgsm_sys_mbox_t* b, void** m, uint32_t timeout) {
    uint32_t time = xTaskGetTickCount();          /* Get current time */
    uint32_t res = xQueueReceive(*b, m, !timeout ? portMAX_DELAY : pdMS_TO_TICKS(timeout)) ;
    if (res == pdPASS) {
        return pdMS_TO_TICKS(xTaskGetTickCount - time);
    }
    *m = NULL;
    return LWGSM_SYS_TIMEOUT;
}

/**
 * \brief           Put a new entry to message queue without timeout (now or
 * fail) \param[in]       b: Pointer to message queue structure \param[in] m:
 * Pointer to message to save to queue \return          `1` on success, `0`
 * otherwise
 */
uint8_t lwgsm_sys_mbox_putnow(lwgsm_sys_mbox_t* b, void* m) {
    return xQueueSendToBack(*b, &m, 0); /* Put new message without timeout */
}

/**
 * \brief           Get an entry from message queue immediately
 * \param[in]       b: Pointer to message queue structure
 * \param[in]       m: Pointer to pointer to result to save value from message
 * queue to \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_mbox_getnow(lwgsm_sys_mbox_t* b, void** m) {
    return xQueueReceive(*b, m, 0) == pdPASS;
}

/**
 * \brief           Check if message queue is valid
 * \param[in]       b: Pointer to message queue structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_mbox_isvalid(lwgsm_sys_mbox_t* b) {
    return b != NULL && *b != NULL;
}

/**
 * \brief           Invalid message queue
 * \param[in]       b: Pointer to message queue structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_mbox_invalid(lwgsm_sys_mbox_t* b) {
    *b = LWGSM_SYS_MBOX_NULL;
    return 1;
}

/**
 * \brief           Create a new thread
 * \param[out]      t: Pointer to thread identifier if create was successful.
 *                     It may be set to `NULL`
 * \param[in]       name: Name of a new thread
 * \param[in]       thread_func: Thread function to use as thread body
 * \param[in]       arg: Thread function argument
 * \param[in]       stack_size: Size of thread stack in uints of bytes. If set
 * to 0, reserve default stack size \param[in]       prio: Thread priority
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_thread_create(lwgsm_sys_thread_t* t, const char* name,
                                lwgsm_sys_thread_fn thread_func,
                                void* const arg, size_t stack_size,
                                lwgsm_sys_thread_prio_t prio) {
    return xTaskCreate(thread_func, name, stack_size ? stack_size : LWGSM_SYS_THREAD_SS, arg, prio, t) == pdPASS; /* Create thread description */;
}

/**
 * \brief           Terminate thread (shut it down and remove)
 * \param[in]       t: Pointer to thread handle to terminate.
 *                      If set to `NULL`, terminate current thread (thread from
 * where function is called) \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_thread_terminate(lwgsm_sys_thread_t* t) {
    vTaskDelete(*t);
    return 1;
}

/**
 * \brief           Yield current thread
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsm_sys_thread_yield(void) {
    
    return 1;
}
