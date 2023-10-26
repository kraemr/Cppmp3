#include <cstddef>
#include <thread>
#include <mutex>
#include <condition_variable>
namespace Mp3Player
{
int initMp3Player();
int initPlaySound(const std::string filepath);
void cleanupMp3Player();
void loadPlaylistsDir(const std::string filepath);
int playSongAtIndex(int i);
void processSignals();
extern std::mutex mtx;
extern std::condition_variable cv;
extern std::lock_guard<std::mutex> lock;
extern int signal;
}
