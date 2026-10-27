#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <coreinit/screen.h>
#include <whb/proc.h>

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    WHBProcInit();
    OSScreenInit();

    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);

    void *tvBuffer = memalign(0x100, tvBufferSize);
    void *drcBuffer = memalign(0x100, drcBufferSize);

    OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);

    OSScreenEnableEx(SCREEN_TV, TRUE);
    OSScreenEnableEx(SCREEN_DRC, TRUE);

    while (WHBProcIsRunning())
    {
        OSScreenClearBufferEx(SCREEN_TV, 0xFFFFFFFF);
        OSScreenClearBufferEx(SCREEN_DRC, 0xFFFFFFFF);

        OSScreenPutFontEx(SCREEN_TV, 0, 0, "BUTTON TEST");
        OSScreenPutFontEx(SCREEN_DRC, 0, 0, "BUTTON TEST");

        OSScreenFlipBuffersEx(SCREEN_TV);
        OSScreenFlipBuffersEx(SCREEN_DRC);
    }

    OSScreenShutdown();
    WHBProcShutdown();

    free(tvBuffer);
    free(drcBuffer);

    return 0;
}
