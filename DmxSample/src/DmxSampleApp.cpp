#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "DMXPro.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DmxSampleApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    
    DMXProRef   mDmxController;
};


void DmxSampleApp::setup()
{
    // list all the available serial devices
    // DMX Pro is usually something like "tty.usbserial-EN.."
    DMXPro::listDevices();
    
    // create a device passing the device name or a partial device name
    // useful if you want to swap device without changing the name
    mDmxController  = DMXPro::create( "tty.usbserial-EN" );
    
    if ( mDmxController->isConnected() )
        console() << "DMX device connected" << endl;
}


void DmxSampleApp::mouseDown( MouseEvent event )
{
}


void DmxSampleApp::update()
{
    int dmxChannel  = 2;
    int dmxValue    = 255;
    
    if ( mDmxController && mDmxController->isConnected() )
    {
        // send value 255 to channel 2
        mDmxController->setValue( dmxValue, dmxChannel );
    }
}


void DmxSampleApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}


CINDER_APP_NATIVE( DmxSampleApp, RendererGl )
