#ifndef Sound_hpp
#define Sound_hpp

#include "common.h"
#include "ScriptableClass.hpp"
#include "SoundResource.hpp"

/// class for playing sound
class Sound : public ScriptableClass {
public:
	
	/// used to route notification of sound ending
	static unordered_map<int, Sound*> soundChannels;
	static Sound* musicSound;
	
	SoundResource* soundResource = NULL;
	
	int soundChannel = -1;
	bool paused = false;
	bool playing = false;
	
	float volume = 0.75f;
	float pan = 0;
	
	// scripting
	
	static void InitClass();	
	
	// actions
	
	void Play( int loops=0 );
	void Pause();
	void Stop();
	void Finished();
	void SetVolume( float vol );
	void SetPan( float p );
	
	// callbacks for sound stop
	static void ChannelFinished( int );
	static void MusicFinished();
	
	// init/destroy
	Sound();
	Sound( ScriptArguments* args );
	~Sound();
	
};

SCRIPT_CLASS_NAME( Sound, "Sound" );

#endif /* Sound_hpp */
