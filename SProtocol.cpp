#include "SProtocol.h"

SProtocol::SProtocol() {
}

/* begin method with default serial bitrate */
void SProtocol::begin(void)
{
    begin(155200);
}

/* begin method for overriding default serial bitrate */
void SProtocol::begin(long speed)
{
    Serial.begin(speed);
    _transport = &Serial;
}

/* begin method for overriding default stream */
void SProtocol::begin(Stream &s)
{
  _transport = &s;
}

void SProtocol::received(byte channel, byte *data, size_t size) {
    if (_listeners[channel]) {
        _listeners[channel](channel, data, size);
    }
}

int SProtocol::attach(byte channel, SPCallback callback) {
    if (channel > SP_MAX_CHANNEL_COUNT) {
        return -1;
    }

    _listeners[channel] = callback;

    return 1;
}


size_t SProtocol::send(byte channel) {
    return send(channel, NULL, 0);
}

size_t SProtocol::send(byte channel, byte *data, size_t length) {
    byte buff[_MAX_PACKAGE_SIZE] = {0};
    size_t headerSize, index = 0;
    
    if ( channel > SP_MAX_CHANNEL_COUNT || channel < 0 )
        return -1;
    
    if ( length > SP_MAX_DATA_SIZE )
        return -1;
    
    byte marker = (0x80 | (channel & 0x0F) << 3) + length; // Package Marker
    if ( !length ) {
        return _transport->write(&marker, 1);
    }
    
    headerSize = length / 7 + 2;
    buff[0] = marker; 
    
    for ( ; index < length; index++) {
        byte byte = data[index];
        buff[headerSize + index] = byte & 0x7f;
        buff[index / 8 + 1] |= ((byte >> 7) << index % 8) & 0x7f;
    }
    
    return _transport->write(buff, headerSize + length);
}

void SProtocol::processInput() {
    if (!_transport->available()) {
        return;
    }
    
    byte byte = _transport->read();
    if ( processByte(byte) ) {
        received(_decoder.channel, _decoder.data, _decoder.dataSize);
    }
}

// Private methods

int SProtocol::processByte(byte byte) {
    int headerSize, dataOffset;
    
    if (byte >= 0x80) { // Check if byte is marker
        
        _decoder.dataSize = byte & 0x07;
        _decoder.channel = (byte & 0x7F) >> 3;
        _decoder.step = 0;
        
        if ( !_decoder.dataSize )
            return true;
        
    } else if (_decoder.dataSize) {
        headerSize = _decoder.dataSize / 7 + 1;
        
        if (_decoder.step < headerSize) {
            _decoder.buff[_decoder.step] = byte;
            
        } else {
            dataOffset = _decoder.step - headerSize;
            if (_decoder.buff[dataOffset / 7] & (1 << dataOffset % 7)) {
                byte |= 0x80;
            }

            _decoder.buff[_decoder.step] = byte;
        }

        _decoder.step++;
        if (_decoder.dataSize == _decoder.step - headerSize) {
            _decoder.data = _decoder.buff + headerSize;
            return true;
        }
            
    }

    return false;
}