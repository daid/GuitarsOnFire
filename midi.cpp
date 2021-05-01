#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midi.h"
#include "strfunc.h"
#include "input.h"

/*
The following midi loading code is based on the Guitarfun code. Which is a piece of shit, but works.
	Guitarfun is GPL, and so is all of this. So license is fine.
*/
/****************************************************************************************************************************************/
// MIDI
/****************************************************************************************************************************************/
//midi for fof start at 60 and add 12 for each difficulty.
#define NOTE_BASE 60
#define NOTE_MAX (12 * 4)

#define MIDI_MAX_TRACKS (MAX_INSTRUMENT + 1)

static int MIDI_MAX_SIZE = 0;

static int track_status[MIDI_MAX_TRACKS + 1];
static int track_offset[MIDI_MAX_TRACKS + 1];
static int track_end[MIDI_MAX_TRACKS + 1];
static int pri_track_timer[MIDI_MAX_TRACKS + 1];
static int track_timer[MIDI_MAX_TRACKS + 1];
static double track_dtimer[MIDI_MAX_TRACKS + 1];
static int track_ticks[MIDI_MAX_TRACKS + 1];
static int midi_notes_keys[MIDI_MAX_TRACKS + 1][NOTE_MAX];
static int midi_keyonoff[MIDI_MAX_TRACKS + 1];

static unsigned char *midi_file = NULL;

static int midi_pos = 0;

static int midi_mode = 0;
static int midi_tracks = 0;

static int midi_ticks = 96;		// divisor tiempo de cuarto de nota del MIDI
static int midi_tempo = 500000;
static int midi_ticks_quarter = 24;	// ticks por cuarto de nota

int open_midi(char *name);
/****************************************************************************************************************************************/
// !MIDI
/****************************************************************************************************************************************/

TTrackInfo track;

void set_note(int instrument, int note, int time_pos, int time_len)
{
	fprintf(stderr, "Note %u) %u / %u\n", note, time_pos, time_len);
	int difficulty = note / 12;
	int num = note % 12;
	if (difficulty < 0 || difficulty >= MAX_DIFFICULTY)
        return;
    
	if (num < 0 || num > 4)
        return;
    if (instrument < 0 || instrument >= MAX_INSTRUMENT)
        return;
    
	TNote* n = track.difficulty[difficulty].noteList[instrument];
	while(n && n->next && n->time < time_pos)
	{
	    n = n->next;
	}
	
	if (n != NULL && n->time == time_pos)
	{
	    n->mask |= (1 << num);
	    return;
	}
	
	TNote* nn = new TNote();
	nn->length = time_len;
	nn->time = time_pos;
	nn->next = NULL;
	nn->mask = (1 << num);
	
	if (n == NULL)
	{
		track.difficulty[difficulty].noteList[instrument] = nn;
	}else{
	    nn->next = n->next;
	    n->next = nn;
	}
}

int TNote::match(int keyMask)
{
    if (type == NT_Pull || type == NT_Single)
    {
        if ((mask & keyMask) && (keyMask & ~mask) < mask)
        {
            return 1;
        }
        return 0;
    }
    return mask == keyMask;
}

int TNote::matchDrums(int keyMask)
{
    int note = 0;
    int mis = 0;
    if (mask & 0x01)
    {
        if (!(keyMask & BUTTON_DRUM_BASS))
            return 0;
    }
    if (mask & 0x02)
    {
        note++;
        if (!(keyMask & BUTTON_DRUM_PAD(0)))
            mis++;
    }
    if (mask & 0x04)
    {
        note++;
        if (!(keyMask & BUTTON_DRUM_PAD(1)))
            mis++;
    }
    if (mask & 0x08)
    {
        note++;
        if (!(keyMask & BUTTON_DRUM_PAD(2)))
            mis++;
    }
    if (mask & 0x10)
    {
        note++;
        if (!(keyMask & (BUTTON_DRUM_PAD(3) | BUTTON_DRUM_PAD(4))))
            mis++;
    }
    if (note < 3)
    {
        if (mis == 0)
            return 1;
    }else{
        if (mis <= note - 2)
            return 1;
    }
    return 0;
}

TDifficulty::TDifficulty()
{
    noteList[0] = NULL;
    noteList[1] = NULL;
}

void TDifficulty::reset()
{
    for(int i=0;i<MAX_INSTRUMENT;i++)
    {
        while(noteList[i])
        {
            TNote* n = noteList[i];
            noteList[i] = n->next;
            delete n;
        }
        noteList[i] = NULL;
    }
}

TTrackInfo::TTrackInfo()
{
    reset();
}

void TTrackInfo::reset()
{
    for(int i=0;i<MAX_DIFFICULTY;i++)
    {
        difficulty[i].reset();
    }
}

void TTrackInfo::load(const char* path)
{
    char filename[1024];

    nprintf(filename, 1024, "%s/notes.mid", path);

    reset();
    open_midi(filename);
    
    for(int i=0;i<MAX_DIFFICULTY;i++)
    {
        for(int j=0;j<MAX_INSTRUMENT;j++)
        {
            int lastMask = -1;
            int prevTime = -5000;
            int noteCount = 0;
            int max_pulltime = 200;
            
            for(TNote* n = difficulty[i].noteList[j]; n; n = n->next)
            {
                noteCount++;
                
                n->type = NT_Coord;
                if (n->mask == 1 || n->mask == 2 || n->mask == 4 || n->mask == 8 || n->mask == 16)
                {
                    n->type = NT_Single;
                    
                    if (!(lastMask & n->mask) && n->time - prevTime < max_pulltime)
                    {
                        n->type = NT_Pull;
                    }
                }
                
                lastMask = n->mask;
                prevTime = n->time;// + n->length;
            }
            
            if (noteCount < 10)
            {
                //Almost no notes, just clear this difficulty.
                while(difficulty[i].noteList[j])
                {
                    TNote* n = difficulty[i].noteList[j];
                    difficulty[i].noteList[j] = n->next;
                    delete n;
                }
                difficulty[i].noteList[j] = NULL;
            }
        }
    }
    
    beatsPerMinute = 60000000.0 / float(midi_tempo);

    for(int i=0;i<MAX_DIFFICULTY;i++)
    {
        for(int j=0;j<MAX_INSTRUMENT;j++)
        {
            for(TNote* n = difficulty[i].noteList[j]; n; n = n->next)
            {
                n->time += currentSongEntry->info->delay;
            }
        }
    }
}

/*
The following midi loading code is based on the Guitarfun code. Which is a piece of shit, but works.
	Guitarfun is GPL, and so is all of this. So license is fine.
*/

/****************************************************************************************************************************************/
// MIDI
/****************************************************************************************************************************************/

static int get_32b()
{
	int r = 0;

	r = midi_file[midi_pos];
	midi_pos++;
	r = (r << 8) + midi_file[midi_pos];
	midi_pos++;
	r = (r << 8) + midi_file[midi_pos];
	midi_pos++;
	r = (r << 8) + midi_file[midi_pos];
	midi_pos++;

	return r;
}

static int get_24b()
{
	int r = 0;

	r = midi_file[midi_pos];
	midi_pos++;
	r = (r << 8) + midi_file[midi_pos];
	midi_pos++;
	r = (r << 8) + midi_file[midi_pos];
	midi_pos++;

	return r;
}

static int get_16b()
{
	int r = 0;

	r = midi_file[midi_pos];
	midi_pos++;
	r = (r << 8) + midi_file[midi_pos];
	midi_pos++;

	return r;
}

static unsigned ReadVlen()
{
	register unsigned val;
	register unsigned char c;

	val = midi_file[midi_pos];
	midi_pos++;
	if (val & 0x80)
	{
		val &= 0x7f;
		do
		{
			c = midi_file[midi_pos];
			midi_pos++;
			val = (val << 7) | (c & 0x7f);
		}
		while (c & 0x80);
	}

	return (val);
}


static double d_get_time_midi(unsigned t)
{
	double time;
	double tempo;

	time = ((double) (midi_ticks * 24));


	tempo = ((double)midi_tempo) / 1000.0;
	time = ((tempo * ((double) t)) * ((double) midi_ticks_quarter)) / time;

	return time;
}

int open_midi(char *name)
{
	static FILE *FMID = NULL;
	int n = 0, l = 0;
	int flag = 255;

	int mid_time = 0;
	int mid_time_ms = 0;

	for (n = 0; n < MIDI_MAX_TRACKS; n++)
		track_status[n] = 0;	// tracks off


	FMID = fopen(name, "rb");

	if (FMID == NULL)
	{
		printf("MIDI: open fail");
		return -1;
	}

	fseek(FMID, 0, SEEK_END);
	MIDI_MAX_SIZE = ftell(FMID);
	fseek(FMID, 0, SEEK_SET);

	if (MIDI_MAX_SIZE <= 0)
	{
		MIDI_MAX_SIZE = 0;
		fclose(FMID);
		return -1;
	}

	midi_file = (unsigned char *) malloc(MIDI_MAX_SIZE + 16);


	if (midi_file == NULL)
	{
		MIDI_MAX_SIZE = 0;
		fclose(FMID);
		FMID = 0;
		printf("MIDI: malloc fail");
		return -1;
	}

	if ((int)fread(midi_file, 1, MIDI_MAX_SIZE, FMID) != MIDI_MAX_SIZE)
	{
		free(midi_file);
		midi_file = 0;
		MIDI_MAX_SIZE = 0;
		fclose(FMID);
		FMID = 0;
		printf("MIDI: truncated file");
		return -1;
	}

	fclose(FMID);
	FMID = 0;

	midi_pos = 0;

	mid_time = 0;				// contador de ticks global

	midi_ticks = 96;			// divisor tiempo de cuarto de nota del MIDI
	midi_ticks_quarter = 24;	// ticks por cuarto de nota
	midi_tempo = 500000;		// tiempo en microsegundos de un cuarto de nota

	if (!strncmp((char *) &midi_file[midi_pos], "MThd", 4))
	{
		int len_header = 0;

		midi_pos += 4;
		len_header = get_32b();

		//printf("MIDI Header len %i\n",len_header);

		midi_mode = get_16b();
		midi_tracks = get_16b();
		midi_ticks = get_16b();

		n = 6;

		while (n < len_header)
		{
			midi_pos++;
			n++;
		}

		for (l = 0; l < midi_tracks && l < MIDI_MAX_TRACKS; l++)
		{
			if (!strncmp((char *) &midi_file[midi_pos], "MTrk", 4))
			{
				track_status[l] = 1;
				track_timer[l] = 0;
				pri_track_timer[l] = 0;
				track_dtimer[l] = 0LL;	//0.0;
				midi_pos += 4;
				track_end[l] = get_32b();
				track_offset[l] = midi_pos;
				track_end[l] += track_offset[l];
				midi_pos = track_end[l];
			}
		}

		for (n = 0; n < MIDI_MAX_TRACKS; n++)
			for (l = 0; l < NOTE_MAX; l++)
			{
				midi_notes_keys[n][l] = -1;
				midi_keyonoff[n] = 0;
			}

		flag = 1;

		while (1)
		{
			if (flag == 0)
				break;			// ninguna pista activa, salir

			if (flag != 0)		// actualizar contador de ticks
			{
				int temp = 0x7fffffff;

				for (l = 0; l < MIDI_MAX_TRACKS; l++)	// get ticks to wait
				{
//					if ((l == 1 && midi_ritmo == 1) || (l == 2 && midi_ritmo == 0 && twoplayers == 0) ||
//						(l == 2 && midi_ritmo == 0 && twoplayers != 0 && modeplayers == 0))
//						continue;

					midi_pos = track_offset[l];

					if (midi_pos >= track_end[l])
					{
						track_status[l] = 0;
						continue;
					}			// si fin de pista, ignora

					if (track_status[l] == 1)	// leer numero de ticks
					{
						track_ticks[l] = ReadVlen();
						track_timer[l] += track_ticks[l];

						track_status[l] = 2;
						track_offset[l] = midi_pos;
					}


					if (track_status[l] != 0)
					{
						if (track_timer[l] < temp)
							temp = track_timer[l];
					}
				}

				if (temp == 0x7fffffff)
				{
					mid_time = 0;
				}
				else
				{
					mid_time = temp;
				}
			}

			flag = 0;

			for (l = 0; l < MIDI_MAX_TRACKS; l++)	// rastrea todas las pistas
			{
				unsigned char cmd, dat1, dat2, dat3;
/*
				if ((l == 1 && midi_ritmo == 1)
					|| (l == 2 && midi_ritmo == 0 && twoplayers == 0) || (l == 2 && midi_ritmo == 0 && twoplayers != 0
																		  && modeplayers == 0))
				{
					track_status[l] = 0;
					continue;
				}
*/
				if (track_status[l] == 0)
					continue;	//  pista en deshuso

				midi_pos = track_offset[l];

				if (midi_pos >= track_end[l])
				{
					track_status[l] = 0;
					continue;
				}				// si fin de pista, ignora

				flag |= 1;		// activa flag 1 si pista activa


				if (track_status[l] == 1)
					continue;

				if (track_status[l] == 2)	// leer comando/nota
				{
					if (mid_time < track_timer[l])
						continue;	// espera a que el timer general alcance al de la pista
				}

				track_dtimer[l] += d_get_time_midi(track_timer[l] - pri_track_timer[l]);
				pri_track_timer[l] = track_timer[l];
				mid_time_ms = (unsigned) (track_dtimer[l]);


				cmd = midi_file[midi_pos];
				midi_pos++;

				if (cmd == 0xff)
				{

					dat1 = midi_file[midi_pos];
					midi_pos++;

					switch (dat1)
					{
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
						dat2 = midi_file[midi_pos];
						midi_pos += dat2 + 1;
						break;
					case 0x2d:
						dat2 = midi_file[midi_pos];
						midi_pos++;
						break;
					case 0x2f:
						dat2 = midi_file[midi_pos];
						midi_pos++;
						track_status[l] = 0;	// flag de fin de pista
						//printf("End of track\n");
						break;
					case 0:
						dat2 = midi_file[midi_pos];
						midi_pos++;
						break;
					case 0x51:
						dat2 = midi_file[midi_pos];
						midi_pos++;

						// ajusta el tiempo ante el cambio

						//track_dtimer[0]+=d_get_time_midi(mid_time-pri_track_timer[l]);
						for (n = 0; n < MIDI_MAX_TRACKS; n++)
						{
						/*
							if ((n == 1 && midi_ritmo == 1) || (n == 2 && midi_ritmo == 0 && twoplayers == 0) ||
								(n == 2 && midi_ritmo == 0 && twoplayers != 0 && modeplayers == 0))
							{
								continue;
							}
*/
							if (track_status[n] == 0)
								continue;

							if (mid_time >= pri_track_timer[n])
							{
								pri_track_timer[n] = mid_time;
							}
						}
						for (n = 0; n < MIDI_MAX_TRACKS; n++)
							track_dtimer[n] = track_dtimer[l];

						midi_tempo = get_24b();

						//printf("CMD 0x51 midi_tempo %i\n",midi_tempo);
						break;

					case 0x54:
						//printf("CMD 0x54 midi \n");
						dat2 = midi_file[midi_pos];
						midi_pos++;
						while (dat2)
						{
							midi_pos++;
							dat2--;
						}
						break;
					case 0x58:

						dat2 = midi_file[midi_pos];
						midi_pos++;

						dat3 = midi_file[midi_pos];
						midi_pos++;


						dat3 = midi_file[midi_pos];
						midi_pos++;


						dat3 = midi_file[midi_pos];
						midi_pos++;

						// ajusta el tiempo ante el cambio

						for (n = 0; n < MIDI_MAX_TRACKS; n++)
						{
						/*
							if ((n == 1 && midi_ritmo == 1) || (n == 2 && midi_ritmo == 0 && twoplayers == 0) ||
								(n == 2 && midi_ritmo == 0 && twoplayers != 0 && modeplayers == 0))
							{
								continue;
							}
*/
							if (track_status[n] == 0)
								continue;

							if (mid_time >= pri_track_timer[n])
							{
								pri_track_timer[n] = mid_time;
							}
						}

						for (n = 0; n < MIDI_MAX_TRACKS; n++)
							track_dtimer[n] = track_dtimer[l];

						midi_ticks_quarter = dat3;

						//printf("ticks_quarter %i\n",midi_ticks_quarter);

						dat3 = midi_file[midi_pos];
						midi_pos++;

						break;
					case 0x8f:
						midi_pos++;
					default:
						dat2 = midi_file[midi_pos];
						midi_pos++;
						while (dat2)
						{
							midi_pos++;
							dat2--;
						}

						break;


					}
				}				// cmd //255

				else
				{

					switch (cmd >> 4)
					{

					case 8:

						dat2 = midi_file[midi_pos];
						midi_pos++;
						dat3 = midi_file[midi_pos];
						midi_pos++;

						if (dat2 >= NOTE_BASE && dat2 < NOTE_BASE + NOTE_MAX)
						{
							int vel;
							vel = (mid_time_ms - midi_notes_keys[l][dat2 - NOTE_BASE]);

							if (l < 2)
								set_note(0, dat2 - NOTE_BASE, midi_notes_keys[l][dat2 - NOTE_BASE], vel);
							else if (l == 2)
								set_note(1, dat2 - NOTE_BASE, midi_notes_keys[l][dat2 - NOTE_BASE], vel);
							else if (l == 3)
								set_note(2, dat2 - NOTE_BASE, midi_notes_keys[l][dat2 - NOTE_BASE], vel);

						}
						midi_keyonoff[l] = 0;
						break;

					case 9:

						dat2 = midi_file[midi_pos];
						midi_pos++;
						dat3 = midi_file[midi_pos];
						midi_pos++;

						if (dat2 >= NOTE_BASE && dat2 < NOTE_BASE + NOTE_MAX)
						{
							midi_notes_keys[l][dat2 - NOTE_BASE] = mid_time_ms;
						}
						midi_keyonoff[l] = 1;
						break;

					case 0xa:
					case 0xb:
					case 0xe:

						dat2 = midi_file[midi_pos];
						midi_pos++;
						dat3 = midi_file[midi_pos];
						midi_pos++;
						break;

					case 0xc:
					case 0xd:

						dat2 = midi_file[midi_pos];
						midi_pos++;
						break;

					case 0xf:

						if (cmd == 0xf0)
						{
							dat2 = midi_file[midi_pos];
							midi_pos++;
							midi_pos += dat2;
						}
						else if (cmd == 0xf7)
						{
							dat2 = midi_file[midi_pos];
							midi_pos++;
							midi_pos += dat2;
						}
						else if (cmd == 0xf2)
							midi_pos += 2;	//  song position
						else if (cmd == 0xf4)
							midi_pos += 1;
						break;

					default:

						dat2 = cmd;
						dat3 = midi_file[midi_pos];
						midi_pos++;

						if (dat2 >= NOTE_BASE && dat2 < NOTE_BASE + NOTE_MAX)
						{

							if (midi_keyonoff[l] == 0 || dat3 == 0)
							{
								int vel;

								vel = (mid_time_ms - midi_notes_keys[l][dat2 - NOTE_BASE]);
								if (l < 2)
									set_note(0, dat2 - NOTE_BASE, midi_notes_keys[l][dat2 - NOTE_BASE], vel);
								else if (l == 2)
									set_note(1, dat2 - NOTE_BASE, midi_notes_keys[l][dat2 - NOTE_BASE], vel);
								else if (l == 3)
									set_note(2, dat2 - NOTE_BASE, midi_notes_keys[l][dat2 - NOTE_BASE], vel);
							}
							else
							{
								midi_notes_keys[l][dat2 - NOTE_BASE] = mid_time_ms;
							}

						}


						break;
					}
				}

				track_offset[l] = midi_pos;

				if (midi_pos >= track_end[l])
				{
					track_status[l] = 0;
					continue;
				}

				if (track_status[l] != 0)	// cambia a estado 1 (coger ticks) y guard la posicion
				{
					track_status[l] = 1;
				}
			}					// for
		}

	}

	free(midi_file);
	midi_file = NULL;

	return 0;
}

