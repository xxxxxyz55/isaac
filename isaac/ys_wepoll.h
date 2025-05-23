#ifndef _YS_WEPOLL_H_
#define _YS_WEPOLL_H_
// 兼容windows 的epoll

#define YS_WEPOLL 1

#if __linux__
#include <sys/epoll.h>
#elif _WIN32
#include <stdint.h>

enum EPOLL_EVENTS
{
    EPOLLIN      = (int)(1U << 0),
    EPOLLPRI     = (int)(1U << 1),
    EPOLLOUT     = (int)(1U << 2),
    EPOLLERR     = (int)(1U << 3),
    EPOLLHUP     = (int)(1U << 4),
    EPOLLRDNORM  = (int)(1U << 6),
    EPOLLRDBAND  = (int)(1U << 7),
    EPOLLWRNORM  = (int)(1U << 8),
    EPOLLWRBAND  = (int)(1U << 9),
    EPOLLMSG     = (int)(1U << 10), /* Never reported. */
    EPOLLRDHUP   = (int)(1U << 13),
    EPOLLONESHOT = (int)(1U << 31)
};

#define EPOLLIN      (1U << 0)
#define EPOLLPRI     (1U << 1)
#define EPOLLOUT     (1U << 2)
#define EPOLLERR     (1U << 3)
#define EPOLLHUP     (1U << 4)
#define EPOLLRDNORM  (1U << 6)
#define EPOLLRDBAND  (1U << 7)
#define EPOLLWRNORM  (1U << 8)
#define EPOLLWRBAND  (1U << 9)
#define EPOLLMSG     (1U << 10)
#define EPOLLRDHUP   (1U << 13)
#define EPOLLONESHOT (1U << 31)

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_MOD 2
#define EPOLL_CTL_DEL 3

typedef void     *HANDLE;
typedef uintptr_t SOCKET;

typedef union epoll_data
{
    void    *ptr;
    int      fd;
    uint32_t u32;
    uint64_t u64;
    SOCKET   sock; /* Windows specific */
    HANDLE   hnd;  /* Windows specific */
} epoll_data_t;

struct epoll_event
{
    uint32_t     events; /* Epoll events and flags */
    epoll_data_t data;   /* User data variable */
};

#ifdef YS_WEPOLL

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnonportable-system-include-path"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined(_MSC_VER)
#pragma warning(push, 1)
#endif

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif

class YS_Wepoll
{
public:
    typedef LONG      NTSTATUS;
    typedef NTSTATUS *PNTSTATUS;

#ifndef NT_SUCCESS
#define NT_SUCCESS(status) (((NTSTATUS)(status)) >= 0)
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif

#ifndef STATUS_PENDING
#define STATUS_PENDING ((NTSTATUS)0x00000103L)
#endif

#ifndef STATUS_CANCELLED
#define STATUS_CANCELLED ((NTSTATUS)0xC0000120L)
#endif

#ifndef STATUS_NOT_FOUND
#define STATUS_NOT_FOUND ((NTSTATUS)0xC0000225L)
#endif

    typedef struct _IO_STATUS_BLOCK
    {
        NTSTATUS  Status;
        ULONG_PTR Information;
    } IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

    typedef VOID(NTAPI *PIO_APC_ROUTINE)(PVOID            ApcContext,
                                         PIO_STATUS_BLOCK IoStatusBlock,
                                         ULONG            Reserved);

    typedef struct _UNICODE_STRING
    {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING, *PUNICODE_STRING;

#define RTL_CONSTANT_STRING(s)                          \
    {                                                   \
        sizeof(s) - sizeof((s)[0]), sizeof(s), (PWSTR)s \
    }

    typedef struct _OBJECT_ATTRIBUTES
    {
        ULONG           Length;
        HANDLE          RootDirectory;
        PUNICODE_STRING ObjectName;
        ULONG           Attributes;
        PVOID           SecurityDescriptor;
        PVOID           SecurityQualityOfService;
    } OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

#define RTL_CONSTANT_OBJECT_ATTRIBUTES(ObjectName, Attributes)                       \
    {                                                                                \
        sizeof(OBJECT_ATTRIBUTES), nullptr, ObjectName, Attributes, nullptr, nullptr \
    }

#ifndef FILE_OPEN
#define FILE_OPEN 0x00000001UL
#endif

#define KEYEDEVENT_WAIT 0x00000001UL
#define KEYEDEVENT_WAKE 0x00000002UL
#define KEYEDEVENT_ALL_ACCESS \
    (STANDARD_RIGHTS_REQUIRED | KEYEDEVENT_WAIT | KEYEDEVENT_WAKE)

#define AFD_POLL_RECEIVE           0x0001
#define AFD_POLL_RECEIVE_EXPEDITED 0x0002
#define AFD_POLL_SEND              0x0004
#define AFD_POLL_DISCONNECT        0x0008
#define AFD_POLL_ABORT             0x0010
#define AFD_POLL_LOCAL_CLOSE       0x0020
#define AFD_POLL_ACCEPT            0x0080
#define AFD_POLL_CONNECT_FAIL      0x0100

    typedef struct _AFD_POLL_HANDLE_INFO
    {
        HANDLE   Handle;
        ULONG    Events;
        NTSTATUS Status;
    } AFD_POLL_HANDLE_INFO, *PAFD_POLL_HANDLE_INFO;

    typedef struct _AFD_POLL_INFO
    {
        LARGE_INTEGER        Timeout;
        ULONG                NumberOfHandles;
        ULONG                Exclusive;
        AFD_POLL_HANDLE_INFO Handles[1];
    } AFD_POLL_INFO, *PAFD_POLL_INFO;

#define return_map_error(value) \
    do                          \
    {                           \
        err_map_win_error();    \
        return (value);         \
    } while (0)

#define return_set_error(value, error) \
    do                                 \
    {                                  \
        err_set_win_error(error);      \
        return (value);                \
    } while (0)

#define IOCTL_AFD_POLL 0x00012024

    struct YS_WepollPf
    {
        NTSTATUS(*NtCancelIoFileEx)
        (HANDLE FileHandle, PIO_STATUS_BLOCK IoRequestToCancel, PIO_STATUS_BLOCK IoStatusBlock);
        NTSTATUS(*NtCreateFile)
        (PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength);
        NTSTATUS(*NtCreateKeyedEvent)
        (PHANDLE KeyedEventHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, ULONG Flags);
        NTSTATUS(*NtDeviceIoControlFile)
        (HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, ULONG IoControlCode, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength);
        NTSTATUS(*NtReleaseKeyedEvent)
        (HANDLE KeyedEventHandle, PVOID KeyValue, BOOLEAN Alertable, PLARGE_INTEGER Timeout);
        NTSTATUS(*NtWaitForKeyedEvent)
        (HANDLE KeyedEventHandle, PVOID KeyValue, BOOLEAN Alertable, PLARGE_INTEGER Timeout);
        ULONG(*RtlNtStatusToDosError)
        (NTSTATUS Status);
    };

    static YS_WepollPf &getExternPf()
    {
        static YS_WepollPf tPf = {0};
        return tPf;
    }

    static OBJECT_ATTRIBUTES *getDeviceAttribute()
    {
        static UNICODE_STRING afd__device_name =
            RTL_CONSTANT_STRING(L"\\Device\\Afd\\Wepoll");

        static OBJECT_ATTRIBUTES afd__device_attributes =
            RTL_CONSTANT_OBJECT_ATTRIBUTES(&afd__device_name, 0);
        return &afd__device_attributes;
    }

    static int afd_create_device_handle(HANDLE  iocp_handle,
                                        HANDLE *afd_device_handle_out)
    {
        HANDLE          afd_device_handle;
        IO_STATUS_BLOCK iosb;
        NTSTATUS        status;

        /* By opening \Device\Afd without specifying any extended attributes, we'll
         * get a handle that lets us talk to the AFD driver, but that doesn't have an
         * associated endpoint (so it's not a socket). */
        status = getExternPf().NtCreateFile(&afd_device_handle,
                                            SYNCHRONIZE,
                                            getDeviceAttribute(),
                                            &iosb,
                                            nullptr,
                                            0,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                                            FILE_OPEN,
                                            0,
                                            nullptr,
                                            0);
        if (status != STATUS_SUCCESS)
            return_set_error(-1, getExternPf().RtlNtStatusToDosError(status));

        if (CreateIoCompletionPort(afd_device_handle, iocp_handle, 0, 0) == nullptr)
            goto error;

        if (!SetFileCompletionNotificationModes(afd_device_handle,
                                                FILE_SKIP_SET_EVENT_ON_HANDLE))
            goto error;

        *afd_device_handle_out = afd_device_handle;
        return 0;

    error:
        CloseHandle(afd_device_handle);
        return_map_error(-1);
    }

    static int afd_poll(HANDLE           afd_device_handle,
                        AFD_POLL_INFO   *poll_info,
                        IO_STATUS_BLOCK *io_status_block)
    {
        NTSTATUS status;

        /* Blocking operation is not supported. */
        assert(io_status_block != nullptr);

        io_status_block->Status = STATUS_PENDING;
        status                  = getExternPf().NtDeviceIoControlFile(afd_device_handle,
                                                                      nullptr,
                                                                      nullptr,
                                                                      io_status_block,
                                                                      io_status_block,
                                                                      IOCTL_AFD_POLL,
                                                                      poll_info,
                                                                      sizeof *poll_info,
                                                                      poll_info,
                                                                      sizeof *poll_info);

        if (status == STATUS_SUCCESS)
            return 0;
        else if (status == STATUS_PENDING)
            return_set_error(-1, ERROR_IO_PENDING);
        else
            return_set_error(-1, getExternPf().RtlNtStatusToDosError(status));
    }

    static int afd_cancel_poll(HANDLE           afd_device_handle,
                               IO_STATUS_BLOCK *io_status_block)
    {
        NTSTATUS        cancel_status;
        IO_STATUS_BLOCK cancel_iosb;

        /* If the poll operation has already completed or has been cancelled earlier,
         * there's nothing left for us to do. */
        if (io_status_block->Status != STATUS_PENDING)
            return 0;

        cancel_status =
            getExternPf().NtCancelIoFileEx(afd_device_handle, io_status_block, &cancel_iosb);

        /* NtCancelIoFileEx() may return STATUS_NOT_FOUND if the operation completed
         * just before calling NtCancelIoFileEx(). This is not an error. */
        if (cancel_status == STATUS_SUCCESS || cancel_status == STATUS_NOT_FOUND)
            return 0;
        else
            return_set_error(-1, getExternPf().RtlNtStatusToDosError(cancel_status));
    }

    struct tree_node_t
    {
        tree_node_t *left;
        tree_node_t *right;
        tree_node_t *parent;
        uintptr_t    key;
        bool         red;
    };

    struct tree_t
    {
        tree_node_t *root;
    };

    struct queue_node_t
    {
        queue_node_t *prev;
        queue_node_t *next;
    };

    typedef struct queue
    {
        queue_node_t head;
    } queue_t;

    typedef struct ts_tree
    {
        tree_t  tree;
        SRWLOCK lock;
    } ts_tree_t;

    typedef struct reflock
    {
        volatile long state; /* 32-bit Interlocked APIs operate on `long` values. */
    } reflock_t;

    typedef struct ts_tree_node
    {
        tree_node_t tree_node;
        reflock_t   reflock;
    } ts_tree_node_t;

    struct port_state_t
    {
        HANDLE           iocp_handle;
        tree_t           sock_tree;
        queue_t          sock_update_queue;
        queue_t          sock_deleted_queue;
        queue_t          poll_group_queue;
        ts_tree_node_t   handle_tree_node;
        CRITICAL_SECTION lock;
        size_t           active_poll_count;
    };

    struct poll_group_t
    {
        port_state_t *port_state;
        queue_node_t  queue_node;
        HANDLE        afd_device_handle;
        size_t        group_size;
    };

    typedef enum sock__poll_status
    {
        SOCK__POLL_IDLE = 0,
        SOCK__POLL_PENDING,
        SOCK__POLL_CANCELLED
    } sock__poll_status_t;

    typedef struct sock_state
    {
        IO_STATUS_BLOCK     io_status_block;
        AFD_POLL_INFO       poll_info;
        queue_node_t        queue_node;
        tree_node_t         tree_node;
        poll_group_t       *poll_group;
        SOCKET              base_socket;
        epoll_data_t        user_data;
        uint32_t            user_events;
        uint32_t            pending_events;
        sock__poll_status_t poll_status;
        bool                delete_pending;
    } sock_state_t;

    /* N.b.: the tree functions do not set errno or LastError when they fail. Each
     * of the API functions has at most one failure mode. It is up to the caller to
     * set an appropriate error code when necessary. */

    static ts_tree_t &getEpollHandleTree()
    {
        static ts_tree_t epoll__handle_tree;
        return epoll__handle_tree;
    }

    static int epoll_global_init(void)
    {
        ts_tree_init(&getEpollHandleTree());
        return 0;
    }

    static HANDLE epoll__create(void)
    {
        port_state_t   *port_state;
        HANDLE          ephnd;
        ts_tree_node_t *tree_node;

        if (init() < 0)
            return nullptr;

        port_state = port_new(&ephnd);
        if (port_state == nullptr)
            return nullptr;

        tree_node = port_state_to_handle_tree_node(port_state);
        if (ts_tree_add(&getEpollHandleTree(), tree_node, (uintptr_t)ephnd) < 0)
        {
            /* This should never happen. */
            port_delete(port_state);
            return_set_error(nullptr, ERROR_ALREADY_EXISTS);
        }

        return ephnd;
    }

    static HANDLE epoll_create(int size)
    {
        if (size <= 0)
            return_set_error(nullptr, ERROR_INVALID_PARAMETER);

        return epoll__create();
    }

    static HANDLE epoll_create1(int flags)
    {
        if (flags != 0)
            return_set_error(nullptr, ERROR_INVALID_PARAMETER);

        return epoll__create();
    }

    static int epoll_close(HANDLE ephnd)
    {
        ts_tree_node_t *tree_node;
        port_state_t   *port_state;

        if (init() < 0)
            return -1;

        tree_node = ts_tree_del_and_ref(&getEpollHandleTree(), (uintptr_t)ephnd);
        if (tree_node == nullptr)
        {
            err_set_win_error(ERROR_INVALID_PARAMETER);
            goto err;
        }

        port_state = port_state_from_handle_tree_node(tree_node);
        port_close(port_state);

        ts_tree_node_unref_and_destroy(tree_node);

        return port_delete(port_state);

    err:
        err_check_handle(ephnd);
        return -1;
    }

    static int epoll_ctl(HANDLE ephnd, int op, SOCKET sock, struct epoll_event *ev)
    {
        ts_tree_node_t *tree_node;
        port_state_t   *port_state;
        int             r;

        if (init() < 0)
            return -1;

        tree_node = ts_tree_find_and_ref(&getEpollHandleTree(), (uintptr_t)ephnd);
        if (tree_node == nullptr)
        {
            err_set_win_error(ERROR_INVALID_PARAMETER);
            goto err;
        }

        port_state = port_state_from_handle_tree_node(tree_node);
        r          = port_ctl(port_state, op, sock, ev);

        ts_tree_node_unref(tree_node);

        if (r < 0)
            goto err;

        return 0;

    err:
        /* On Linux, in the case of epoll_ctl(), EBADF takes priority over other
         * errors. Wepoll mimics this behavior. */
        err_check_handle(ephnd);
        err_check_handle((HANDLE)sock);
        return -1;
    }

    static int epoll_wait(HANDLE              ephnd,
                          struct epoll_event *events,
                          int                 maxevents,
                          int                 timeout)
    {
        ts_tree_node_t *tree_node;
        port_state_t   *port_state;
        int             num_events;

        if (maxevents <= 0)
            return_set_error(-1, ERROR_INVALID_PARAMETER);

        if (init() < 0)
            return -1;

        tree_node = ts_tree_find_and_ref(&getEpollHandleTree(), (uintptr_t)ephnd);
        if (tree_node == nullptr)
        {
            err_set_win_error(ERROR_INVALID_PARAMETER);
            goto err;
        }

        port_state = port_state_from_handle_tree_node(tree_node);
        num_events = port_wait(port_state, events, maxevents, timeout);

        ts_tree_node_unref(tree_node);

        if (num_events < 0)
            goto err;

        return num_events;

    err:
        err_check_handle(ephnd);
        return -1;
    }

#include <errno.h>

#define ERR__ERRNO_MAPPINGS(X)                 \
    X(ERROR_ACCESS_DENIED, EACCES)             \
    X(ERROR_ALREADY_EXISTS, EEXIST)            \
    X(ERROR_BAD_COMMAND, EACCES)               \
    X(ERROR_BAD_EXE_FORMAT, ENOEXEC)           \
    X(ERROR_BAD_LENGTH, EACCES)                \
    X(ERROR_BAD_NETPATH, ENOENT)               \
    X(ERROR_BAD_NET_NAME, ENOENT)              \
    X(ERROR_BAD_NET_RESP, ENETDOWN)            \
    X(ERROR_BAD_PATHNAME, ENOENT)              \
    X(ERROR_BROKEN_PIPE, EPIPE)                \
    X(ERROR_CANNOT_MAKE, EACCES)               \
    X(ERROR_COMMITMENT_LIMIT, ENOMEM)          \
    X(ERROR_CONNECTION_ABORTED, ECONNABORTED)  \
    X(ERROR_CONNECTION_ACTIVE, EISCONN)        \
    X(ERROR_CONNECTION_REFUSED, ECONNREFUSED)  \
    X(ERROR_CRC, EACCES)                       \
    X(ERROR_DIR_NOT_EMPTY, ENOTEMPTY)          \
    X(ERROR_DISK_FULL, ENOSPC)                 \
    X(ERROR_DUP_NAME, EADDRINUSE)              \
    X(ERROR_FILENAME_EXCED_RANGE, ENOENT)      \
    X(ERROR_FILE_NOT_FOUND, ENOENT)            \
    X(ERROR_GEN_FAILURE, EACCES)               \
    X(ERROR_GRACEFUL_DISCONNECT, EPIPE)        \
    X(ERROR_HOST_DOWN, EHOSTUNREACH)           \
    X(ERROR_HOST_UNREACHABLE, EHOSTUNREACH)    \
    X(ERROR_INSUFFICIENT_BUFFER, EFAULT)       \
    X(ERROR_INVALID_ADDRESS, EADDRNOTAVAIL)    \
    X(ERROR_INVALID_FUNCTION, EINVAL)          \
    X(ERROR_INVALID_HANDLE, EBADF)             \
    X(ERROR_INVALID_NETNAME, EADDRNOTAVAIL)    \
    X(ERROR_INVALID_PARAMETER, EINVAL)         \
    X(ERROR_INVALID_USER_BUFFER, EMSGSIZE)     \
    X(ERROR_IO_PENDING, EINPROGRESS)           \
    X(ERROR_LOCK_VIOLATION, EACCES)            \
    X(ERROR_MORE_DATA, EMSGSIZE)               \
    X(ERROR_NETNAME_DELETED, ECONNABORTED)     \
    X(ERROR_NETWORK_ACCESS_DENIED, EACCES)     \
    X(ERROR_NETWORK_BUSY, ENETDOWN)            \
    X(ERROR_NETWORK_UNREACHABLE, ENETUNREACH)  \
    X(ERROR_NOACCESS, EFAULT)                  \
    X(ERROR_NONPAGED_SYSTEM_RESOURCES, ENOMEM) \
    X(ERROR_NOT_ENOUGH_MEMORY, ENOMEM)         \
    X(ERROR_NOT_ENOUGH_QUOTA, ENOMEM)          \
    X(ERROR_NOT_FOUND, ENOENT)                 \
    X(ERROR_NOT_LOCKED, EACCES)                \
    X(ERROR_NOT_READY, EACCES)                 \
    X(ERROR_NOT_SAME_DEVICE, EXDEV)            \
    X(ERROR_NOT_SUPPORTED, ENOTSUP)            \
    X(ERROR_NO_MORE_FILES, ENOENT)             \
    X(ERROR_NO_SYSTEM_RESOURCES, ENOMEM)       \
    X(ERROR_OPERATION_ABORTED, EINTR)          \
    X(ERROR_OUT_OF_PAPER, EACCES)              \
    X(ERROR_PAGED_SYSTEM_RESOURCES, ENOMEM)    \
    X(ERROR_PAGEFILE_QUOTA, ENOMEM)            \
    X(ERROR_PATH_NOT_FOUND, ENOENT)            \
    X(ERROR_PIPE_NOT_CONNECTED, EPIPE)         \
    X(ERROR_PORT_UNREACHABLE, ECONNRESET)      \
    X(ERROR_PROTOCOL_UNREACHABLE, ENETUNREACH) \
    X(ERROR_REM_NOT_LIST, ECONNREFUSED)        \
    X(ERROR_REQUEST_ABORTED, EINTR)            \
    X(ERROR_REQ_NOT_ACCEP, EWOULDBLOCK)        \
    X(ERROR_SECTOR_NOT_FOUND, EACCES)          \
    X(ERROR_SEM_TIMEOUT, ETIMEDOUT)            \
    X(ERROR_SHARING_VIOLATION, EACCES)         \
    X(ERROR_TOO_MANY_NAMES, ENOMEM)            \
    X(ERROR_TOO_MANY_OPEN_FILES, EMFILE)       \
    X(ERROR_UNEXP_NET_ERR, ECONNABORTED)       \
    X(ERROR_WAIT_NO_CHILDREN, ECHILD)          \
    X(ERROR_WORKING_SET_QUOTA, ENOMEM)         \
    X(ERROR_WRITE_PROTECT, EACCES)             \
    X(ERROR_WRONG_DISK, EACCES)                \
    X(WSAEACCES, EACCES)                       \
    X(WSAEADDRINUSE, EADDRINUSE)               \
    X(WSAEADDRNOTAVAIL, EADDRNOTAVAIL)         \
    X(WSAEAFNOSUPPORT, EAFNOSUPPORT)           \
    X(WSAECONNABORTED, ECONNABORTED)           \
    X(WSAECONNREFUSED, ECONNREFUSED)           \
    X(WSAECONNRESET, ECONNRESET)               \
    X(WSAEDISCON, EPIPE)                       \
    X(WSAEFAULT, EFAULT)                       \
    X(WSAEHOSTDOWN, EHOSTUNREACH)              \
    X(WSAEHOSTUNREACH, EHOSTUNREACH)           \
    X(WSAEINPROGRESS, EBUSY)                   \
    X(WSAEINTR, EINTR)                         \
    X(WSAEINVAL, EINVAL)                       \
    X(WSAEISCONN, EISCONN)                     \
    X(WSAEMSGSIZE, EMSGSIZE)                   \
    X(WSAENETDOWN, ENETDOWN)                   \
    X(WSAENETRESET, EHOSTUNREACH)              \
    X(WSAENETUNREACH, ENETUNREACH)             \
    X(WSAENOBUFS, ENOMEM)                      \
    X(WSAENOTCONN, ENOTCONN)                   \
    X(WSAENOTSOCK, ENOTSOCK)                   \
    X(WSAEOPNOTSUPP, EOPNOTSUPP)               \
    X(WSAEPROCLIM, ENOMEM)                     \
    X(WSAESHUTDOWN, EPIPE)                     \
    X(WSAETIMEDOUT, ETIMEDOUT)                 \
    X(WSAEWOULDBLOCK, EWOULDBLOCK)             \
    X(WSANOTINITIALISED, ENETDOWN)             \
    X(WSASYSNOTREADY, ENETDOWN)                \
    X(WSAVERNOTSUPPORTED, ENOSYS)

    static errno_t err__map_win_error_to_errno(DWORD error)
    {
        switch (error)
        {
#define X(error_sym, errno_sym) \
    case error_sym:             \
        return errno_sym;
            ERR__ERRNO_MAPPINGS(X)
#undef X
        }
        return EINVAL;
    }

    static void err_map_win_error(void)
    {
        errno = err__map_win_error_to_errno(GetLastError());
    }

    static void err_set_win_error(DWORD error)
    {
        SetLastError(error);
        errno = err__map_win_error_to_errno(error);
    }

    static int err_check_handle(HANDLE handle)
    {
        DWORD flags;

        /* GetHandleInformation() succeeds when passed INVALID_HANDLE_VALUE, so check
         * for this condition explicitly. */
        if (handle == INVALID_HANDLE_VALUE)
            return_set_error(-1, ERROR_INVALID_HANDLE);

        if (!GetHandleInformation(handle, &flags))
            return_map_error(-1);

        return 0;
    }

#include <stddef.h>

#define array_count(a) (sizeof(a) / (sizeof((a)[0])))

#define container_of(ptr, type, member) \
    ((type *)((uintptr_t)(ptr)-offsetof(type, member)))

#define unused_var(v) ((void)(v))

/* Polyfill `inline` for older versions of msvc (up to Visual Studio 2013) */
#if defined(_MSC_VER) && _MSC_VER < 1900
#define inline __inline
#endif

    static bool &getinitDone()
    {
        static bool init__done = false;
        return init__done;
    }

    static BOOL CALLBACK init__once_callback(INIT_ONCE *once,
                                             void      *parameter,
                                             void     **context)
    {
        unused_var(once);
        unused_var(parameter);
        unused_var(context);

        /* N.b. that initialization order matters here. */
        if (ws_global_init() < 0 || nt_global_init() < 0 ||
            reflock_global_init() < 0 || epoll_global_init() < 0)
            return FALSE;

        getinitDone() = true;
        return TRUE;
    }

    static int init(void)
    {
        static INIT_ONCE init__once = INIT_ONCE_STATIC_INIT;
        if (!getinitDone() &&
            !InitOnceExecuteOnce(&init__once, init__once_callback, nullptr, nullptr))
            /* `InitOnceExecuteOnce()` itself is infallible, and it doesn't set any
             * error code when the once-callback returns FALSE. We return -1 here to
             * indicate that global initialization failed; the failing init function is
             * resposible for setting `errno` and calling `SetLastError()`. */
            return -1;

        return 0;
    }

/* Set up a workaround for the following problem:
 *   FARPROC addr = GetProcAddress(...);
 *   MY_FUNC func = (MY_FUNC) addr;          <-- GCC 8 warning/error.
 *   MY_FUNC func = (MY_FUNC) (void*) addr;  <-- MSVC  warning/error.
 * To compile cleanly with either compiler, do casts with this "bridge" type:
 *   MY_FUNC func = (MY_FUNC) (nt__fn_ptr_cast_t) addr; */
#ifdef __GNUC__
    typedef void *nt__fn_ptr_cast_t;
#else
    typedef FARPROC nt__fn_ptr_cast_t;
#endif

    static int nt_global_init(void)
    {
        HMODULE ntdll;
        FARPROC fn_ptr;

        ntdll = GetModuleHandleW(L"ntdll.dll");
        if (ntdll == nullptr)
            return -1;

        fn_ptr = GetProcAddress(ntdll, "NtCancelIoFileEx");
        if (fn_ptr == 0)
            return -1;
        getExternPf().NtCancelIoFileEx = (NTSTATUS(*)(HANDLE FileHandle, PIO_STATUS_BLOCK IoRequestToCancel, PIO_STATUS_BLOCK IoStatusBlock))(nt__fn_ptr_cast_t)fn_ptr;
        fn_ptr                         = GetProcAddress(ntdll, "NtCreateFile");
        if (fn_ptr == 0)
            return -1;
        getExternPf().NtCreateFile = (NTSTATUS(*)(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength))(nt__fn_ptr_cast_t)fn_ptr;
        fn_ptr                     = GetProcAddress(ntdll, "NtCreateKeyedEvent");
        if (fn_ptr == 0)
            return -1;
        getExternPf().NtCreateKeyedEvent = (NTSTATUS(*)(PHANDLE KeyedEventHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, ULONG Flags))(nt__fn_ptr_cast_t)fn_ptr;
        fn_ptr                           = GetProcAddress(ntdll, "NtDeviceIoControlFile");
        if (fn_ptr == 0)
            return -1;
        getExternPf().NtDeviceIoControlFile = (NTSTATUS(*)(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, ULONG IoControlCode, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength))(nt__fn_ptr_cast_t)fn_ptr;
        fn_ptr                              = GetProcAddress(ntdll, "NtReleaseKeyedEvent");
        if (fn_ptr == 0)
            return -1;
        getExternPf().NtReleaseKeyedEvent = (NTSTATUS(*)(HANDLE KeyedEventHandle, PVOID KeyValue, BOOLEAN Alertable, PLARGE_INTEGER Timeout))(nt__fn_ptr_cast_t)fn_ptr;
        fn_ptr                            = GetProcAddress(ntdll, "NtWaitForKeyedEvent");
        if (fn_ptr == 0)
            return -1;
        getExternPf().NtWaitForKeyedEvent = (NTSTATUS(*)(HANDLE KeyedEventHandle, PVOID KeyValue, BOOLEAN Alertable, PLARGE_INTEGER Timeout))(nt__fn_ptr_cast_t)fn_ptr;
        fn_ptr                            = GetProcAddress(ntdll, "RtlNtStatusToDosError");
        if (fn_ptr == 0)
            return -1;
        getExternPf().RtlNtStatusToDosError = (ULONG(*)(NTSTATUS Status))(nt__fn_ptr_cast_t)fn_ptr;

        return 0;
    }

#include <string.h>

#define POLL_GROUP__MAX_GROUP_SIZE 32

    static poll_group_t *poll_group__new(port_state_t *port_state)
    {
        HANDLE   iocp_handle      = port_get_iocp_handle(port_state);
        queue_t *poll_group_queue = port_get_poll_group_queue(port_state);

        poll_group_t *poll_group = (poll_group_t *)malloc(sizeof(poll_group_t));
        if (poll_group == nullptr)
            return_set_error(nullptr, ERROR_NOT_ENOUGH_MEMORY);

        memset(poll_group, 0, sizeof *poll_group);

        queue_node_init(&poll_group->queue_node);
        poll_group->port_state = port_state;

        if (afd_create_device_handle(iocp_handle, &poll_group->afd_device_handle) <
            0)
        {
            free(poll_group);
            return nullptr;
        }

        queue_append(poll_group_queue, &poll_group->queue_node);

        return poll_group;
    }

    static void poll_group_delete(poll_group_t *poll_group)
    {
        assert(poll_group->group_size == 0);
        CloseHandle(poll_group->afd_device_handle);
        queue_remove(&poll_group->queue_node);
        free(poll_group);
    }

    static poll_group_t *poll_group_from_queue_node(queue_node_t *queue_node)
    {
        return container_of(queue_node, poll_group_t, queue_node);
    }

    static HANDLE poll_group_get_afd_device_handle(poll_group_t *poll_group)
    {
        return poll_group->afd_device_handle;
    }

    static poll_group_t *poll_group_acquire(port_state_t *port_state)
    {
        queue_t      *poll_group_queue = port_get_poll_group_queue(port_state);
        poll_group_t *poll_group =
            !queue_is_empty(poll_group_queue)
                ? container_of(
                      queue_last(poll_group_queue), poll_group_t, queue_node)
                : nullptr;

        if (poll_group == nullptr ||
            poll_group->group_size >= POLL_GROUP__MAX_GROUP_SIZE)
            poll_group = poll_group__new(port_state);
        if (poll_group == nullptr)
            return nullptr;

        if (++poll_group->group_size == POLL_GROUP__MAX_GROUP_SIZE)
            queue_move_to_start(poll_group_queue, &poll_group->queue_node);

        return poll_group;
    }

    static void poll_group_release(poll_group_t *poll_group)
    {
        port_state_t *port_state       = poll_group->port_state;
        queue_t      *poll_group_queue = port_get_poll_group_queue(port_state);

        poll_group->group_size--;
        assert(poll_group->group_size < POLL_GROUP__MAX_GROUP_SIZE);

        queue_move_to_end(poll_group_queue, &poll_group->queue_node);

        /* Poll groups are currently only freed when the epoll port is closed. */
    }

#define PORT__MAX_ON_STACK_COMPLETIONS 256

    static inline port_state_t *port__alloc(void)
    {
        port_state_t *port_state = (port_state_t *)malloc(sizeof(port_state_t));
        if (port_state == nullptr)
            return_set_error(nullptr, ERROR_NOT_ENOUGH_MEMORY);

        return port_state;
    }

    static inline void port__free(port_state_t *port)
    {
        assert(port != nullptr);
        free(port);
    }

    static inline HANDLE port__create_iocp(void)
    {
        HANDLE iocp_handle =
            CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
        if (iocp_handle == nullptr)
            return_map_error(nullptr);

        return iocp_handle;
    }

    static port_state_t *port_new(HANDLE *iocp_handle_out)
    {
        port_state_t *port_state;
        HANDLE        iocp_handle;

        port_state = port__alloc();
        if (port_state == nullptr)
            goto err1;

        iocp_handle = port__create_iocp();
        if (iocp_handle == nullptr)
            goto err2;

        memset(port_state, 0, sizeof *port_state);

        port_state->iocp_handle = iocp_handle;
        tree_init(&port_state->sock_tree);
        queue_init(&port_state->sock_update_queue);
        queue_init(&port_state->sock_deleted_queue);
        queue_init(&port_state->poll_group_queue);
        ts_tree_node_init(&port_state->handle_tree_node);
        InitializeCriticalSection(&port_state->lock);

        *iocp_handle_out = iocp_handle;
        return port_state;

    err2:
        port__free(port_state);
    err1:
        return nullptr;
    }

    static inline int port__close_iocp(port_state_t *port_state)
    {
        HANDLE iocp_handle      = port_state->iocp_handle;
        port_state->iocp_handle = nullptr;

        if (!CloseHandle(iocp_handle))
            return_map_error(-1);

        return 0;
    }

    static int port_close(port_state_t *port_state)
    {
        int result;

        EnterCriticalSection(&port_state->lock);
        result = port__close_iocp(port_state);
        LeaveCriticalSection(&port_state->lock);

        return result;
    }

    static int port_delete(port_state_t *port_state)
    {
        tree_node_t  *tree_node;
        queue_node_t *queue_node;

        /* At this point the IOCP port should have been closed. */
        assert(port_state->iocp_handle == nullptr);

        while ((tree_node = tree_root(&port_state->sock_tree)) != nullptr)
        {
            sock_state_t *sock_state = sock_state_from_tree_node(tree_node);
            sock_force_delete(port_state, sock_state);
        }

        while ((queue_node = queue_first(&port_state->sock_deleted_queue)) != nullptr)
        {
            sock_state_t *sock_state = sock_state_from_queue_node(queue_node);
            sock_force_delete(port_state, sock_state);
        }

        while ((queue_node = queue_first(&port_state->poll_group_queue)) != nullptr)
        {
            poll_group_t *poll_group = poll_group_from_queue_node(queue_node);
            poll_group_delete(poll_group);
        }

        assert(queue_is_empty(&port_state->sock_update_queue));

        DeleteCriticalSection(&port_state->lock);

        port__free(port_state);

        return 0;
    }

    static int port__update_events(port_state_t *port_state)
    {
        queue_t *sock_update_queue = &port_state->sock_update_queue;

        /* Walk the queue, submitting new poll requests for every socket that needs
         * it. */
        while (!queue_is_empty(sock_update_queue))
        {
            queue_node_t *queue_node = queue_first(sock_update_queue);
            sock_state_t *sock_state = sock_state_from_queue_node(queue_node);

            if (sock_update(port_state, sock_state) < 0)
                return -1;

            /* sock_update() removes the socket from the update queue. */
        }

        return 0;
    }

    static inline void port__update_events_if_polling(port_state_t *port_state)
    {
        if (port_state->active_poll_count > 0)
            port__update_events(port_state);
    }

    static inline int port__feed_events(port_state_t       *port_state,
                                        struct epoll_event *epoll_events,
                                        OVERLAPPED_ENTRY   *iocp_events,
                                        DWORD               iocp_event_count)
    {
        int   epoll_event_count = 0;
        DWORD i;

        for (i = 0; i < iocp_event_count; i++)
        {
            IO_STATUS_BLOCK *io_status_block =
                (IO_STATUS_BLOCK *)iocp_events[i].lpOverlapped;
            struct epoll_event *ev = &epoll_events[epoll_event_count];

            epoll_event_count += sock_feed_event(port_state, io_status_block, ev);
        }

        return epoll_event_count;
    }

    static inline int port__poll(port_state_t       *port_state,
                                 struct epoll_event *epoll_events,
                                 OVERLAPPED_ENTRY   *iocp_events,
                                 DWORD               maxevents,
                                 DWORD               timeout)
    {
        DWORD completion_count;

        if (port__update_events(port_state) < 0)
            return -1;

        port_state->active_poll_count++;

        LeaveCriticalSection(&port_state->lock);

        BOOL r = GetQueuedCompletionStatusEx(port_state->iocp_handle,
                                             iocp_events,
                                             maxevents,
                                             &completion_count,
                                             timeout,
                                             FALSE);

        EnterCriticalSection(&port_state->lock);

        port_state->active_poll_count--;

        if (!r)
            return_map_error(-1);

        return port__feed_events(
            port_state, epoll_events, iocp_events, completion_count);
    }

    static int port_wait(port_state_t       *port_state,
                         struct epoll_event *events,
                         int                 maxevents,
                         int                 timeout)
    {
        OVERLAPPED_ENTRY  stack_iocp_events[PORT__MAX_ON_STACK_COMPLETIONS];
        OVERLAPPED_ENTRY *iocp_events;
        uint64_t          due = 0;
        DWORD             gqcs_timeout;
        int               result;

        /* Check whether `maxevents` is in range. */
        if (maxevents <= 0)
            return_set_error(-1, ERROR_INVALID_PARAMETER);

        /* Decide whether the IOCP completion list can live on the stack, or allocate
         * memory for it on the heap. */
        if ((size_t)maxevents <= array_count(stack_iocp_events))
        {
            iocp_events = stack_iocp_events;
        }
        else if ((iocp_events =
                      (OVERLAPPED_ENTRY *)malloc((size_t)maxevents * sizeof *iocp_events)) == nullptr)
        {
            iocp_events = stack_iocp_events;
            maxevents   = array_count(stack_iocp_events);
        }

        /* Compute the timeout for GetQueuedCompletionStatus, and the wait end
         * time, if the user specified a timeout other than zero or infinite. */
        if (timeout > 0)
        {
            due          = GetTickCount64() + (uint64_t)timeout;
            gqcs_timeout = (DWORD)timeout;
        }
        else if (timeout == 0)
        {
            gqcs_timeout = 0;
        }
        else
        {
            gqcs_timeout = INFINITE;
        }

        EnterCriticalSection(&port_state->lock);

        /* Dequeue completion packets until either at least one interesting event
         * has been discovered, or the timeout is reached. */
        for (;;)
        {
            uint64_t now;

            result = port__poll(
                port_state, events, iocp_events, (DWORD)maxevents, gqcs_timeout);
            if (result < 0 || result > 0)
                break; /* Result, error, or time-out. */

            if (timeout < 0)
                continue; /* When timeout is negative, never time out. */

            /* Update time. */
            now = GetTickCount64();

            /* Do not allow the due time to be in the past. */
            if (now >= due)
            {
                SetLastError(WAIT_TIMEOUT);
                break;
            }

            /* Recompute time-out argument for GetQueuedCompletionStatus. */
            gqcs_timeout = (DWORD)(due - now);
        }

        port__update_events_if_polling(port_state);

        LeaveCriticalSection(&port_state->lock);

        if (iocp_events != stack_iocp_events)
            free(iocp_events);

        if (result >= 0)
            return result;
        else if (GetLastError() == WAIT_TIMEOUT)
            return 0;
        else
            return -1;
    }

    static inline int port__ctl_add(port_state_t       *port_state,
                                    SOCKET              sock,
                                    struct epoll_event *ev)
    {
        sock_state_t *sock_state = sock_new(port_state, sock);
        if (sock_state == nullptr)
            return -1;

        if (sock_set_event(port_state, sock_state, ev) < 0)
        {
            sock_delete(port_state, sock_state);
            return -1;
        }

        port__update_events_if_polling(port_state);

        return 0;
    }

    static inline int port__ctl_mod(port_state_t       *port_state,
                                    SOCKET              sock,
                                    struct epoll_event *ev)
    {
        sock_state_t *sock_state = port_find_socket(port_state, sock);
        if (sock_state == nullptr)
            return -1;

        if (sock_set_event(port_state, sock_state, ev) < 0)
            return -1;

        port__update_events_if_polling(port_state);

        return 0;
    }

    static inline int port__ctl_del(port_state_t *port_state, SOCKET sock)
    {
        sock_state_t *sock_state = port_find_socket(port_state, sock);
        if (sock_state == nullptr)
            return -1;

        sock_delete(port_state, sock_state);

        return 0;
    }

    static inline int port__ctl_op(port_state_t       *port_state,
                                   int                 op,
                                   SOCKET              sock,
                                   struct epoll_event *ev)
    {
        switch (op)
        {
        case EPOLL_CTL_ADD:
            return port__ctl_add(port_state, sock, ev);
        case EPOLL_CTL_MOD:
            return port__ctl_mod(port_state, sock, ev);
        case EPOLL_CTL_DEL:
            return port__ctl_del(port_state, sock);
        default:
            return_set_error(-1, ERROR_INVALID_PARAMETER);
        }
    }

    static int port_ctl(port_state_t       *port_state,
                        int                 op,
                        SOCKET              sock,
                        struct epoll_event *ev)
    {
        int result;

        EnterCriticalSection(&port_state->lock);
        result = port__ctl_op(port_state, op, sock, ev);
        LeaveCriticalSection(&port_state->lock);

        return result;
    }

    static int port_register_socket(port_state_t *port_state,
                                    sock_state_t *sock_state,
                                    SOCKET        socket)
    {
        if (tree_add(&port_state->sock_tree,
                     sock_state_to_tree_node(sock_state),
                     socket) < 0)
            return_set_error(-1, ERROR_ALREADY_EXISTS);
        return 0;
    }

    static void port_unregister_socket(port_state_t *port_state,
                                       sock_state_t *sock_state)
    {
        tree_del(&port_state->sock_tree, sock_state_to_tree_node(sock_state));
    }

    static sock_state_t *port_find_socket(port_state_t *port_state, SOCKET socket)
    {
        tree_node_t *tree_node = tree_find(&port_state->sock_tree, socket);
        if (tree_node == nullptr)
            return_set_error(nullptr, ERROR_NOT_FOUND);
        return sock_state_from_tree_node(tree_node);
    }

    static void port_request_socket_update(port_state_t *port_state,
                                           sock_state_t *sock_state)
    {
        if (queue_is_enqueued(sock_state_to_queue_node(sock_state)))
            return;
        queue_append(&port_state->sock_update_queue,
                     sock_state_to_queue_node(sock_state));
    }

    static void port_cancel_socket_update(port_state_t *port_state,
                                          sock_state_t *sock_state)
    {
        unused_var(port_state);
        if (!queue_is_enqueued(sock_state_to_queue_node(sock_state)))
            return;
        queue_remove(sock_state_to_queue_node(sock_state));
    }

    static void port_add_deleted_socket(port_state_t *port_state,
                                        sock_state_t *sock_state)
    {
        if (queue_is_enqueued(sock_state_to_queue_node(sock_state)))
            return;
        queue_append(&port_state->sock_deleted_queue,
                     sock_state_to_queue_node(sock_state));
    }

    static void port_remove_deleted_socket(port_state_t *port_state,
                                           sock_state_t *sock_state)
    {
        unused_var(port_state);
        if (!queue_is_enqueued(sock_state_to_queue_node(sock_state)))
            return;
        queue_remove(sock_state_to_queue_node(sock_state));
    }

    static HANDLE port_get_iocp_handle(port_state_t *port_state)
    {
        assert(port_state->iocp_handle != nullptr);
        return port_state->iocp_handle;
    }

    static queue_t *port_get_poll_group_queue(port_state_t *port_state)
    {
        return &port_state->poll_group_queue;
    }

    static port_state_t *port_state_from_handle_tree_node(ts_tree_node_t *tree_node)
    {
        return container_of(tree_node, port_state_t, handle_tree_node);
    }

    static ts_tree_node_t *port_state_to_handle_tree_node(port_state_t *port_state)
    {
        return &port_state->handle_tree_node;
    }

    static void queue_init(queue_t *queue)
    {
        queue_node_init(&queue->head);
    }

    static void queue_node_init(queue_node_t *node)
    {
        node->prev = node;
        node->next = node;
    }

    static inline void queue__detach_node(queue_node_t *node)
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    static queue_node_t *queue_first(const queue_t *queue)
    {
        return !queue_is_empty(queue) ? queue->head.next : nullptr;
    }

    static queue_node_t *queue_last(const queue_t *queue)
    {
        return !queue_is_empty(queue) ? queue->head.prev : nullptr;
    }

    static void queue_prepend(queue_t *queue, queue_node_t *node)
    {
        node->next       = queue->head.next;
        node->prev       = &queue->head;
        node->next->prev = node;
        queue->head.next = node;
    }

    static void queue_append(queue_t *queue, queue_node_t *node)
    {
        node->next       = &queue->head;
        node->prev       = queue->head.prev;
        node->prev->next = node;
        queue->head.prev = node;
    }

    static void queue_move_to_start(queue_t *queue, queue_node_t *node)
    {
        queue__detach_node(node);
        queue_prepend(queue, node);
    }

    static void queue_move_to_end(queue_t *queue, queue_node_t *node)
    {
        queue__detach_node(node);
        queue_append(queue, node);
    }

    static void queue_remove(queue_node_t *node)
    {
        queue__detach_node(node);
        queue_node_init(node);
    }

    static bool queue_is_empty(const queue_t *queue)
    {
        return !queue_is_enqueued(&queue->head);
    }

    static bool queue_is_enqueued(const queue_node_t *node)
    {
        return node->prev != node;
    }

#define REFLOCK__REF          ((long)0x00000001UL)
#define REFLOCK__REF_MASK     ((long)0x0fffffffUL)
#define REFLOCK__DESTROY      ((long)0x10000000UL)
#define REFLOCK__DESTROY_MASK ((long)0xf0000000UL)
#define REFLOCK__POISON       ((long)0x300dead0UL)

    static HANDLE &getKeyedEvent()
    {
        static HANDLE reflock__keyed_event = nullptr;
        return reflock__keyed_event;
    }

    static int reflock_global_init(void)
    {
        NTSTATUS status = getExternPf().NtCreateKeyedEvent(
            &getKeyedEvent(), KEYEDEVENT_ALL_ACCESS, nullptr, 0);
        if (status != STATUS_SUCCESS)
            return_set_error(-1, getExternPf().RtlNtStatusToDosError(status));
        return 0;
    }

    static void reflock_init(reflock_t *reflock)
    {
        reflock->state = 0;
    }

    static void reflock__signal_event(void *address)
    {
        NTSTATUS status =
            getExternPf().NtReleaseKeyedEvent(getKeyedEvent(), address, FALSE, nullptr);
        if (status != STATUS_SUCCESS)
            abort();
    }

    static void reflock__await_event(void *address)
    {
        NTSTATUS status =
            getExternPf().NtWaitForKeyedEvent(getKeyedEvent(), address, FALSE, nullptr);
        if (status != STATUS_SUCCESS)
            abort();
    }

    static void reflock_ref(reflock_t *reflock)
    {
        long state = InterlockedAdd(&reflock->state, REFLOCK__REF);

        /* Verify that the counter didn't overflow and the lock isn't destroyed. */
        assert((state & REFLOCK__DESTROY_MASK) == 0);
        unused_var(state);
    }

    static void reflock_unref(reflock_t *reflock)
    {
        long state = InterlockedAdd(&reflock->state, -REFLOCK__REF);

        /* Verify that the lock was referenced and not already destroyed. */
        assert((state & REFLOCK__DESTROY_MASK & ~REFLOCK__DESTROY) == 0);

        if (state == REFLOCK__DESTROY)
            reflock__signal_event(reflock);
    }

    static void reflock_unref_and_destroy(reflock_t *reflock)
    {
        long state =
            InterlockedAdd(&reflock->state, REFLOCK__DESTROY - REFLOCK__REF);
        long ref_count = state & REFLOCK__REF_MASK;

        /* Verify that the lock was referenced and not already destroyed. */
        assert((state & REFLOCK__DESTROY_MASK) == REFLOCK__DESTROY);

        if (ref_count != 0)
            reflock__await_event(reflock);

        state = InterlockedExchange(&reflock->state, REFLOCK__POISON);
        assert(state == REFLOCK__DESTROY);
    }

#define SOCK__KNOWN_EPOLL_EVENTS                                         \
    (EPOLLIN | EPOLLPRI | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDNORM | \
     EPOLLRDBAND | EPOLLWRNORM | EPOLLWRBAND | EPOLLMSG | EPOLLRDHUP)

    static inline sock_state_t *sock__alloc(void)
    {
        sock_state_t *sock_state = (sock_state_t *)malloc(sizeof *sock_state);
        if (sock_state == nullptr)
            return_set_error(nullptr, ERROR_NOT_ENOUGH_MEMORY);
        return sock_state;
    }

    static inline void sock__free(sock_state_t *sock_state)
    {
        assert(sock_state != nullptr);
        free(sock_state);
    }

    static inline int sock__cancel_poll(sock_state_t *sock_state)
    {
        assert(sock_state->poll_status == SOCK__POLL_PENDING);

        if (afd_cancel_poll(poll_group_get_afd_device_handle(sock_state->poll_group),
                            &sock_state->io_status_block) < 0)
            return -1;

        sock_state->poll_status    = SOCK__POLL_CANCELLED;
        sock_state->pending_events = 0;
        return 0;
    }

    static sock_state_t *sock_new(port_state_t *port_state, SOCKET socket)
    {
        SOCKET        base_socket;
        poll_group_t *poll_group;
        sock_state_t *sock_state;

        if (socket == 0 || socket == INVALID_SOCKET)
            return_set_error(nullptr, ERROR_INVALID_HANDLE);

        base_socket = ws_get_base_socket(socket);
        if (base_socket == INVALID_SOCKET)
            return nullptr;

        poll_group = poll_group_acquire(port_state);
        if (poll_group == nullptr)
            return nullptr;

        sock_state = sock__alloc();
        if (sock_state == nullptr)
            goto err1;

        memset(sock_state, 0, sizeof *sock_state);

        sock_state->base_socket = base_socket;
        sock_state->poll_group  = poll_group;

        tree_node_init(&sock_state->tree_node);
        queue_node_init(&sock_state->queue_node);

        if (port_register_socket(port_state, sock_state, socket) < 0)
            goto err2;

        return sock_state;

    err2:
        sock__free(sock_state);
    err1:
        poll_group_release(poll_group);

        return nullptr;
    }

    static int sock__delete(port_state_t *port_state,
                            sock_state_t *sock_state,
                            bool          force)
    {
        if (!sock_state->delete_pending)
        {
            if (sock_state->poll_status == SOCK__POLL_PENDING)
                sock__cancel_poll(sock_state);

            port_cancel_socket_update(port_state, sock_state);
            port_unregister_socket(port_state, sock_state);

            sock_state->delete_pending = true;
        }

        /* If the poll request still needs to complete, the sock_state object can't
         * be free()d yet. `sock_feed_event()` or `port_close()` will take care
         * of this later. */
        if (force || sock_state->poll_status == SOCK__POLL_IDLE)
        {
            /* Free the sock_state now. */
            port_remove_deleted_socket(port_state, sock_state);
            poll_group_release(sock_state->poll_group);
            sock__free(sock_state);
        }
        else
        {
            /* Free the socket later. */
            port_add_deleted_socket(port_state, sock_state);
        }

        return 0;
    }

    static void sock_delete(port_state_t *port_state, sock_state_t *sock_state)
    {
        sock__delete(port_state, sock_state, false);
    }

    static void sock_force_delete(port_state_t *port_state, sock_state_t *sock_state)
    {
        sock__delete(port_state, sock_state, true);
    }

    static int sock_set_event(port_state_t             *port_state,
                              sock_state_t             *sock_state,
                              const struct epoll_event *ev)
    {
        /* EPOLLERR and EPOLLHUP are always reported, even when not requested by the
         * caller. However they are disabled after a event has been reported for a
         * socket for which the EPOLLONESHOT flag was set. */
        uint32_t events = ev->events | EPOLLERR | EPOLLHUP;

        sock_state->user_events = events;
        sock_state->user_data   = ev->data;

        if ((events & SOCK__KNOWN_EPOLL_EVENTS & ~sock_state->pending_events) != 0)
            port_request_socket_update(port_state, sock_state);

        return 0;
    }

    static inline DWORD sock__epoll_events_to_afd_events(uint32_t epoll_events)
    {
        /* Always monitor for AFD_POLL_LOCAL_CLOSE, which is triggered when the
         * socket is closed with closesocket() or CloseHandle(). */
        DWORD afd_events = AFD_POLL_LOCAL_CLOSE;

        if (epoll_events & (EPOLLIN | EPOLLRDNORM))
            afd_events |= AFD_POLL_RECEIVE | AFD_POLL_ACCEPT;
        if (epoll_events & (EPOLLPRI | EPOLLRDBAND))
            afd_events |= AFD_POLL_RECEIVE_EXPEDITED;
        if (epoll_events & (EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND))
            afd_events |= AFD_POLL_SEND;
        if (epoll_events & (EPOLLIN | EPOLLRDNORM | EPOLLRDHUP))
            afd_events |= AFD_POLL_DISCONNECT;
        if (epoll_events & EPOLLHUP)
            afd_events |= AFD_POLL_ABORT;
        if (epoll_events & EPOLLERR)
            afd_events |= AFD_POLL_CONNECT_FAIL;

        return afd_events;
    }

    static inline uint32_t sock__afd_events_to_epoll_events(DWORD afd_events)
    {
        uint32_t epoll_events = 0;

        if (afd_events & (AFD_POLL_RECEIVE | AFD_POLL_ACCEPT))
            epoll_events |= EPOLLIN | EPOLLRDNORM;
        if (afd_events & AFD_POLL_RECEIVE_EXPEDITED)
            epoll_events |= EPOLLPRI | EPOLLRDBAND;
        if (afd_events & AFD_POLL_SEND)
            epoll_events |= EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND;
        if (afd_events & AFD_POLL_DISCONNECT)
            epoll_events |= EPOLLIN | EPOLLRDNORM | EPOLLRDHUP;
        if (afd_events & AFD_POLL_ABORT)
            epoll_events |= EPOLLHUP;
        if (afd_events & AFD_POLL_CONNECT_FAIL)
            /* Linux reports all these events after connect() has failed. */
            epoll_events |=
                EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDNORM | EPOLLWRNORM | EPOLLRDHUP;

        return epoll_events;
    }

    static int sock_update(port_state_t *port_state, sock_state_t *sock_state)
    {
        assert(!sock_state->delete_pending);

        if ((sock_state->poll_status == SOCK__POLL_PENDING) &&
            (sock_state->user_events & SOCK__KNOWN_EPOLL_EVENTS &
             ~sock_state->pending_events) == 0)
        {
            /* All the events the user is interested in are already being monitored by
             * the pending poll operation. It might spuriously complete because of an
             * event that we're no longer interested in; when that happens we'll submit
             * a new poll operation with the updated event mask. */
        }
        else if (sock_state->poll_status == SOCK__POLL_PENDING)
        {
            /* A poll operation is already pending, but it's not monitoring for all the
             * events that the user is interested in. Therefore, cancel the pending
             * poll operation; when we receive it's completion package, a new poll
             * operation will be submitted with the correct event mask. */
            if (sock__cancel_poll(sock_state) < 0)
                return -1;
        }
        else if (sock_state->poll_status == SOCK__POLL_CANCELLED)
        {
            /* The poll operation has already been cancelled, we're still waiting for
             * it to return. For now, there's nothing that needs to be done. */
        }
        else if (sock_state->poll_status == SOCK__POLL_IDLE)
        {
            /* No poll operation is pending; start one. */
            sock_state->poll_info.Exclusive         = FALSE;
            sock_state->poll_info.NumberOfHandles   = 1;
            sock_state->poll_info.Timeout.QuadPart  = INT64_MAX;
            sock_state->poll_info.Handles[0].Handle = (HANDLE)sock_state->base_socket;
            sock_state->poll_info.Handles[0].Status = 0;
            sock_state->poll_info.Handles[0].Events =
                sock__epoll_events_to_afd_events(sock_state->user_events);

            if (afd_poll(poll_group_get_afd_device_handle(sock_state->poll_group),
                         &sock_state->poll_info,
                         &sock_state->io_status_block) < 0)
            {
                switch (GetLastError())
                {
                case ERROR_IO_PENDING:
                    /* Overlapped poll operation in progress; this is expected. */
                    break;
                case ERROR_INVALID_HANDLE:
                    /* Socket closed; it'll be dropped from the epoll set. */
                    return sock__delete(port_state, sock_state, false);
                default:
                    /* Other errors are propagated to the caller. */
                    return_map_error(-1);
                }
            }

            /* The poll request was successfully submitted. */
            sock_state->poll_status    = SOCK__POLL_PENDING;
            sock_state->pending_events = sock_state->user_events;
        }
        else
        {
            /* Unreachable. */
            assert(false);
        }

        port_cancel_socket_update(port_state, sock_state);
        return 0;
    }

    static int sock_feed_event(port_state_t       *port_state,
                               IO_STATUS_BLOCK    *io_status_block,
                               struct epoll_event *ev)
    {
        sock_state_t *sock_state =
            container_of(io_status_block, sock_state_t, io_status_block);
        AFD_POLL_INFO *poll_info    = &sock_state->poll_info;
        uint32_t       epoll_events = 0;

        sock_state->poll_status    = SOCK__POLL_IDLE;
        sock_state->pending_events = 0;

        if (sock_state->delete_pending)
        {
            /* Socket has been deleted earlier and can now be freed. */
            return sock__delete(port_state, sock_state, false);
        }
        else if (io_status_block->Status == STATUS_CANCELLED)
        {
            /* The poll request was cancelled by CancelIoEx. */
        }
        else if (!NT_SUCCESS(io_status_block->Status))
        {
            /* The overlapped request itself failed in an unexpected way. */
            epoll_events = EPOLLERR;
        }
        else if (poll_info->NumberOfHandles < 1)
        {
            /* This poll operation succeeded but didn't report any socket events. */
        }
        else if (poll_info->Handles[0].Events & AFD_POLL_LOCAL_CLOSE)
        {
            /* The poll operation reported that the socket was closed. */
            return sock__delete(port_state, sock_state, false);
        }
        else
        {
            /* Events related to our socket were reported. */
            epoll_events =
                sock__afd_events_to_epoll_events(poll_info->Handles[0].Events);
        }

        /* Requeue the socket so a new poll request will be submitted. */
        port_request_socket_update(port_state, sock_state);

        /* Filter out events that the user didn't ask for. */
        epoll_events &= sock_state->user_events;

        /* Return if there are no epoll events to report. */
        if (epoll_events == 0)
            return 0;

        /* If the the socket has the EPOLLONESHOT flag set, unmonitor all events,
         * even EPOLLERR and EPOLLHUP. But always keep looking for closed sockets. */
        if (sock_state->user_events & EPOLLONESHOT)
            sock_state->user_events = 0;

        ev->data   = sock_state->user_data;
        ev->events = epoll_events;
        return 1;
    }

    static sock_state_t *sock_state_from_queue_node(queue_node_t *queue_node)
    {
        return container_of(queue_node, sock_state_t, queue_node);
    }

    static queue_node_t *sock_state_to_queue_node(sock_state_t *sock_state)
    {
        return &sock_state->queue_node;
    }

    static sock_state_t *sock_state_from_tree_node(tree_node_t *tree_node)
    {
        return container_of(tree_node, sock_state_t, tree_node);
    }

    static tree_node_t *sock_state_to_tree_node(sock_state_t *sock_state)
    {
        return &sock_state->tree_node;
    }

    static void ts_tree_init(ts_tree_t *ts_tree)
    {
        tree_init(&ts_tree->tree);
        InitializeSRWLock(&ts_tree->lock);
    }

    static void ts_tree_node_init(ts_tree_node_t *node)
    {
        tree_node_init(&node->tree_node);
        reflock_init(&node->reflock);
    }

    static int ts_tree_add(ts_tree_t *ts_tree, ts_tree_node_t *node, uintptr_t key)
    {
        int r;

        AcquireSRWLockExclusive(&ts_tree->lock);
        r = tree_add(&ts_tree->tree, &node->tree_node, key);
        ReleaseSRWLockExclusive(&ts_tree->lock);

        return r;
    }

    static inline ts_tree_node_t *ts_tree__find_node(ts_tree_t *ts_tree,
                                                     uintptr_t  key)
    {
        tree_node_t *tree_node = tree_find(&ts_tree->tree, key);
        if (tree_node == nullptr)
            return nullptr;

        return container_of(tree_node, ts_tree_node_t, tree_node);
    }

    static ts_tree_node_t *ts_tree_del_and_ref(ts_tree_t *ts_tree, uintptr_t key)
    {
        ts_tree_node_t *ts_tree_node;

        AcquireSRWLockExclusive(&ts_tree->lock);

        ts_tree_node = ts_tree__find_node(ts_tree, key);
        if (ts_tree_node != nullptr)
        {
            tree_del(&ts_tree->tree, &ts_tree_node->tree_node);
            reflock_ref(&ts_tree_node->reflock);
        }

        ReleaseSRWLockExclusive(&ts_tree->lock);

        return ts_tree_node;
    }

    static ts_tree_node_t *ts_tree_find_and_ref(ts_tree_t *ts_tree, uintptr_t key)
    {
        ts_tree_node_t *ts_tree_node;

        AcquireSRWLockShared(&ts_tree->lock);

        ts_tree_node = ts_tree__find_node(ts_tree, key);
        if (ts_tree_node != nullptr)
            reflock_ref(&ts_tree_node->reflock);

        ReleaseSRWLockShared(&ts_tree->lock);

        return ts_tree_node;
    }

    static void ts_tree_node_unref(ts_tree_node_t *node)
    {
        reflock_unref(&node->reflock);
    }

    static void ts_tree_node_unref_and_destroy(ts_tree_node_t *node)
    {
        reflock_unref_and_destroy(&node->reflock);
    }

    static void tree_init(tree_t *tree)
    {
        memset(tree, 0, sizeof *tree);
    }

    static void tree_node_init(tree_node_t *node)
    {
        memset(node, 0, sizeof *node);
    }

#define TREE__ROTATE(cis, trans)       \
    tree_node_t *p      = node;        \
    tree_node_t *q      = node->trans; \
    tree_node_t *parent = p->parent;   \
                                       \
    if (parent)                        \
    {                                  \
        if (parent->left == p)         \
            parent->left = q;          \
        else                           \
            parent->right = q;         \
    }                                  \
    else                               \
    {                                  \
        tree->root = q;                \
    }                                  \
                                       \
    q->parent = parent;                \
    p->parent = q;                     \
    p->trans  = q->cis;                \
    if (p->trans)                      \
        p->trans->parent = p;          \
    q->cis = p;

    static inline void tree__rotate_left(tree_t *tree, tree_node_t *node)
    {
        TREE__ROTATE(left, right)
    }

    static inline void tree__rotate_right(tree_t *tree, tree_node_t *node)
    {
        TREE__ROTATE(right, left)
    }

#define TREE__INSERT_OR_DESCEND(side) \
    if (parent->side)                 \
    {                                 \
        parent = parent->side;        \
    }                                 \
    else                              \
    {                                 \
        parent->side = node;          \
        break;                        \
    }

#define TREE__REBALANCE_AFTER_INSERT(cis, trans)   \
    tree_node_t *grandparent = parent->parent;     \
    tree_node_t *uncle       = grandparent->trans; \
                                                   \
    if (uncle && uncle->red)                       \
    {                                              \
        parent->red = uncle->red = false;          \
        grandparent->red         = true;           \
        node                     = grandparent;    \
    }                                              \
    else                                           \
    {                                              \
        if (node == parent->trans)                 \
        {                                          \
            tree__rotate_##cis(tree, parent);      \
            node   = parent;                       \
            parent = node->parent;                 \
        }                                          \
        parent->red      = false;                  \
        grandparent->red = true;                   \
        tree__rotate_##trans(tree, grandparent);   \
    }

    static int tree_add(tree_t *tree, tree_node_t *node, uintptr_t key)
    {
        tree_node_t *parent;

        parent = tree->root;
        if (parent)
        {
            for (;;)
            {
                if (key < parent->key)
                {
                    TREE__INSERT_OR_DESCEND(left)
                }
                else if (key > parent->key)
                {
                    TREE__INSERT_OR_DESCEND(right)
                }
                else
                {
                    return -1;
                }
            }
        }
        else
        {
            tree->root = node;
        }

        node->key  = key;
        node->left = node->right = nullptr;
        node->parent             = parent;
        node->red                = true;

        for (; parent && parent->red; parent = node->parent)
        {
            if (parent == parent->parent->left)
            {
                TREE__REBALANCE_AFTER_INSERT(left, right)
            }
            else
            {
                TREE__REBALANCE_AFTER_INSERT(right, left)
            }
        }
        tree->root->red = false;

        return 0;
    }

#define TREE__REBALANCE_AFTER_REMOVE(cis, trans)     \
    tree_node_t *sibling = parent->trans;            \
                                                     \
    if (sibling->red)                                \
    {                                                \
        sibling->red = false;                        \
        parent->red  = true;                         \
        tree__rotate_##cis(tree, parent);            \
        sibling = parent->trans;                     \
    }                                                \
    if ((sibling->left && sibling->left->red) ||     \
        (sibling->right && sibling->right->red))     \
    {                                                \
        if (!sibling->trans || !sibling->trans->red) \
        {                                            \
            sibling->cis->red = false;               \
            sibling->red      = true;                \
            tree__rotate_##trans(tree, sibling);     \
            sibling = parent->trans;                 \
        }                                            \
        sibling->red = parent->red;                  \
        parent->red = sibling->trans->red = false;   \
        tree__rotate_##cis(tree, parent);            \
        node = tree->root;                           \
        break;                                       \
    }                                                \
    sibling->red = true;

    static void tree_del(tree_t *tree, tree_node_t *node)
    {
        tree_node_t *parent = node->parent;
        tree_node_t *left   = node->left;
        tree_node_t *right  = node->right;
        tree_node_t *next;
        bool         red;

        if (!left)
        {
            next = right;
        }
        else if (!right)
        {
            next = left;
        }
        else
        {
            next = right;
            while (next->left)
                next = next->left;
        }

        if (parent)
        {
            if (parent->left == node)
                parent->left = next;
            else
                parent->right = next;
        }
        else
        {
            tree->root = next;
        }

        if (left && right)
        {
            red          = next->red;
            next->red    = node->red;
            next->left   = left;
            left->parent = next;
            if (next != right)
            {
                parent        = next->parent;
                next->parent  = node->parent;
                node          = next->right;
                parent->left  = node;
                next->right   = right;
                right->parent = next;
            }
            else
            {
                next->parent = parent;
                parent       = next;
                node         = next->right;
            }
        }
        else
        {
            red  = node->red;
            node = next;
        }

        if (node)
            node->parent = parent;
        if (red)
            return;
        if (node && node->red)
        {
            node->red = false;
            return;
        }

        do
        {
            if (node == tree->root)
                break;
            if (node == parent->left)
            {
                TREE__REBALANCE_AFTER_REMOVE(left, right)
            }
            else
            {
                TREE__REBALANCE_AFTER_REMOVE(right, left)
            }
            node   = parent;
            parent = parent->parent;
        } while (!node->red);

        if (node)
            node->red = false;
    }

    static tree_node_t *tree_find(const tree_t *tree, uintptr_t key)
    {
        tree_node_t *node = tree->root;
        while (node)
        {
            if (key < node->key)
                node = node->left;
            else if (key > node->key)
                node = node->right;
            else
                return node;
        }
        return nullptr;
    }

    static tree_node_t *tree_root(const tree_t *tree)
    {
        return tree->root;
    }

#ifndef SIO_BSP_HANDLE_POLL
#define SIO_BSP_HANDLE_POLL 0x4800001D
#endif

#ifndef SIO_BASE_HANDLE
#define SIO_BASE_HANDLE 0x48000022
#endif

    static int ws_global_init(void)
    {
        int     r;
        WSADATA wsa_data;

        r = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (r != 0)
            return_set_error(-1, (DWORD)r);

        return 0;
    }

    static inline SOCKET ws__ioctl_get_bsp_socket(SOCKET socket, DWORD ioctl)
    {
        SOCKET bsp_socket;
        DWORD  bytes;

        if (WSAIoctl(socket,
                     ioctl,
                     nullptr,
                     0,
                     &bsp_socket,
                     sizeof bsp_socket,
                     &bytes,
                     nullptr,
                     nullptr) != SOCKET_ERROR)
            return bsp_socket;
        else
            return INVALID_SOCKET;
    }

    static SOCKET ws_get_base_socket(SOCKET socket)
    {
        SOCKET base_socket;
        DWORD  error;

        for (;;)
        {
            base_socket = ws__ioctl_get_bsp_socket(socket, SIO_BASE_HANDLE);
            if (base_socket != INVALID_SOCKET)
                return base_socket;

            error = GetLastError();
            if (error == WSAENOTSOCK)
                return_set_error(INVALID_SOCKET, error);

            /* Even though Microsoft documentation clearly states that LSPs should
             * never intercept the `SIO_BASE_HANDLE` ioctl [1], Komodia based LSPs do
             * so anyway, breaking it, with the apparent intention of preventing LSP
             * bypass [2]. Fortunately they don't handle `SIO_BSP_HANDLE_POLL`, which
             * will at least let us obtain the socket associated with the next winsock
             * protocol chain entry. If this succeeds, loop around and call
             * `SIO_BASE_HANDLE` again with the returned BSP socket, to make sure that
             * we unwrap all layers and retrieve the actual base socket.
             *  [1] https://docs.microsoft.com/en-us/windows/win32/winsock/winsock-ioctls
             *  [2] https://www.komodia.com/newwiki/index.php?title=Komodia%27s_Redirector_bug_fixes#Version_2.2.2.6
             */
            base_socket = ws__ioctl_get_bsp_socket(socket, SIO_BSP_HANDLE_POLL);
            if (base_socket != INVALID_SOCKET && base_socket != socket)
                socket = base_socket;
            else
                return_set_error(INVALID_SOCKET, error);
        }
    }
};

#ifdef __cplusplus
extern "C" {
#endif

inline HANDLE epoll_create(int size)
{
    return YS_Wepoll::epoll_create(size);
}

inline HANDLE epoll_create1(int flags)
{
    return YS_Wepoll::epoll_create1(flags);
}

inline int epoll_close(HANDLE ephnd)
{
    return YS_Wepoll::epoll_close(ephnd);
}

inline int epoll_ctl(HANDLE              ephnd,
                     int                 op,
                     SOCKET              sock,
                     struct epoll_event *event)
{
    return YS_Wepoll::epoll_ctl(ephnd, op, sock, event);
}

inline int epoll_wait(HANDLE              ephnd,
                      struct epoll_event *events,
                      int                 maxevents,
                      int                 timeout)
{
    return YS_Wepoll::epoll_wait(ephnd, events, maxevents, timeout);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#undef YS_WEPOLL
#undef RTL_CONSTANT_STRING
#undef RTL_CONSTANT_OBJECT_ATTRIBUTES
#undef KEYEDEVENT_WAIT
#undef KEYEDEVENT_WAKE
#undef KEYEDEVENT_ALL_ACCESS
#undef AFD_POLL_RECEIVE
#undef AFD_POLL_RECEIVE_EXPEDITED
#undef AFD_POLL_SEND
#undef AFD_POLL_DISCONNECT
#undef AFD_POLL_ABORT
#undef AFD_POLL_LOCAL_CLOSE
#undef AFD_POLL_ACCEPT
#undef AFD_POLL_CONNECT_FAIL
#undef return_map_error
#undef return_set_error
#undef IOCTL_AFD_POLL
#undef ERR__ERRNO_MAPPINGS
#undef array_count
#undef container_of
#undef unused_var
#undef POLL_GROUP__MAX_GROUP_SIZE
#undef PORT__MAX_ON_STACK_COMPLETIONS
#undef REFLOCK__REF
#undef REFLOCK__REF_MASK
#undef REFLOCK__DESTROY
#undef REFLOCK__DESTROY_MASK
#undef REFLOCK__POISON
#undef SOCK__KNOWN_EPOLL_EVENTS
#undef TREE__ROTATE
#undef TREE__INSERT_OR_DESCEND
#undef TREE__REBALANCE_AFTER_INSERT
#undef TREE__REBALANCE_AFTER_REMOVE

#endif

#endif