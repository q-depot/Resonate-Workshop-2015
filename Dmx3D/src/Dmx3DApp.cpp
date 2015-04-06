/*
 *  Dmx3DApp.cpp
 *
 *  Created by Andrea Cuius
 *  The MIT License (MIT)
 *  Copyright (c) 2015 Nocte Studio Ltd.
 *
 *  Resonate 2015 - Connecting Lights workshop
 *
 *  www.resonate.io/2015/
 *
 *  www.nocte.co.uk
 *
 */

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"
#include "cinder/MayaCamUI.h"
#include "DMXPro.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define FIXTURES_NUM    12
#define TRIGGERS_NUM    1


class Dmx3DApp : public AppNative {
    
  public:
    
    void setup();
    
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    
	void update();
	void draw();
    
    void updateTriggers();
    void updateFixtures();
    void drawLabels();
    
    void drawGrid( int steps=10, float size=1.0f );      // size in meters
    
    // we define a structure to hold the position and the brightness
    // value is a float and it store the brightness value normalised(0.0-1.0)
    struct Fixture {
        Vec3f   pos;
        float   value;
        uint8_t dmxChannel;
    };
    
    // the trigger is an abstract object that somehow change the fixtures brightness
    struct Trigger {
        Vec3f   pos;
        float   radius;
        float   speed;
    };
    
    DMXProRef               mDmxController;     // the DMX device
    vector<Fixture>         mFixtures;          // a vector of light fixtures
    vector<Trigger>         mTriggers;          // a vector of triggers
    
    params::InterfaceGl     mParams;            // GUI
    float                   mSpeed;
    bool                    mDrawLabels;
    
    gl::TextureFontRef      mFont;
    MayaCamUI               mMayaCam;
};


void Dmx3DApp::setup()
{
    
    setWindowSize( 1200, 800 );
    
    mSpeed      = 1.0f;
    mDrawLabels = true;
    mFont       = gl::TextureFont::create( Font( "Arial", 16 ) );
    
    // Initialise the GUI
    mParams = params::InterfaceGl("Params" , Vec2i( 180, 100 ) );
    
    mParams.addParam( "Speed", &mSpeed ).min(0.01).max(20.0f).step(0.01f);
    mParams.addParam( "Draw labels", &mDrawLabels ).key("l");
    
    // list all the available serial devices
    // DMX Pro is usually something like "tty.usbserial-EN.."
    DMXPro::listDevices();
    
    // create a device passing the device name or a partial device name
    // useful if you want to swap device without changing the name
    
    string dmxDevice;
    
#if defined( CINDER_MAC )
    dmxDevice = "tty.usbserial-EN";
#elif defined( CINDER_MSW )
    dmxDevice = "USB Serial Port";
#endif
    
    mDmxController      = DMXPro::create( dmxDevice );
    
    if ( mDmxController->isConnected() )
        console() << "DMX device connected" << endl;
    
    float posAngle;
    float radius = 5.0f;
    float height = 4.0f;
    
    // create the fixtures
    for ( int k=0; k < FIXTURES_NUM; k++ )
    {
        posAngle            = 2.0f * M_PI * (float)k/(float)FIXTURES_NUM;
        
        Fixture fixture;
        
        fixture.pos         = Vec3f( radius * cos(posAngle), height, radius * sin(posAngle) );
        fixture.value       = 0.0f;
        fixture.dmxChannel  = k;
        
        mFixtures.push_back( fixture );
    }
    
    // create the triggers
    for ( int k=0; k < TRIGGERS_NUM; k++ )
    {
        posAngle        = randFloat( 1000.0f * M_PI ) * (float)k/(float)FIXTURES_NUM;
        
        Trigger trigger;
        
        trigger.pos     = Vec3f( radius * cos(posAngle), height, radius * sin(posAngle) );
        trigger.radius  = randFloat( 1.0f, 5.0f );
        trigger.speed   = randFloat( 0.005, 0.01 );
        
        mTriggers.push_back( trigger );
    }
    
    // init camera
    CameraPersp initialCam;
    initialCam.setPerspective( 35.0f, getWindowAspectRatio(), 0.1, 10000 );
    mMayaCam.setCurrentCam( initialCam );
}


void Dmx3DApp::mouseDown( MouseEvent event )
{
    if( event.isAltDown() )
        mMayaCam.mouseDown( event.getPos() );
}


void Dmx3DApp::mouseDrag( MouseEvent event )
{
    if( event.isAltDown() )
        mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}


void Dmx3DApp::update()
{
    updateTriggers();
    
    updateFixtures();
}


void Dmx3DApp::updateTriggers()
{
    for( size_t k=0; k < mTriggers.size(); k++ )
        mTriggers[k].pos.rotateY( mTriggers[k].speed * mSpeed );
}


void Dmx3DApp::updateFixtures()
{
    float dist;
    
    for( size_t k=0; k < mFixtures.size(); k++ )
    {
        mFixtures[k].value = 0.0f;                                                          // reset the value
        
        for( size_t i=0; i < mTriggers.size(); i++ )
        {
            dist = mFixtures[k].pos.distance( mTriggers[i].pos );                           // calculate the fixture-trigger distance
            
            if ( dist <= mTriggers[i].radius )                                              // only consider fixtures inside the trigger's radius
            {
                mFixtures[k].value += 1.0f - dist / mTriggers[i].radius;                    // the closer, the brighter
            }
        }
        
        mFixtures[k].value = math<float>::clamp( mFixtures[k].value, 0.0f, 1.0f );          // ensure the value is between 0.0 and 1.0
        
        if ( mDmxController && mDmxController->isConnected() )
            mDmxController->setValue( mFixtures[k].value * 255, mFixtures[k].dmxChannel );  // set the DMX value, range 0-255
    }
}


void Dmx3DApp::draw()
{
    // clear out the window with black
    gl::clear( Color::gray( 0.1f ) );
    
    gl::enableAlphaBlending();
    
    // set the camera matrices
    gl::setMatrices( mMayaCam.getCamera() );
    
    ci::gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.2f ) );
    drawGrid();
    
    float fixtureSize = 0.3f;
    
    
    // draw the fixtures
    for( size_t k=0; k < mFixtures.size(); k++ )
    {
        gl::color( Color::white() * 0.3f );
        gl::drawStrokedCube(mFixtures[k].pos, Vec3f::one() * fixtureSize );
        
        gl::color( ColorA( 1.0f, 1.0f, 1.0f, mFixtures[k].value ) );
        gl::drawCube(mFixtures[k].pos, Vec3f::one() * fixtureSize );
    }
    
    
    // draw the triggers
    gl::enableWireframe();
    gl::color( ColorA( 0.87f, 0.26f, 0.28f, 0.5f ) );
    for( size_t k=0; k < mTriggers.size(); k++ )
        gl::drawSphere( mTriggers[k].pos, mTriggers[k].radius );
    gl::disableWireframe();
    
    
    // draw labels
    if ( mDrawLabels )
    {
        gl::setMatricesWindow( getWindowSize() );
        
        gl::color( Color::gray( 0.8f ) );
        
        Vec2f screenPos;
        for( auto k=0; k < mFixtures.size(); k++ )
        {
            // get the laser screen(2D) position
            screenPos = Vec2f( 12, 0 ) + mMayaCam.getCamera().worldToScreen( mFixtures[k].pos, getWindowWidth(), getWindowHeight() );
            mFont->drawString( to_string(mFixtures[k].dmxChannel), screenPos );
        }
    }
    
    mParams.draw();
}


void Dmx3DApp::drawGrid( int steps, float size )
{
    float halfLineLength = size * steps * 0.5f;     // half line length
    
    for(float i=-halfLineLength; i<=halfLineLength; i+=size)
    {
        ci::gl::drawLine( ci::Vec3f(i, 0.0f, -halfLineLength), ci::Vec3f(i, 0.0f, halfLineLength) );
        ci::gl::drawLine( ci::Vec3f(-halfLineLength, 0.0f, i), ci::Vec3f(halfLineLength, 0.0f, i) );
    }
    
    ci::gl::drawCoordinateFrame();
    gl::color( Color::white() );
}


CINDER_APP_NATIVE( Dmx3DApp, RendererGl )

