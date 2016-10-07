#include "audio_source_position.h"
#include "halley/core/api/audio_api.h"

using namespace Halley;


AudioSourcePosition::AudioSourcePosition()
	: isUI(true)
	, isPannable(false)
{
}

AudioSourcePosition AudioSourcePosition::makeUI(float pan)
{
	auto result = AudioSourcePosition();
	result.pos = Vector3f(pan, 0, 0);
	result.isUI = true;
	result.isPannable = true;
	return result;
}

AudioSourcePosition AudioSourcePosition::makePositional(Vector2f pos, float referenceDistance, float maxDistance)
{
	return makePositional(Vector3f(pos), referenceDistance, maxDistance);
}

AudioSourcePosition AudioSourcePosition::makePositional(Vector3f pos, float referenceDistance, float maxDistance)
{
	auto result = AudioSourcePosition();
	result.pos = pos;
	result.isUI = false;
	result.isPannable = true;
	result.referenceDistance = std::max(0.1f, referenceDistance);
	result.maxDistance = std::max(result.referenceDistance, maxDistance);
	return result;
}

AudioSourcePosition AudioSourcePosition::makeFixed()
{
	auto result = AudioSourcePosition();
	result.isUI = true;
	result.isPannable = false;
	return result;
}

static float gain2DPan(float srcPan, float dstPan)
{
	return std::max(0.0f, 1.0f - 0.5f * std::abs(srcPan - dstPan));
}

void AudioSourcePosition::setMix(size_t nSrcChannels, gsl::span<const AudioChannelData> dstChannels, gsl::span<float, 16> dst, float gain, const AudioListenerData& listener) const
{
	const size_t nDstChannels = size_t(dstChannels.size());

	if (isPannable) {
		// Pannable (mono) sounds
		Expects(nSrcChannels == 1);
		if (isUI) {
			// UI sound
			for (size_t i = 0; i < nDstChannels; ++i) {
				dst[i] = gain2DPan(pos.x, dstChannels[i].pan) * gain;
			}
		} else {
			// In-world sound
			auto delta = pos - listener.position;
			float pan = clamp(delta.x * 0.01f, 0.0f, 1.0f);
			float len = delta.length();

			float rolloff = 1.0f - clamp((len - referenceDistance) / (maxDistance - referenceDistance), 0.0f, 1.0f);

			for (size_t i = 0; i < nDstChannels; ++i) {
				dst[i] = gain2DPan(pan, dstChannels[i].pan) * gain * rolloff;
			}
		}
	} else {
		// Non-pannable (stereo) sounds
		Expects(nSrcChannels == 2);
		for (size_t i = 0; i < nSrcChannels; ++i) {
			float srcPan = i == 0 ? -1.0f : 1.0f;
			for (size_t j = 0; j < nDstChannels; ++j) {
				dst[i * nSrcChannels + j] = gain2DPan(srcPan, dstChannels[j].pan) * gain;
			}
		}
	}
}