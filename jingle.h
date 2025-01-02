#include <ESP8266WiFi.h>




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



// —— Examples —— //



/*
## How To Make A Song ##

1. Define an array like "const double song_notes[song_length][2]".
  * "song_notes" won't be the actual name of your song (that'll come up later). 
  * "song_length" will be the number of notes in the song (you can do that at the end of step #2).

2. Fill the array with note macros.
  * If you don't know what note macros are, go back up to the "Note Macros" section.
  * Note macros will be played in the order they're listed.
  * Rests cound as notes, so don't forget to count them too.

3. Use the `COMPOSE` macro to create the song.
  * The first argument should be the name of the array of note macros.
  * The second argument should be the name of the song.
  * The third argument should be what you want the play function to be named.
  * The fourth argument should be the tempo of the song.
  * The fifth argument should be the length of the song.

P.S.
You have to make a variable for the note macros; you can't just pass the list as an argument.
I'm not a fan of this, but it's the best solution I could come up with. At least the
syntax looks clean. The play function thing is just dumb though.
*/

// Steps 1 & 2
const double c_scale_notes[15][2] = {
  C(4, 1), D(4, 2), E(4, 2), F_(4, 2), G(4, 2), A(4, 2), B(4, 2), C(5, 1),
  B(4, 2), A(4, 2), G(4, 2), F_(4, 2), E(4, 2), D(4, 2), C(4, 1)
};

// Step 3
COMPOSE(c_scale_notes, C_Scale, play_c_scale, 60, 15)



// —— Run-Time Stuff —— //



void setup() {
  play_vitality();
  play_vitality();
}

void loop() {}



// —— Song Listing —— //



//    —— Song ——   |   —— Note Sheet ——   |   —— Variable ——  |   —— Play Function ——
// Vitality        | vitality_notes       | Vitality          | play_vitality()
// Industry Baby   | industry_baby_notes  | Industry_Baby     | play_industry_baby()


/// Todo List

// Imperial March
// The Grinch
// Through The Fire And Flames
// 9mm
// Bad Apple!!
// Smooth Criminal
// Come A Little Bit Closer
// The Game Theory Theme Song
// Harder, Better, Faster, Stronger
// Nigelcore
// The Real Slim Shady
// Virtual Gaming
// Your New Home

const double vitality_notes[66][2] = {
  C(4, 2), R(2), G(4, 2), Eb(4, 2), R(2), C(4, 2), R(2), C(5, 2), // 8 Notes
  R(2), C(4, 2), R(2), Eb(4, 2), R(2), C(4, 2), D(4, 2), Eb(4, 2), // 8 Notes
  G(4, 2), R(2), C(5, 2), D(5, 2), R(2), G(4, 2), R(2), B(4, 2), // 8 Notes
  R(2), G(4, 2), R(2), Eb(4, 2), R(2), G(4, 2), F_(4, 2), Eb(4, 2), // 8 Notes

  C(4, 2), R(2), G(4, 2), Eb(4, 2), R(2), C(4, 2), R(2), D(4, 2), // 8 Notes
  R(2), C(4, 2), R(2), Eb(4, 2), R(2), C(4, 2), D(4, 2), Eb(4, 2), // 8 Notes
  G(4, 2), R(2), Eb(4, 2), G(4, 2), R(2), Eb(4, 2), R(2), G(4, 2), // 8 Notes
  R(2), Eb(4, 2), R(2), Bb(4, 3), G(4, 3), F_(4, 3), Eb(4, 3), D(4, 2), Eb(4, 2), D(4, 2), // 10 Notes
};

COMPOSE_SHIFTED(vitality_notes, Vitality, play_vitality, 300, 66, -1)

const double industry_baby_notes[83][2] = {
  D(3, 3), R(3), tD(3, 3, 1), tR(3, 1), tD(3, 3, 1), tR(3, 1), tD(3, 3, 1), tR(3, 1), D(3, 3), R(3), D(3, 3), R(3), // 12 Notes
  A(3, 3), R(3), tA(3, 3, 1), tR(3, 1), tA(3, 3, 1), tR(3, 1), tA(3, 3, 1), tR(3, 1), A(3, 3), R(3), A(3, 3), R(3), // 12 Notes
  D(4, 3), R(3), tD(4, 3, 1), tR(3, 1), tD(4, 3, 1), tR(3, 1), tD(4, 3, 1), tR(3, 1), D(4, 3), R(3), D(4, 3), R(3), // 12 Notes
  D(4, 1), D(4, 2), R(2), // 3 Notes

  A(3, 3), G(3, 3), F_(3, 1), G(3, 3), F_(3, 3), // 5 Notes
  E(3, 2), E(3, 3), R(3), E(3, 4), R(4), E(3, 4), R(4), E(3, 4), R(4), E(3, 4), R(4), // 11 Notes
  F_(3, 3), E(3, 3), D(3, 1), D(3, 2), // 4 Notes
  R(2), D(3, 2), E(3, 2), F_(3, 2), // 4 Notes

  A(3, 3), G(3, 3), F_(3, 1), G(3, 3), F_(3, 3), // 5 Notes
  E(3, 2), E(3, 3), R(3), E(3, 4), R(4), E(3, 4), R(4), E(3, 4), R(4), E(3, 4), R(4), // 11 Notes
  F_(3, 3), E(3, 3), D(3, 1), D(3, 2), // 4 Notes
};

COMPOSE(industry_baby_notes, Industry_Baby, play_industry_baby, 145, 83)

const double bad_apple_notes[66][2] = {
  Eb(3, 3), F_(3, 3), Gb(3, 3), Ab(3, 3), Bb(3, 2), Eb(4, 3), Db(4, 3), // 7 Notes
  Bb(3, 2), Eb(3, 2), Bb(3, 3), Ab(3, 3), Gb(3, 3), F_(3, 3), // 6 Notes
  Eb(3, 3), F_(3, 3), Gb(3, 3), Ab(3, 3), Bb(3, 2), Ab(3, 3), Gb(3, 3), // 7 Notes
  F_(3, 3), Eb(3, 3), F_(3, 3), Gb(3, 3), F_(3, 3), Eb(3, 3), D(3, 3), F_(3, 3), // 8 Notes
  Eb(3, 3), F_(3, 3), Gb(3, 3), Ab(3, 3), Bb(3, 2), Eb(4, 3), Db(4, 3), // 7 Notes
  Bb(3, 2), Eb(3, 2), Bb(3, 3), Ab(3, 3), Gb(3, 3), F_(3, 3), // 6 Notes
  Eb(3, 3), F_(3, 3), Gb(3, 3), Ab(3, 3), Bb(3, 2), Ab(3, 3), Gb(3, 3), // 7 Notes
  Eb(3, 2), Gb(3, 2), Ab(3, 2), Bb(3, 2), // 4 Notes

  Db(4, 3), Eb(4, 3), Bb(3, 3), Ab(3, 3), Bb(3, 2), Ab(3, 3), Bb(3, 3), // 7 Notes
  Db(4, 3), Eb(4, 3), Bb(3, 3), Ab(3, 3), Bb(3, 2), Ab(3, 3), Bb(3, 3), // 7.3333 Notes
};

COMPOSE(bad_apple_notes, Bad_Apple, play_bad_apple, 138, 66)