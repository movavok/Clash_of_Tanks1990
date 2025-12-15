#ifndef AUDIO_H
#define AUDIO_H

#include <QString>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QHash>
#include <QVector>
#include <QUrl>
#include <QElapsedTimer>

class Audio {
public:
    static void play(const QString& id);
    static void preloadAll();
    static void stopAll();
    static void setMasterVolume(double);
    static double getMasterVolume();
};

#endif
