/**
 * \file            gsm_sys_template.c
 * \brief           System dependant functions
 */

/*
 * Copyright (c) 2019 Tilen MAJERLE
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
 */
#include "system/gsm_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

/*******************************************/
/*******************************************/
/**   Modify this file for your system    **/
/*******************************************/
/*******************************************/

static SemaphoreHandle_t sys_mutex;                     /* Mutex ID for main protection */

/**
 * \brief           Init system dependant parameters
 * \note            Called from high-level application layer when required
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_init(void) {
    gsm_sys_mutex_create(&sys_mutex);           /* Create system mutex */
    return 1;
}

/**
 * \brief           Get current time in units of milliseconds
 * \return          Current time in units of milliseconds
 */
uint32_t
gsm_sys_now(void) {
    return xTaskGetTickCount();                   /* Get current tick in units of milliseconds */
}

/**
 * \brief           Protect stack core
 * \note            This function is required with OS
 *
 * \note            This function may be called multiple times, recursive protection is required
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_protect(void) {
    gsm_sys_mutex_lock(&sys_mutex);             /* Lock system and protect it */
    return 1;
}

/**
 * \brief           Protect stack core
 * \note            This function is required with OS
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_unprotect(void) {
    gsm_sys_mutex_unlock(&sys_mutex);           /* Release lock */
    return 1;
}

/**
 * \brief           Create a new mutex and pass it to input pointer
 * \note            This function is required with OS
 * \note            Recursive mutex must be created as it may be locked multiple times before unlocked
 * \param[out]      p: Pointer to mutex structure to save result to
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mutex_create(gsm_sys_mutex_t* p) {
    *p = xSemaphoreCreateRecursiveMutex(); /* Create recursive mutex */
    return !!*p;                                /* Return status */
}

/**
 * \brief           Delete mutex from OS
 * \note            This function is required with OS
 * \param[in]       p: Pointer to mutex structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mutex_delete(gsm_sys_mutex_t* p) {
    vSemaphoreDelete(*p);
    return pdPASS;           /* Delete mutex */
}

/**
 * \brief           Wait forever to lock the mutex
 * \note            This function is required with OS
 * \param[in]       p: Pointer to mutex structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mutex_lock(gsm_sys_mutex_t* p) {
    return xSemaphoreTakeRecursive(*p, portMAX_DELAY) == pdPASS; /* Wait forever for mutex */
}

/**
 * \brief           Unlock mutex
 * \note            This function is required with OS
 * \param[in]       p: Pointer to mutex structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mutex_unlock(gsm_sys_mutex_t* p) {
    return xSemaphoreGiveRecursive(*p) == pdPASS; /* Release mutex */
}

/**
 * \brief           Check if mutex structure is valid OS entry
 * \note            This function is required with OS
 * \param[in]       p: Pointer to mutex structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mutex_isvalid(gsm_sys_mutex_t* p) {
    return p != NULL && *p != NULL;             /* Check if mutex is valid */
}

/**
 * \brief           Set mutex structure as invalid
 * \note            This function is required with OS
 * \param[in]       p: Pointer to mutex structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mutex_invalid(gsm_sys_mutex_t* p) {
    *p = GSM_SYS_MUTEX_NULL;                    /* Set mutex as invalid */
    return 1;
}

/**
 * \brief           Create a new binary semaphore and set initial state
 * \note            Semaphore may only have 1 token available
 * \note            This function is required with OS
 * \param[out]      p: Pointer to semaphore structure to fill with result
 * \param[in]       cnt: Count indicating default semaphore state:
 *                     0: Lock it immediteally
 *                     1: Leave it unlocked
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_sem_create(gsm_sys_sem_t* p, uint8_t cnt) {
    *p = xSemaphoreCreateBinary();    /* Create semaphore with one token */

    if (*p && cnt) {                         /* We have valid entry */
        xSemaphoreGive(*p);                 /* UnLock semaphore */
    }
    return !!*p;
}

/**
 * \brief           Delete binary semaphore
 * \note            This function is required with OS
 * \param[in]       p: Pointer to semaphore structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_sem_delete(gsm_sys_sem_t* p) {
    vSemaphoreDelete(*p);
    return pdPASS;       /* Delete semaphore */
}

/**
 * \brief           Wait for semaphore to be available
 * \note            This function is required with OS
 * \param[in]       p: Pointer to semaphore structure
 * \param[in]       timeout: Timeout to wait in milliseconds. When 0 is applied, wait forever
 * \return          Number of milliseconds waited for semaphore to become available
 */
uint32_t
gsm_sys_sem_wait(gsm_sys_sem_t* p, uint32_t timeout) {
    uint32_t tick = xTaskGetTickCount();          /* Get start tick time */
    return (xSemaphoreTake(*p, !timeout ? portMAX_DELAY : pdMS_TO_TICKS(timeout)) == pdPASS) ? pdMS_TO_TICKS(xTaskGetTickCount() - tick) : GSM_SYS_TIMEOUT;    /* Wait for semaphore with specific time */
}

/**
 * \brief           Release semaphore
 * \note            This function is required with OS
 * \param[in]       p: Pointer to semaphore structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_sem_release(gsm_sys_sem_t* p) {
    return xSemaphoreGive(*p) == pdPASS;      /* Release semaphore */
}

/**
 * \brief           Check if semaphore is valid
 * \note            This function is required with OS
 * \param[in]       p: Pointer to semaphore structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_sem_isvalid(gsm_sys_sem_t* p) {
    return p != NULL && *p != NULL;             /* Check if valid */
}

/**
 * \brief           Invalid semaphore
 * \note            This function is required with OS
 * \param[in]       p: Pointer to semaphore structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_sem_invalid(gsm_sys_sem_t* p) {
    *p = GSM_SYS_SEM_NULL;                      /* Invaldiate semaphore */
    return 1;
}

/**
 * \brief           Create a new message queue with entry type of "void *"
 * \note            This function is required with OS
 * \param[out]      b: Pointer to message queue structure
 * \param[in]       size: Number of entries for message queue to hold
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mbox_create(gsm_sys_mbox_t* b, size_t size) {
    *b = xQueueCreate(size, sizeof(void*)); /* Create message box */
    return !!*b;
}

/**
 * \brief           Delete message queue
 * \note            This function is required with OS
 * \param[in]       b: Pointer to message queue structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mbox_delete(gsm_sys_mbox_t* b) {
    if (uxQueueMessagesWaiting(*b)) {                 /* We still have messages in queue, should not delete queue */
        return 0;                               /* Return error as we still have entries in message queue */
    }
    vQueueDelete(*b);
    return  pdPASS;         /* Delete message queue */
}

/**
 * \brief           Put a new entry to message queue and wait until memory available
 * \note            This function is required with OS
 * \param[in]       b: Pointer to message queue structure
 * \param[in]       m: Pointer to entry to insert to message queue
 * \return          Time in units of milliseconds needed to put a message to queue
 */
uint32_t
gsm_sys_mbox_put(gsm_sys_mbox_t* b, void* m) {
    uint32_t tick = xTaskGetTickCount();          /* Get start time */
    return xQueueSendToBack(*b, &m, portMAX_DELAY) == pdPASS ? pdMS_TO_TICKS(xTaskGetTickCount() - tick) : GSM_SYS_TIMEOUT; /* Put new message with forever timeout */
}

/**
 * \brief           Get a new entry from message queue with timeout
 * \note            This function is required with OS
 * \param[in]       b: Pointer to message queue structure
 * \param[in]       m: Pointer to pointer to result to save value from message queue to
 * \param[in]       timeout: Maximal timeout to wait for new message. When 0 is applied, wait for unlimited time
 * \return          Time in units of milliseconds needed to put a message to queue
 */
uint32_t
gsm_sys_mbox_get(gsm_sys_mbox_t* b, void** m, uint32_t timeout) {
    uint32_t time = xTaskGetTickCount();          /* Get current time */
    uint32_t res = xQueueReceive(*b, m, !timeout ? portMAX_DELAY : pdMS_TO_TICKS(timeout)) ;
    if (res == pdPASS) {
        return pdMS_TO_TICKS(xTaskGetTickCount - time);
    }
    *m = NULL;
    return GSM_SYS_TIMEOUT;
}

/**
 * \brief           Put a new entry to message queue without timeout (now or fail)
 * \note            This function is required with OS
 * \param[in]       b: Pointer to message queue structure
 * \param[in]       m: Pointer to message to save to queue
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mbox_putnow(gsm_sys_mbox_t* b, void* m) {
    return xQueueSendToBack(*b, &m, 0); /* Put new message without timeout */
}

/**
 * \brief           Get an entry from message queue immediatelly
 * \note            This function is required with OS
 * \param[in]       b: Pointer to message queue structure
 * \param[in]       m: Pointer to pointer to result to save value from message queue to
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mbox_getnow(gsm_sys_mbox_t* b, void** m) {
    return xQueueReceive(*b, m, 0) == pdPASS;
}

/**
 * \brief           Check if message queue is valid
 * \note            This function is required with OS
 * \param[in]       b: Pointer to message queue structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mbox_isvalid(gsm_sys_mbox_t* b) {
    return b != NULL && *b != NULL;             /* Return status if message box is valid */
}

/**
 * \brief           Invalid message queue
 * \note            This function is required with OS
 * \param[in]       b: Pointer to message queue structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_mbox_invalid(gsm_sys_mbox_t* b) {
    *b = GSM_SYS_MBOX_NULL;                     /* Invalidate message box */
    return 1;
}

/**
 * \brief           Create a new thread
 * \note            This function is required with OS
 * \param[out]      t: Pointer to thread identifier if create was successful
 * \param[in]       name: Name of a new thread
 * \param[in]       thread_func: Thread function to use as thread body
 * \param[in]       arg: Thread function argument
 * \param[in]       stack_size: Size of thread stack in uints of bytes. If set to 0, reserve default stack size
 * \param[in]       prio: Thread priority
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_thread_create(gsm_sys_thread_t* t, const char* name, gsm_sys_thread_fn thread_func, void* const arg, size_t stack_size, gsm_sys_thread_prio_t prio) {
    return xTaskCreate(thread_func, name, stack_size ? stack_size : GSM_SYS_THREAD_SS, arg, prio, t) == pdPASS; /* Create thread description */;
}

/**
 * \brief           Terminate thread (shut it down and remove)
 * \note            This function is required with OS
 * \param[in]       t: Pointer to thread handle to terminate. If set to NULL, terminate current thread (thread from where function is called)
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_thread_terminate(gsm_sys_thread_t* t) {
    vTaskDelete(*t);
    return 1;
}

/**
 * \brief           Yield current thread
 * \note            This function is required with OS
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_thread_yield(void) {
    return 1;
}
