// Record_RS232.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//
// Librairie Standard!
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <malloc.h>
#include <time.h>
#include <string.h>
#include "conio.h"
#include <iostream>

#define COM_PORT 4
#define BAUDRATE 220115
#define BYTE_SIZE 8
#define STOPBITS 2
#define PARITY_TYPE 1          /* 0-4=None,Odd,Even,Mark,Space    */
#define BINARY_MOD TRUE

#define NB_TO_READ 26000
#define FILE_TO_SAVE "C:\\Users\\mafassi\\Desktop\\csv format-20230601T073237Z-001\\rs232\\Record_RS232\\sortie.bin"





void* OpenPort(int nPort, int sBuff);								/* fonction qui ouvre le port serie! nPort : numero du port com. sBuff : taille du buffer max du port com */
int ReadPort(void* hSer, char* buf, int size, int* dwRead);			/* lit le port com et place les donnees recu dans buf */
wchar_t* convertCharArrayToLPCWSTR(const char* charArray);



int main(int argc, char* argv[]) {

	void* hd = NULL;
	char tc_data[512];
	int	i_len;

	system("cls");

	// ouverture du port rs232
	hd = OpenPort(COM_PORT, 128);
	if (hd == NULL)
	{
		printf("Erreur d ouverture du port!");
		exit(0);
	}

	char tc_buf[NB_TO_READ] = "";
	char* MonPointeur = tc_buf;
	int compteur;
	//Lecture de la commande
	char unCar[10];
	compteur = 0;
	int compt_prev = -1;
	while (compteur < NB_TO_READ)
	{
		compt_prev = compteur;
		ReadPort(hd, unCar, 1, &i_len);
		if (i_len == 1)
		{
			*MonPointeur = unCar[0];
			MonPointeur++;
			compteur++;
			printf(" Compteur : %d", compteur);
		}
	};

	CloseHandle(hd);
	FILE* fd;
	fd = fopen(FILE_TO_SAVE, "wb");
	fwrite(tc_buf, 1, compteur, fd);
	fclose(fd);

	return 0;
}

void* OpenPort(int nPort, int sBuff) {
	// declarations de variables
	void* hSer;
	DCB   dcb;
	//COMMTIMEOUTS cto;

	char port[5];

	sprintf_s(port, "COM%d", nPort);

	// ouvre le port serie
	hSer = CreateFile(convertCharArrayToLPCWSTR(port), GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);

	// verifie que le port serie est bien ouvert!

	if (hSer == INVALID_HANDLE_VALUE) {
		printf("Erreur!\nSelectionnez un numero de port valide.\n");
		exit(0);
	}

	// initialise le port
	//L'evenement EV_RXCHAR doit �tre surveill� par le port ouvert

	SetCommMask(hSer, EV_RXCHAR);
	//Initialise la taille des buffers de r�ception et d'emission
	SetupComm(hSer, sBuff, sBuff);
	PurgeComm(hSer, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	// lecture des param�tres du port dans la structure dcb
	GetCommState(hSer, &dcb);

	//Modification des parametres du port 
	dcb.BaudRate = BAUDRATE; //CBR_9600;//CBR_38400;//CBR_115200;//CBR_57600;
	dcb.fParity = PARITY_TYPE; //ODD
	dcb.ByteSize = BYTE_SIZE;
	dcb.StopBits = STOPBITS;
	dcb.fBinary = BINARY_MOD;
	SetCommState(hSer, &dcb);

	COMMTIMEOUTS cto;
	GetCommTimeouts(hSer, &cto);
	cto.ReadIntervalTimeout = 6000;
	cto.ReadTotalTimeoutConstant = 6000;
	SetCommTimeouts(hSer, &cto);

	// retourne un handle didentification au port
	return(hSer);
}

int ReadPort(void* hSer, char* buf, int size, int* dwRead)
{

	// declaration de variables
	OVERLAPPED	osRead = { 0 };
	//DWORD		  dwRead;
	DWORD		dwRes;
	DWORD		error;
	COMSTAT		comstat;

	ClearCommError(hSer, &error, &comstat);
	osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// gestion d'erreur sur le port
	if (osRead.hEvent == NULL)
		printf("Erreur\n");
	else
	{
		// lis le port et retourne les donnees dans buf
		if (!ReadFile(hSer, buf, size, (DWORD*)dwRead, &osRead)) {

			// gestion des erreurs de lecture
			if (GetLastError() != ERROR_IO_PENDING)
				// Read failed!!!
				printf("erreur\n");
			else
				dwRes = WaitForSingleObject(osRead.hEvent, INFINITE);
			switch (dwRes)
			{
			case WAIT_OBJECT_0:
				GetOverlappedResult(hSer, &osRead, (DWORD*)dwRead, FALSE);
				break;
			default:
				break;
			}
		}
	}

	// retourne la quantit� d'octet lu sur le port!
	return (*dwRead);
}

wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}