#ifndef SOCKET______

#define SOCKET______

#include <ntifs.h>
#include <ntddk.h>
#include <wsk.h> // Winsock


/*
    이 헤더는 WSK.h를 이용한, 이전부터 사용된 소켓이다.
*/
typedef enum SOCKET_INFORMATION {
    NONE,
    TCP_DATA_SEND,
    TCP_DATA_RECEIVE,
    SERVER_DATA_PROCESS // 서버의 요청을 받으면 바로 보내는 역할을 해야함. ( 동기 ) 
}SOCKET_INFORMATION;

typedef struct _KSOCKET_ASYNC_CONTEXT
{
    KEVENT CompletionEvent;
    BOOLEAN is_Locked_by_always_receive;
    PIRP Irp;
} KSOCKET_ASYNC_CONTEXT, * PKSOCKET_ASYNC_CONTEXT;

typedef struct _KSOCKET
{
    KMUTEX Mutex;

    PWSK_SOCKET	WskSocket;

    union
    {
        PVOID WskDispatch;

        PWSK_PROVIDER_CONNECTION_DISPATCH WskConnectionDispatch;
        PWSK_PROVIDER_LISTEN_DISPATCH WskListenDispatch;
        PWSK_PROVIDER_DATAGRAM_DISPATCH WskDatagramDispatch;
        PWSK_PROVIDER_STREAM_DISPATCH WskStreamDispatch;

    };

    KSOCKET_ASYNC_CONTEXT AsyncContext;
} KSOCKET, * PKSOCKET;

extern PKSOCKET COMMAND_NewSocket;
extern KMUTEX always_receive_MUTEX;

extern WSK_REGISTRATION     WskRegistration;
extern WSK_PROVIDER_NPI     WskProvider;
extern WSK_CLIENT_DISPATCH  WskDispatch;

NTSTATUS KspAsyncContextCompletionRoutine(_In_ PDEVICE_OBJECT	DeviceObject, _In_ PIRP Irp, _In_ PKEVENT Completion_context);

/*TCP 송-수신*/
NTSTATUS Send_or_Receive_TCP_Server(PVOID Buffer, SIZE_T Buffer_size, SOCKET_INFORMATION send_or_receive, PKSOCKET* NewSocket_parm);

/*TCP 소켓 생성*/
NTSTATUS Make_TCP_Connection(_In_ PCHAR Server_ip, _In_ USHORT Server_port, PKSOCKET* NewSocket_parm);

/*TCP 소켓 할당해제*/
VOID TCP_socket_release(PKSOCKET* NewSocket_parm);

#endif