/*
 *  DmxSampleApp.cpp
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
#include "DMXPro.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define FIXTURES_NUM    12
#define TRIGGERS_NUM    1


class DmxSampleApp : public AppNative {

public:

    void setup();
    void update();
    void draw();

    void updateTriggers();
    void updateFixtures();
    void drawLabels();
    
    // we define a structure to hold the position and the brightness
    // value is a float and it store the brightness value normalised(0.0-1.0)
    struct Fixture {
        Vec2f   pos;
        float   velocity;
        float   value;
        uint8_t dmxChannel;
    };

    // the trigger is an abstract object that somehow change the fixtures brightness
    struct Trigger {
        Vec2f   pos;
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
};


void DmxSampleApp::setup()
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
    float radius = 250.0f;
    
    // create the fixtures
    for ( int k=0; k < FIXTURES_NUM; k++ )
    {
        posAngle            = 2.0f * M_PI * (float)k/(float)FIXTURES_NUM;
        
        Fixture fixture;
        
        fixture.pos         = getWindowCenter() + radius * Vec2f( cos(posAngle), sin(posAngle) );
        fixture.value       = 0.0f;
        fixture.dmxChannel  = k;
    
        mFixtures.push_back( fixture );
    }
    
    // create the triggers
    for ( int k=0; k < TRIGGERS_NUM; k++ )
    {
        posAngle        = randFloat( 1000.0f * M_PI ) * (float)k/(float)FIXTURES_NUM;
        
        Trigger trigger;
        
        trigger.pos     = getWindowCenter() + radius * Vec2f( cos(posAngle), sin(posAngle) );
        trigger.radius  = randFloat( 35.0f, 250.0f );
        trigger.speed   = randFloat( 0.005, 0.01 );
        
        mTriggers.push_back( trigger );
    }
}


void DmxSampleApp::update()
{
    updateTriggers();
    
    updateFixtures();
}


void DmxSampleApp::updateTriggers()
{
    Vec2f pos;
    for( size_t k=0; k < mTriggers.size(); k++ )
    {
        // move the trigger
        pos = ( mTriggers[k].pos - getWindowCenter() );
        pos.rotate( mTriggers[k].speed * mSpeed );
        pos += getWindowCenter();
        
        mTriggers[k].pos = pos;
    }
    
}


void DmxSampleApp::updateFixtures()
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


void DmxSampleApp::draw()
{
	// clear out the window with black
    gl::clear( Color::gray(0.1f) );
    gl::enableAlphaBlending();
    
    float fixtureCircleSize = 15.0f;
    float triggerCircleSize = 5.0f;
    
    // draw the fixtures
    for( size_t k=0; k < mFixtures.size(); k++ )
    {
        // fill up the circle with the actual value
        gl::color( Color::white() * mFixtures[k].value );
        gl::drawSolidCircle( mFixtures[k].pos, fixtureCircleSize );
        
        // draw the contour
        gl::color( Color::white() );
        gl::drawStrokedCircle( mFixtures[k].pos, fixtureCircleSize );
        
        // draw the channel label
        if ( mDrawLabels )
        {
            gl::color( Color::gray( 0.4f ) );
            mFont->drawString( to_string( mFixtures[k].dmxChannel ), mFixtures[k].pos + Vec2i( 22, 15 ) );
        }
    }

    // draw the triggers
    gl::color( Color( 0.87f, 0.26f, 0.28f ) );
    for( size_t k=0; k < mTriggers.size(); k++ )
    {
        gl::drawSolidCircle( mTriggers[k].pos, triggerCircleSize );
        gl::drawStrokedCircle( mTriggers[k].pos, mTriggers[k].radius );
    }
    
    mParams.draw();
}


CINDER_APP_NATIVE( DmxSampleApp, RendererGl )
