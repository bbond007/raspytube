#include "bcm_host.h"
#include "ilclient.h"

typedef struct {
   sem_t sema;
   ILCLIENT_T *client;
   COMPONENT_T *audio_render;
   COMPONENT_T *list[2];
   OMX_BUFFERHEADERTYPE *user_buffer_list; // buffers owned by the client
   uint32_t num_buffers;
   uint32_t bytes_per_sample;
} AUDIOPLAY_STATE_T;


typedef struct 
{
   AUDIOPLAY_STATE_T *st;
   uint8_t *outputbuffer;
   int buffer_size;
   int samplerate;
} AudioSampleInfo;


void load_sample(AudioSampleInfo * audioSampleInfo, uint8_t *buffer, int buffer_size, int samplerate, int bitdepth, int nchannels, int dest);
void play_sample(AudioSampleInfo * audioSampleInfo, bool wait);
void delete_sample(AudioSampleInfo * audioSampleInfo);
void play_sound_buffer(uint8_t *buffer, int buffer_size, int samplerate, int bitdepth, int nchannels, int dest);

