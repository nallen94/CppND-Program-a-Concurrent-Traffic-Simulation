#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> ulock(_mutex);
    _cond.wait(ulock, [this]{return !_queue.empty();});

    T msg = std::move(_queue.back()); //get last vector element
    _queue.pop_back(); //remove last element in queue
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::lock_guard<std::mutex> ulock(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true)
    {
        TrafficLightPhase phase = _msgqueue.receive();

        if(phase== TrafficLightPhase::green)
        return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this)); 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

   auto t_start = std::chrono::system_clock::now();
   std::random_device rd;     // Only used once to initialise (seed) engine
   std::mt19937 rng(rd());    // Random-number engine used (Mersenne-Twister in this case)
   std::uniform_int_distribution<int> uni(4,6); // Guaranteed unbiased

   auto cycle_duration = uni(rng);

   while(true)
   {
    std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
    auto time_elapsed= std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now()-t_start).count();     
    if(time_elapsed>cycle_duration)
    {
        if(_currentPhase==TrafficLightPhase::red)
        _currentPhase=TrafficLightPhase::green;
        else if(_currentPhase==TrafficLightPhase::green)
        _currentPhase=TrafficLightPhase::red;

        t_start = std::chrono::system_clock::now();
        _msgqueue.send(std::move(_currentPhase));
        cycle_duration = uni(rng);

    }
   }

   } 




