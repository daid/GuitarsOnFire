0.8 release of GuitarsOnFire

==This first==
This is a major upgrade on the 0.1a release done a few weeks back. Yes, it still shows '?' menu items, those are placeholders for the 1.0 release.

GuitarsOnFire has a few selling features over GuitarFun:
	* 5 player support, instead of 2
	* Correct notes (not on 100ms intervals) and correct use of the delay= value in the ini file. Makes it playable!
	* no lag (because of preloading)
GuitarsOnFire also has a few selling features over the offical GuitarHero games:
	* Play any FretsOnFire song, not just those included in the GH games
	* Play with 4 guitars, all as guitar, or as bass, whatever you want. Not limited to 1 guitar, 1 bass. (Now also possible in GH5)
GuitarsOnFire even has selling features over FretsOnFire:
	* Playable on the Wii
	* With more then 1 player
	* Can fail a song (more pushing gameplay)

==ChangeLog==

===Version 1.0:===
New Features:
	* Practice mode
	* Versus mode
	* Last man standing mode
	* 50/100/150/200 streak effect
	* GuitarHero World Tour Drums support
	* Experimental USB support
	* Saving settings
	* Saving of top scores and showing which songs you have compleeted already in the song list.
Bugfixes/changes:
	* Song selection redone (now returns to where you last where)
	* Default neck is now gray
	* Custom stages can supply a own neck/effect texture and note 3D models.
	* Ajusted the hammeron note generation, makes less hammerons
	* Fixed crash bug related to audio. Also fixed a few causes where the audio would get out of sync.
	* Make sure they is a bit of time after the game start before the first song.

===Version 0.8:===
New Features:
    * Failing a song when playing badly 
    * Customizable scriptable stages. 3 stages included by default (Guitarfun, GuitarsOnFire, Mario)
    * Lefty flip
    * Load all GFX from png files on SD card
    * Add neck/highway gfx from FretsOnFire
    * 'lower fret overriding'
    * Sound fix when no song.ogg is found
    * Gamecube controller support (Untested)
    * Classic controller support (Untested)
    * 'No songs' message when no songs are found
    * Keyboard keymap customizing
	* Basic Support for drum tracks
Bugfixes:
    * Added 'start' key for keyboard
    * Load drums.ogg if it exists
    * Fix scoring for sustained notes
    * Remove tails for short notes
    * Not showing a difficulty if there are less then 10 notes (some songs just add a single note to show up as having all difficulties)
    * Crispy sound. (Sound quality is improved, because of a bug in the upsampling from 44.1kHz to 48kHz)

==Licence info==
Everything is under GPL v3, and source should be available at:
http://wiibrew.org/wiki/User:Daid/GuitarsOnFire
Or in a page linked from:
http://wiibrew.org/wiki/User:Daid

==What now==
Well, one of the downloads includes a few free sample songs from FretsOnFire. The default FretsOnFire songs have been reduced in quality to keep the download size smaller.
All FretsOnFire songs should work with GuitarsOnFire. You can find many more songs all over the internet.

Guitars on Fire loads songs from the following locations on your SD card:
/GuitarsOnFire/songs/
/guitarfun/songs/
/apps/GuitarsOnFire/songs/
/apps/guitarfun/songs/
So it should be path compatible with GuitarFun.

The other data files (/sfx/ /gfx/ /stage/) are ALWAYS loaded from /GuitarsOnFire/* so don't move those files.

==Custom stages==
You can build custom stages yourself. As an example look at the GuitarFun stage, which is very simpel. A stage 'viewer' for windows is also included.
The 'stage viewer' automaticly reloads the stage when you change the stage.lua file, so you can quickly test the effects of changes.

Don't create stages from copyrighted stuff, so I won't tollerate copies from GuitarHero stages. No. I won't. Don't even think about it.
and if anyone builds a stage off the 'metallicops' animation graphics then he wins a free cookie! Because that would just be fucking awesome.

==Controls==
Wii Guitars are the main target. Every Wii wireless guitar I've heard of works so far, no reports of not working guitars! (No drums yet)
The mapping on those matches Guitar Hero (3/WT/Metallica) Keyboard emulates the Guitar buttons.
You can play with a classic controller or a GameCube controller, but that isn't the selling feature. Guitar is first, then keyboard, then the rest.
Whammy bar and touch bar (for world tour guitars) are read, but not yet used. Custom stages could use them, but none of the default do.

And the 'home' key on the first wiimote is a quick escape key, which will dump you back to your loader (homebrew channel in most cases)

Note on the gamecube controllers, if no useable extention is found on a wiimote, then it tries to use a gamecube controller on that slot number.
So if you want to use 1 guitar and 1 gamecube controller you should connect the guitar to wiimote 1 and the gamecube controller to slot 2.

==Special thanks==
Everyone that reported issues on the Talk page: http://wiibrew.org/wiki/Talk:GuitarsOnFire
Without them this 0.8 release couldn't have happened.
The FretsOnFire people, they made a great game. And I used quite a bit of graphics from them.
Whoever made Guitarfun, while it sucks on itself, it made me make GuitarsOnFire. And it has some decent graphics that I used.
The 5k+ downloads on HBB. While I don't think it's an accurate number, it does say something.

Not thanking libogc, the library is fine. But it lacks documentation everywhere, making you seek out rare and sometimes wrong examples.
