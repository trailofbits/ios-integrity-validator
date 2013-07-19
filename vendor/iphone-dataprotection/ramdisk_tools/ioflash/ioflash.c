#include <stdio.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CommonCrypto/CommonCryptor.h>
#include <IOKit/IOKitLib.h>
#include "ioflash.h"

/**
used to decrypt special pages when checking if the physical banks parameter is correct
when reading/dumping pages are not decrypted
**/
uint8_t META_KEY[16] = {0x92, 0xa7, 0x42, 0xab, 0x08, 0xc9, 0x69, 0xbf, 0x00, 0x6c, 0x94, 0x12, 0xd3, 0xcc, 0x79, 0xa5};
//uint8_t FILESYSTEM_KEY[16] = {0xf6, 0x5d, 0xae, 0x95, 0x0e, 0x90, 0x6c, 0x42, 0xb2, 0x54, 0xcc, 0x58, 0xfc, 0x78, 0xee, 0xce};


uint32_t gDeviceReadID = 0;
uint32_t gCECount = 0;
uint32_t gBlocksPerCE = 0;
uint32_t gPagesPerBlock = 0;
uint32_t gBytesPerPage = 0;
uint32_t gBytesPerSpare = 0;
uint32_t gBootloaderBytes = 0;
uint32_t gIsBootFromNand = 0;
uint32_t gPagesPerCE = 0;
uint32_t gTotalBlocks = 0;
uint32_t metaPerLogicalPage = 0;
uint32_t validmetaPerLogicalPage = 0;
uint32_t banksPerCE = 0;
uint32_t use_4k_aes_chain = 0;
uint32_t gFSStartBlock= 0;
uint32_t gDumpPageSize = 0;
uint32_t gPPNdevice = 0;
uint32_t banksPerCEphysical = 1;
uint32_t bank_address_space = 0;
uint32_t blocks_per_bank = 0;

//from ioflashstoragetool
io_service_t fsdService = 0;
io_connect_t fsdConnection = 0;


CFMutableDictionaryRef MakeOneStringProp(CFStringRef key, CFStringRef value)
{
    CFMutableDictionaryRef dict  = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);	
    CFDictionarySetValue(dict, key, value);
    return dict;
}

io_service_t _wait_for_io_service_matching_dict(CFDictionaryRef matching)
{
    io_service_t  service = 0;
   /* while(!service) {
        */CFRetain(matching);
        service = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
        //if(service) break;
        /*sleep(1);
        //CFRelease(matching);
    }
    CFRelease(matching);*/
    return service;
}


int FSDConnect(const char* name)
{
    CFMutableDictionaryRef matching;
    
    matching = IOServiceMatching(name);
    
    fsdService = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
    
    IOServiceOpen(fsdService, mach_task_self(), 0, &fsdConnection);
    
    return fsdConnection != 0;
}

int FSDGetPropertyForKey(io_object_t obj, CFStringRef name, void* out, uint32_t outLen, CFMutableDictionaryRef dict)
{
    CFTypeRef data = IORegistryEntryCreateCFProperty(obj, name, kCFAllocatorDefault, 0);
    
    if (!data)
    {
        return 0;
    }
    if (dict != NULL)
    {
        CFDictionaryAddValue(dict, name, data);
    }
    if (out == NULL)
        return 0;
    if(CFGetTypeID(data) == CFNumberGetTypeID())
    {
        CFNumberGetValue((CFNumberRef)data, kCFNumberIntType, out);
        return 1;
    }
    else if(CFGetTypeID(data) == CFDataGetTypeID())
    {
        CFIndex dataLen = CFDataGetLength(data);
        CFDataGetBytes(data, CFRangeMake(0,dataLen < outLen ? dataLen : outLen), out);
        return 1;
    }
    return 0;
}
IOReturn FSDReadPageHelper(struct kIOFlashControllerReadPageIn* in, struct kIOFlashControllerOut* out)
{
    size_t outLen = sizeof(struct kIOFlashControllerOut);
    
    IOConnectCallStructMethod(fsdConnection,
                              kIOFlashControllerReadPage,
                              in,
                              sizeof(struct kIOFlashControllerReadPageIn),
                              out,
                              &outLen);
    //    fprintf(stderr, "%x %x %x %x %x\n", in->page, in->ce, r, out->ret1, out->ret2);
    
    return out->ret1;
}

IOReturn FSDReadPageWithOptions(uint32_t ceNum,
                            uint32_t pageNum,
                            void* buffer,
                            void* spareBuffer,
                            uint32_t spareSize,
                            uint32_t options,
                            struct kIOFlashControllerOut* out
                            )
{
    struct kIOFlashControllerReadPageIn in;
    
    in.page = pageNum;
    in.ce = ceNum;
    in.options = options;
    in.buffer = buffer;
    in.bufferSize = gBytesPerPage;
    in.spare = spareBuffer;
    in.spareSize = spareSize;
    return FSDReadPageHelper(&in, out);
}

IOReturn FSDReadBootPage(uint32_t ceNum, uint32_t pageNum,uint32_t* buffer, struct kIOFlashControllerOut* out)
{
    return FSDReadPageWithOptions(ceNum, pageNum, buffer, NULL, 0, kIOFlashStorageOptionBootPageIO, out);
}

void findPartitionLocation(CFStringRef contentHint, CFMutableDictionaryRef dict)
{
    const void* keys[2] = {CFSTR("Block Count"), CFSTR("Block Offset")};
    const void* values[2] = {NULL, NULL};
    
    CFMutableDictionaryRef match = MakeOneStringProp(CFSTR("IOProviderClass"), CFSTR("IOFlashStoragePartition"));
    CFMutableDictionaryRef iopmatch = MakeOneStringProp(CFSTR("Content Hint"), contentHint);
    CFDictionarySetValue(match, CFSTR("IOPropertyMatch"), iopmatch);
    
    CFRelease(iopmatch);
    
    io_service_t service = _wait_for_io_service_matching_dict(match);
    if (service)
    {
        CFNumberRef blockCount = (CFNumberRef) IORegistryEntryCreateCFProperty(service, CFSTR("Block Count"), kCFAllocatorDefault, 0);
        CFNumberRef blockOffset = (CFNumberRef) IORegistryEntryCreateCFProperty(service, CFSTR("Block Offset"), kCFAllocatorDefault, 0);
        if (dict != NULL)
        {
            values[0] = (void*) blockCount;
            values[1] = (void*) blockOffset;
            CFDictionaryRef d = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            if (d != NULL)
                CFDictionaryAddValue(dict, contentHint, d);
        }
        if( CFStringCompare(contentHint, CFSTR("Filesystem"), 0) == kCFCompareEqualTo)
        {
            CFNumberGetValue(blockOffset, kCFNumberIntType, &gFSStartBlock);
        }
    }
    CFRelease(match);
}



//openiBoot/util.c
uint32_t next_power_of_two(uint32_t n) {    
    uint32_t val = 1 << (31 - __builtin_clz(n));

    if (n % val)
        val *= 2;

    return val;
}

void generate_IV(unsigned long lbn, unsigned long *iv)
{
    int i;
    for(i = 0; i < 4; i++)
    {
        if(lbn & 1)
            lbn = 0x80000061 ^ (lbn >> 1);
        else
            lbn = lbn >> 1;
        iv[i] = lbn;
    }
}

void decrypt_page(uint8_t* data, uint32_t dataLength, uint8_t* key, uint32_t keyLength, uint32_t pn)
{
    char iv[16];
    size_t dataOutMoved=dataLength;
    generate_IV(pn, (unsigned long*) iv);
    
    CCCryptorStatus s = CCCrypt(kCCDecrypt,
                                kCCAlgorithmAES128,
                                0,
                                (const void*) key,
                                keyLength,
                                (const void*) iv,
                                (const void*) data,
                                dataLength,
                                (const void*) data,
                                dataLength,
                                &dataOutMoved);
    if (s != kCCSuccess)
    {
        fprintf(stderr, "decrypt_page: CCCrypt error %x\n", s);
    }
}

void set_physical_banks(int n)
{
    banksPerCEphysical = n;
    blocks_per_bank = gBlocksPerCE / banksPerCEphysical;
    
    if((gBlocksPerCE & (gBlocksPerCE-1)) == 0)
    {
        // Already a power of two.
        bank_address_space = blocks_per_bank;
        //total_block_space = gBlocksPerCE;
    }
    else
    {
        // Calculate the bank address space.
        bank_address_space = next_power_of_two(blocks_per_bank);
        //total_block_space = ((banksPerCEphysical-1)*bank_address_space) + blocks_per_bank;
    }
}

//"bruteforce" the number of physical banks
//except for PPN devices, DEVICEINFOBBT special pages should always be somewhere at the end
void check_special_pages()
{
    if(gPPNdevice)
    {
        fprintf(stderr, "PPN device, i don't know how to guess the number of physical banks, assuming 1\n");
        set_physical_banks(1);
        return;
    }

    uint32_t i,x=1;
    uint32_t ok = 0;
    uint32_t bank, block, page;
    uint8_t* pageBuffer = (uint8_t*) valloc(gDumpPageSize);

    printf("Searching for correct banksPerCEphysical value ...\n");
    
    while(!ok && x < 10)
    {
        set_physical_banks(x);
        bank = banksPerCEphysical - 1;
        
        for(block = blocks_per_bank-1; !ok && block > blocks_per_bank-10 ; block--)
        {
        page = (bank_address_space * bank +  block) * gPagesPerBlock;

        struct kIOFlashControllerOut *out = (&pageBuffer[gBytesPerPage + metaPerLogicalPage]);
     
        for(i=0; i < gPagesPerBlock; i++)
        {
            if(FSDReadPageWithOptions(0, page + i, pageBuffer, &pageBuffer[gBytesPerPage], metaPerLogicalPage, 0, out))
                continue;
            
            if(pageBuffer[gBytesPerPage] != 0xA5)
                continue;
            if(!memcmp(pageBuffer, "DEVICEINFOBBT", 13))
            {
                printf("Found cleartext DEVICEINFOBBT at block %d page %d with banksPerCEphyiscal=%d\n", blocks_per_bank*bank +block, i, banksPerCEphysical);
                ok = 1;
                break;
            }
            decrypt_page(pageBuffer, gBytesPerPage, META_KEY, kCCKeySizeAES128, page + i);
            if(!memcmp(pageBuffer, "DEVICEINFOBBT", 13))
            {
                printf("Found encrypted DEVICEINFOBBT at block %d page %d with banksPerCEphyiscal=%d\n", blocks_per_bank*bank +block, i, banksPerCEphysical);
                ok = 1;
                break;
            }
        }
        }
        x++;
    }
    if(!ok)
    {
        fprintf(stderr, "Couldnt guess the number of physical banks, exiting\n");
        exit(0);
    }
    free(pageBuffer);
    return;
}


//XXX dont read the NAND from this function as it can be called from multiple processes
CFMutableDictionaryRef FSDGetInfo(int printInfo)
{
    io_iterator_t iterator = 0;
    io_object_t obj = 0;
    
    FSDConnect("IOFlashController");
    
    if(IORegistryEntryCreateIterator(fsdService, "IOService",0, &iterator))
        return NULL;
    
    obj = IOIteratorNext(iterator);
    if (!obj)
        return NULL;

    CFMutableDictionaryRef dict = CFDictionaryCreateMutable (kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    FSDGetPropertyForKey(obj, CFSTR("device-readid"), &gDeviceReadID, sizeof(gDeviceReadID), dict);
    FSDGetPropertyForKey(obj, CFSTR("#ce"), &gCECount, sizeof(gCECount), dict);
    FSDGetPropertyForKey(obj, CFSTR("#ce-blocks"), &gBlocksPerCE, sizeof(gBlocksPerCE), dict);
    FSDGetPropertyForKey(obj, CFSTR("#block-pages"), &gPagesPerBlock, sizeof(gPagesPerBlock), dict);
    FSDGetPropertyForKey(obj, CFSTR("#page-bytes"), &gBytesPerPage, sizeof(gBytesPerPage), dict);
    FSDGetPropertyForKey(obj, CFSTR("#spare-bytes"), &gBytesPerSpare, sizeof(gBytesPerSpare), dict);
    FSDGetPropertyForKey(obj, CFSTR("#bootloader-bytes"), &gBootloaderBytes, sizeof(gBootloaderBytes), dict);
   
    FSDGetPropertyForKey(obj, CFSTR("metadata-whitening"), NULL, 0, dict);
    FSDGetPropertyForKey(obj, CFSTR("name"), NULL, 0, dict);
    FSDGetPropertyForKey(obj, CFSTR("is-bfn-partitioned"), NULL, 0, dict);
    FSDGetPropertyForKey(obj, CFSTR("bbt-format"), NULL, 0, dict);
    //FSDGetPropertyForKey(obj, CFSTR("channels"), NULL, 0, dict);
    FSDGetPropertyForKey(obj, CFSTR("vendor-type"), NULL, 0, dict);
    FSDGetPropertyForKey(obj, CFSTR("ppn-device"), &gPPNdevice, sizeof(gPPNdevice), dict);


    FSDGetPropertyForKey(obj, CFSTR("valid-meta-per-logical-page"), &validmetaPerLogicalPage, sizeof(gBytesPerSpare), dict);
    FSDGetPropertyForKey(obj, CFSTR("meta-per-logical-page"), &metaPerLogicalPage, sizeof(gBytesPerSpare), dict);
    if (metaPerLogicalPage == 0)
    {
        metaPerLogicalPage = 12;//default value?
    }
    
    //XXX: returns (possibly wrong) default value (1) when nand-disable-driver is set, use vendor-type + info from openiboot to get correct value : bank-per-ce VFL (!=physical banks)
    FSDGetPropertyForKey(obj, CFSTR("banks-per-ce"), &banksPerCE, sizeof(gBytesPerSpare), dict);
    FSDGetPropertyForKey(obj, CFSTR("use-4k-aes-chain"), &use_4k_aes_chain, sizeof(gBytesPerSpare), dict);
     
    gPagesPerCE = gBlocksPerCE * gPagesPerBlock;
    gTotalBlocks = gCECount * gBlocksPerCE;
     
    FSDGetPropertyForKey(obj, CFSTR("boot-from-nand"), &gIsBootFromNand, sizeof(gIsBootFromNand), dict);
    
    CFMutableDictionaryRef dictPartitions = CFDictionaryCreateMutable (kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    findPartitionLocation(CFSTR("Boot Block"), dictPartitions);
    findPartitionLocation(CFSTR("Effaceable"), dictPartitions);
    findPartitionLocation(CFSTR("NVRAM"), dictPartitions);
    findPartitionLocation(CFSTR("Firmware"), dictPartitions);
    findPartitionLocation(CFSTR("Filesystem"), dictPartitions);
    /*findPartitionLocation(CFSTR("Diagnostic Data"));
    findPartitionLocation(CFSTR("System Config"));
    findPartitionLocation(CFSTR("Bad Block Table"));*/
    //findPartitionLocation(CFSTR("Unpartitioned"));//never matches

    CFDictionaryAddValue(dict, CFSTR("partitions"), dictPartitions);
    IOObjectRelease(obj);
    IOObjectRelease(iterator);
    
    gDumpPageSize = gBytesPerPage + metaPerLogicalPage + sizeof(struct kIOFlashControllerOut);
    CFNumberRef n = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &gDumpPageSize);
    CFDictionarySetValue(dict, CFSTR("dumpedPageSize"), n);
    CFRelease(n);
    
    if (printInfo)
    {
        uint64_t total_size = ((uint64_t)gBytesPerPage) * ((uint64_t) (gPagesPerBlock * gBlocksPerCE * gCECount));
        total_size /= 1024*1024*1024;
        fprintf(stderr, "NAND configuration: %uGiB (%d CEs of %d blocks of %d pages of %d bytes data, %d bytes spare\n",
            (uint32_t) total_size,
            gCECount,
            gBlocksPerCE,
            gPagesPerBlock,
            gBytesPerPage,
            gBytesPerSpare);
    }
    
    set_physical_banks(1);
    return dict;
}

CFDictionaryRef nand_dump(int fd)
{
    uint64_t totalSize = (uint64_t)gPagesPerBlock * (uint64_t)gBlocksPerCE * (uint64_t)gCECount * (uint64_t)gDumpPageSize;
    write(fd, &totalSize, sizeof(uint64_t));
    
    dump_nand_to_socket(fd);
    return NULL;
}

int dump_nand_to_socket(int fd)
{
    int ceNum=0,pageNum,bankNum;
    IOReturn r;
    uint64_t totalPages = gPagesPerBlock * gBlocksPerCE * gCECount;
    uint64_t validPages = 0;
    uint64_t blankPages = 0;
    uint64_t errorPages = 0;
    uint64_t otherPages = 0;
    uint64_t counter = 0;
    
    //page data + spare metadata + kernel return values
    uint8_t* pageBuffer = (uint8_t*) valloc(gDumpPageSize);
    
    if (pageBuffer == NULL)
    {
        fprintf(stderr, "valloc(%d) FAIL", gDumpPageSize);
        return 0;
    }
    struct kIOFlashControllerOut *out = (&pageBuffer[gBytesPerPage + metaPerLogicalPage]);

    time_t start = time(NULL);
    
    for(bankNum=0; bankNum < banksPerCEphysical; bankNum++)
    {
    uint32_t start_page = bank_address_space * bankNum * gPagesPerBlock;
    uint32_t end_page = start_page + gPagesPerBlock * blocks_per_bank;
    for(pageNum=start_page; pageNum < end_page; pageNum++)
    {
        for(ceNum=0; ceNum < gCECount; ceNum++)
        {
            uint32_t blockNum = pageNum / gPagesPerBlock;
            uint32_t boot = (blockNum < gFSStartBlock);

            if(boot)
                r = FSDReadBootPage(ceNum, pageNum, pageBuffer, out);
            else
                r = FSDReadPageWithOptions(ceNum, pageNum, pageBuffer, &pageBuffer[gBytesPerPage], metaPerLogicalPage, 0, out);
                //r = FSDReadPageWithOptions(ceNum, pageNum, pageBuffer,NULL, 0, 0x100, out);
    
            if(r == 0)
            {
                validPages++;
            }
            else
            {
                if (r == kIOReturnBadMedia)
                {
                    fprintf(stderr, "CE %x page %x : uncorrectable ECC error\n", ceNum, pageNum);
                    errorPages++;
                }
                else if (r == kIOReturnUnformattedMedia)
                {
                    memset(pageBuffer, 0xFF, gBytesPerPage + metaPerLogicalPage);
                    blankPages++;
                }
                else if (r == 0xdeadbeef)
                {
                    fprintf(stderr, "0xdeadbeef return code, something is wrong with injected kernel code\n");
                    exit(0);
                }
                else if (r == kIOReturnBadArgument || r == kIOReturnNotPrivileged)
                {
                    fprintf(stderr, "Error 0x%x (kIOReturnBadArgument/kIOReturnNotPrivileged)\n", r);
                    exit(0);
                }
                else
                {
                    fprintf(stderr, "CE %x page %x : unknown return code 0x%x\n", ceNum, pageNum, r);
                    otherPages++;
                }
            }
            out->ret2 = 0;
        
            if(write(fd, pageBuffer, gDumpPageSize) != gDumpPageSize)
            {
                pageNum = gPagesPerBlock * gBlocksPerCE;
                fprintf(stderr, "Abort dump\n");
                break;
            }
            if (ceNum == 0 && pageNum % gPagesPerBlock == 0)
            {
                fprintf(stderr, "Block %d/%d (%d%%)\n", (counter/gPagesPerBlock), gBlocksPerCE, (counter*100)/(gPagesPerBlock*gBlocksPerCE));
            }
        }
        counter++;
    }
    }
    if (ceNum == gCECount && pageNum == (gPagesPerBlock * gBlocksPerCE))
    {
        time_t duration = time(NULL) - start;
        fprintf(stderr, "Finished NAND dump in %lu hours %lu minutes %lu seconds\n", duration / 3600, (duration % 3600) / 60, (duration % 3600) % 60);
        fprintf(stderr, "Total pages %llu\n", totalPages);
        fprintf(stderr, "In-use pages %llu (%d%%)\n", validPages, (int) (validPages * 100 / totalPages));
        fprintf(stderr, "Blank pages %llu (%d%%)\n", blankPages, (int) (blankPages * 100 / totalPages));
        fprintf(stderr, "Error pages %llu (%d%%)\n", errorPages, (int) (errorPages * 100 / totalPages));
        fprintf(stderr, "Other pages %llu (%d%%)\n", otherPages, (int) (otherPages * 100 / totalPages));
    }
    free(pageBuffer);
    return 0;
}

int nand_proxy(int fd)
{
    struct kIOFlashControllerOut out;
    proxy_read_cmd cmd;
    
    uint8_t* pageBuffer = (uint8_t*) valloc(gBytesPerPage);
    if( pageBuffer == NULL)
    {
        fprintf(stderr, "pageBuffer = valloc(%d) failed\n", gBytesPerPage);
        return 0;
    }

    while(1)
    {
        int z = read(fd, &cmd, sizeof(proxy_read_cmd));
        if (z != sizeof(proxy_read_cmd))
            break;

        void* spareBuf = NULL;
        
        uint32_t blockNum = cmd.page / gPagesPerBlock;
        uint32_t boot = (blockNum < gFSStartBlock);
        if(boot)
        {
            cmd.spareSize = 0;
            cmd.options |= kIOFlashStorageOptionBootPageIO;
        }
        else if (cmd.spareSize)
        {
            spareBuf = valloc(cmd.spareSize);
        }
        FSDReadPageWithOptions(cmd.ce, cmd.page, pageBuffer, spareBuf, cmd.spareSize, cmd.options, &out);
        
        write(fd, pageBuffer, gBytesPerPage);
        if (spareBuf != NULL)
        {
            write(fd, spareBuf, cmd.spareSize);
        }
        write(fd, &out, sizeof(struct kIOFlashControllerOut));
        
        if (spareBuf != NULL)
        {
            free(spareBuf);
        }
    }
    free(pageBuffer);
    return 0;
}
