//
// Created by peter on 12/28/19.
//

#include <cstdlib>
#include <alsa/asoundlib.h>
#include <iostream>

#include <audio/play_music.h>

void play_music(std::string const &filename)
{
    int pcm;
    unsigned int tmp;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    char *buff;
    unsigned long buff_size;

    auto rate = 44100u;

    auto const fd = open(filename.c_str(), 'r');

    /* Open the PCM device in playback mode */
    pcm = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (pcm < 0)
    {
        std::cout << "ERROR: Can't open default device. " << snd_strerror(pcm) << '\n';
    }

    /* Allocate parameters object and fill it with default values*/
    snd_pcm_hw_params_alloca(&params);

    snd_pcm_hw_params_any(pcm_handle, params);

    /* Set parameters */
    pcm = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (pcm < 0)
    {
        std::cout << "ERROR: Can't set interleaved mode." << snd_strerror(pcm) << '\n';
    }

    pcm = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    if (pcm < 0)
    {
        std::cout << "ERROR: Can't set format." << snd_strerror(pcm) << '\n';
    }

    snd_pcm_hw_params_set_channels(pcm_handle, params, 2);
    pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, nullptr);
    if (pcm < 0)
    {
        std::cout << "ERROR: Can't set rate." << snd_strerror(pcm) << '\n';
    }

    /* Write parameters */
    pcm = snd_pcm_hw_params(pcm_handle, params);
    if (pcm < 0)
    {
        printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));
    }

    snd_pcm_hw_params_get_channels(params, &tmp);

    snd_pcm_hw_params_get_rate(params, &tmp, nullptr);

    /* Allocate buffer to hold single period */
    snd_pcm_hw_params_get_period_size(params, &frames, nullptr);
    std::cout << frames << '\n';

    buff_size = frames * 2 * 2;
    buff = (char *) malloc(buff_size);

    snd_pcm_hw_params_get_period_time(params, &tmp, nullptr);

    while (true)
    {

        auto const bytes_read = read(fd, buff, buff_size);
        if (bytes_read == 0)
        {
            break;
        }

        auto const write_result = snd_pcm_writei(pcm_handle, buff, frames);
        if (write_result == -EPIPE)
        {
            std::cout << "XRUN.\n";
            snd_pcm_prepare(pcm_handle);
        } else if (pcm < 0)
        {
            std::cout << "ERROR. Can't write to PCM device. " << snd_strerror(pcm) << "\n";
        }

    }

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    free(buff);
}