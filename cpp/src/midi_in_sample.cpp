#include <SDKDDKVer.h>
#include <Windows.h>

#include <stdio.h>
#include <conio.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

void PrintMidiDevices()
{
	UINT nMidiDeviceNum;
	MIDIINCAPS caps;

	nMidiDeviceNum = midiInGetNumDevs();
	if (nMidiDeviceNum == 0) {
		fprintf(stderr, "midiInGetNumDevs() return 0...");
		return;
	}

	printf("== PrintMidiDevices() == \n");
	for (unsigned int i = 0; i < nMidiDeviceNum; ++i) {
		midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
		printf("\t%d : name = %s\n", i, caps.szPname);
	}
	printf("=====\n");
}

void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch(wMsg) {
	case MIM_OPEN:
		printf("wMsg=MIM_OPEN\n");
		break;
	case MIM_CLOSE:
		printf("wMsg=MIM_CLOSE\n");
		break;
	case MIM_DATA:
		printf("wMsg=MIM_DATA, dwInstance=%08x, dwParam1=%08x, dwParam2=%08x\n", dwInstance, dwParam1, dwParam2);
		break;
	case MIM_LONGDATA:
		printf("wMsg=MIM_LONGDATA\n"); 
		break;
	case MIM_ERROR:
		printf("wMsg=MIM_ERROR\n");
		break;
	case MIM_LONGERROR:
		printf("wMsg=MIM_LONGERROR\n");
		break;
	case MIM_MOREDATA:
		printf("wMsg=MIM_MOREDATA\n");
		break;
	default:
		printf("wMsg = unknown\n");
		break;
	}
	return;
}

int main(int argc, char* argv[])
{
	HMIDIIN hMidiDevice = NULL;;
	DWORD nMidiPort = 0;
	UINT nMidiDeviceNum;
	MMRESULT rv;

	PrintMidiDevices();
	
	nMidiDeviceNum = midiInGetNumDevs();
	if (nMidiDeviceNum == 0) {
		fprintf(stderr, "midiInGetNumDevs() return 0...");
		return -1;
	}

	rv = midiInOpen(&hMidiDevice, nMidiPort, (DWORD)(void*)MidiInProc, 0, CALLBACK_FUNCTION);
	if (rv != MMSYSERR_NOERROR) {
		fprintf(stderr, "midiInOpen() failed...rv=%d", rv);
		return -1;
	}

	midiInStart(hMidiDevice);

	while(true) {
		if (!_kbhit()) {
			Sleep(100);
			continue;
		}
		int c = _getch();
		if (c == VK_ESCAPE) break;
		if (c == 'q') break;
	}

	midiInStop(hMidiDevice);
	midiInClose(hMidiDevice);
	hMidiDevice = NULL;

	return 0;
}
