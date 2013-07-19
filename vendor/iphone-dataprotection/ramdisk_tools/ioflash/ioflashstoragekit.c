#include <stdio.h>
#include <sys/socket.h>
#include <CoreFoundation/CoreFoundation.h>
#include "ioflash.h"
#include "../util.h"

#define LISTEN_PORT     2000

#define CMD_DUMP        0
#define CMD_PROXY       1

int main(int argc, char* argv[])
{
    int one=1;
    int cmd=0;

    if(!IOFlashStorage_kernel_patch())
        return -1;    
    
    CFMutableDictionaryRef dict = FSDGetInfo(1);
    if(dict == NULL)
    {
        fprintf(stderr, "FAILed to get NAND infos");
        return -1;
    }

    check_special_pages();

    int sl = create_listening_socket(LISTEN_PORT);
    if(sl == -1)
    {
        fprintf(stderr, "Error calling create_listening_socket\n");
        return -1;
    }

    fprintf(stderr, "NAND dumper listening on port %d\n", LISTEN_PORT);

    while(1)
    {
        int  s = accept(sl, NULL, NULL);
        setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, (void *)&one, sizeof(int));
        
        int r = read(s, (void*) &cmd, sizeof(int));
        if(r == sizeof(int))
        {
            if(cmd == CMD_DUMP)
            {
                nand_dump(s);
            }
            else if(cmd == CMD_PROXY)
            {
                nand_proxy(s);
            }
        }
        shutdown(s, SHUT_RDWR);
        close(s);
    }
    return 0;
}