/* Online C++ Compiler and Editor */
#include <iostream>

#include <thread>
#include <functional>
#include <atomic>
#include <stdexcept>
#include <cassert>
#include <memory>

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
                      // The tInterruptibleThread instance could be already destroied in detahced mode
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

/******************************************************************/
/*************************** TEST *********************************/

void staticFunction(tInterruptibleThread::tInterruptionHandlerPtr handler, int i)
{
    std::cout << "======================== task " << i << " started" << std::endl;
    const int maxStep = 1000;
    for(int j = 0 ; j < maxStep; j++)
    {
        handler->InterruptionCheckPoint();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "======================= task " << i << " step "<< j <<"/" << maxStep<< std::endl;
    }
        
    
    std::cout << "======================== task " << i << " ended." << std::endl;

}

class TestClass
{
public:
    void doStuff(tInterruptibleThread::tInterruptionHandlerPtr handler, int i) 
    { 
        staticFunction(handler, i);
    }
};

int main()
{ 
    TestClass testClass;
    auto mytask = std::bind(&TestClass::doStuff, &testClass, std::placeholders::_1, std::placeholders::_2);
    tInterruptibleThread::tInterruptionHandlerPtr handler1 = std::make_shared<tInterruptibleThread::tInterruptionHandler>();
    tInterruptibleThread myThread1(true, mytask, handler1, 1);
    
    tInterruptibleThread::tInterruptionHandlerPtr handler2 = std::make_shared<tInterruptibleThread::tInterruptionHandler>();
    tInterruptibleThread myThread2(false, staticFunction, handler2, 2);
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "======================== Interrupting Thread 1"  << "======================== " << std::endl;
    myThread1.Interrupt();
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "======================== Interrupting Thread 2"  << "======================== " << std::endl;
    myThread2.Interrupt();
    
    try
    {
        myThread2.Join(); // Rethrow the exception causing the thread interruption
    }
    catch( std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}