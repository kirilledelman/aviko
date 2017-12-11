#include "Sound.hpp"
#include "Application.hpp"

Sound* Sound::musicSound = NULL;
unordered_map<int, Sound*> Sound::soundChannels;

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

Sound::Sound( ScriptArguments* args ) {
	
	// add scriptObject
	script.NewScriptObject<Sound>( this );
	
	// with arguments
	if ( args && args->args.size() ) {
		// first argument can be a string
		if ( args->args[ 0 ].type == TypeString ) {
			script.SetProperty( "source", args->args[ 0 ], this->scriptObject );
		}
	}
	
}

Sound::Sound() {}

Sound::~Sound() {
	// clean up
	if ( this->soundResource ) {
		// stop if playing
		if ( this->playing ) {
			this->Stop();
		}
		// release resource
		this->soundResource->AdjustUseCount( -1 );
		this->soundResource = NULL;
	}
	
}


/* MARK:	-				Scripting
 -------------------------------------------------------------------- */


void Sound::InitClass() {
	
	// create class
	script.RegisterClass<Sound>( "ScriptableObject" );
	
	// props
	
	script.AddProperty<Sound>
	( "source",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		Sound* rs = (Sound*) b;
		if ( rs->soundResource ) {
			return ArgValue( rs->soundResource->key.c_str() );
		} else {
			return ArgValue(); // undefined
		}
	} ),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		Sound* rs = (Sound*) b;
		SoundResource* res = NULL;
		
		// make sure it exists
		if ( val.type == TypeString ) {
			res = app.soundManager.Get( val.value.stringValue->c_str() );
			if ( res->error == ERROR_NONE ) {
				res->AdjustUseCount( 1 );
			} else {
				res = NULL;
			}
		}
		
		// if playing, stop
		if ( rs->playing ) rs->Stop();
		
		// clear previous
		if ( rs->soundResource ) rs->soundResource->AdjustUseCount( -1 );
		
		// set new
		rs->soundResource = res;
		return val;
	}));
	
	script.AddProperty<Sound>
	( "playing",
	 static_cast<ScriptBoolCallback>([](void *b, bool val) { return ((Sound*) b)->playing; }));
	
	script.AddProperty<Sound>
	( "paused",
	 static_cast<ScriptBoolCallback>([](void *b, bool val) { return ((Sound*) b)->paused; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val) {
		Sound* snd = (Sound*) b;
		// if sound is playing
		if ( snd->playing ) {
			if ( val )
				snd->Pause();
			else
				snd->Play();
		}
		return val;
	}) );
	
	script.AddProperty<Sound>
	( "volume",
	 static_cast<ScriptFloatCallback>([](void *b, float val) { return ((Sound*) b)->volume; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Sound* snd = (Sound*) b;
		snd->SetVolume( val );
		return snd->volume;
	}) );
	
	script.AddProperty<Sound>
	( "pan",
	 static_cast<ScriptFloatCallback>([](void *b, float val) { return ((Sound*) b)->pan; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Sound* snd = (Sound*) b;
		snd->SetPan( val );
		return snd->pan;
	}) );
	
	// functions
	
	script.DefineFunction<Sound>
	( "play", // play( [ Int times=1 ] )
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		Sound* snd = (Sound*) obj;
		int loops = 1;
		if ( sa.args.size() && !sa.ReadArguments( 1, TypeInt, &loops ) ){
			script.ReportError( "usage: play( [ Integer numLoops ] )" );
			return false;
		}
		snd->Play( loops - 1 );
		return true;
	}));

	script.DefineFunction<Sound>
	( "pause", // pause()
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		Sound* snd = (Sound*) obj;
		if ( snd->playing && !snd->paused ) {
			sa.ReturnBool( true );
			snd->Pause();
		} else {
			sa.ReturnBool( false );
		}
		return true;
	}));
	
	script.DefineFunction<Sound>
	( "stop", // stop()
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		Sound* snd = (Sound*) obj;
		if ( snd->playing ) {
			sa.ReturnBool( true );
			snd->Stop();
		} else {
			sa.ReturnBool( false );
		}
		return true;
	}));

	// register hooks
	Mix_HookMusicFinished( &Sound::MusicFinished );
	Mix_ChannelFinished( &Sound::ChannelFinished );

}


/* MARK:	-				Control
 -------------------------------------------------------------------- */

/// set volume on the fly
void Sound::SetVolume( float vol ) {
	this->volume = max( 0.0f, min( 1.0f, vol ) );
	// if sound is playing
	if ( this->playing && this->soundResource ) {
		if ( this->soundResource->sample ) {
			Mix_Volume( this->soundChannel, 128 * this->volume );
		} else {
			Mix_VolumeMusic( 128 * this->volume );
		}
	}
}

/// set pan on the fly
void Sound::SetPan( float p ) {
	this->pan = max( -1.0f, min( 1.0f, p ) );
	// if sound is playing
	if ( this->playing && this->soundResource ) {
		if ( this->soundResource->sample ) {
			Uint8 left = 255;
			Uint8 right = 255;
			if ( pan < 0 ) { right += pan * 254; }
			else if ( pan > 0 ) { left -= pan * 254; }
			Mix_SetPanning( this->soundChannel, left, right );
		}
	}
}

/// play sound
void Sound::Play( int loops ) {

	// make sure sound is ready
	if ( !this->soundResource || this->soundResource->error != ERROR_NONE ) return;
	
	// if paused, resume
	if ( this->playing && this->paused ) {
		
		// resume sample
		if ( this->soundResource->sample ) {
			Mix_Resume( this->soundChannel );
		// resume music
		} else if ( this->soundResource->music ) {
			if ( Sound::musicSound && Sound::musicSound != this && Sound::musicSound->playing ) Sound::musicSound->Pause();
			Sound::musicSound = this;
			Mix_ResumeMusic();
		}
		
		// prevent this sound from being garbage collected
		script.ProtectObject( &this->scriptObject, true );
		
		// flag
		this->paused = false;
		return;
	}
	
	// reset flags
	this->playing = false;
	this->paused = false;
	
	// sample?
	if ( this->soundResource->sample ) {
		this->soundChannel = Mix_PlayChannel( -1, this->soundResource->sample, loops );
		if ( this->soundChannel == -1 ) {
			printf( "Error: %s\n", Mix_GetError() );
			return;
		}
		// store
		Sound::soundChannels[ this->soundChannel ] = this;
		
	// music
	} else if ( this->soundResource->music ){
		if ( Mix_PlayMusic( this->soundResource->music, loops ) == -1 ){
			printf( "Error: %s\n", Mix_GetError() );
			return;
		}
		// stop other
		if ( Sound::musicSound && Sound::musicSound != this && Sound::musicSound->playing ) Sound::musicSound->Pause();
		// store
		Sound::musicSound = this;
	}
	
	// prevent this sound from being garbage collected
	script.ProtectObject( &this->scriptObject, true );
	
	// set
	this->playing = true;
	this->paused = false;
	
	// set vol and pan
	this->SetPan( this->pan );
	this->SetVolume( this->volume );
}

void Sound::Pause() {
	
	// make sure sound is ready
	if ( !this->soundResource || this->soundResource->error != ERROR_NONE ) return;
	
	// pause channel
	if ( this->soundResource->sample ) {
		Mix_Pause( this->soundChannel );
	// pause music
	} else if ( this->soundResource->music ) {
		Mix_PauseMusic();
	}

	// flag
	this->paused = true;
	
	// allow this sound from being garbage collected
	script.ProtectObject( &this->scriptObject, false );
	
}

void Sound::Stop() {
	
	// make sure sound is ready
	if ( !this->soundResource || this->soundResource->error != ERROR_NONE ) return;
	
	// stop channel
	if ( this->soundResource->sample ) {
		Mix_HaltChannel( this->soundChannel );
		if ( Sound::soundChannels[ this->soundChannel ] == this )
			Sound::soundChannels.erase( this->soundChannel );
	// stop music
	} else if ( this->soundResource->music ) {
		Mix_HaltMusic();
		if ( Sound::musicSound == this ) Sound::musicSound = NULL;
	}
	
	// reset
	this->playing = false;
	this->paused = false;
	
	// allow this sound from being garbage collected
	script.ProtectObject( &this->scriptObject, false );
	
}

/// callback for when sound finished playing
void Sound::Finished() {
	// reset
	this->playing = false;
	
	// dispatch event
	Event event( EVENT_FINISHED, this->scriptObject );
	this->CallEvent( event );
	
	// allow this sound from being garbage collected
	script.ProtectObject( &this->scriptObject, false );
}


/* MARK:	-				Mixer callbacks
 -------------------------------------------------------------------- */


/// called by mixer when channel finishes
void Sound::ChannelFinished( int channel ) {
	// find sound
	unordered_map<int, Sound*>::iterator it = Sound::soundChannels.find( channel );
	if ( it != Sound::soundChannels.end() ) {
		Sound* snd = it->second;
		if ( snd->soundChannel == channel ) snd->Finished();
	}
}

void Sound::MusicFinished() {
	// music sound
	if ( musicSound != NULL ) {
		// notify, clear
		musicSound->Finished();
		musicSound = NULL;
	}

}



