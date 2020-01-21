#include "usbmon.h"
#include "fcntl.h"
#include <string>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const std::string	usbmonDev =						"/sys/kernel/debug/usb/usbmon/0u";
const std::string	usbmonDevBinary =				"/dev/usbmon0";
const std::string trim =							"\n\r\t\b";
const int MAX_LINE_SIZE =							2048;

Usbmon::Usbmon() :
	fileHandle(-1)
{
	buf = new char[MAX_LINE_SIZE];
	arg.hdr = new usbmon_packet;
	arg.data = new unsigned char[USBMON_MAX_SIZE];
	arg.alloc = USBMON_MAX_SIZE;
}

Usbmon::~Usbmon()
{
	close();
	delete arg.hdr;
	delete arg.data;
	delete[] buf;
}

std::string Usbmon::getNextWord(std::string& from, char delimiter)
{
	while (from.size() && from[0] == ' ')
		from.erase(0, 1);
	std::string res;
	size_t found = from.find(delimiter);
	if (found != std::string::npos)
	{
		res = from.substr(0, found);
		from = from.substr(found + 1);
	}
	else
	{
		res = from;
		from.clear();
	}
	return res;
}

void Usbmon::open()
{
	fileHandle = ::open(usbmonDev.data(), O_RDONLY, 0);
	if(fileHandle == -1)
		throw ErrorUsbmonOpen();
}

void Usbmon::openBinary()
{
	fileHandle = ::open(usbmonDevBinary.data(), O_RDONLY, 0);
	if(fileHandle == -1)
		throw ErrorUsbmonOpen();
}

void Usbmon::close()
{
	if (isActive())
	{
		::close(fileHandle);
		fileHandle = -1;
	}
}

std::string Usbmon::read()
{
	int size = ::read(fileHandle, buf, MAX_LINE_SIZE);
	buf[size] = '\x0';
	std::string str(buf);
	size_t i = 0;
	while (i < str.size())
	{
		if (trim.find(str[i]) != std::string::npos)
			str.erase(i, 1);
		else
			++i;
	}
	return str;
}

EVENT_TYPE Usbmon::readRequest(USB_REQUEST *req)
{
	EVENT_TYPE res = EVENT_ERROR;
	std::string str(read());
	std::string word, addr;
	int val;
	//ignore URB
	getNextWord(str, ' ');
	//timestamp
	word = getNextWord(str, ' ');
	sscanf(word.c_str(), "%lu", &req->time);
	//direction
	word = getNextWord(str, ' ');
	res = EVENT_ERROR;
	if (word.size())
	{
		if (word[0] == 'S')
			res = EVENT_SUBMISSION;
		if (word[0] == 'C')
			res = EVENT_CALLBACK;
	}
	//decode address
	addr = getNextWord(str, ' ');
	word = getNextWord(addr, ':');
	req->type = word == "Co"	? EP_TYPE_CONTROL_OUT : word == "Ci" ? EP_TYPE_CONTROL_IN : word == "Zo" ? EP_TYPE_ISOCRONOUS_OUT : word == "Zi" ? EP_TYPE_ISOCRONOUS_IN
				: word == "Bo" ? EP_TYPE_BULK_OUT : word == "Bi" ? EP_TYPE_BULK_IN : word == "Io" ? EP_TYPE_INTERRUPT_OUT : word == "Ii" ? EP_TYPE_INTERRUPT_IN : EP_TYPE_UNKNOWN;
	word = getNextWord(addr, ':');
	sscanf(word.c_str(), "%u", &req->bus);
	word = getNextWord(addr, ':');
	sscanf(word.c_str(), "%u", &req->addr);
	word = getNextWord(addr, ':');
	sscanf(word.c_str(), "%u", &req->ep);
	//status
	word = getNextWord(str, ' ');
	if (word == "s")
	{
		word = getNextWord(str, ' ');
		sscanf(word.c_str(), "%x", &val);
		req->setup.bmRequestType = static_cast<uint8_t>(val);
		word = getNextWord(str, ' ');
		sscanf(word.c_str(), "%x", &val);
		req->setup.bRequest = static_cast<uint8_t>(val);
		word = getNextWord(str, ' ');
		sscanf(word.c_str(), "%hx", &req->setup.wValue);
		word = getNextWord(str, ' ');
		sscanf(word.c_str(), "%hx", &req->setup.wIndex);
		word = getNextWord(str, ' ');
		sscanf(word.c_str(), "%hx", &req->setup.wLength);
	}
	//data (optional)
	word = getNextWord(str, ' ');
	sscanf(word.c_str(), "%u", &req->data_size);
	req->data = NULL;
	if (req->data_size)
	{
		word = getNextWord(str, ' ');
		req->data = new unsigned char[req->data_size];
		int data_ptr = 0;
		if (word == "=")
		{
			word = getNextWord(str, ' ');
			while (word.size())
			{
				for (size_t i = 0; i < word.size() / 2; ++i)
				{
					sscanf(word.substr(i * 2, 2).c_str(), "%x", &val);
					req->data[data_ptr++] = static_cast<unsigned char>(val);
				}
				word = getNextWord(str, ' ');
			}
		}
	}
	return res;
}

EVENT_TYPE Usbmon::readRequestBinary(USB_REQUEST *req)
{
	EVENT_TYPE res = EVENT_ERROR;
	if (ioctl(fileHandle, MON_IOCX_GETX, &arg) == -1)
		throw ErrorUsbmonIoctl();
	req->time = arg.hdr->ts_sec;
	res = arg.hdr->type == 'S' ? EVENT_SUBMISSION : arg.hdr->type == 'C' ? EVENT_CALLBACK : EVENT_ERROR;
	req->addr = arg.hdr->devnum;
	req->bus = arg.hdr->busnum;
	req->ep = arg.hdr->epnum & 0x7f;
	switch (arg.hdr->xfer_type)
	{
	case 0:
		req->type =  arg.hdr->epnum & 0x80 ? EP_TYPE_ISOCRONOUS_IN : EP_TYPE_ISOCRONOUS_OUT;
		break;
	case 1:
		req->type =  arg.hdr->epnum & 0x80 ? EP_TYPE_INTERRUPT_IN : EP_TYPE_INTERRUPT_OUT;
		break;
	case 2:
		req->type =  arg.hdr->epnum & 0x80 ? EP_TYPE_CONTROL_IN : EP_TYPE_CONTROL_OUT;
		break;
	case 3:
		req->type =  arg.hdr->epnum & 0x80 ? EP_TYPE_BULK_IN : EP_TYPE_BULK_OUT;
		break;
	}
	if (arg.hdr->flag_setup != '-')
		memcpy(&req->setup, &arg.hdr->s.setup, SETUP_LEN);
	req->data_size = arg.hdr->len_cap;
	req->data = NULL;
	if (req->data_size)
	{
		req->data = new unsigned char[req->data_size];
		memcpy(req->data, arg.data, req->data_size);
	}
	return res;
}
