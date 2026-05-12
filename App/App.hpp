/*
 * App.hpp
 *
 *  Created on: 19. 4. 2026
 *      Author: radek
 */

#ifndef APP_HPP_
#define APP_HPP_

#include  "DataTransmit.hpp"


class App {
public:
    void init();
    void loop();

    private:
    DataTransmit DataTransmitter; // Instance pro správu přenosu dat
   
};



#endif /* APP_HPP_ */
