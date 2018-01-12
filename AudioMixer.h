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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_MIXER_H
#define __AUDIO_MIXER_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "ff.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
  AUDIO_STATE_IDLE = 0,
  AUDIO_STATE_WAIT,    
  AUDIO_STATE_INIT,    
  AUDIO_STATE_PLAY,
  AUDIO_STATE_PRERECORD,
  AUDIO_STATE_RECORD,  
  AUDIO_STATE_NEXT,  
  AUDIO_STATE_PREVIOUS,
  AUDIO_STATE_FORWARD,   
  AUDIO_STATE_BACKWARD,
  AUDIO_STATE_STOP,   
  AUDIO_STATE_PAUSE,
  AUDIO_STATE_RESUME,
  AUDIO_STATE_VOLUME_UP,
  AUDIO_STATE_VOLUME_DOWN,
  AUDIO_STATE_ERROR,  
}AUDIO_PLAYBACK_StateTypeDef;

typedef enum {
   BUFFER_OFFSET_NONE = 0,
   BUFFER_OFFSET_HALF,
   BUFFER_OFFSET_FULL,
}BUFFER_StateTypeDef;

typedef struct _SoundType
{
	uint32_t *ptr;
	uint32_t len;
	uint32_t pos;
	uint32_t fpos;

	BUFFER_StateTypeDef prevbuffstate;
	BUFFER_StateTypeDef buffstate;
	AUDIO_PLAYBACK_StateTypeDef state;
} SoundType;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern SoundType sound[];

/* Exported functions ------------------------------------------------------- */
int8_t InitSound(void);
int8_t StartSound(FIL *pFile, uint8_t channel);
int8_t SoundProcess(FIL *pFile, SoundType *s);
void StopSound(uint8_t channel);
int8_t SoundIsPlaying(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_MIXER_H */
