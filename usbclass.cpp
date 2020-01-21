#include "usbclass.h"
#include <stdio.h>

UsbClass::UsbClass(int vid, int pid)
{
	UsbClass::vid = vid;
	UsbClass::pid = pid;
}

void UsbClass::lineEvent(LINE_EVENT event)
{
	switch (event)
	{
	case LINE_CTS_ON:
		printf("CTS ON\n");
		break;
	case LINE_CTS_OFF:
		printf("CTS OFF\n");
		break;
	case LINE_DSR_ON:
		printf("DSR ON\n");
		break;
	case LINE_DSR_OFF:
		printf("DSR OFF\n");
		break;
	case LINE_RLSD_ON:
		printf("RLSD ON\n");
		break;
	case LINE_RLSD_OFF:
		printf("RLSD OFF\n");
		break;
	case LINE_RING:
		printf("Ring\n");
		break;
	case LINE_BREAK:
		printf("Break\n");
		break;
	case LINE_OVERRUN_ERROR:
		printf("Overrun error\n");
		break;
	case LINE_PARITY_ERROR:
		printf("Parity error\n");
		break;
	case LINE_FRAME_ERROR:
		printf("Frame error\n");
		break;
	}
}

void UsbClass::flowEvent(FLOW_EVENT event)
{
	switch (event)
	{
	case FLOW_OFF:
		printf("Flow control: software\n");
		break;
	case FLOW_RTS_CTS:
		printf("Flow control: RTS/CTS\n");
		break;
	case FLOW_DTR_DSR:
		printf("Flow control: DTR/DSR\n");
		break;
	case FLOW_XON_XOFF:
		printf("Flow control: XON/XOFF\n");
		break;
	}
}

void UsbClass::dataFormatEvent(char parity, int dataBits, int stopBits)
{
	printf("Parity: %s, %d data bits, ", parity == 'O' ? "ODD" : parity == 'E' ? "EVEN" : parity == 'M' ? "MARK" : parity == ' ' ? "SPACE" : "NONE", dataBits);
	switch (stopBits)
	{
	case 2:
		printf(" 2 stop bits\n\r");
		break;
	case 5:
		printf(" 0..5 stop bits\n\r");
		break;
	case 15:
		printf(" 1.5 stop bits\n\r");
		break;
	default:
		printf(" 1 stop bits\n\r");
		break;
	}
}

void UsbClass::baudRateEvent(int baudrate)
{
	printf("Baudrate set: %d\n", baudrate);
}

void UsbClass::dataIn(unsigned char *data, unsigned int dataSize)
{
	printf("%x:%x Data in (%d)\n", vid, pid, dataSize);
	if (dataSize)
	{
	for (size_t i = 0; i < dataSize; ++i)
	{
		printf("%02x", data[i]);
		printf((i % 0x20) == 0x1f ? "\n" : " ");
	}
	if (dataSize % 0x20)
		printf("\n");
	}
}

void UsbClass::dataOut(unsigned char *data, unsigned int dataSize)
{
	printf("%x:%x Data out (%d)\n", vid, pid, dataSize);
	if (dataSize)
	{
	for (size_t i = 0; i < dataSize; ++i)
	{
		printf("%02x", data[i]);
		printf((i % 0x20) == 0x1f ? "\n" : " ");
	}
	if (dataSize % 0x20)
		printf("\n");
	}
}

UsbClassCreators::~UsbClassCreators()
{
	for (std::vector<UsbClassCreator*>::iterator it = creators.begin(); it < creators.end(); ++it)
		delete (*it);
}

void UsbClassCreators::registerCreator(UsbClassCreator *creator)
{
	creators.push_back(creator);
}

UsbClass* UsbClassCreators::create(int vid, int pid)
{
	UsbClass* res = NULL;
	for (std::vector<UsbClassCreator*>::iterator it = creators.begin(); it < creators.end(); ++it)
		if ((*it)->check(vid, pid))
		{
			res = (*it)->create(vid, pid);
			break;
		}
	return res;
}
