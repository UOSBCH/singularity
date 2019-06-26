#ifndef TIMELOG
#define TIMELOG

#include <ctime>
#include <string>
#include <ostream>

class timelog
{
public:
    timelog()
    {
        _start = std::time(nullptr);
    };
    
    void log(std::string message)
    {
        time_t current = std::time(nullptr);
        
        time_t diff = current - _start;
        
        std::cout << "TIME " << diff << ": " << message << std::endl;
    };
private:
    time_t _start;
};

#endif 
