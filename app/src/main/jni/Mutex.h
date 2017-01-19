#ifndef AA_MUTEX_H__
#define AA_MUTEX_H__
#include <pthread.h>
namespace AA{
class Lock;
class Condition;
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
    void lock () const
    {
        pthread_mutex_lock(&mMutex);
    }
    void unlock () const
    {
        pthread_mutex_unlock(&mMutex);
    }
private:
    mutable pthread_mutex_t mMutex;
    friend class Lock;
    friend class Condition;
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

class Condition{
    public:
        Condition(){
            pthread_cond_init(&cond, NULL);
        }
        ~Condition(){
            pthread_cond_destroy(&cond);
        }
        int wait(const Mutex& m){
            pthread_cond_wait(&cond, &m.mMutex);
            return 0;
        }
        int signal(){
            pthread_cond_signal(&cond);
            return 0;
        }
        int broadcast(){
            pthread_cond_broadcast(&cond);
            return 0;
        }
    private:
        pthread_cond_t cond;
    };
}
#endif // AA_MUTEX_H__
