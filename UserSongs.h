#ifndef USERSONGS_H
#define USERSONGS_H

#include <Arduino.h>
#include "jingle.h" // Include the file where song functions are declared

// Define the UserSong struct
struct UserSong {
    String userName;
    void (*playSong)(); // Pointer to the function that plays the song
};

// Define the user-song array
UserSong userSongs[] = {
    {"mrmcconnell10", play_vitality},
    {"brendan_mcconnell", play_industry_baby},
    // Add more users and their corresponding songs here
};

// Define the user count
const int userCount = sizeof(userSongs) / sizeof(userSongs[0]);

#endif // USERSONGS_H