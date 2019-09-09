/**************************************************************************************
/
/ File Name :- lameEngine.cpp
/
**************************************************************************************/


/******************************Header Files*******************************************/
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "../inc/lame.h"
#include <time.h>
#include <pthread.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif



#define WAV_SIZE  8192
#define MP3_SIZE  8192

using namespace std;

typedef struct _tHread_Data_t {
        char* st_fileName;
        char* st_folderName;
} tHread_Data_t;

int SamplingRate;

/********************************LAME Library Class*****************************************/

class lameEngine
{
	public:
     lameEngine();
    ~lameEngine();

    int set_in_samplerate( int sample_rate );
    int set_brate( int byte_rate );

    int set_num_channels( int channels );
    int set_mode( MPEG_mode_e mode );

    int set_decode_only( int );

    int set_VBR( vbr_mode_e mode );
    int init_params();
    int encode_flush( unsigned char * mp3buf, int size );
    int encode_buffer_interleaved(
            short int           pcm[],         /* PCM data for left and right
                                                  channel, interleaved          */
            int                 num_samples,   /* number of samples per channel,
                                                  _not_ number of samples in
                                                  pcm[]                         */
            unsigned char*      mp3buf,        /* pointer to encoded MP3 stream */
            int                 mp3buf_size ); /* number of valid octets in this
                                                  stream                        */
private:
    lame_t  lame_;
};

/***********************************Wrapper for LAME API********************************/

lameEngine::lameEngine()
{
    lame_ = lame_init();
}

lameEngine::~lameEngine()
{
    lame_close( lame_ );
}

int lameEngine::set_in_samplerate( int sample_rate )
{
    return lame_set_in_samplerate( lame_, sample_rate );
}

int lameEngine::set_brate( int byte_rate )
{
    return lame_set_brate( lame_, byte_rate );
}

int lameEngine::set_num_channels( int channels )
{
    return lame_set_num_channels( lame_, channels );
}

int lameEngine::set_mode( MPEG_mode_e mode )
{
    return lame_set_mode( lame_, mode );
}

int lameEngine::set_decode_only( int p )
{
    return lame_set_decode_only( lame_, p );
}

int lameEngine::set_VBR( vbr_mode_e mode )
{
    return lame_set_VBR( lame_, mode );
}

int lameEngine::init_params()
{
    return lame_init_params( lame_ );
}

int lameEngine::encode_flush( unsigned char * mp3buf, int size )
{
    return lame_encode_flush( lame_, mp3buf, size );
}

int lameEngine::encode_buffer_interleaved(
        short int           pcm[],
        int                 num_samples,
        unsigned char*      mp3buf,
        int                 mp3buf_size )
{
    return lame_encode_buffer_interleaved( lame_, pcm, num_samples, mp3buf, mp3buf_size );
}

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
  Config cfg;

  // Read the file. If there is an error, report it and exit.
  try
  {
    cfg.readFile("lameParameter.cfg");
  }
  catch(const FileIOException &fioex)
  {
    std::cerr << "I/O error while reading file." << std::endl;
    return(EXIT_FAILURE);
  }
  catch(const ParseException &pex)
  {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
    return(EXIT_FAILURE);
  }

  try
  {
    SamplingRate = cfg.lookup("SamplingRate");
    cout << "Sampling Rate : " << SamplingRate << endl << endl;
  }
  catch(const SettingNotFoundException &nfex)
  {
    cerr << "No 'name' setting in configuration file." << endl;
  }

}
*/

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
        char *fLocal,*rmDot;
	string fNewExt = ".mp3";

	printf("in fname =%s",fName);
        if(NULL == fName)
                return NULL; 
        if(NULL == (fLocal = new char[(strlen (fName) + 1 )]))
                return NULL;
        strcpy(fLocal,fName);
        rmDot = strrchr (fLocal, '.');
        if ( NULL != rmDot)
                *rmDot = '\0';
        strcat(fLocal,fNewExt.c_str());
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



void *mp3Fromwav(void *arg){

	tHread_Data_t *inData = (tHread_Data_t*) arg;
        char *fileName = inData->st_fileName;
        char *address = inData-> st_folderName;
        int read, write;

	printf(" data =%s adr = %s \n",fileName,address);
        // obtain the abosolut path of the file to be converted
        char AbsltAddrchange[255];
        strcpy(AbsltAddrchange, address);
        //strcat(AbsltAddrchange, "/");
        strcat(AbsltAddrchange, fileName);

        FILE *wav_Fd = fopen(AbsltAddrchange, "rb");
        if(!wav_Fd){
                cout <<" not found!" << AbsltAddrchange<<endl;
        }
        else{
                cout <<"start to convert" << AbsltAddrchange<<endl;
        }

        char *fileNameNew=renameFile(fileName);
        // obtain the abosolut path of the file to be created
  	char AbsltAddrNew[255];
        strcpy(AbsltAddrNew, address);
        strcat(AbsltAddrNew, "/");
        strcat(AbsltAddrNew, fileNameNew);

        FILE *mp3_Fd = fopen(AbsltAddrNew, "wb");
 
	printf("abs= %s\n",AbsltAddrNew);
	short int wav_Buffer[WAV_SIZE*2];
        unsigned char mp3_Buffer[MP3_SIZE];

	lameEngine lameObj;
	
	lameObj.set_in_samplerate(44100);
	lameObj.set_VBR(vbr_default);
	lameObj.init_params();
	

    do {
        read = fread(wav_Buffer, 2*sizeof(short int), WAV_SIZE, wav_Fd);
        if (read == 0)
            write = lameObj.encode_flush(mp3_Buffer, MP3_SIZE);
        else
            write = lameObj.encode_buffer_interleaved(wav_Buffer, read, mp3_Buffer, MP3_SIZE);
        fwrite(mp3_Buffer, write, 1, mp3_Fd);
    } while (read != 0);

    delete fileNameNew; 

    fclose(mp3_Fd);
    fclose(wav_Fd);

    return NULL;
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


int main(int argc,char* argv[])
{
	clock_t startClk,stopClk;
	double duraTion;
        int numberFile =0;

        // Obtain the number of wav files
        DIR *directoryFd;
	
		
	if (argc != 2) {
        cout << "Usage:  " << argv[0] <<"   </path of .wave file>  " << endl;
        return -1;
    }

	string l_folderAddr = argv[1];

	cout << "The folder address is " << l_folderAddr <<  endl;	

        startClk = clock();

        //readConfig();

        directoryFd = opendir(l_folderAddr.c_str());
	
	 if (directoryFd != NULL){
                char *l_fileName;
                struct dirent *enTfd;
                enTfd = readdir(directoryFd);
		int rError;
                while (enTfd){
                        l_fileName = enTfd->d_name;

                        // Iterate all the files in the given folder
                        if(strlen(l_fileName) >= strlen(".wav")){
                                if(!strcmp(l_fileName + strlen(l_fileName) - strlen(".wav"), ".wav"))
                                {

                                 //       printf("Find one wav file %s\n", l_fileName);
                                        numberFile ++;

					pthread_t thR;
                                        tHread_Data_t thR_Data; // array of parameters to be passed into thread

                                        thR_Data.st_fileName = new char[(strlen(l_fileName) + 1)];
                                        strcpy(thR_Data.st_fileName, l_fileName);

                                        thR_Data.st_folderName = new char[l_folderAddr.size() + 1];
                                        strcpy(thR_Data.st_folderName, l_folderAddr.c_str());
					
					 if ( (rError = pthread_create(&thR, NULL,mp3Fromwav, &thR_Data)) ) {
                                                fprintf(stderr, "error: pthread_create, rc: %d\n", rError);
                                                return EXIT_FAILURE;
                                        }
                                        else{
                                                /* waiting to complete thread and join here */
                                                if ( pthread_join ( thR, NULL ) ) {
                                                        fprintf(stderr, "error joining thread, rc: %d\n", rError);
                                                        return EXIT_FAILURE;

                                                }
				
					}	
                                        delete thR_Data.st_fileName;
                                        delete thR_Data.st_folderName;
                                }
                        }
                        enTfd = readdir(directoryFd);
                }

                printf("The number of files is: %d\n", numberFile);

        }else{
                printf("The input address can not be found, please check the input.\n");
        }

        closedir(directoryFd);

 	stopClk = clock();
        duraTion = (double)(stopClk - startClk) / CLOCKS_PER_SEC;
        printf("The total time is: %f\n", duraTion);
        return EXIT_SUCCESS;

}
