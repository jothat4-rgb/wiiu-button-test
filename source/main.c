#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>

#include <coreinit/screen.h>
#include <whb/proc.h>
#include <vpad/input.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/* Replace with your phone's local IP address. */
#define PHONE_IP "10.178.61.104"
#define PHONE_PORT 26761

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    WHBProcInit();
    OSScreenInit();

    size_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    size_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);

    void *tvBuffer = memalign(0x100, tvBufferSize);
    void *drcBuffer = memalign(0x100, drcBufferSize);

    if (tvBuffer == NULL || drcBuffer == NULL)
    {
        free(tvBuffer);
        free(drcBuffer);
        OSScreenShutdown();
        WHBProcShutdown();
        return 1;
    }

    OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);

    OSScreenEnableEx(SCREEN_TV, TRUE);
    OSScreenEnableEx(SCREEN_DRC, TRUE);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in destination;
    memset(&destination, 0, sizeof(destination));

    destination.sin_family = AF_INET;
    destination.sin_port = htons(PHONE_PORT);
    destination.sin_addr.s_addr = inet_addr(PHONE_IP);

    bool networkReady = (sock >= 0);

    VPADStatus status;
    VPADReadError error;

    while (WHBProcIsRunning())
    {
        OSScreenClearBufferEx(SCREEN_TV, 0xFFFFFFFF);
        OSScreenClearBufferEx(SCREEN_DRC, 0xFFFFFFFF);

        OSScreenPutFontEx(SCREEN_TV, 0, 0, "BUTTON TEST");
        OSScreenPutFontEx(SCREEN_DRC, 0, 0, "BUTTON TEST");

        error = VPAD_READ_NO_SAMPLES;
        VPADRead(VPAD_CHAN_0, &status, 1, &error);

        if (error == VPAD_READ_SUCCESS)
        {
            char message[160];

            snprintf(
                message,
                sizeof(message),
                "buttons=%08lX LX=%.3f LY=%.3f RX=%.3f RY=%.3f",
                (unsigned long)status.hold,
                (double)status.leftStick.x,
                (double)status.leftStick.y,
                (double)status.rightStick.x,
                (double)status.rightStick.y
            );

            if (networkReady)
            {
                sendto(
                    sock,
                    message,
                    strlen(message),
                    0,
                    (struct sockaddr *)&destination,
                    sizeof(destination)
                );
            }

            OSScreenPutFontEx(
                SCREEN_DRC, 0, 2, "GamePad input: OK"
            );
            OSScreenPutFontEx(
                SCREEN_DRC, 0, 4, message
            );
            OSScreenPutFontEx(
                SCREEN_TV, 0, 2, "Reading GamePad input"
            );
        }
        else
        {
            OSScreenPutFontEx(
                SCREEN_DRC, 0, 2, "Waiting for GamePad input"
            );
        }

        OSScreenPutFontEx(
            SCREEN_DRC, 0, 8,
            networkReady ? "UDP socket: READY" : "UDP socket: FAILED"
        );
        OSScreenPutFontEx(
            SCREEN_DRC, 0, 10, "UDP port: 26761"
        );

        OSScreenFlipBuffersEx(SCREEN_TV);
        OSScreenFlipBuffersEx(SCREEN_DRC);
    }

    if (networkReady)
    {
        close(sock);
    }

    OSScreenShutdown();
    WHBProcShutdown();

    free(tvBuffer);
    free(drcBuffer);

    return 0;
}
