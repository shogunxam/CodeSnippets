#include <thread>
#include <functional>
#include <atomic>
#include <stdexcept>
#include <cassert>

#define ASSERT(expr, msg) assert(((void)(msg), (expr)))

class tInterruptibleThread
{
public:
    class tInterruptionHandler
    {
    public:
        tInterruptionHandler() : m_Interrupt(false) {}
        virtual void InterruptionCheckPoint()
        {
            if (m_Interrupt)
            {
                throw std::runtime_error("Thread interrupted");
            }
        }
        bool Interrupted() { return m_Interrupt; }
        void Interrupt() { m_Interrupt = true; }

    protected:
        std::atomic<bool> m_Interrupt;
    };
    typedef std::shared_ptr<tInterruptionHandler> tInterruptionHandlerPtr;

    using Id = std::thread::id;

    using NativeHandleType = std::thread::native_handle_type;

    
    tInterruptibleThread() noexcept = default;
    tInterruptibleThread(tInterruptibleThread&& t) noexcept : m_ExceptionPtr(std::move(t.m_ExceptionPtr)), m_Thread(std::move(t.m_Thread)) {}

    tInterruptibleThread& operator=(tInterruptibleThread&& t) noexcept
    {
        m_ExceptionPtr = std::move(t.m_ExceptionPtr);
        m_Thread = std::move(t.m_Thread);
        return *this;
    }

    ~tInterruptibleThread() 
    { 
        if (m_Thread.joinable())
        {
            m_Thread.join();
        }
    }

    template <typename Callable, typename... Args>
    tInterruptibleThread(bool&& detached, Callable&& f, tInterruptionHandlerPtr xInterrupHandler, Args&&... args)
        : m_ExceptionPtr(nullptr)
        , m_Thread(
              [&](bool&& detached,
                  typename std::decay<Callable>::type&& f,
                  tInterruptionHandlerPtr xInterrupHandler,
                  typename std::decay<Args>::type&&... args)
              {
                  try
                  {
                      std::bind(f, std::forward<tInterruptionHandlerPtr>(xInterrupHandler), args...)();
                  }
                  catch (...)
                  {
                      if (!detached)
                      {
                          m_ExceptionPtr = std::current_exception();
                      }
                  }
              },
              detached,
              std::forward<Callable>(f),
              xInterrupHandler,
              std::forward<Args>(args)...) 
        , m_Interruptionhandler(xInterrupHandler)
    {
        ASSERT(m_Interruptionhandler, "Interruption Handler cannot be null");
        if (detached)
        {
            m_Thread.detach();
        }
    }

    bool Joinable() const noexcept { return m_Thread.joinable(); }

    void Join()
    {
        m_Thread.join();

        if (m_ExceptionPtr != nullptr)
        {
            std::rethrow_exception(m_ExceptionPtr);
        }
    }

    void Interrupt() { m_Interruptionhandler->Interrupt(); }

    Id GetId() const noexcept { return m_Thread.get_id(); }

    NativeHandleType NativeHandle() { return m_Thread.native_handle(); }

    static uint32_t HardwareConcurrency() noexcept { return std::thread::hardware_concurrency(); }

    static void Wait(std::chrono::duration<double, std::milli> t) { std::this_thread::sleep_for(t); }

private:
    std::exception_ptr m_ExceptionPtr;
    std::thread m_Thread;
    tInterruptionHandlerPtr m_Interruptionhandler;
};
