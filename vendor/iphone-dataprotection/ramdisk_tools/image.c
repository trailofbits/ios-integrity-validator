#include <stdio.h>
#include <string.h>
#include <CoreGraphics/CoreGraphics.h>
#include <IOKit/IOKitLib.h>

int screenWidth, screenHeight;
CGContextRef context = NULL;

CGContextRef fb_open() {
	io_connect_t conn = NULL;
	int bytesPerRow;
	void *surfaceBuffer;
	void *frameBuffer;
	CGColorSpaceRef colorSpace;
	
	if (context != NULL)
        return context;

	io_service_t fb_service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleCLCD"));
	if (!fb_service) {
		fb_service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleM2CLCD"));
		if (!fb_service) {
			printf("Couldn't find framebuffer.\n");
			return NULL;
		}
	}

	IOMobileFramebufferOpen(fb_service, mach_task_self(), 0, &conn);
	IOMobileFramebufferGetLayerDefaultSurface(conn, 0, &surfaceBuffer);

	screenHeight = CoreSurfaceBufferGetHeight(surfaceBuffer);
	screenWidth = CoreSurfaceBufferGetWidth(surfaceBuffer);
	bytesPerRow = CoreSurfaceBufferGetBytesPerRow(surfaceBuffer);

	CoreSurfaceBufferLock(surfaceBuffer, 3);
	frameBuffer = CoreSurfaceBufferGetBaseAddress(surfaceBuffer);
	CoreSurfaceBufferUnlock(surfaceBuffer);

	// create bitmap context
	colorSpace = CGColorSpaceCreateDeviceRGB();
	context = CGBitmapContextCreate(frameBuffer, screenWidth, screenHeight, 8, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedLast);
	if(context == NULL) {
		printf("Couldn't create screen context!\n");
		return NULL;
	}

	CGColorSpaceRelease(colorSpace);

	return context;
}

int drawImage(const char* pngFileName)
{
 	CGContextRef c = fb_open();
	if (c == NULL)
		return -1;

    CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, pngFileName, strlen(pngFileName), 0);
    void* imageSource = CGImageSourceCreateWithURL(url, NULL);
    CFRelease(url);
    
    if (imageSource != NULL)
    {
        CGImageRef img = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
        if (img != NULL)
        {
            CGContextClearRect (c, CGRectMake(0, 0, screenWidth, screenHeight));
            CGContextDrawImage(c, CGRectMake(0, 0, screenWidth, screenHeight), img);
        }
    }
    return 0;
}
