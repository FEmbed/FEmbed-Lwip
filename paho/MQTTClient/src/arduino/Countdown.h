/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#if !defined(COUNTDOWN_H)
#define COUNTDOWN_H

class Countdown
{
public:
    Countdown()
    { 
		countdown_ms(10*1000);
    }
    
    Countdown(int ms)
    {
        countdown_ms(ms);
    }
    
    bool expired()
    {
    	if(interval_end_ms)
    	{
			uint32_t now = millis();
			if(now < interval_end_ms && (interval_end_ms - now) < (uint32_t)0x80000000)
				return false;
			return true;
    	}
    	return false;
    }
    
    void countdown_ms(unsigned long ms)  
    {
        interval_end_ms = millis() + ms;
    }
    
    void countdown(int seconds)
    {
        countdown_ms((unsigned long)seconds * 1000L);
    }
    
    int left_ms()
    {
        uint32_t now = millis();
    	if(now < interval_end_ms && (interval_end_ms - now) < (uint32_t)0x80000000)
    		return interval_end_ms - now;
    	return 0;
    }
    
private:
    unsigned long interval_end_ms; 
};

#endif
