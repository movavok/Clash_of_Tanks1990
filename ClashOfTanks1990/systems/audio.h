#ifndef AUDIO_H
#define AUDIO_H

#include <QString>

class Audio {
public:
    static void play(const QString& id);
    static void preloadAll();
    static void stopAll();
    static void setMasterVolume(double volume01);
    static double masterVolume();
};

#endif
