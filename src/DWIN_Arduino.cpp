#include "DWIN_Arduino.h"
#include <stdio.h>

#define CMD_HEAD1 0x5A
#define CMD_HEAD2 0xA5
#define CMD_WRITE 0x82
#define CMD_READ 0x83

#define MIN_ASCII 32
#define MAX_ASCII 255


#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(FORCEHWSERIAL)
DWIN::DWIN(HardwareSerial &port, long baud)
{
    port.begin(baud, SERIAL_8N1);
    init((Stream *)&port, false);
}

#elif defined(ARDUINO_ARCH_RP2040)
DWIN::DWIN(HardwareSerial &port, long baud, bool initSerial)
{
    if (initSerial)
    {
        port.begin(baud, SERIAL_8N1);
        init((Stream *)&port, false);
    }
}

#elif ARDUINO_ARCH_STM32
DWIN::DWIN(HardwareSerial &port){
    init((Stream *)&port, false);
}

#elif defined(ESP32)
DWIN::DWIN(HardwareSerial &port, uint8_t receivePin, uint8_t transmitPin, long baud)
{
    port.begin(baud, SERIAL_8N1, receivePin, transmitPin);
    init((Stream *)&port, false);
}

#elif defined(ESP8266)
DWIN::DWIN(uint8_t receivePin, uint8_t transmitPin, long baud)
{
    localSWserial = new SoftwareSerial(receivePin, transmitPin);
    localSWserial->begin(baud);
    init((Stream *)localSWserial, true);
}
DWIN::DWIN(SoftwareSerial &port, long baud)
{
    port.begin(baud);
    init((Stream *)&port, true);
}
DWIN::DWIN(Stream &port, long baud)
{
    init(&port, true);
}

#else
DWIN::DWIN(uint8_t rx, uint8_t tx, long baud)
{
    localSWserial = new SoftwareSerial(rx, tx);
    localSWserial->begin(baud);
    _baud = baud;
    init((Stream *)localSWserial, true);
}

#endif

void DWIN::init(Stream *port, bool isSoft)
{
    this->_dwinSerial = port;
    this->_isSoft = isSoft;
}
void DWIN::echoEnabled(bool echoEnabled)
{
    _echo = echoEnabled;
}
void DWIN::returnWord(bool retWord )
{
   _retWord = retWord;
}

void DWIN::ackDisabled(bool noACK)
{
    _noACK = noACK;
}

// Get Hardware Firmware Version of DWIN HMI
double DWIN::getHWVersion()
{ //  HEX(5A A5 04 83 00 0F 01)
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x0F, 0x01};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    delay(10);
    return readCMDLastByte();
}

double DWIN::getGUISoftVersion()
{ //  HEX(5A A5 04 83 00 0F 01)
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x0F, 0x01};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    delay(10);
    return readCMDLastByte(1);
}

// Restart DWIN HMI
void DWIN::restartHMI()
{ // HEX(5A A5 07 82 00 04 55 aa 5a a5 )
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x04, 0x55, 0xaa, CMD_HEAD1, CMD_HEAD2};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    delay(10);
    readDWIN();
}

// SET DWIN Brightness
void DWIN::setBrightness(byte brightness)
{
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_WRITE, 0x00, 0x82, brightness};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// GET DWIN Brightness
byte DWIN::getBrightness()
{
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x31, 0x01};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    return readCMDLastByte();
}

// Change Page
void DWIN::setPage(byte page)
{
    // 5A A5 07 82 00 84 5a 01 00 02
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x84, 0x5A, 0x01, 0x00, page};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}
// Play a sound
void DWIN::playSound(byte soundID)
{
    // 5A A5 07 82 00 A0 soundID 01 40 00
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0xA0, soundID, 0x01, 0x40, 0x00};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// Get Current Page ID
byte DWIN::getPage()
{
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x14, 0x01};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    return readCMDLastByte();
}
// set the hardware RTC The first two digits of the year are automatically added
void DWIN::setRTC(byte year, byte month, byte day, byte hour, byte minute, byte second)
{
    // 5A A5 0B 82 00 9C 5A A5 year month day hour minute second
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x0B, CMD_WRITE, 0x00, 0x9C, 0x5A, 0xA5, year, month, day, hour, minute, second};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}
// update the software RTC The first two digits of the year are automatically added
void DWIN::setRTCSOFT(byte year, byte month, byte day, byte weekday, byte hour, byte minute, byte second)
{
    // 5A A5 0B 82 0010 year month day weekday(0-6 0=Sunday) hour minute second 00
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x0B, CMD_WRITE, 0x00, 0x10, year, month, day, weekday, hour, minute, second, 0x00};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}
// Set Text on VP Address
void DWIN::setText(long address, String textData)
{
    byte ffEnding[2] = {0xFF,0xFF};
    int dataLen = textData.length();
    byte startCMD[] = {CMD_HEAD1, CMD_HEAD2, (uint8_t)(dataLen + 5), CMD_WRITE,
                       (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address)&0xFF)};
    byte dataCMD[dataLen];
    textData.getBytes(dataCMD, dataLen + 1);
    byte sendBuffer[8 + dataLen];

    memcpy(sendBuffer, startCMD, sizeof(startCMD));
    memcpy(sendBuffer + 6, dataCMD, sizeof(dataCMD));
    memcpy(sendBuffer + (6 + sizeof(dataCMD)),ffEnding,2); // add ending 0xFFFF
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}
    // Set Byte Data on VP Address makes more sense alias of below
void DWIN::setVPByte(long address, byte data){  
    setVP(address, data);
 }
void DWIN::setVP(long address, byte data)
{
    // 0x5A, 0xA5, 0x05, 0x82, 0x40, 0x20, 0x00, state
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address)&0xFF), 0x00, data};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// Set WordData on VP Address
void DWIN::setVPWord(long address, int data)
{
    // 0x5A, 0xA5, 0x05, 0x82, hiVPaddress, loVPaddress, hiData, loData
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address)&0xFF), (uint8_t)((data >> 8) & 0xFF),(uint8_t)((data)&0xFF)};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// read WordData from VP Address you can read sequential multiple words returned in rx event
void DWIN::readVPWord(long address, byte numWords)
{
    // 0x5A, 0xA5, 0x04, 0x83, hiVPaddress, loVPaddress, 0x01 (1 vp to read)
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address)&0xFF), numWords};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
}

// read byte from VP Address (if hiByte = true read HiByte of word)
byte DWIN::readVPByte(long address, bool hiByte)
{
    // 0x5A, 0xA5, 0x04, 0x83, hiVPaddress, loVPaddress, 0x01)
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address)&0xFF),0x1};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    return readCMDLastByte(hiByte);
}

// read or write the NOR from/to VP must be on a even address 2 words are written or read
void DWIN::norReadWrite(bool write, long VPAddress, long NORAddress)
{
    byte readWrite;
    (write) ? (readWrite = 0xa5) : (readWrite = 0x5a);
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x0B, CMD_WRITE, 0x00, 0x08, readWrite, (uint8_t)((NORAddress >> 16) & 0xFF), (uint8_t)((NORAddress >> 8) & 0xFF),
                         (uint8_t)((NORAddress)&0xFF), (uint8_t)((VPAddress >> 8) & 0xFF), (uint8_t)((VPAddress)&0xFF), 0x00, 0x02};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
    delay(30); // DWIN Docs say - appropriate delay - is this it?
}
// beep Buzzer for 1 Sec
void DWIN::beepHMI()
{
    // 0x5A, 0xA5, 0x05, 0x82, 0x00, 0xA0, 0x00, 0x7D
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, 0x00, 0xA0, 0x00, 0x7D};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}
// set text color (16-bit RGB) on controls which allow it ie. text control.
// changes the control sp address space (sp=description pointer) content see the DWIN docs.  
void DWIN::setTextColor(long spAddress, long spOffset, long color)
{ 
    // 0x5A, 0xA5, 0x05, hi spAddress, lo spAddress, hi color, lo color
    spAddress = (spAddress + spOffset);
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, (uint8_t)((spAddress >> 8) & 0xFF), (uint8_t)((spAddress)&0xFF),(uint8_t)((color >> 8) & 0xFF), (uint8_t)((color)&0xFF)};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

//set float value to 32bit DATA Variable  
void DWIN::setFloatValue(long vpAddress, float fValue){
    byte hx[4] = {0};
    byte* new_bytes = reinterpret_cast<byte*>(&fValue);
    memcpy(hx, new_bytes, 4);
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, (uint8_t)((vpAddress >> 8) & 0xFF), (uint8_t)((vpAddress)&0xFF),hx[3],hx[2],hx[1],hx[0] };
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// Send array to the display we dont need the 5A A5 or 
// the size byte hopefully we can work this out.
void DWIN::sendArray(byte dwinSendArray[],byte arraySize)
{
    byte sendBuffer[3+arraySize] = {CMD_HEAD1, CMD_HEAD2, arraySize};

    memcpy(sendBuffer + 3, dwinSendArray, arraySize);
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));

    //look for the ack. on write 
    if (dwinSendArray[0] == CMD_WRITE) 
    {  
     readDWIN();
    }
}

    // Send int array to the display we dont need the 5A A5 or size - words only
void DWIN::sendIntArray(uint16_t instruction,uint16_t dwinIntArray[],byte arraySize){

    // turn our int array to array of bytes
    byte j = 0;
    byte dwinSendByteArray[arraySize];
    for (int i = 0; i < (arraySize >> 1) ; i++) {
        dwinSendByteArray[j] = (uint8_t)((dwinIntArray[i] >> 8) & 0xFF);
        j ++;
        dwinSendByteArray[j] = (uint8_t)((dwinIntArray[i])&0xFF);
        j ++;
    }

    byte sendBuffer[4 + sizeof(dwinSendByteArray)] = {CMD_HEAD1, CMD_HEAD2, (uint8_t)((arraySize + 1)),(uint8_t)((instruction)&0xFF) };
    memcpy(sendBuffer + 4, dwinSendByteArray, sizeof(dwinSendByteArray));
    _dwinSerial->write(sendBuffer,sizeof(sendBuffer));
          
    //look for the ack. on write
    if ((uint8_t)((instruction)&0xFF) == CMD_WRITE) { // or some others?
        readDWIN();
    }

}


// init the serial port in setup useful for Pico boards
void DWIN::initSerial(HardwareSerial &port, long baud)
{
    port.begin(baud, SERIAL_8N1);
    init((Stream *)&port, false);
}

// SET CallBack Event
void DWIN::hmiCallBack(hmiListener callBack)
{
    listenerCallback = callBack;
}

// Listen For incoming callback  event from HMI
void DWIN::listen()
{
      handle();    
}

String DWIN::readDWIN()
{
    String resp = "";
    if (_noACK){
      return resp;   // using no response kernel
    } 
    //* This has to only be enabled for Software serial
#if defined(DWIN_SOFTSERIAL)
    if (_isSoft)
    {
        ((SoftwareSerial *)_dwinSerial)->listen(); // Start software serial listen
    }
#endif

 
    unsigned long startTime = millis(); // Start time for Timeout
    
    while ((millis() - startTime < readTimeout))
    {
        if (_dwinSerial->available() > 0)
        {
            int c = _dwinSerial->read();
            resp.concat(" " + String(c, HEX));
        }
    }
    if (_echo)
    {
        Serial.println("->> " + resp);
    }
    return resp;
}

String DWIN::checkHex(byte currentNo)
{
    if (currentNo < 0x10)
    {
        return "0" + String(currentNo, HEX);
    }
    return String(currentNo, HEX);
}

String DWIN::handle()
{
    int lastBytes;
    int previousByte; 
    String response;
    String address;
    String message;
    bool isSubstr = false;
    bool messageEnd = false;
    bool isFirstByte = false;
    unsigned long startTime = millis();
    while ((millis() - startTime < readTimeout))
    {
        while (_dwinSerial->available() > 0)
        {
            delay(10);
            int inhex = _dwinSerial->read();
            if (inhex == 90 || inhex == 165)
            { // 5A A5
                isFirstByte = true;
                message = "";
                address = ""; 
                response.concat(checkHex(inhex) + " ");
                continue;
            }
            for (int i = 1; i <= inhex; i++)
            {
                int inByte = _dwinSerial->read();  
                if (i == 1)response.concat(checkHex(inhex) + " ");
                if (i == (inhex-1))previousByte=inByte;                           
                response.concat(checkHex(inByte) + " ");
                if (i <= 3)
                {               
                    if ((i == 2) || (i == 3))
                    {
                        address.concat(checkHex(inByte));
                    }
                    continue;
                }
                else
                {
                    if (messageEnd == false )
                    {
                        if (isSubstr && inByte != MAX_ASCII && inByte >= MIN_ASCII)
                        {
                            message += char(inByte);
                        }
                        else
                        {
                            if (inByte == MAX_ASCII)
                            {
                                messageEnd = true;
                            }
                            isSubstr = true;
                        }
                    }
                }
                lastBytes = inByte;
            }
        }
    }

    if (isFirstByte)
    {
        if (_retWord) lastBytes = (previousByte << 8) + lastBytes;
        if (callbackObject != nullptr) {
            callbackObject->dwin_callback(address, lastBytes, message, response);
        } else {
            listenerCallback(address, lastBytes, message, response);
        }
    }
    if (isFirstByte && _echo)
    {
        Serial.println("Address :0x" + address + " | Data :0x" + String(lastBytes, HEX) + " | Message : " + message + " | Response " + response);
    }
    return response;
}

byte DWIN::readCMDLastByte(bool hiByte)
{
    //* This has to only be enabled for Software serial
#if defined(DWIN_SOFTSERIAL)
    if (_isSoft)
    {
        ((SoftwareSerial *)_dwinSerial)->listen(); // Start software serial listen
    }
#endif

    byte lastByte = -1;
    byte previousByte = -1;
    unsigned long startTime = millis(); // Start time for Timeout
    while ((millis() - startTime < commandTimeout))
    {
        while (_dwinSerial->available() > 0)
        {
            previousByte = lastByte;
            lastByte = _dwinSerial->read();
        }
    }
    if (hiByte){
      return previousByte;
    }else{
      return lastByte;  
    }
   
}

void DWIN::flushSerial()
{
    Serial.flush();
    _dwinSerial->flush();
}

void DWIN::setReadTimeout(int timeout_ms) {
    readTimeout = timeout_ms;
}

int DWIN::getReadTimeout() {
    return readTimeout;
}

void DWIN::setCommandTimeout(int timeout_ms) {
    commandTimeout = timeout_ms;
}

int DWIN::getCommandTimeout() {
    return commandTimeout;
}

/**
 * Disable a touch control on the requested page.  You will need to know the PageID, Control ID, and Control Type (code)
 * in order to disable it.  The control ID can be found in DGUS in the sidebar control menu. It is a sequential number
 * based on the order the controls are added.
 *
 * NOTE: Basic Touch controls do NOT get a control ID number.  They cannot be interacted with via the serial interface.
 *
 * @param pageID The DWIN Page ID that the control is located on.
 * @param controlId The ID number of the touch control
 * @param controlCode The type of control being disabled.
 */
void DWIN::disableTouchControl(byte pageID, byte controlId, TOUCH_CONTROL_CODE controlCode) {
    // 5a a5 0b 82 00b0 5aa5  {PAGE} {CONTROL ID} {CONTROL CODE} 0000

    // Before interacting with the touch instruction access, we should check to make sure it is cleared, and ready
    // to accept new instructions.
    byte chkBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x0b, 01};
    byte status;

    do {
        _dwinSerial->write(chkBuffer, sizeof(chkBuffer));
        status = readCMDLastByte();
    } while (status != 0x00);

    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x0b, CMD_WRITE, 0x00, 0xb0, 0x5a,0xa5, 0x00, pageID, controlId, controlCode, 0x00, 0x00};

    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

/**
 * Enable a touch control on the requested page.  You will need to know the PageID, Control ID, and Control Type (code)
 * in order to disable it.  The control ID can be found in DGUS in the sidebar control menu. It is a sequential number
 * based on the order the controls are added.
 *
 * NOTE: Basic Touch controls do NOT get a control ID number.  They cannot be interacted with via the serial interface.
 *
 * @param pageID The DWIN Page ID that the control is located on.
 * @param controlId The ID number of the touch control
 * @param controlCode The type of control being disabled.
 */
void DWIN::enableTouchControl(byte pageID, byte controlId, TOUCH_CONTROL_CODE controlCode) {
    // 5a a5 0b 82 00b0 5aa5  {PAGE} {CONTROL ID} {CONTROL CODE} 0001
    // Before interacting with the touch instruction access, we should check to make sure it is cleared, and ready
    // to accept new instructions.
    byte chkBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x0b, 01};
    byte status;

    do {
        _dwinSerial->write(chkBuffer, sizeof(chkBuffer));
        status = readCMDLastByte();
    } while (status != 0x00);

    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x0b, CMD_WRITE, 0x00, 0xb0, 0x5a,0xa5, 0x00, pageID, controlId, controlCode, 0x00, 0x01};

    // byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x0b, CMD_WRITE, 0x00, 0x0b0, 0x5a, 0xa5, pageID, controlId, controlCode, 0x00, 0x01};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}


/**
 * Hide a control form being displayed by writing 0xFF00 to the SP.   See page: 157 of the manual.
 *
 * @param SP SP for the control we need to hide.
 */
void DWIN::hideSP(long spAddress) {
    // 5a a5 05 82 {SP} FF00
    byte chkBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, (uint8_t)((spAddress >> 8) & 0xFF), (uint8_t)((spAddress)&0xFF), 0xFF, 0x00};
    _dwinSerial->write(chkBuffer, sizeof(chkBuffer));
    readDWIN();
}

/**
 * Show a control by writing the VP back to the SP.   See page: 157 of the manual.
 *
 * @param spAddress SP for the control we need to hide.
 */
void DWIN::showSP(long spAddress, long vpAddress) {
    // 5a a5 05 82 {SP} FF00
    byte chkBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, (uint8_t)((spAddress >> 8) & 0xFF), (uint8_t)((spAddress)&0xFF), (uint8_t)((vpAddress >> 8) & 0xFF), (uint8_t)((vpAddress)&0xFF)};
    _dwinSerial->write(chkBuffer, sizeof(chkBuffer));
    readDWIN();
}

void DWIN::objectCallBack(DWIN_Callable *callback) {
    this->callbackObject = callback;

}

DWIN_Callable::~DWIN_Callable() {
// noop
}
