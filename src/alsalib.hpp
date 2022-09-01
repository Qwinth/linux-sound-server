#include <string>
#include <alsa/asoundlib.h>

class PCM {
	snd_pcm_uframes_t frames;
	
	
	_snd_pcm_format _format;
	public:
	snd_pcm_hw_params_t *params;
	snd_pcm_t *pcm;
		PCM() {}
		PCM(std::string device, _snd_pcm_stream stream, int mode) {
			snd_pcm_open(&pcm, device.c_str(), stream, mode);
			snd_pcm_hw_params_malloc(&params);
			snd_pcm_hw_params_any(pcm, params);
		}

		~PCM() {
			snd_pcm_hw_params_free(params);
		}
		
		int setAccess(_snd_pcm_access _access) {
			return snd_pcm_hw_params_set_access(pcm, params, _access);
		}

		int setFormat(_snd_pcm_format format) {
			_format = format;
			return snd_pcm_hw_params_set_format(pcm, params, format);
		}

		int setChannels(int channels) {
			return snd_pcm_hw_params_set_channels(pcm, params, channels);
		}

		int setRate(unsigned int rate, int dir) {
			return snd_pcm_hw_params_set_rate_near(pcm, params, &rate, &dir);
		}

		int paramsApply() {
			return snd_pcm_hw_params(pcm, params);
		}

		std::string getName() {
			return std::string(snd_pcm_name(pcm));
		}

		std::string getState() {
			return std::string(snd_pcm_state_name(snd_pcm_state(pcm)));
		}

		int getChannels() {
			unsigned int tmp;
			snd_pcm_hw_params_get_channels(params, &tmp);
			return tmp;
		}

		int getRate(int dir) {
			unsigned int tmp;
			snd_pcm_hw_params_get_rate(params, &tmp, &dir);
			return tmp;
		}

		int getPeriod(int dir) {
			snd_pcm_hw_params_get_period_size(params, &frames, &dir);
			return frames;
		}

		int getFormatWidth() {
			return snd_pcm_format_width(_format);
		}

		int prepare() {
			return snd_pcm_prepare(pcm);
		}

		int recover(int err, int silent) {
			return snd_pcm_recover(pcm, err, silent);
		}

		int writei(char * buff, snd_pcm_uframes_t frames) {
			return snd_pcm_writei(pcm, buff, frames);
		}

		int readi(char * buff) {
			return snd_pcm_readi(pcm, buff, frames);
		}

		int drain() {
			return snd_pcm_drain(pcm);
		}

		int drop() {
			return snd_pcm_drop(pcm);
		}

		int close() {
			return snd_pcm_close(pcm);
		}
};
