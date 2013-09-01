#ifndef SPROTOCOL_H
#define	SPROTOCOL_H

#define SP_MAX_DATA_SIZE 7
#define SP_MAX_CHANNEL_COUNT 15

#define _MAX_PACKAGE_SIZE SP_MAX_DATA_SIZE + SP_MAX_DATA_SIZE / 7 + 2

#define SP_STATUS_FREE 0
#define SP_STATUS_BUSY 1
#define SP_STATUS_READY 2

#include <Arduino.h>

class SProtocol;
typedef void (*SPCallback)(byte, byte *, size_t);

class SProtocol {
public:
    SProtocol();
    
    void begin();
    void begin(long);
    void begin(Stream &s);
    
    void processInput(void);
    
    size_t send(byte);
    size_t send(byte, byte *, size_t);
    
    int attach(byte channel, SPCallback callback);
    
protected: 
    virtual void received(byte, byte *, size_t length);
    
private:
    Stream *_transport;
    
    SPCallback _listeners[SP_MAX_CHANNEL_COUNT];
    
    struct {
        int dataSize;
        byte channel;
        byte *data;
        byte buff[_MAX_PACKAGE_SIZE];
        int step;
    } _decoder;
    
    int processByte( byte byte );
};

#endif	/* SPROTOCOL_H */

