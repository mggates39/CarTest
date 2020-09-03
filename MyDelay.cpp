#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "MyDelay.h"

myDelay::myDelay(void) {
  this->_preMills = 0;
  this->setDelay(0);
}

myDelay::myDelay(unsigned long dtime) {
  this->_preMills = 0;
  this->setDelay(dtime);
}

myDelay::myDelay(unsigned long dtime, funTocall funcall) {
  this->_preMills = 0;
  this->setDelay(dtime);
  this->setCallback(funcall);
}

myDelay::myDelay(unsigned long dtime, funTocall funcall, int repeatCount) {
  this->_preMills = 0;
  this->setDelay(dtime);
  this->setCallback(funcall);
  this->setRepeat(repeatCount);
}

void myDelay::setDelay(unsigned long dtime) {
  this->_delaytime = dtime;
}

void myDelay::setCallback(funTocall funcall) {
  if (funcall != 0) {
    this->_funcall = funcall;
    this->_useFunction = true;
  } else {
    this->_useFunction = false;
  }
}

void myDelay::setRepeat(int repeatCount) {
  this->_initialRepeatCount = repeatCount;
  this->_currentRepeatCount = repeatCount;
  this->_repeating = true;
}

void myDelay::start() {
  this->_preMills = millis();
  this->_currentRepeatCount = this->_initialRepeatCount;
  this->_running = true;
}

void myDelay::stop() {
  this->_preMills = 0;
  this->_running = false;
}

bool myDelay::update() {
  if (this->_running) {
    this->_curMills = millis();
    if (this->_curMills - this->_preMills >= this->_delaytime) {
      if (this->_repeating) {
        if (this->_initialRepeatCount != MYDELAY_REPEAT_FOREVER) {
          this->_currentRepeatCount--;
          if (this->_currentRepeatCount == 0) {
            this->stop();
          }
        }
      }
      this->_preMills = this->_curMills;
      if (this->_useFunction == true) {
        this->_funcall();
        return true;
      } else {
        return true;
      }
    } else  {
      return false;
    }
  } else {
    return false;
  }
}

bool myDelay::isRunning() {
  return this->_running;
}
