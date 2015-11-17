


#ifndef __WS2812B_H__
#define __WS2812B_H__



typedef struct {
    void    (*init)(void);
} ws2812b_t;



extern ws2812b_t ws2812b;



#endif



