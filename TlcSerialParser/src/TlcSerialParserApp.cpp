#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "cinder/Thread.h"
#include "cinder/Serial.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define BAUD_RATE           115200
#define DATA_PACKET_SIZE    1000     // @9600 & 60fps, data packet must be < 160


class TlcSerialParserApp : public AppNative {
public:
    void setup();
    void keyDown( KeyEvent event );
    void update();
    void draw();
    
    void listDevicesList();
    void sendData();
    
    ci::Serial          *mSerial;
    unsigned char       *mDataPacket;
    int                 mCounter;
    bool                mSendData;
    int                 mDataSize;
    float               mFps;
    int                 mTestVal;
    int                 mTestCh;
    
    params::InterfaceGl mParams;
    vector<int>         mData;
    bool                mLoop;
    int                 mLoopSpeed;
    int                 mLoopVal;
    int                 mLoopDir;
};


void TlcSerialParserApp::setup()
{
    mParams = params::InterfaceGl( "params", Vec2i( 200, 200 ) );
    mParams.addParam( "FPS",        &mFps );
    mParams.addParam( "Ch",         &mTestCh ).min(-1).max(31);
    mParams.addParam( "Value",      &mTestVal ).min(0).max(4095);
    mParams.addParam( "Send data",  &mSendData );
    mParams.addParam( "Loop",       &mLoop );
    mParams.addParam( "Loop speed",       &mLoopSpeed );
    
    listDevicesList();
    
    mDataPacket	= new unsigned char[DATA_PACKET_SIZE];
    mSendData   = false;
    mTestVal    = 0;
    mLoop       = false;
    mLoopVal    = 0;
    mLoopSpeed  = 10;
    mLoopDir    = 1;
    mTestCh     = -1;
    
    for( int k=0; k < 32; k++ )
        mData.push_back(0);
    
    string serialDevName = "tty.usbserial";
    
    try
    {
        Serial::Device dev  = Serial::findDeviceByNameContains( serialDevName );
        mSerial             = new Serial( dev, BAUD_RATE );
        
        console() << "Connected to serial device: " << serialDevName << endl;
    }
    catch ( ... )
    {
        console() << "Cannot connect to serial device: " << serialDevName << endl;
        exit(-1);
    }
    
    //    setFrameRate( 5 );
}


void TlcSerialParserApp::keyDown( KeyEvent event )
{
    int code = event.getCode();
    
    if ( code == KeyEvent::KEY_SPACE )
        mSendData = !mSendData;
    
    else if ( code == KeyEvent::KEY_RETURN )
        sendData();
}


void TlcSerialParserApp::update()
{
    if ( mSendData )
        sendData();
    
    mFps = getAverageFps();
}


void TlcSerialParserApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    mParams.draw();
}


void TlcSerialParserApp::listDevicesList()
{
    console() << "--- List serial devices ---" << endl;
    std::vector<ci::Serial::Device> devices = ci::Serial::getDevices(true);
    for( std::vector<ci::Serial::Device>::const_iterator deviceIt = devices.begin(); deviceIt != devices.end(); ++deviceIt )
        console() << deviceIt->getPath() << endl;
    console() << "---------------------------" << endl;
}


void TlcSerialParserApp::sendData()
{
    uint16_t val;
    
    mLoopVal += mLoopSpeed * mLoopDir;
    mLoopVal = math<int>::clamp( mLoopVal, 0, 4095 );
    if ( mLoopVal == 4095 || mLoopVal == 0 )
        mLoopDir *= -1;
    
    // update data
    for( size_t k=0; k < mData.size(); k++ )
    {
        if ( mTestCh < 0 || mTestCh == k )
        {
            if ( mLoop )
            {
                val = mLoopVal;
            }
            else
                val = mTestVal;
        }
        else
            val = 0;
        
        mDataPacket[k*2]    = ( val & 0xFF00 ) >> 8;       // high byte
        mDataPacket[k*2+1]    = val & 0x00FF;              // low byte
        
    }
    
    // send data
    if ( mSerial )
        mSerial->writeBytes( mDataPacket, mData.size() * 2 );
}


CINDER_APP_NATIVE( TlcSerialParserApp, RendererGl )


