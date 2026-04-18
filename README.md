# QNX RTOS Real-Time Programming

Collection of QNX Neutrino RTOS programs demonstrating core real-time OS concepts: message-passing IPC, resource managers, interrupt handling, thread synchronization, shared memory, and POSIX timers.

Built for **QNX 8.0** targeting **x86_64**.

## Project Structure

```
qnx-rtos/
├── resource-manager/     # Custom /dev/example device (open/read/write)
├── interrupts/           # Hardware interrupt handling (event + IST approaches)
├── ipc/                  # Inter-Process Communication
│   ├── message-passing/  # Basic client-server with ChannelCreate/MsgSend
│   ├── name-lookup/      # Service discovery via name_attach/name_open
│   ├── pulses/           # Asynchronous pulse messaging
│   ├── events/           # Server-to-client event delivery (MsgDeliverEvent)
│   ├── shared-memory/
│   │   ├── posix/        # POSIX shm with process-shared mutex + condvar
│   │   ├── qnx-typed/    # QNX typed memory handles (shm_create_handle)
│   │   └── mutex-recovery/  # Robust mutex recovery after process death
│   └── iov-messaging/    # Scatter/gather IOV multi-part messages
├── threads/              # Process & Thread Synchronization
│   ├── mutex-demo/       # Race condition demo (nomutex) + fix (mutex_sync)
│   ├── producer-consumer/# Condvar-based producer-consumer pattern
│   ├── condvar/          # Multi-state machine with condition variables
│   ├── condvar-queue/    # Thread-safe queue with condvar notification
│   ├── process-spawn/    # posix_spawn() and zombie cleanup
│   └── death-notification/ # Process death pulses from system manager
├── timers/               # POSIX repeating timers + non-blocking pulse drain
└── boot-image/           # QNX IFS boot image buildfile
```

## Building

Each module has its own `Makefile`. From any module directory:

```bash
make        # build all binaries
make clean  # remove binaries and object files
```

Or from the root `qnx-rtos/` directory to build everything:

```bash
make        # build all modules
make clean  # clean all modules
```

**Prerequisites:** QNX SDP with `qcc` compiler in PATH.

## Key QNX Concepts Demonstrated

| Concept | Files |
|---------|-------|
| **Resource Manager** | `resource-manager/example.c` — registers `/dev/example`, handles POSIX open/read/write via dispatch framework |
| **Message Passing** | `ipc/message-passing/` — synchronous send/receive/reply between client and server processes |
| **Name Service** | `ipc/name-lookup/` — `name_attach()` / `name_open()` for location-transparent IPC |
| **Pulses** | `ipc/pulses/` — lightweight asynchronous notifications alongside messages |
| **Event Delivery** | `ipc/events/` — `MsgDeliverEvent()` for server-initiated client notifications |
| **Shared Memory** | `ipc/shared-memory/posix/` — `shm_open()` + process-shared mutex/condvar |
| **QNX Typed Memory** | `ipc/shared-memory/qnx-typed/` — `shm_create_handle()` for secure memory sharing |
| **Robust Mutexes** | `ipc/shared-memory/mutex-recovery/` — `PTHREAD_MUTEX_ROBUST` + `pthread_mutex_consistent()` |
| **IOV Messaging** | `ipc/iov-messaging/` — scatter/gather multi-part messages with `MsgSendvs()` |
| **Mutex Sync** | `threads/mutex-demo/` — before/after mutex protection of shared data |
| **Condition Variables** | `threads/condvar/`, `threads/producer-consumer/` — state machines and producer/consumer |
| **Interrupt Handling** | `interrupts/` — `InterruptAttachEvent()` and `InterruptAttachThread()` |
| **Timers** | `timers/reptimer.c` — repeating timer with `timer_create()` + `CLOCK_MONOTONIC` |
| **Process Lifecycle** | `threads/process-spawn/` — `posix_spawn()`, SIGCHLD, zombie cleanup |

## License

Educational examples for QNX real-time programming.
