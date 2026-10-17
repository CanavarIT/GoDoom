#include "doomtype.h"
#include "i_sound.h"

int snd_musicdevice = 0;

void I_InitSound(boolean use_sfx_prefix) { (void)use_sfx_prefix; }
void I_ShutdownSound(void) {}
int  I_GetSfxLumpNum(sfxinfo_t *sfxinfo) { (void)sfxinfo; return 0; }
void I_UpdateSound(void) {}
void I_UpdateSoundParams(int channel, int vol, int sep) { (void)channel; (void)vol; (void)sep; }
int  I_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep) { (void)sfxinfo; (void)vol; (void)sep; return channel; }
void I_StopSound(int channel) { (void)channel; }
boolean I_SoundIsPlaying(int channel) { (void)channel; return false; }
void I_PrecacheSounds(sfxinfo_t *sounds, int num_sounds) { (void)sounds; (void)num_sounds; }
void I_BindSoundVariables(void) {}

void I_InitMusic(void) {}
void I_ShutdownMusic(void) {}
void I_SetMusicVolume(int volume) { (void)volume; }
void I_PauseSong(void) {}
void I_ResumeSong(void) {}
void *I_RegisterSong(void *data, int len) { (void)data; (void)len; return NULL; }
void I_UnRegisterSong(void *handle) { (void)handle; }
void I_PlaySong(void *handle, boolean looping) { (void)handle; (void)looping; }
void I_StopSong(void) {}
boolean I_MusicIsPlaying(void) { return false; }