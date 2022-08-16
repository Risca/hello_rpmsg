/*
 *  ======== main.c ========
 */

#include <xdc/std.h>

#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>

#include <ti/sysbios/knl/Task.h>

#include <ti/ipc/rpmsg/NameMap.h>
#include <ti/ipc/rpmsg/RPMessage.h>

static UInt16 dstProc = 0;
static RPMessage_Handle handle = NULL;
static UInt32 myEndpoint = 0;
static UInt32 counter = 0;

/* Send me a zero length data payload to tear down the MesssageQCopy object: */
static Void pingCallbackFxn(RPMessage_Handle h, UArg arg, Ptr data,
    UInt16 len, UInt32 src)
{
    const Char *reply = "Pong!";
    UInt  replyLen = sizeof("Pong!");

    System_printf("%d: Received msg: %s from %d, len:%d\n",
                  counter++, data, src, len);

    /* Send data back to remote endpoint: */
    RPMessage_send(dstProc, arg, myEndpoint, (Ptr)reply, replyLen);
}

/*
 *  ======== taskFxn ========
 */
Void pingTaskFxn(UArg arg0, UArg arg1)
{
    System_printf("ping_task at port %d: Entered...\n", arg0);

    /* Create the messageQ for receiving, and register callback: */
    handle = RPMessage_create(arg0, pingCallbackFxn, arg0, &myEndpoint);
    if (!handle) {
        System_abort("RPMessage_createEx failed\n");
    }

    /* Announce we are here: */
#ifdef RPMSG_NS_2_0
    NameMap_register("rpmsg-proto", "ping", arg0);
#else
    NameMap_register("rpmsg-proto", arg0);
#endif

    /* Note: we don't get a chance to teardown with RPMessage_destroy() */
}

/*
 *  ======== main ========
 */
Int main()
{
    Task_Params params;

    /*
     * use ROV->SysMin to view the characters in the circular buffer
     */
    System_printf("enter main()\n");

    /* Create a Task to respond to the ping from the Host side */
    Task_Params_init(&params);
    params.instance->name = "ping";
    params.arg0 = 51;
    Task_create(pingTaskFxn, &params, NULL);
	
    BIOS_start();    /* does not return */

    return(0);
}
