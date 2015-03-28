/*
 *  Lasers3DApp.cpp
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
#include "cinder/Rand.h"
#include "cinder/MayaCamUI.h"
#include "cinder/params/Params.h"
#include "TlcSerial.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Lasers3DApp : public AppNative {
    
public:
    
    struct LaserObj {
        Vec3f   pos;
        Vec3f   dir;
        float   value;
    };
    
    
public:
    
	void setup();
    
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    
	void update();
	void draw();
    
    void drawGrid( int steps=10, float size=1.0f );      // size in meters
    
    TlcSerialRef        mTlcSerial;
    vector<LaserObj>    mLasers;
    
    MayaCamUI           mMayaCam;
    gl::TextureFontRef  mFont;
    bool                mDrawLabels;
    float               mBeamLength;
    params::InterfaceGl mParams;
    
};


void Lasers3DApp::setup()
{
    setWindowSize( 1200, 800 );
    
    TlcSerial::listDevices();
    
    mTlcSerial = TlcSerial::create( "", 1 );
    
    // create a laser object for each channel
    // we'll use LaserObj to store position, direction and value for the preview
    for( auto ch=0; ch < mTlcSerial->getNumChannels(); ch++ )
    {
        LaserObj laser;
        laser.pos   = Vec3f( 0, 5.0f, 0 ) + randVec3f() * 5.0f;     // assign a random position
        laser.dir   = randVec3f().normalized();                     // assign a random direction
        laser.value = 0.0f;                                         // set the init value
        
        mLasers.push_back( laser );
    }
    
    // init camera
    CameraPersp initialCam;
    initialCam.setPerspective( 35.0f, getWindowAspectRatio(), 0.1, 10000 );
    mMayaCam.setCurrentCam( initialCam );
    
    mFont       = gl::TextureFont::create( Font("Arial", 14 ) );
    mDrawLabels = true;
    mBeamLength = 5.0f;

    // initialise the GUI
    mParams     = params::InterfaceGl( "Params", Vec2i( 180, 120 ) );
    mParams.addParam( "Draw labels", &mDrawLabels ).key("l");
    mParams.addParam( "Beam length", &mBeamLength ).min(0.1f).max(20.0f).step(0.1f);
}


void Lasers3DApp::mouseDown( MouseEvent event )
{
    if( event.isAltDown() )
    {
        mMayaCam.mouseDown( event.getPos() );
        event.setHandled();
    }
}


void Lasers3DApp::mouseDrag( MouseEvent event )
{
    if( event.isAltDown() )
    {
        mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
        event.setHandled();
    }
}


void Lasers3DApp::update()
{
    float val, offset;

    // set some data
    for( auto ch=0; ch < mTlcSerial->getNumChannels(); ch++ )
    {
        // use the sine and offset to set each channel value(normalised)
        offset  = M_PI * 2 * (float)ch / (float)mTlcSerial->getNumChannels();
        val     = ( sin( getElapsedSeconds() + offset ) + 1.0f ) * 0.5f;
        
        
        mTlcSerial->setValue( ch, val );        // set the value
        
        mLasers[ch].value = val;                // set the LaserObj value for preview
    }
    
    // send serial data
    mTlcSerial->sendData();
}


void Lasers3DApp::draw()
{
	// clear out the window with black
    gl::clear( Color::gray( 0.1f ) );
    
    gl::enableAlphaBlending();
    
    // set the camera matrices
    gl::setMatrices( mMayaCam.getCamera() );
    

    ci::gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.2f ) );
    drawGrid();
    
    // draw Lasers
    for( auto &laser : mLasers )
    {
        gl::color( Color::gray( 0.3f ) );
        gl::drawStrokedCube( laser.pos, Vec3f::one() * 0.15f );
        
        gl::color( ColorA( 1.0f, 0.0f, 0.0f, laser.value ) );
        gl::drawLine( laser.pos, laser.pos + laser.dir * mBeamLength );
    }
    
    // draw labels
    if ( mDrawLabels )
    {
        gl::setMatricesWindow( getWindowSize() );
        
        gl::color( Color::gray( 0.8f ) );
        
        Vec2f screenPos;
        for( auto ch=0; ch < mLasers.size(); ch++ )
        {
            // get the laser screen(2D) position
            screenPos = Vec2f( 12, 0 ) + mMayaCam.getCamera().worldToScreen( mLasers[ch].pos, getWindowWidth(), getWindowHeight() );
            mFont->drawString( to_string(ch), screenPos );
        }
    }
    
    mParams.draw();
}


void Lasers3DApp::drawGrid( int steps, float size )
{
    float halfLineLength = size * steps * 0.5f;     // half line length
    
    for(float i=-halfLineLength; i<=halfLineLength; i+=size) {
        ci::gl::drawLine( ci::Vec3f(i, 0.0f, -halfLineLength), ci::Vec3f(i, 0.0f, halfLineLength) );
        ci::gl::drawLine( ci::Vec3f(-halfLineLength, 0.0f, i), ci::Vec3f(halfLineLength, 0.0f, i) );
    }
    
    ci::gl::drawCoordinateFrame();
    gl::color( Color::white() );
}



CINDER_APP_NATIVE( Lasers3DApp, RendererGl )
