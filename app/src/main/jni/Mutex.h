#ifndef AA_MUTEX_H__
#define AA_MUTEX_H__
#include <pthread.h>
namespace AA{
class Lock;
class Mutex
{
  public:
    Mutex ()
    {
        pthread_mutex_init(&mMutex, NULL);
    }
    virtual ~Mutex ()
    {
        pthread_mutex_destroy(&mMutex);
    }
  private:
    void lock () const
    {
        pthread_mutex_lock(&mMutex);
    }
    void unlock () const
    {
        pthread_mutex_unlock(&mMutex);
    }
    mutable pthread_mutex_t mMutex;
    friend class Lock;
};
class Lock
{
  public:
    Lock (const Mutex& m, bool autoLock = true):
    _mutex (m)
    {
        if (autoLock)
        {
            _mutex.lock();
        }
    }
    ~Lock ()
    {
        _mutex.unlock();
    }
    void acquire ()
    {
        _mutex.lock();
    }
    void release ()
    {
        _mutex.unlock();
    }
  private:
    const Mutex &_mutex;
};
}
#endif // AA_MUTEX_H__
