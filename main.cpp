#include "CXBOXController.h"
#include <iostream>
#include <Mmsystem.h>
#include <time.h>
/* DeadZone is the region where XBox controller value cannot be read, i.e. it thinks you are not touching the control stick*/
#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  6500 
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 7250
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30

using namespace std;
CXBOXController* Player1; /*Player Object*/

int main(int argc, char* argv[])
{
	Player1 = new CXBOXController(1);
	int MAX_CONTROLLERS = 4;
	DWORD dwBytesWritten = 0;
	int n = 8; // Amount of Bytes to Read
	HANDLE hSerial;
	/************************************** GO FROM ROUTER TO COORDINATOR FOR BETTER PERFORMANCE!!!!!!!!!!!!!!*/
	/*COM8 refesssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssrs to the port that your XBEE router is connected to. To find this out you need to use XCTU. Ask Zotto to clarify in person*/
	hSerial = CreateFile("COM8", GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);// dont need to GENERIC _ WRITE changed to com4 on laptop
	if (hSerial == INVALID_HANDLE_VALUE /*|| hSerial2mhyjjk == INVALID_HANDLE_VALUE*/) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			//serial port does not exist. Inform user.
			cout << "Serial port error, does not exist" << endl;
		}
		//some other error occurred. Inform user.
		cout << "Serial port probably in use" << endl;
	}

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams)) {
		cout << "error getting state" << endl;
	}
	/*********************Crucial Information about Serial port. This config is 8N1 9600 baud******************/
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(hSerial, &dcbSerialParams)) {
		cout << "error setting serial port state" << endl;

	}

	COMMTIMEOUTS timeouts = { 0 };

	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 5000;
	timeouts.WriteTotalTimeoutMultiplier = 1000;

	if (!SetCommTimeouts(hSerial, &timeouts)) {
		cout << "Error occurred" << endl;
	}

	while (1)
	{

		//controler code

		XINPUT_STATE gpState;	// Gamepad state
		float normalizedMagnitude = 0;
		int player = -1;	// Gamepad ID


							// Polling all 4 gamepads to see who's alive
		for (int i = 0; i < 4; i++)
		{
			DWORD res = XInputGetState(i, &gpState);	// Getting state
			if (res == ERROR_SUCCESS)			// If alive - print message
			{
				printf("Controller #%d is ON!\n", i + 1);
				player = i;	// Assign last alive gamepad as active

			}
		}
		if (player < 0) // If player==-1 in other words...
		{
			printf("Haven't found any gamepads...\n");
		}
		else
		{
			while (true)
			{
				system("CLS");					// Clear screen
				memset(&gpState, 0, sizeof(XINPUT_STATE)); 	// Reset state
				DWORD res = XInputGetState(0, &gpState);		// Get new state
				printf("LX\tLY\tRX\tRY\tLTrig\tRTrig\tButtons\n"); // Print header

				float LY = gpState.Gamepad.sThumbLY;/*Returns 16 bit number 0-65535*/
				float LX = gpState.Gamepad.sThumbLX;
				float RY = gpState.Gamepad.sThumbRY;
				float RX = gpState.Gamepad.sThumbRX;
				float LT = gpState.Gamepad.bLeftTrigger;
				float RT = gpState.Gamepad.bRightTrigger;

				/*I use these values for deadzone determiations*/
				float magnitude_L = sqrt(LX*LX + LY*LY);
				float magnitude_R = sqrt(RX*RX + RY*RY);

				/*Direction byte that says which stick is being used, helps for parsing on Receiver side*/
				unsigned char dir = '0';
				/*Converts 16 bit number to 8 bit*/
				int LY_MAG = LY / 256;
				int RY_MAG = RY / 256;
				cout << "RY MAG = " << RY_MAG << endl;
				cout << "LY_MAG = " << LY_MAG << endl;

				if (magnitude_L > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
				{
					//clip the magnitude at its expected maximum value
					if (magnitude_L > 32767) magnitude_L = 32767;
					if (magnitude_R > 32767) magnitude_R = 32767;

					//adjust magnitude relative to the end of the dead zone
					magnitude_L -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

					//adjust magnitude relative to the end of the dead zone
					magnitude_R -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;


					if (magnitude_R > 32767) magnitude_R = 32767;


					//optionally normalize the magnitude with respect to its expected range
					//giving a magnitude value of 0.0 to 1.0
					//normalizedMagnitude = magnitude_L / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

					cout << "magnitude_ L =" << magnitude_L << endl;
					cout << "magnitude_ R =" << magnitude_R << endl;
					printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
						gpState.Gamepad.sThumbLX / 256,
						gpState.Gamepad.sThumbLY / 256,
						gpState.Gamepad.sThumbRX / 256,
						gpState.Gamepad.sThumbRY / 256,
						gpState.Gamepad.bLeftTrigger,
						gpState.Gamepad.bRightTrigger,
						gpState.Gamepad.wButtons);// Thumbstick values are divided by 256 for better consistency

					if (LY_MAG < 0) { /*If the control stick is pointing down remap values from 128-255*/
						LY_MAG *= -1; LY_MAG += 128; /*This is how I did direction, you dont need do this...*/
					}
					if (LY_MAG > 255) { LY_MAG = 255; }

					cout << "LY_MAG = " << LY_MAG << endl;
					dir = 'L';
					LT = gpState.Gamepad.bLeftTrigger;
					cout << "dir = " << dir << endl;
					cout << "LT = " << LT << endl;
					/*Write to Serial Port*/
					WriteFile(hSerial, &dir, 1, &dwBytesWritten, NULL);
					Sleep(20);
					WriteFile(hSerial, &LY_MAG, 1, &dwBytesWritten, NULL);
					Sleep(20);
					WriteFile(hSerial, &gpState.Gamepad.bLeftTrigger, 1, &dwBytesWritten, NULL);
					Sleep(20);
				}
				else
				{
					dir = 'L';
					LY_MAG = 0;
					LT = gpState.Gamepad.bLeftTrigger;
					/*Write to Serial Port*/
					cout << "LT = " << LT << endl;
					WriteFile(hSerial, &dir, 1, &dwBytesWritten, NULL);
					Sleep(20);
					WriteFile(hSerial, &LY_MAG, 1, &dwBytesWritten, NULL);
					Sleep(20);
					WriteFile(hSerial, &gpState.Gamepad.bLeftTrigger, 1, &dwBytesWritten, NULL);
					Sleep(20);
				}

				if (magnitude_R > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)/*MAKE SURE TO make it speed < 32 = 0,
																	  and send 2 bytes for each case i.e. make char_L and char_R = 'L'/'R' respectively, send that with coordinate data to specify which wheel*/
				{

					//clip the magnitude at its expected maximum value
					if (magnitude_L > 32767) magnitude_L = 32767;
					if (magnitude_R > 32767) magnitude_R = 32767;

					//adjust magnitude relative to the end of the dead zone
					magnitude_L -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

					//adjust magnitude relative to the end of the dead zone
					magnitude_R -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

					//optionally normalize the magnitude with respect to its expected range
					//giving a magnitude value of 0.0 to 1.0
					//normalizedMagnitude = magnitude_R / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

					cout << "magnitude_ L =" << magnitude_L << endl;
					cout << "magnitude_ R =" << magnitude_R << endl;
					printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
						gpState.Gamepad.sThumbLX / 256,
						gpState.Gamepad.sThumbLY / 256,
						gpState.Gamepad.sThumbRX / 256,
						gpState.Gamepad.sThumbRY / 256,
						gpState.Gamepad.bLeftTrigger,
						gpState.Gamepad.bRightTrigger,
						gpState.Gamepad.wButtons);// Thumbstick values are divided by 256 for better consistency
					if (RY_MAG < 0) { /*If the control stick is pointing down remap values from 128-255*/
						RY_MAG *= -1; RY_MAG += 128; /*This is how I did direction, you dont need do this...*/
					}
					if (RY_MAG > 255) { RY_MAG = 255; }//Make sure that data is encapsulated into a byte

					cout << "RY_MAG = " << RY_MAG << endl;
					dir = 'R';
					RT = gpState.Gamepad.bRightTrigger;
					cout << "RT = " << RT << endl;
					cout << "dir = " << dir << endl;
					/*Write to Serial Port*/
					WriteFile(hSerial, &dir, 1, &dwBytesWritten, NULL);
					Sleep(20);
					WriteFile(hSerial, &RY_MAG, 1, &dwBytesWritten, NULL);
					Sleep(20);
					WriteFile(hSerial, &gpState.Gamepad.bRightTrigger, 1, &dwBytesWritten, NULL);
					Sleep(20);

				}
				else
				{
					dir = 'R';
					RY_MAG = 0;
					RT = gpState.Gamepad.bRightTrigger;
					cout << "RT = " << RT << endl;
					/*Write to serial port*/
					WriteFile(hSerial, &dir, 1, &dwBytesWritten, NULL);
					Sleep(20);
					WriteFile(hSerial, &RY_MAG, 1, &dwBytesWritten, NULL);
					Sleep(20);
					WriteFile(hSerial, &gpState.Gamepad.bRightTrigger, 1, &dwBytesWritten, NULL);
					Sleep(20);
				}
			}
		}
	}
	delete(Player1);

	return(0);
}
//1 Comment