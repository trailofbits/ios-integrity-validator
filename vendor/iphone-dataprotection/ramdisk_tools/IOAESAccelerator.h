#define kIOAESAcceleratorInfo 0
#define kIOAESAcceleratorTask 1
#define kIOAESAcceleratorTest 2

#define kIOAESAcceleratorEncrypt 0
#define kIOAESAcceleratorDecrypt 1

#define kIOAESAcceleratorGIDMask 0x3E8
#define kIOAESAcceleratorUIDMask 0x7D0
#define kIOAESAcceleratorCustomMask 0

typedef struct 
{
	void*		inbuf;
	void*		outbuf;
	uint32_t	size;
	uint8_t		iv[16];
	uint32_t	mode;
	uint32_t	bits;
	uint8_t		keybuf[32];
	uint32_t	mask;
	uint32_t	zero; //ios 4.2.1
} IOAESStruct;

#define IOAESStruct_size41    (sizeof(IOAESStruct))
#define IOAESStruct_sizeold    (sizeof(IOAESStruct) - 4)

void aes_init();
io_connect_t IOAESAccelerator_getIOconnect();
int doAES(void* inbuf, void *outbuf, uint32_t size, uint32_t keyMask, void* key, void* iv, int mode, int bits);
int AES_UID_Encrypt(void* input, void* output, size_t len);

uint8_t* IOAES_key835();
uint8_t* IOAES_key89A();
uint8_t* IOAES_key89B();

