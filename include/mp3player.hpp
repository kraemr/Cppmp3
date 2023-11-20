#include <cstddef>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include "playlist.hpp"

namespace Mp3Player
{
int initMp3Player();
int initPlaySound(const std::string filepath);
void cleanupMp3Player();
void loadPlaylistsDir(const std::string filepath);
int playSongAtIndex(int i);
void processSignals();
extern Song currentSong;
extern std::atomic<bool> recvdSignal;
extern std::mutex mtx;
extern std::condition_variable cv;
extern std::unique_lock<std::mutex> lock;
extern int signal;
}
