#include "AudioPlayer.h"
#include "Log.h"
#include "miniaudio.h"

#include <algorithm>
#include <cstring>

namespace sn
{
void data_callback(ma_device*                   device,
                   void*                        output,
                   [[maybe_unused]] const void* input,
                   ma_uint32                    required_output_frame_count)
{
    auto* samples = static_cast<float*>(output);
    std::fill_n(samples, required_output_frame_count, 0.0f);

    if (device->pUserData == nullptr)
    {
        return;
    }

    CallbackData& cb_data = *(CallbackData*)device->pUserData;

    if (cb_data.mute)
    {
        return;
    }

    if (cb_data.remaining_buffer_rounds-- > 0)
    {
        return;
    }

    ma_uint64 presample_input_frames = 0;
    ma_result result                 = ma_resampler_get_required_input_frame_count(
      cb_data.resampler, required_output_frame_count, &presample_input_frames);
    if (result != MA_SUCCESS)
    {
        presample_input_frames =
          required_output_frame_count * cb_data.resampler->sampleRateIn / cb_data.resampler->sampleRateOut;
    }

    presample_input_frames = std::min<ma_uint64>(presample_input_frames,
                                                cb_data.input_frames_buffer.size());
    ma_uint64 presample_frames_avail =
      cb_data.ring_buffer.pop(cb_data.input_frames_buffer.data(), presample_input_frames);
    if (presample_frames_avail < presample_input_frames)
    {
        const float heldSample = presample_frames_avail > 0
                               ? cb_data.input_frames_buffer[presample_frames_avail - 1]
                               : 0.0f;
        for (auto idx = presample_frames_avail; idx < presample_input_frames; ++idx)
        {
            cb_data.input_frames_buffer[idx] = heldSample;
        }
    }

    ma_uint64 output_frame_count64 = required_output_frame_count;
    ma_resampler_process_pcm_frames(cb_data.resampler,
                                    cb_data.input_frames_buffer.data(),
                                    &presample_input_frames,
                                    output,
                                    &output_frame_count64);
}

bool AudioPlayer::start()
{
    ma_resampler_config config = ma_resampler_config_init(ma_format_f32,
                                                          1,
                                                          input_sample_rate,
                                                          output_sample_rate,
                                                          ma_resample_algorithm_linear);
    auto result = ma_resampler_init(&config, nullptr, &resampler);
    if (result != MA_SUCCESS)
    {
        LOG(Error) << "Failed to initialize audio resampler: error code = " << result << std::endl;
        return false;
    }

    const auto bufferedInputFrames = static_cast<std::size_t>(input_sample_rate) *
                                     callback_period_ms.count() / 500 + 4096;
    cb_data.input_frames_buffer.assign(bufferedInputFrames, 0.0f);

    deviceConfig                          = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format          = ma_format_f32;
    deviceConfig.playback.channels        = 1;
    deviceConfig.sampleRate               = output_sample_rate;
    deviceConfig.dataCallback             = data_callback;
    deviceConfig.pUserData                = &cb_data;
    deviceConfig.periodSizeInMilliseconds = callback_period_ms.count();

    result                                = ma_device_init(nullptr, &deviceConfig, &device);
    if (result != MA_SUCCESS)
    {
        LOG(Error) << "Failed to open playback device: error code = " << result << std::endl;
        ma_resampler_uninit(&resampler, nullptr);
        return false;
    }

    result = ma_device_start(&device);
    if (result != MA_SUCCESS)
    {
        LOG(Error) << "Failed to start playback device: error code = " << result << std::endl;
        ma_device_uninit(&device);
        ma_resampler_uninit(&resampler, nullptr);
        return false;
    }

    initialized = true;
    return true;
}

AudioPlayer::~AudioPlayer()
{
    if (!initialized)
    {
        return;
    }

    ma_device_uninit(&device);
    ma_resampler_uninit(&resampler, nullptr);
}

void AudioPlayer::mute()
{
    cb_data.mute = true;
}

}