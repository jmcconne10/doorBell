#ifndef MUSIC_H
#define MUSIC_H

#include <Arduino.h>



// —— Utility Stuff —— //



#define AUDIO_PIN D6

#define COMPOSE(note_sheet, song_name, function_name, tempo, length) \
const auto song_name = Song<tempo, length>(note_sheet); \
void function_name() { \
	for (int i = 0; i < length; ++i) { \
		if (song_name.notes[i].pitch != 0.0) { tone(AUDIO_PIN, song_name.notes[i].pitch, song_name.notes[i].len); } \
		delay(song_name.notes[i].len); \
	} \
}

#define COMPOSE_SHIFTED(note_sheet, song_name, function_name, tempo, length, octave_shift) \
const auto song_name = Song<tempo, length>(note_sheet, octave_shift); \
void function_name() { \
	for (int i = 0; i < length; ++i) { \
		if (song_name.notes[i].pitch != 0.0) { tone(AUDIO_PIN, song_name.notes[i].pitch, song_name.notes[i].len); } \
		delay(song_name.notes[i].len); \
	} \
}

const double pow2(unsigned int n) { return n == 0 ? 1.0 : 2.0 * pow2(n - 1); }
const double pow3(unsigned int n) { return n == 0 ? 1.0 : 3.0 * pow2(n - 1); }



// —— Note Macros —— //



/*
Note macros are used to write the notes in a song.
Each note has a macro that takes two arguments: octave and size.

Format - Note(Octave, Size)

Note Pitch (In Hertz) = Base * 2^Octave
Note Length (In Beats) = 4 / 2^Size

C(4, 0) = Fourth Octave C, Whole Note
Bb(3, 2) = Third Octave B Flat, Quarter Note
Eb(5, 4) = Fifth Octave E Flat, Sixteenth Note

To add rests, use the `R` macro.
It only takes the `Size` argument since rests are silent.

R(1) = Rest, Half Note

Each macro creates an array of two doubles: one for the note's pitch and one for its length.
Also, F is actually `F_` because the `F` function identifier is already taken by something from `ESP8266WiFi`.
*/

// Standard Notes
#define R(size) { 0.0, pow2(size) }
#define C(octave, size) { 16.35 * pow2(octave), pow2(size) }
#define Db(octave, size) { 17.32 * pow2(octave), pow2(size) }
#define D(octave, size) { 18.35 * pow2(octave), pow2(size) }
#define Eb(octave, size) { 19.45 * pow2(octave), pow2(size) }
#define E(octave, size) { 20.6 * pow2(octave), pow2(size) }
#define F_(octave, size) { 21.83 * pow2(octave), pow2(size) }
#define Gb(octave, size) { 23.12 * pow2(octave), pow2(size) }
#define G(octave, size) { 24.5 * pow2(octave), pow2(size) }
#define Ab(octave, size) { 25.96 * pow2(octave), pow2(size) }
#define A(octave, size) { 27.5 * pow2(octave), pow2(size) }
#define Bb(octave, size) { 29.14 * pow2(octave), pow2(size) }
#define B(octave, size) { 30.87 * pow2(octave), pow2(size) }

// Tri-Base Notes - Note(Octave, Half-Base, Third-Base)
#define tR(halfs, thirds) { 0.0, pow2(halfs) * pow3(thirds) }
#define tC(octave, halfs, thirds) { 16.35 * pow2(octave), pow2(halfs) * pow3(thirds) }
#define tDb(octave, halfs, thirds) { 17.32 * pow2(octave), pow2(halfs) * pow3(thirds) }
#define tD(octave, halfs, thirds) { 18.35 * pow2(octave), pow2(halfs) * pow3(thirds) }
#define tEb(octave, halfs, thirds) { 19.45 * pow2(octave), pow2(halfs) * pow3(thirds) }
#define tE(octave, halfs, thirds) { 20.6 * pow2(octave), pow2(halfs) * pow3(thirds) }
#define tF(octave, halfs, thirds) { 21.83 * pow2(octave), pow2(halfs) * pow3(thirds) }
#define tGb(octave, halfs, thirds) { 23.12 * pow2(octave), pow2(halfs) * pow3(thirds) }
#define tG(octave, halfs, thirds) { 24.5 * pow2(octave), pow2(halfs) * pow3(thirds) }
#define tAb(octave, halfs, thirds) { 25.96 * pow2(octave), pow2(halfs) * pow3(thirds) }
#define tA(octave, halfs, thirds) { 27.5 * pow2(octave), pow2(halfs) * pow3(thirds) }
#define tBb(octave, halfs, thirds) { 29.14 * pow2(octave), pow2(halfs) * pow3(thirds) }
#define tB(octave, halfs, thirds) { 30.87 * pow2(octave), pow2(halfs) * pow3(thirds) }



// —— Structures —— //



/// Stores the pitch and length of a single note.
template <unsigned int TEMPO> struct Note {
	unsigned int pitch; // Measured in Hz
	unsigned int len; // Measured in ms.

	// Default Constructor
	Note() : pitch(0), len(0) {} 

	// This turns the value produced from the note macros into an actual note.
	Note(const double vals[2]) : pitch(static_cast<unsigned int>(vals[0])), len(static_cast<unsigned int>(4.0 * 60.0 * 1000.0 / static_cast<double>(TEMPO) / vals[1])) {}

	// Same thing, but for Octave shifts.
	Note(const double pitch, const double len) : pitch(static_cast<unsigned int>(pitch)), len(static_cast<unsigned int>(4.0 * 60.0 * 1000.0 / static_cast<double>(TEMPO) / len)) {}
};

/// Stores a list of `Note`s.
template <unsigned int TEMPO, unsigned int LENGTH> struct Song {
	Note<TEMPO> notes[LENGTH]; // The notes.

	// This converts a list of note macros into a list of `Note`s.
	Song(const double vals[LENGTH][2]) { for(int i = 0; i < LENGTH; ++i) { notes[i] = Note<TEMPO>(vals[i]); } }

	// Use this to shift the song up and down octaves.
	Song(const double vals[LENGTH][2], const int octave_shift) {
		const double shift = octave_shift >= 0 ? pow2(static_cast<unsigned int>(octave_shift)) : 1.0 / pow2(static_cast<unsigned int>(-octave_shift));

	  	for(int i = 0; i < LENGTH; ++i) {
			notes[i] = Note<TEMPO>(vals[i][0] * shift, vals[i][1]);
		}
	}
};



// —— User Guide —— //



/* ## How To Play A Song ##

1. Go down to the `Song Listing

*/

/* ## How To Add A Song ##

1. Define an array like this: "const double song_notes[song_length][2]".
	* "song_notes" won't be the actual name of your song (that'll come up later). 
	* "song_length" will be the number of notes in the song.
  
2. Fill the array with note macros.
  	* If you don't know what note macros are, go back up to the "Note Macros" section.
  	* Note macros will be played in the order they are listed.
  	* Rests are also notes, so make sure to count them in your song length.

3. Use the `COMPOSE` macro to create the song.
	* The first argument should be the name of the array you made in step one.
	* The second argument should be the variable name you want to use for the song.
	* The third argument should be the name you want for the play function.
	* The fourth argument should be the tempo of the song in BPM.
	* The fifth argument should be the number of notes in the song.

// Steps 1 & 2 //

const double c_scale_notes[15][2] = {
	C(4, 1), D(4, 2), E(4, 2),
 	F_(4, 2), G(4, 2), A(4, 2), B(4, 2),
	C(5, 1), B(4, 2), A(4, 2),
	G(4, 2), F_(4, 2), E(4, 2), D(4, 2), C(4, 1)
};

// Step 3 //

 		[Macro Notes]		  [Function Name]	[Length]
COMPOSE(c_scale_notes, C_Scale, play_c_scale, 60, 15)
					 [Song Name]	        [Tempo]
*/



// —— Song Listings —— //



//    ———— Song ————   |   ——— Macro Sheets ———   |   ——— Variables ———   |   ——— Play Function ———
// Vitality            | vitality_notes           | Vitality              | play_vitality()
// Industry Baby       | industry_baby_notes      | Industry_Baby         | play_industry_baby()
// Bad Apple!!         | bad_apple_notes          | Bad_Apple             | play_bad_apple()
// Bad Apple!! (Intro) | bad_apple_intro_notes    | Bad_Apple_Intro       | play_bad_apple_intro()
// Bad Apple!! (Bridge)| bad_apple_bridge_notes   | Bad_Apple_Bridge      | play_bad_apple_bridge()
// Imperial March      | imperial_march_notes     | Imperial_March        | play_imperial_march()
// You're A Mean One   | mean_one_notes           | Mean_One              | play_mean_one()

/// Todo List

// Through The Fire And Flames
// 9mm
// Smooth Criminal
// Come A Little Bit Closer
// The Game Theory Theme Song
// Harder, Better, Faster, Stronger
// Nigelcore
// The Real Slim Shady
// Virtual Gaming
// Your New Home

const double vitality_notes[66][2] = {
	C(4, 2), R(2), G(4, 2), Eb(4, 2), R(2), C(4, 2), R(2), C(5, 2),    // 8 Notes
	R(2), C(4, 2), R(2), Eb(4, 2), R(2), C(4, 2), D(4, 2), Eb(4, 2),   // 8 Notes
	G(4, 2), R(2), C(5, 2), D(5, 2), R(2), G(4, 2), R(2), B(4, 2),     // 8 Notes
	R(2), G(4, 2), R(2), Eb(4, 2), R(2), G(4, 2), F_(4, 2), Eb(4, 2),  // 8 Notes
	
	C(4, 2), R(2), G(4, 2), Eb(4, 2), R(2), C(4, 2), R(2), D(4, 2),                           // 8 Notes
	R(2), C(4, 2), R(2), Eb(4, 2), R(2), C(4, 2), D(4, 2), Eb(4, 2),                          // 8 Notes
	G(4, 2), R(2), Eb(4, 2), G(4, 2), R(2), Eb(4, 2), R(2), G(4, 2),                          // 8 Notes
	R(2), Eb(4, 2), R(2), Bb(4, 3), G(4, 3), F_(4, 3), Eb(4, 3), D(4, 2), Eb(4, 2), D(4, 2),  // 10 Notes
};

COMPOSE(vitality_notes, Vitality, play_vitality, 300, 66)

const double industry_baby_notes[83][2] = {
	D(3, 3), R(3), tD(3, 3, 1), tR(3, 1), tD(3, 3, 1), tR(3, 1), tD(3, 3, 1), tR(3, 1), D(3, 3), R(3), D(3, 3), R(3),  // 12 Notes
	A(3, 3), R(3), tA(3, 3, 1), tR(3, 1), tA(3, 3, 1), tR(3, 1), tA(3, 3, 1), tR(3, 1), A(3, 3), R(3), A(3, 3), R(3),  // 12 Notes
	D(4, 3), R(3), tD(4, 3, 1), tR(3, 1), tD(4, 3, 1), tR(3, 1), tD(4, 3, 1), tR(3, 1), D(4, 3), R(3), D(4, 3), R(3),  // 12 Notes
	D(4, 1), D(4, 2), R(2),                                                                                            // 3 Notes
	
	A(3, 3), G(3, 3), F_(3, 1), G(3, 3), F_(3, 3),                                       // 5 Notes
	E(3, 2), E(3, 3), R(3), E(3, 4), R(4), E(3, 4), R(4), E(3, 4), R(4), E(3, 4), R(4),  // 11 Notes
	F_(3, 3), E(3, 3), D(3, 1), D(3, 2),                                                 // 4 Notes
	R(2), D(3, 2), E(3, 2), F_(3, 2),                                                    // 4 Notes
	
	A(3, 3), G(3, 3), F_(3, 1), G(3, 3), F_(3, 3),                                       // 5 Notes
	E(3, 2), E(3, 3), R(3), E(3, 4), R(4), E(3, 4), R(4), E(3, 4), R(4), E(3, 4), R(4),  // 11 Notes
	F_(3, 3), E(3, 3), D(3, 1), D(3, 2),                                                 // 4 Notes
};

COMPOSE(industry_baby_notes, Industry_Baby, play_industry_baby, 145, 83)

const double bad_apple_notes[52][2] = {
 	Eb(3, 3), F_(3, 3), Gb(3, 3), Ab(3, 3), Bb(3, 2), Eb(4, 3), Db(4, 3),           // 7 Notes
  	Bb(3, 2), Eb(3, 2), Bb(3, 3), Ab(3, 3), Gb(3, 3), F_(3, 3),                     // 6 Notes
  	Eb(3, 3), F_(3, 3), Gb(3, 3), Ab(3, 3), Bb(3, 2), Ab(3, 3), Gb(3, 3),           // 7 Notes
  	F_(3, 3), Eb(3, 3), F_(3, 3), Gb(3, 3), F_(3, 3), Eb(3, 3), D(3, 3), F_(3, 3),  // 8 Notes
	Eb(3, 3), F_(3, 3), Gb(3, 3), Ab(3, 3), Bb(3, 2), Eb(4, 3), Db(4, 3),           // 7 Notes
  	Bb(3, 2), Eb(3, 2), Bb(3, 3), Ab(3, 3), Gb(3, 3), F_(3, 3),                     // 6 Notes
  	Eb(3, 3), F_(3, 3), Gb(3, 3), Ab(3, 3), Bb(3, 2), Ab(3, 3), Gb(3, 3),           // 7 Notes
  	F_(3, 2), Gb(3, 2), Ab(3, 2), Bb(3, 2),                                         // 4 Notes
};

COMPOSE(bad_apple_notes, Bad_Apple, play_bad_apple, 138, 52)

const double bad_apple_intro_notes[132][2] = {
  	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),  // 9 Notes
  	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),  // 9 Notes
  	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),  // 9 Notes
  	Gb(4, 3), Eb(4, 4), Gb(4, 4), Ab(4, 3), Gb(4, 4), Ab(4, 4),                        // 6 Notes

  	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),  // 9 Notes
  	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),  // 9 Notes
  	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),  // 9 Notes
  	Ab(4, 3), Gb(4, 4), Ab(4, 4), Gb(4, 3), Eb(4, 4), Gb(4, 4),                        // 6 Notes

  	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),  // 9 Notes
  	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),  // 9 Notes
 	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),  // 9 Notes
  	Gb(4, 3), Eb(4, 4), Gb(4, 4), Ab(4, 3), Gb(4, 4), Ab(4, 4),                        // 6 Notes

  	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),    // 9 Notes
  	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),    // 9 Notes
 	Eb(3, 4), Eb(3, 5), R(5), Eb(3, 4), Eb(4, 4), R(4), Eb(4, 4), Db(4, 4), Eb(4, 4),    // 9 Notes
  	tAb(4, 2, 1), tGb(4, 2, 1), tAb(4, 2, 1), tGb(4, 2, 1), tEb(4, 2, 1), tGb(4, 2, 1),  // 6 Notes
};

COMPOSE(bad_apple_intro_notes, Bad_Apple_Intro, play_bad_apple_intro, 138, 132)

const double bad_apple_bridge_notes[55][2] = {
	Db(4, 3), Eb(4, 3), Bb(3, 3), Ab(3, 3), Bb(3, 2), Ab(3, 3), Bb(3, 3),  // 7 Notes
	Db(4, 3), Eb(4, 3), Bb(3, 3), Ab(3, 3), Bb(3, 2), Ab(3, 3), Bb(3, 3),  // 7 Notes
	Ab(3, 3), Gb(3, 3), F_(3, 3), Db(3, 3), Eb(3, 2), Db(3, 3), Eb(3, 3),  // 7 Notes
	F_(3, 3), Gb(3, 3), Ab(3, 3), Bb(3, 3), Eb(3, 2), Bb(3, 3), Db(4, 3),  // 7 Notes
	Db(4, 3), Eb(4, 3), Bb(3, 3), Ab(3, 3), Bb(3, 2), Ab(3, 3), Bb(3, 3),  // Take A Guess
  	Db(4, 3), Eb(4, 3), Bb(3, 3), Ab(3, 3), Bb(3, 2), Eb(4, 3), F_(4, 3),  // 7 Notes
  	Gb(4, 3), F_(4, 3), Eb(4, 3), Db(4, 3), Bb(3, 2), Ab(3, 3), Bb(3, 3),  // 7 Notes
  	Ab(3, 3), Gb(3, 3), F_(3, 3), Db(3, 3), Eb(3, 2), R(2),                // 6 Notes
};

COMPOSE_SHIFTED(bad_apple_bridge_notes, Bad_Apple_Bridge, play_bad_apple_bridge, 138, 55, 1)

const double imperial_march_notes[62][2] = {
	G(3, 3), R(3), G(3, 3), R(3), G(3, 3), R(3), Eb(3, 3), R(4), Bb(3, 4),  // 9 Notes
	G(3, 2), Eb(3, 3), R(4), Bb(3, 4), G(3, 2), R(2),                       // 6 Notes
	D(4, 3), R(3), D(4, 3), R(3), D(4, 3), R(3), Eb(4, 3), R(4), Bb(3, 4),  // 9 Notes
	Gb(3, 2), Eb(3, 3), R(4), Bb(3, 4), G(3, 2), R(2),                      // 6 Notes
	
	G(4, 2), G(3, 3), R(4), G(3, 4), G(4, 2), Gb(4, 3), Gb(4, 4), F_(4, 4),                                                                   // 8 Notes
	E(4, 4), Eb(4, 4), E(4, 3), R(3), Ab(3, 3), Db(4, 2), C(4, 3), C(4, 4), B(3, 4),                                                          // 9 Notes
	Bb(3, 4), A(3, 4), Bb(3, 3), R(3), Eb(3, 3), Gb(3, 2), Eb(3, 3), Eb(3, 4), Gb(3, 4), Bb(3, 2), G(3, 3), G(3, 4), Bb(3, 4), D(4, 2), R(2)  // 15 Notes
};

COMPOSE(imperial_march_notes, Imperial_March, play_imperial_march, 80, 62)

const double mean_one_notes[74][2] = {
	D(4, 1), B(3, 1), Bb(3, 1), tA(3, 2, 1), tR(1, 1),                                                    // 5 Notes - 1.75 Measures
	tF(3, 2, 1), tR(2, 1), tG(3, 2, 1), tA(3, 1, 1), tD(3, 1, 1), tR(1, 1), R(2),                         // 7 Notes
	tF(3, 2, 1), tR(2, 1), tA(3, 2, 1), tG(3, 1, 1), tR(2, 1), R(1), tF(3, 2, 1), tR(2, 1), tG(3, 2, 1),  // 9 Notes
	A(3, 2), tA(3, 1, 1), tD(3, 2, 1), B(3, 2), tB(3, 1, 1), tE(3, 2, 1),                                 // 6 Notes
	Db(4, 1), R(2), tA(3, 2, 1), tR(2, 1), tA(3, 2, 1),                                                   // 5 Notes
	
	tD(4, 1, 1), tC(4, 2, 1), tBb(3, 1, 1), tA(3, 2, 1), tBb(3, 1, 1), tG(3, 2, 1), tG(3, 2, 1), tR(2, 1), tG(3, 2, 1),  // 9 Notes
	tC(4, 1, 1), tBb(3, 2, 1), tA(3, 1, 1), tG(3, 2, 1), A(3, 2), tF(3, 1, 1), tE(3, 2, 1),                              // 7 Notes
	D(3, 1), tD(4, 2, 1), tR(1, 1), R(2),                                                                                // 4 Notes
	tBb(3, 2, 1), tR(2, 1), tBb(3, 3, 1), tR(3, 1), tBb(3, 2, 1), tR(2, 1), tBb(3, 3, 1), tR(3, 1),                      // 8 Notes
	tBb(3, 2, 1), tR(2, 1), tBb(3, 3, 1), tR(3, 1), tBb(3, 2, 1), tR(2, 1), tBb(3, 2, 1),                                // 7 Notes
	R(1), tDb(4, 1, 1), tBb(3, 2, 1), Db(4, 2),                                                                          // 4 Notes
	D(4, 1), D(4, 2), R(2),                                                                                              // 3 Notes
};

COMPOSE_SHIFTED(mean_one_notes, Mean_One, play_mean_one, 104, 74, 1)

const double virtual_insanity_notes[36][2] = {
	R(2), tG(4, 2, 1), tA(4, 3, 1), tB(4, 3, 1), tR(3, 1), tB(4, 3, 1),                                       // 7 Notes
	B(4, 3), A(4, 3), E(4, 3), tD(4, 2, 1), tE(4, 3, 1),                                                      // 5 Notes
	tE(4, 3, 1), tR(3, 1), tE(4, 3, 1), tR(2, 1), tG(4, 3, 1), G(4, 3), tG(4, 2, 1), tE(4, 3, 1),             // 8 Notes
	tA(4, 2, 1), tR(3, 1), tA(4, 3, 1), tR(3, 1), tG(4, 2, 1), tR(3, 1), tD(4, 3, 1), tR(2, 1), tE(4, 3, 1),  // 9 Notes
	tE(4, 3, 1), tR(3, 1), tE(4, 3, 1), tR(2, 1), tE(4, 3, 1), tE(4, 1, 1), tR(2, 1),                         // 7 Notes
};

COMPOSE(virtual_insanity_notes, Virtual_Insanity, play_virtual_insanity, 93, 36)



// —— End —— //



#endif
