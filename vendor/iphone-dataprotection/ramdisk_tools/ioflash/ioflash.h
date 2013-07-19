#include <IOKit/IOKitLib.h>

#define kIOFlashStorageOptionBootPageIO     0x100
#define kIOFlashStorageOptionRawPageIO      0x002
#define kIOFlashStorageOptionXXXX           0x004
//0xC0 == kIOFlashStorageOptionUseAES | kIOFlashStorageOptionHomogenize

#define kIOFlashControllerReadPage         0x1
#define kIOFlashControllerWritePage        0x2
#define kIOFlashControllerDisableKeepout   0xa


struct kIOFlashControllerReadPageIn;
struct kIOFlashControllerOut;

//from ioflashstoragetool
CFMutableDictionaryRef MakeOneStringProp(CFStringRef key, CFStringRef value);
io_service_t _wait_for_io_service_matching_dict(CFDictionaryRef matching);
int FSDConnect(const char* name);
int FSDGetPropertyForKey(io_object_t obj, CFStringRef name, void* out, uint32_t outLen, CFMutableDictionaryRef dict);
void findPartitionLocation(CFStringRef contentHint, CFMutableDictionaryRef dict);//findNvramLocation
IOReturn FSDReadPageHelper(struct kIOFlashControllerReadPageIn* in, struct kIOFlashControllerOut* out);
IOReturn FSDReadPageWithOptions(uint32_t ceNum, uint32_t pageNum, void* buffer, void* spareBuffer, uint32_t spareSize, uint32_t options, struct kIOFlashControllerOut* out);
IOReturn FSDReadBootPage(uint32_t ceNum, uint32_t pageNum,uint32_t* buffer, struct kIOFlashControllerOut* out);

CFMutableDictionaryRef FSDGetInfo(int);
int IOFlashStorage_kernel_patch();

CFDictionaryRef nand_dump(int fd);
int dump_nand_to_socket(int fd);
int nand_proxy(int fd);

struct kIOFlashControllerReadPageIn
{
    uint32_t page;
    uint32_t ce;
    uint32_t options;
    void* buffer;
    uint32_t bufferSize;
    void* spare;
    uint32_t spareSize;
};//sizeof = 0x1C

//sizeof=0x8
struct kIOFlashControllerOut
{
    uint32_t ret1;
    uint32_t ret2;
};

//sizeof=0x50 AppleIOPFMIDMACommand?
struct IOFlashCommandStruct 
{
    uint32_t flags0;
    uint32_t flags1;
    uint32_t field8;
    uint32_t fieldC;
    uint32_t* page_ptr;
    void* bufferDesc;//IOMemoryDescriptor*
    uint32_t field18;
    uint32_t field1C;
    uint32_t field20;
    uint32_t* ce_ptr;
    void* spareVA;
    uint32_t spareSize;
    uint32_t field30;
    uint32_t field34;
    uint32_t field38;
    uint32_t errorCode;
    uint32_t field40;
    uint32_t field44;
    uint32_t field48;
    uint32_t field4C;
};

typedef struct IOExternalMethodArguments
{
    uint32_t        version;

    uint32_t        selector;

    mach_port_t           asyncWakePort;
    io_user_reference_t * asyncReference;
    uint32_t              asyncReferenceCount;

    const uint64_t *    scalarInput;
    uint32_t        scalarInputCount;

    const void *    structureInput;
    uint32_t        structureInputSize;

    //IOMemoryDescriptor * structureInputDescriptor;
    void * structureInputDescriptor;
   
    uint64_t *        scalarOutput;
    uint32_t        scalarOutputCount;

    void *        structureOutput;
    uint32_t        structureOutputSize;

    void * structureOutputDescriptor;
    uint32_t         structureOutputDescriptorSize;

    uint32_t        __reservedA;

    //OSObject **         structureVariableOutputData;
    void **         structureVariableOutputData;

    uint32_t        __reserved[30];
} IOExternalMethodArguments;

typedef struct proxy_read_cmd
{
    uint32_t ce;
    uint32_t page;
    uint32_t spareSize;
    uint32_t options;
} proxy_read_cmd;
