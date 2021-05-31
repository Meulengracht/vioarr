/* MollenOS
 *
 * Copyright 2020, Philip Meulengracht
 *
 * This program is free software : you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ? , either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Vioarr - Vali Compositor
 * - Implements the default system compositor for Vali. It utilizies the gracht library
 *   for communication between compositor clients and the server. The server renders
 *   using Mesa3D with either the soft-renderer or llvmpipe render for improved performance.
 */

#ifndef __VIOARR_UTILS_H__
#define __VIOARR_UTILS_H__

#include <stdint.h>

#if defined(MOLLENOS)
//#define __TRACE
//#define VIOARR_TRACEMODE
#define VIOARR_LAUNCHER "heimdall.app"
//#define VIOARR_REVERSE_FB_BLIT // must be given to asm aswell

#include <ddk/utils.h>
#include <threads.h>

#define vioarr_utils_trace TRACE
#define vioarr_utils_error ERROR
#define VISTR(str)         str
#elif defined(_WIN32)
#include <stdio.h>

#define vioarr_utils_trace(...) fprintf (stdout, __VA_ARGS__)
#define vioarr_utils_error(...) fprintf (stderr, __VA_ARGS__)
#define VISTR(str)              str "\n"

#define ENTRY(...)
#define EXIT(func)

#define VIOARR_LAUNCHER "heimdall.exe"

#elif defined(__linux__)
#include <stdio.h>
#include <threads.h>

#define vioarr_utils_trace(...) fprintf (stdout, __VA_ARGS__)
#define vioarr_utils_error(...) fprintf (stderr, __VA_ARGS__)
#define VISTR(str)              str "\n"

#define ENTRY(...)              fprintf (stdout, __VA_ARGS__)
#define EXIT(func)              fprintf (stdout, func "\n")

#define VIOARR_LAUNCHER "heimdall"

#endif

typedef struct vioarr_rwlock {
    mtx_t sync_object;
    int   readers;
    cnd_t signal;
} vioarr_rwlock_t;

static void vioarr_rwlock_init(vioarr_rwlock_t* lock)
{
    mtx_init(&lock->sync_object, mtx_plain);
    cnd_init(&lock->signal);
    lock->readers = 0;
}

static void vioarr_rwlock_r_lock(vioarr_rwlock_t* lock)
{
    mtx_lock(&lock->sync_object);
    lock->readers++;
    mtx_unlock(&lock->sync_object);
}

static void vioarr_rwlock_r_unlock(vioarr_rwlock_t* lock)
{
    mtx_lock(&lock->sync_object);
    lock->readers--;
    if (!lock->readers) {
        cnd_signal(&lock->signal);
    }
    mtx_unlock(&lock->sync_object);
}

static void vioarr_rwlock_w_lock(vioarr_rwlock_t* lock)
{
    mtx_lock(&lock->sync_object);
    if (lock->readers) {
        cnd_wait(&lock->signal, &lock->sync_object);
    }
}

static void vioarr_rwlock_w_unlock(vioarr_rwlock_t* lock)
{
    mtx_unlock(&lock->sync_object);
    cnd_signal(&lock->signal);
}


#include <gracht/server.h>
extern gracht_server_t* vioarr_get_server_handle(void);

#endif //!__VIOARR_UTILS_H__
