/**
  ******************************************************************************
  * @file    ${file_name} 
  * @author  ${user}
  * @version 
  * @date    ${date}
  * @brief   
  ******************************************************************************
  * @attention
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "AudioMixer.h"
#include "stm32f769i_discovery_audio.h"

/* Private types ------------------------------------------------------------*/
typedef struct {
  uint32_t ChunkID;       /* 0 */
  uint32_t FileSize;      /* 4 */
  uint32_t FileFormat;    /* 8 */
  uint32_t SubChunk1ID;   /* 12 */
  uint32_t SubChunk1Size; /* 16*/
  uint16_t AudioFormat;   /* 20 */
  uint16_t NbrChannels;   /* 22 */
  uint32_t SampleRate;    /* 24 */

  uint32_t ByteRate;      /* 28 */
  uint16_t BlockAlign;    /* 32 */
  uint16_t BitPerSample;  /* 34 */
  uint32_t SubChunk2ID;   /* 36 */
  uint32_t SubChunk2Size; /* 40 */
}WAVE_FormatTypeDef;

/* Private constants --------------------------------------------------------*/
#define AUDIO_BUFFER_SIZE 	(1024 * 1)
#define READ_BUFFER_SIZE 	(AUDIO_BUFFER_SIZE * 8)
#define NUM_CHANNELS 		8

/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
WAVE_FormatTypeDef WaveFormat;
SoundType sound[NUM_CHANNELS];
uint8_t audio_buffer[AUDIO_BUFFER_SIZE] = {0};

/* Private function prototypes ----------------------------------------------*/

/**
  * @brief  
  * @param  
  * @retval 
  */
void MixAudio(uint8_t *dst, const uint8_t *src, uint32_t len)
{
	int16_t src1, src2;
	int dst_sample;
	const int max_audioval = ((1<<(16-1))-1);
	const int min_audioval = -(1<<(16-1));

	len /= 2;
	while ( len-- )
	{
		src1 = ((src[1])<<8|src[0]);
		src2 = ((dst[1])<<8|dst[0]);
		src += 2;
		dst_sample = (src1 + src2) / 2;
		if ( dst_sample > max_audioval )
		{
			dst_sample = max_audioval;
		} 
		else if ( dst_sample < min_audioval ) 
		{
			dst_sample = min_audioval;
		}
		dst[0] = dst_sample&0xFF;
		dst_sample >>= 8;
		dst[1] = dst_sample&0xFF;
		dst += 2;
	}
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
	uint8_t i, channels_playing = 0;

	for(i = 0 ; i < NUM_CHANNELS ; i++)
	{
		if((sound[i].pos == sound[i].len / 2) && (sound[i].len != 0))
		{
			if(sound[i].prevbuffstate == BUFFER_OFFSET_FULL)
				sound[i].buffstate = BUFFER_OFFSET_HALF;
		}
		if((sound[i].pos == sound[i].len) && (sound[i].len != 0))
		{
			if(sound[i].prevbuffstate == BUFFER_OFFSET_HALF)
			{
				sound[i].pos = 0;
				sound[i].buffstate = BUFFER_OFFSET_FULL;
			}
		}
		if(sound[i].pos < sound[i].len)
		{
			MixAudio( &audio_buffer[AUDIO_BUFFER_SIZE / 2],
				  sound[i].ptr + \
				  (sound[i].pos / 4),
				  AUDIO_BUFFER_SIZE / 2 );

			sound[i].pos += AUDIO_BUFFER_SIZE / 2;

			channels_playing++;
		}
	}

	if(channels_playing == 0)
	{
		memset(	&audio_buffer[AUDIO_BUFFER_SIZE / 2],
			0,
			AUDIO_BUFFER_SIZE / 2 );		
	}
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
	uint8_t i, channels_playing = 0;

	for(i = 0 ; i < NUM_CHANNELS ; i++)
	{
		if((sound[i].pos == sound[i].len / 2) && (sound[i].len != 0))
		{
			if(sound[i].prevbuffstate == BUFFER_OFFSET_FULL)
				sound[i].buffstate = BUFFER_OFFSET_HALF;
		}
		if((sound[i].pos == sound[i].len) && (sound[i].len != 0))
		{
			if(sound[i].prevbuffstate == BUFFER_OFFSET_HALF)
			{
				sound[i].pos = 0;
				sound[i].buffstate = BUFFER_OFFSET_FULL;
			}
		}
		if(sound[i].pos < sound[i].len)
		{
			MixAudio( &audio_buffer[0],
				  sound[i].ptr + \
				  (sound[i].pos / 4),
				  AUDIO_BUFFER_SIZE / 2 );

			sound[i].pos += AUDIO_BUFFER_SIZE / 2;

			channels_playing++;
		}
	}

	if(channels_playing == 0)
	{
		memset(	&audio_buffer[0],
			0,
			AUDIO_BUFFER_SIZE / 2 );		
	}
}

/**
  * @brief  
  * @param  
  * @retval 
  */
int8_t InitSound(void)
{
	if( BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_BOTH, 50, BSP_AUDIO_FREQUENCY_44K) == 0 )
	{
		BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);

		BSP_AUDIO_OUT_Play((uint16_t*)&audio_buffer, AUDIO_BUFFER_SIZE);

		return(0); // success
	}
	else
	{
		return(-1); // error
	}	
}

/**
  * @brief  
  * @param  
  * @retval 
  */
int8_t StartSound(FIL *pFile, uint8_t channel)
{
	uint32_t bytesread;

	if (channel > NUM_CHANNELS)
	{
		return -1;
	}

	f_read (pFile, &WaveFormat, sizeof(WaveFormat), &bytesread);

	sound[channel].ptr = (uint32_t *) malloc(READ_BUFFER_SIZE);
	sound[channel].prevbuffstate = BUFFER_OFFSET_FULL;
	sound[channel].buffstate = BUFFER_OFFSET_NONE;

	f_read (pFile, sound[channel].ptr, READ_BUFFER_SIZE, &bytesread);

	sound[channel].fpos += bytesread;
	sound[channel].state = AUDIO_STATE_PLAY;

	sound[channel].len = READ_BUFFER_SIZE;
	sound[channel].pos = 0;

	return channel;	
}

/**
  * @brief  
  * @param  
  * @retval 
  */
int8_t SoundProcess(FIL *pFile, SoundType *s)
{
	uint32_t bytesread;

	switch(s->state)
	{
		case AUDIO_STATE_PLAY:
			if(s->buffstate == BUFFER_OFFSET_HALF)
			{
				f_read (pFile, s->ptr, READ_BUFFER_SIZE/2, &bytesread);
				s->fpos += bytesread;
				if(!bytesread)
				{
					return(-1); // error
				}
				s->prevbuffstate = BUFFER_OFFSET_HALF;
				s->buffstate = BUFFER_OFFSET_NONE;
			}

			if(s->buffstate == BUFFER_OFFSET_FULL)
			{
				f_read (pFile, s->ptr + (READ_BUFFER_SIZE / 8), READ_BUFFER_SIZE/2, &bytesread);
				s->fpos += bytesread;
				if(!bytesread)
				{
					return(-1); // error
				}
				s->prevbuffstate = BUFFER_OFFSET_FULL;
				s->buffstate = BUFFER_OFFSET_NONE;
			}
			break;

		case AUDIO_STATE_STOP:
			break;

		case AUDIO_STATE_PAUSE:
			break;

		case AUDIO_STATE_RESUME:
			break;

		case AUDIO_STATE_WAIT:
  		case AUDIO_STATE_IDLE:
  		case AUDIO_STATE_INIT:
		default:
			break;
	}

	return(0); // success
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void StopSound(uint8_t channel)
{
	if (sound[channel].ptr != NULL)
	{
		free(sound[channel].ptr);
		sound[channel].ptr = NULL;
		sound[channel].len = 0;
		sound[channel].pos = 0;
		sound[channel].fpos = 0;
		sound[channel].prevbuffstate = BUFFER_OFFSET_NONE;
		sound[channel].buffstate = BUFFER_OFFSET_NONE;
		sound[channel].state = AUDIO_STATE_STOP;
	}
}

/**
  * @brief  
  * @param  
  * @retval 
  */
int8_t SoundIsPlaying(uint8_t channel)
{
	return sound[channel].ptr != NULL;
}
