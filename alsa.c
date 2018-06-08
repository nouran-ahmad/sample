#include <alsa/asoundlib.h>

static char *device = "default";                        /* playback device */
static snd_output_t *output = NULL;

int playback(const void* buffer, int length)
{
        int err;
        unsigned int i;
        snd_pcm_t *handle;
        snd_pcm_sframes_t frames;
        
	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
                printf("Playback open error: %s\n", snd_strerror(err));
                return -1; 
        }
        if ((err = snd_pcm_set_params(handle,
                                      SND_PCM_FORMAT_S16,
                                      SND_PCM_ACCESS_RW_INTERLEAVED,
                                      1,
                                      18000,
                                      1,
                                      100000)) < 0) {   /* 0.5sec */
                printf("Playback open error: %s\n", snd_strerror(err));
		return -1;        
	}


	for (i = 0; i < 2; i++) {
                frames = snd_pcm_writei(handle, buffer, length+4000);

		if(frames>=0) break;                

		if (frames < 0)
                        frames = snd_pcm_recover(handle, frames, 0);
                if (frames < 0) {
                        printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
                        break;
                }
                if (frames > 0 && frames < (long) length)
                        printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buffer), frames);

        }

        snd_pcm_close(handle);
        return 0;
}
