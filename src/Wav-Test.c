/*
 ============================================================================
 Name        : Wav-Test.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

typedef struct {
    uint32_t chunk_id;               /*!<chunk id;"RIFF",0X46464952 */
    uint32_t chunk_size;             /*!<file length - 8 */
    uint32_t format;                /*!<WAVE,0X45564157 */
} __attribute__((packed)) chunk_riff_t;

typedef struct {
    uint32_t chunk_id;               /*!<chunk id;"fmt ",0X20746D66 */
    uint32_t chunk_size;             /*!<Size of this fmt block (Not include ID and Size);16 or 18 or 40 bytes. */
    uint16_t audio_format;           /*!<format;0X01:linear PCM;0X11:IMA ADPCM */
    uint16_t num_of_channels;         /*!<Number of channel;1: 1 channel;2: 2 channels; */
    uint32_t samplerate;            /*!<sample rate;0X1F40 = 8Khz */
    uint32_t byterate;              /*!<Byte rate; */
    uint16_t block_align;            /*!<align with byte; */
    uint16_t bits_per_sample;         /*!<Bit lenght per sample point,4 ADPCM */
} __attribute__((packed)) chunk_fmt_t;

typedef struct {
    uint32_t chunk_id;                  /*!<chunk id;"data",0X5453494C */
    uint32_t chunk_size;                /*!<Size of data block(Not include ID and Size) */
} __attribute__((packed)) chunk_data_t;


typedef struct {
    chunk_riff_t riff;                    /*!<riff */
    chunk_fmt_t fmt;                      /*!<fmt */
    chunk_data_t data;                    /*!<data */
} __attribute__((packed)) wav_header_t;


/*

The canonical WAVE format starts with the RIFF header:

0         4   ChunkID          Contains the letters "RIFF" in ASCII form
                               (0x52494646 big-endian form).
4         4   ChunkSize        36 + SubChunk2Size, or more precisely:
                               4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
                               This is the size of the rest of the chunk
                               following this number.  This is the size of the
                               entire file in bytes minus 8 bytes for the
                               two fields not included in this count:
                               ChunkID and ChunkSize.
8         4   Format           Contains the letters "WAVE"
                               (0x57415645 big-endian form).

The "WAVE" format consists of two subchunks: "fmt " and "data":
The "fmt " subchunk describes the sound data's format:

12        4   Subchunk1ID      Contains the letters "fmt "
                               (0x666d7420 big-endian form).
16        4   Subchunk1Size    16 for PCM.  This is the size of the
                               rest of the Subchunk which follows this number.
20        2   AudioFormat      PCM = 1 (i.e. Linear quantization)
                               Values other than 1 indicate some
                               form of compression.
22        2   NumChannels      Mono = 1, Stereo = 2, etc.
24        4   SampleRate       8000, 44100, etc.
28        4   ByteRate         == SampleRate * NumChannels * BitsPerSample/8
32        2   BlockAlign       == NumChannels * BitsPerSample/8
                               The number of bytes for one sample including
                               all channels. I wonder what happens when
                               this number isn't an integer?
34        2   BitsPerSample    8 bits = 8, 16 bits = 16, etc.
          2   ExtraParamSize   if PCM, then doesn't exist
          X   ExtraParams      space for extra parameters

The "data" subchunk contains the size of the data and the actual sound:

36        4   Subchunk2ID      Contains the letters "data"
                               (0x64617461 big-endian form).
40        4   Subchunk2Size    == NumSamples * NumChannels * BitsPerSample/8
                               This is the number of bytes in the data.
                               You can also think of this as the size
                               of the read of the subchunk following this
                               number.
44        *   Data             The actual sound data.

*/


uint32_t le32( uint32_t num )
{
	return
			((num>>24)&0xff) | 		// move byte 3 to byte 0
			((num<<8)&0xff0000) | 	// move byte 1 to byte 2
			((num>>8)&0xff00) | 	// move byte 2 to byte 1
			((num<<24)&0xff000000); // byte 0 to by
}

uint16_t le16( uint16_t num )
{
	return
			((num<<8)&0xff00) | 	// move byte 0 to byte 1
			((num>>8)&0xff); 		// move byte 1 to byte 0
}



int create_wav_data( int16_t**	data, int seconds_of_recording, double frequency, int num_channels, int bits_per_sample, int sample_rate )
{

	int samples = seconds_of_recording * num_channels * sample_rate;
	int len = samples * bits_per_sample / 8;

	printf( "Frequency: %f Length: %d bytes Samples: %d icnt: %d\n", frequency, len, samples, seconds_of_recording * sample_rate );

	*data = (int16_t *) malloc(len);
	int16_t* buf = *data;

	int j = 0;

	float total_count = seconds_of_recording * sample_rate;

    for( int i = 0 ; i < total_count ; i++ )
    {
/*    	double sin_floatl = 5000 * (
    	        		sinf( 2 * i * M_PI / ( sample_rate / 523.25 ) ) +
    					sinf( 2 * i * M_PI / ( sample_rate / 659.25 ) ) +
    	        		sinf( 2 * i * M_PI / ( sample_rate / 783.99 ) )
						);
*/

/*
    	double sin_floatl = 10000 * sinf( 2 * i * M_PI / ( sample_rate / frequency ) );
    	double sin_floatr = 10000 * sinf( 2 * i * M_PI / ( sample_rate / frequency ) );
*/

    	double sin_floatl = 5000 * (
        		sinf( 2 * i * M_PI / ( sample_rate / frequency ) ) +
				sinf( 2 * i * M_PI / ( sample_rate / ( 3 * frequency ) ) ) / 3 +
        		sinf( 2 * i * M_PI / ( sample_rate / ( 5 * frequency ) ) ) / 5 +
        		sinf( 2 * i * M_PI / ( sample_rate / ( 7 * frequency ) ) ) / 7 +
        		sinf( 2 * i * M_PI / ( sample_rate / ( 9 * frequency ) ) ) / 9 +
        		sinf( 2 * i * M_PI / ( sample_rate / ( 11 * frequency ) ) ) / 11 +
        		sinf( 2 * i * M_PI / ( sample_rate / ( 13 * frequency ) ) ) / 13
				);

    	double sin_floatr = 5000 * sinf( 2 * i * M_PI / ( sample_rate / frequency ) );


        int16_t lval = sin_floatl;
        int16_t rval = sin_floatl;

        buf[i*2] = lval;
        buf[i*2+1] = rval;
/*
        if ( i % 22000 == 0 )
        	frequency += 400;
*/

    }

	return len;
}


void create_wav_header( int len, wav_header_t* w, int num_channels, int bits_per_sample, int sample_rate )
{
	w->riff.chunk_id = 0X46464952;			// "RIFF"
	w->riff.format = 0X45564157;			// "WAVE"
	w->riff.chunk_size = len + 36;

	w->fmt.chunk_id = 0X20746D66;			// "fmt "
	w->fmt.audio_format = 1;
	w->fmt.bits_per_sample = bits_per_sample;
	w->fmt.block_align = num_channels * bits_per_sample/8;
	w->fmt.byterate = sample_rate * num_channels * bits_per_sample/8;
	w->fmt.chunk_size = 16;
	w->fmt.num_of_channels = num_channels;
	w->fmt.samplerate = sample_rate;

	w->data.chunk_id = 0X61746164;
	w->data.chunk_size = len;
}

int main(void) {

	int16_t*		data;
	wav_header_t	w;

	int seconds_of_recording = 5;
	int num_channels = 2;
	int bits_per_sample = 16;
	int sample_rate = 44000;

	int len = create_wav_data( &data, seconds_of_recording, 1000, num_channels, bits_per_sample, sample_rate );
	create_wav_header( len, &w, num_channels, bits_per_sample, sample_rate );

	int fd;

	if (( fd = open( "sample.wav", O_RDWR | O_CREAT | O_TRUNC, 0777 ) ) < 0 ) {
		printf( "Error opening file\n");
		return -1;
	}


	write( fd, (char*) &w, sizeof(w) );

	int total = len;
	char* buf = (char*) data;

	printf( "Total: %d bytes\n", len );

	while ( total > 0 ) {
		ssize_t wrote = write( fd, buf, total );
		printf( "Wrote: %ld bytes\n", wrote );
		total -= wrote;
	}

	close(fd);


}
