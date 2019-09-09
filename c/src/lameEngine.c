/**************************************************************************************
/
/ File Name :- lameEngine.c
/
**************************************************************************************/


/******************************Header Files*******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../inc/lame.h"
#include <time.h>
#include <pthread.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


lame_t lame;

typedef struct _tHread_Data_t {
	char* st_fileName;
	char* st_folderName;
} tHread_Data_t;

#define WAV_SIZE  8192
#define MP3_SIZE  8192

#define SoftwareVersion 3.0


/***********************************************************************
 *Function Name :- readConfig
 *
 *Descrption    :- To Read Configration Parameter and Set Variable Value (Ex. Samplingrate,Quality,etc.)
 *
 *Argument      :- void
 *
 *Return Type   :- int
 ************************************************************************/
/*
int readConfig(void)
{
	config_t cFg, *cFi;

	cFi = &cFg;

	config_init(cFi);

	if (!config_read_file(cFi, "lameParameter.cfg")) {
		fprintf(stderr, "%s:%d - %s\n",
				config_error_file(cFi),
				config_error_line(cFi),
				config_error_text(cFi));
		config_destroy(cFi);
		return(EXIT_FAILURE);
	}

	if (config_lookup_int(cFi, "SamplingRate", &fRq))
		printf("Sample: %d\n", fRq);

	config_destroy(cFi);

	return 0;

}
*/

/***********************************************************************
 *Function Name :- lameEncoderInit
 *
 *Descrption    :- LAME library initlization 
 *
 *Argument      :- void
 *
 *Return Type   :- void
 ************************************************************************/

void lameEncoderInit(void)
{
	lame = lame_init();
	/*    lame_set_num_channels(lame,2);//1 is single channel, 2 is the default setting*/
	lame_set_in_samplerate(lame, 44100); // consumer audio like CDs use 44.1KHz sampling,
	lame_set_VBR(lame, vbr_default);

	/*            lame_set_brate(lame,8);*/
	/*            lame_set_mode(lame,3);*/
	/*            lame_set_quality(lame,2); // 2=high 5 = medium  7=low*/
	lame_init_params(lame);
}

/***********************************************************************
 *Function Name :- lameEncoderClose
 *
 *Descrption    :- Terminate LAME library Engine Parameters
 *
 *Argument      :- void
 *
 *Return Type   :- void
 ************************************************************************/

void lameEncoderClose(void)
{
	lame_close(lame);
}

/***********************************************************************
 *Function Name :- renameFile
 *
 *Descrption    :- Make new file name with .mp3 extension from source file
 *
 *Argument      :- char pointer (char *)
 *
 *Return Type   :- char pointer (char *)
 ************************************************************************/

char *renameFile(char *fName)
{
	char *fLocal,*rmDot,*fNewExt=".mp3";

	if(NULL == fName)
		return NULL;
	if(NULL == (fLocal = malloc (strlen (fName) + 1 )))
		return NULL;
	strcpy(fLocal,fName);
	rmDot = strrchr (fLocal, '.');
	if ( NULL != rmDot)
		*rmDot = '\0';
	strcat(fLocal,fNewExt);
	return fLocal;
}

/***********************************************************************
 *Function Name :- mp3Fromwav
 *
 *Descrption    :- covert logic part using LAME lib API (convert from .wav to .mp3)
 *
 *Argument      :- void *
 *
 *Return Type   :- void *
 ************************************************************************/

void *mp3Fromwav(void* arg){
	tHread_Data_t *inData = (tHread_Data_t*) arg;
	char *fileName = inData->st_fileName;
	char *address = inData-> st_folderName;
	int read, write;

	// obtain the abosolut path of the file to be converted
	char AbsltAddrchange[255];
	strcpy(AbsltAddrchange, address);
//	strcat(AbsltAddrchange, "/");
	strcat(AbsltAddrchange, fileName);

	FILE *wav_Fd = fopen(AbsltAddrchange, "rb");
	if(!wav_Fd){
		printf("%s is not found!\n", AbsltAddrchange);
	}
	else{
		printf("\nstart to convert %s : \n", AbsltAddrchange);
	}

	char *fileNameNew=renameFile(fileName);
	// obtain the abosolut path of the file to be created
	char AbsltAddrNew[255];
	strcpy(AbsltAddrNew, address);
	strcat(AbsltAddrNew, "/");
	strcat(AbsltAddrNew, fileNameNew);

	printf("New path is:\n",AbsltAddrNew);	

	FILE *mp3_Fd = fopen(AbsltAddrNew, "wb");
	/*    printf("The newly generated file is: %s \n", addr);*/


	short int wav_Buffer[WAV_SIZE*2];
	unsigned char mp3_Buffer[MP3_SIZE];

	lameEncoderInit();

	do {
		read = fread(wav_Buffer, 2*sizeof(short int), WAV_SIZE, wav_Fd);
		if (read == 0)
		{
			write = lame_encode_flush(lame, mp3_Buffer, MP3_SIZE);
		}
		else{
			write = lame_encode_buffer_interleaved(lame, wav_Buffer, read, mp3_Buffer, MP3_SIZE);
		}
		fwrite(mp3_Buffer, write, 1, mp3_Fd);

	} while (read != 0);

	free(fileNameNew);
	lameEncoderClose();
	fclose(mp3_Fd);
	fclose(wav_Fd);

	pthread_exit(NULL);
	return(NULL);
}

/***********************************************************************
 *Function Name :- main
 *
 *Descrption    :- Program start point and finding .wav file in given location and make thread to convert into .mp3
 *
 *Argument      :- command line arguments (bin with folder path)
 *
 *Return Type   :- int
 ************************************************************************/

int main(int argc, char *argv[])
{
	clock_t startClk,stopClk;
	double duraTion;
	int numberFile =0;

	// Obtain the number of wav files
	DIR *directoryFd;

	if( argc != 2 )
	{
		fprintf( stderr, "Usage: %s <path_to_folder_with_wav_files>\n", argv[0] );

		/* exit with failure */
		return( EXIT_FAILURE );
	}


	char *l_folderAddr = argv[1];


	startClk = clock();
	
	printf(" SoftwareVersion of lameEngine is %lf \n",SoftwareVersion);

//	readConfig();	


	/* open directory from given argument location*/
	directoryFd = opendir(l_folderAddr);

	if (directoryFd != NULL){
		char *l_fileName;
		struct dirent *enTfd;
		enTfd = readdir(directoryFd);  /*read given location files and copy into struct for locate .wav files*/
		int rc;

		while (enTfd){
			l_fileName = enTfd->d_name;	/* file name of given folder location */

			// Iterate all the files in the given folder
			if(strlen(l_fileName) >= strlen(".wav")){
				if(!strcmp(l_fileName + strlen(l_fileName) - strlen(".wav"), ".wav")) /* .wav file locating */
				{

					numberFile ++;

					pthread_t thR;  // thread 
					tHread_Data_t thR_Data; // thread structure for file name and location  

					/* copying located .wav file into thread structure */

					thR_Data.st_fileName = malloc(strlen(l_fileName) + 1);
					strcpy(thR_Data.st_fileName, l_fileName);

					thR_Data.st_folderName = malloc(strlen(l_folderAddr) + 1);
					strcpy(thR_Data.st_folderName, l_folderAddr);
				
					/* thread create */
	
					if ( (rc = pthread_create(&thR, NULL,mp3Fromwav, &thR_Data)) ) {
						fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
						return EXIT_FAILURE;
					}
					else{
						/* waiting to complete thread and join here */
						if ( pthread_join ( thR, NULL ) ) { 
							fprintf(stderr, "error joining thread, rc: %d\n", rc);
							return EXIT_FAILURE;

						}
					}

					free(thR_Data.st_fileName);
					free(thR_Data.st_folderName);
				}
			}
			enTfd = readdir(directoryFd); /*close directory */
		}

		printf(" Number of .wav files is found: %d\n", numberFile);

	}else{
		printf("please check the directory location\n");
	}

	closedir(directoryFd); /*close the directory */

	stopClk = clock(); /*stop clock for calculation of time taken by converstion of wav2mp3 */

	duraTion = (double)(stopClk - startClk) / CLOCKS_PER_SEC;

	printf("The total time is: %f\n", duraTion);

	return EXIT_SUCCESS;
}

