#include "audio.h"

struct PlayerBundle { QMediaPlayer* player; QAudioOutput* output; };
struct SoundPool {
	QVector<PlayerBundle> players;
	int nextIndex = 0;
	QElapsedTimer lastPlay;
};
static QHash<QString, SoundPool> pools;
static double masterVolume = 1.0;

static double volumeFor(const QString& id) {
	if (id == "brickBreaking") return 0.05;
	if (id == "bulletToWall") return 0.05;
    if (id == "bulletToBullet") return 0.7;
    if (id == "shoot") return 0.2;
    if (id == "shootLaser") return 0.5;
    if (id == "tankDestroyed") return 1.0;
    if (id == "win") return 0.8;
    if (id == "lose") return 0.8;
	return 0.9;
}

static SoundPool getPool(const QString& id) {
	if (pools.contains(id)) return pools.value(id);
	QString path;
	if (id == "brickBreaking") path = "qrc:/sounds/brickBreaking.mp3";
	else if (id == "bulletToBullet") path = "qrc:/sounds/bulletToBullet.mp3";
	else if (id == "bulletToWall") path = "qrc:/sounds/bulletToWall.mp3";
	else if (id == "reloadPowerUp") path = "qrc:/sounds/reloadPowerUp.mp3";
	else if (id == "shieldDestroyed") path = "qrc:/sounds/shieldDestroyed.mp3";
	else if (id == "shieldPowerUp") path = "qrc:/sounds/shieldPowerUp.mp3";
	else if (id == "speedPowerUp") path = "qrc:/sounds/speedPowerUp.mp3";
	else if (id == "tankDestroyed") path = "qrc:/sounds/tankDestroyed.mp3";
	else if (id == "shoot") path = "qrc:/sounds/shoot.mp3";
    else if (id == "shootLaser") path = "qrc:/sounds/shootLaser.mp3";
    else if (id == "sniperShot") path = "qrc:/sounds/sniperShot.mp3";
    else if (id == "win") path = "qrc:/sounds/win.mp3";
	else if (id == "lose") path = "qrc:/sounds/lose.mp3";
	SoundPool pool;
	int poolSize = 2;
	if (id == "shoot") poolSize = 3;
	for (int i = 0; i < poolSize; ++i) {
		PlayerBundle bundle{new QMediaPlayer, new QAudioOutput};
		bundle.player->setAudioOutput(bundle.output);
        bundle.output->setVolume(volumeFor(id) * masterVolume);
		if (!path.isEmpty()) bundle.player->setSource(QUrl(path));
		pool.players.append(bundle);
	}
	pool.nextIndex = 0;
	pool.lastPlay.start();
	pools.insert(id, pool);
	return pool;
}

void Audio::play(const QString& id) {
	const int minIntervalMs = (id == "shoot") ? 40 : (id == "bulletToWall" ? 80 : (id == "brickBreaking" ? 80 : 0));
	SoundPool pool = getPool(id);
	if (minIntervalMs > 0 && pool.lastPlay.isValid() && pool.lastPlay.elapsed() < minIntervalMs) return;
	pool.lastPlay.restart();

	if (pool.players.isEmpty()) return;
	int idx = pool.nextIndex;
	pool.nextIndex = (pool.nextIndex + 1) % pool.players.size();
	PlayerBundle& bundle = pool.players[idx];
	if (bundle.player) {
		bundle.player->setPosition(0);
		bundle.player->play();
	}
	pools.insert(id, pool);
}

void Audio::preloadAll() {
	const QStringList ids = {
		"brickBreaking", "bulletToBullet", "bulletToWall",
		"reloadPowerUp", "shieldDestroyed", "shieldPowerUp",
        "speedPowerUp", "tankDestroyed", "shoot", "shootLaser", "win", "lose"
	};
	for (const QString& id : ids) {
		SoundPool pool = getPool(id);
		for (PlayerBundle& bundle : pool.players) {
			double originalVolume = bundle.output->volume();
			bundle.output->setVolume(0.0);
			bundle.player->setPosition(0);
			bundle.player->play();
			bundle.player->stop();
			bundle.output->setVolume(originalVolume);
		}
		pools.insert(id, pool);
	}
}

void Audio::stopAll() {
	for (auto it = pools.begin(); it != pools.end(); ++it) {
		SoundPool& pool = it.value();
		for (PlayerBundle& bundle : pool.players) {
			if (bundle.player) bundle.player->stop();
		}
	}
}

void Audio::stopSound(const QString& id) {
	SoundPool pool = getPool(id);
    for (PlayerBundle& bundle : pool.players)
		if (bundle.player && bundle.player->playbackState() == QMediaPlayer::PlayingState)
			bundle.player->stop();
}

void Audio::setMasterVolume(double volume) {
    if (volume < 0.0) volume = 0.0; else if (volume > 1.0) volume = 1.0;
    masterVolume = volume;
	for (auto it = pools.begin(); it != pools.end(); ++it) {
		const QString id = it.key();
		SoundPool& pool = it.value();
		double baseVolume = volumeFor(id);
		for (PlayerBundle& bundle : pool.players) {
            if (bundle.output) bundle.output->setVolume(baseVolume * masterVolume);
		}
	}
}

double Audio::getMasterVolume() { return masterVolume; }
