#include <iostream>

#include <condition_variable>
#include <functional>
#include <memory>
#include <stdexcept>
#include <atomic>
#include <mutex>
#include <thread>

#include <cassert>
#define ASSERT(expr, msg) assert(((void)(msg), (expr)))

class tInterruptibleThread
{
public:
    class tInterruptionHandler
    {
    public:
        class tException : public std::exception
        {
        public:
            virtual char const* what() const noexcept override { return "Thread interrupted"; }
        };

        tInterruptionHandler() : m_Interrupt(false) {}
        virtual void InterruptionCheckPoint()
        {
            if (m_Interrupt)
            {
                throw tException();
            }
        }
        bool Interrupted() { return m_Interrupt; }
        void Interrupt() { m_Interrupt = true; }

    protected:
        std::atomic_bool m_Interrupt;
    };
    typedef std::shared_ptr<tInterruptionHandler> tInterruptionHandlerPtr;

    using Id = std::thread::id;

    using NativeHandleType = std::thread::native_handle_type;


    tInterruptibleThread() noexcept = default;

    ~tInterruptibleThread()
    {
        if (m_xThread && m_xThread->joinable())
        {
            JoinImp();
        }
    }

    template <typename Callable, typename... Args>
    tInterruptibleThread(bool detached, Callable&& f, tInterruptionHandlerPtr xInterrupHandler, Args&&... args)
        : m_ExceptionPtr(nullptr)
        , m_Mutex()
        , m_Ready(false)
        , m_Interruptionhandler(xInterrupHandler)
    {        
        auto task = [this](bool isDetached,
                        typename std::decay<Callable>::type&& f,
                        tInterruptionHandlerPtr xInterrupHandler,
                        typename std::decay<Args>::type&&... args)
        {
            if (!isDetached)
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_Ready = false;
            }
            try
            {
                std::bind(f, std::forward<tInterruptionHandlerPtr>(xInterrupHandler), args...)();
            }
            catch (...)
            {
                // The tInterruptibleThread instance could be already destroied in detahced mode
                if (!isDetached)
                {
                    m_ExceptionPtr = std::current_exception();
                }
            }

            if (!isDetached)
            {
                {
                    std::lock_guard<std::mutex> lock(m_Mutex);
                    m_Ready = true;
                }
                m_Condition.notify_one();
            }
        };

        m_xThread = std::unique_ptr<std::thread>(new std::thread(
              task,               
              detached,
              std::forward<Callable>(f),
              xInterrupHandler,
              std::forward<Args>(args)...));

        if(!m_xThread)
        {
            throw std::runtime_error("Cannot create the tread");
        }
        
        if (detached)
        {
            m_xThread->detach();
        }
    }

    bool Joinable() const noexcept { return m_xThread->joinable(); }

    void Join()
    {
        JoinImp();

        if (m_ExceptionPtr != nullptr)
        {
            std::rethrow_exception(m_ExceptionPtr);
        }
    }

    tInterruptionHandlerPtr InterruptionHandler() { return m_Interruptionhandler; }

    void Interrupt()
    {
        if (m_Interruptionhandler)
        {
            m_Interruptionhandler->Interrupt();
        }
    }

    Id GetId() const noexcept { return m_xThread->get_id(); }

    NativeHandleType NativeHandle() { return m_xThread->native_handle(); }

    static uint32_t HardwareConcurrency() noexcept { return std::thread::hardware_concurrency(); }

    static void Wait(std::chrono::duration<double, std::milli> t) { std::this_thread::sleep_for(t); }

private:
    void JoinImp()
    {
        // m_Thread.join() could throw No such proccess exception
        // also if the thread is joinable so we detach the tread
        // and we wait for completion
        m_xThread->detach();
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Condition.wait(lock, [&] { return (bool)m_Ready; });
    }
    std::exception_ptr m_ExceptionPtr;
    tInterruptionHandlerPtr m_Interruptionhandler;
    std::condition_variable m_Condition;
    std::mutex m_Mutex;
    bool m_Ready;
    std::unique_ptr<std::thread> m_xThread;
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
