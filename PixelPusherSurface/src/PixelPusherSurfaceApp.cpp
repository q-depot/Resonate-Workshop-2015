/*
 *  PixelPusherSurfaceApp.cpp
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

#include "PusherDiscoveryService.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class PixelPusherSurfaceApp : public AppNative {
    
public:
    
    void setup();
    void keyDown( KeyEvent event );
    void update();
    void updateTexture();
    void updatePushers();
    void draw();
    void drawDebugInfo();
    
    PusherDiscoveryServiceRef   mPusherDiscoveryService;
    gl::TextureFontRef          mFontBig, mFontSmall;
    Surface8u                   mOutputSurf;
    
    params::InterfaceGl         mParams;
    
    Vec2f                       mTestPos;
    Vec2f                       mTestPosOff;
    Vec2f                       mTestSpeed;
    bool                        mTestPattern;
};


void PixelPusherSurfaceApp::setup()
{
    // Create a instance of the Discovery Service
    mPusherDiscoveryService = PusherDiscoveryService::create( io_service() );
    
    //
    mOutputSurf     = Surface8u( 185, 185, false );
    
    mFontBig        = gl::TextureFont::create( Font( "Arial", 16 ) );
    mFontSmall      = gl::TextureFont::create( Font( "Arial", 12 ) );
    mTestPattern    = false;
    
    
    // Initialise the GUI
    mParams         = params::InterfaceGl( "params", Vec2i( 260, 150 ) );
    mParams.addParam( "Speed x",        &mTestSpeed.x );
    mParams.addParam( "Speed y",        &mTestSpeed.y );
    mParams.addParam( "Test pattern",   &mTestPattern );
    mParams.addParam( "Offset X",       &mTestPosOff.x );
    mParams.addParam( "Offset Y",       &mTestPosOff.y );
    mParams.setPosition( Vec2i( 300, 15 ) );
    
    setWindowSize( 800, 600 );
}


void PixelPusherSurfaceApp::keyDown( KeyEvent event )
{
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
    if ( pushers.empty() )
        return;
    
    int code = event.getCode();
    
    if ( code == KeyEvent::KEY_r )
        pushers.front()->reset();
    
    else if ( code == KeyEvent::KEY_SPACE )
    {
        mTestPos        = Vec2i::zero();
        mTestPattern    = !mTestPattern;
    }
    
    else if ( code == KeyEvent::KEY_RIGHT || code == KeyEvent::KEY_LEFT || code == KeyEvent::KEY_UP || code == KeyEvent::KEY_DOWN )
    {
        vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
        for( size_t k=0; k < pushers.size(); k++ )
        {
            if ( code == KeyEvent::KEY_RIGHT )
            {
                pushers[k]->setPixelMap( Vec2i::zero(), Strip::MAP_LEFT_RIGHT );
                console() << "Set pixel map: LEFT to RIGHT" << endl;
            }
            else if ( code == KeyEvent::KEY_LEFT )
            {
                pushers[k]->setPixelMap( Vec2i( mOutputSurf.getWidth() - 1, 0 ), Strip::MAP_RIGHT_LEFT );
                console() << "Set pixel map: RIGHT to LEFT" << endl;
            }
            else if ( code == KeyEvent::KEY_DOWN )
            {
                pushers[k]->setPixelMap( Vec2i::zero(), Strip::MAP_TOP_DOWN );
                console() << "Set pixel map: TOP DOWN" << endl;
            }
            else if ( code == KeyEvent::KEY_UP )
            {
                pushers[k]->setPixelMap( Vec2i( 0, mOutputSurf.getHeight() - 1 ), Strip::MAP_BOTTOM_UP );
                console() << "Set pixel map: BOTTOM UP" << endl;
            }
        }
    }
}


void PixelPusherSurfaceApp::update()
{
    updateTexture();
    updatePushers();
}


void PixelPusherSurfaceApp::updateTexture()
{
    mTestPos += mTestSpeed;
    
    if ( mTestPos.x >= mOutputSurf.getWidth() )     mTestPos.x = 0;
    if ( mTestPos.y >= mOutputSurf.getHeight() )    mTestPos.y = 0;
    
    ColorA  col;
    Vec3f   hsv;
    Vec2i   pos;
    Vec2i   cursorPos;
    
    for( size_t x=0; x < mOutputSurf.getWidth(); x++ )
    {
        for( size_t y=0; y < mOutputSurf.getHeight(); y++ )
        {
            pos = Vec2i( x, y );
            
            if ( mTestPattern )
            {
                cursorPos = mTestPos + mTestPosOff;
                
                if ( pos.x == cursorPos.x && pos.y == cursorPos.y)
                    col = Color( 1.0f, 0.0f, 0.0f );
                else if ( pos.x == cursorPos.x )
                    col = Color( 1.0f, 1.0f, 1.0f );
                else if ( pos.y == cursorPos.y )
                    col = Color( 0.0f, 0.0f, 1.0f );
                else
                    col = Color::black();
            }
            else
            {
                col.r = 0.5f * ( 1.0f + sin( (float)( x + y ) * 0.1f + mTestSpeed.x * getElapsedSeconds() ) );
                col.g = 0.5f * ( 1.0f + cos( (float)( x + y ) * 0.1f + mTestSpeed.y * getElapsedSeconds() ) );
                col.b = 1.0 - 0.5f * ( 1.0f + cos( (float)( x + y ) * 0.1f + 2 * getElapsedSeconds() ) );
            }
            
            mOutputSurf.setPixel( pos, col );
        }
    }
}


void PixelPusherSurfaceApp::updatePushers()
{
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
    std::vector<StripRef>       strips;
    std::vector<PixelRef>       pixels;
    
    // set the pixels using the Surface8u
    for( size_t k=0; k < pushers.size(); k++ )
        pushers[k]->setPixels( &mOutputSurf );
}


void PixelPusherSurfaceApp::draw()
{    
    gl::enableAlphaBlending();
    gl::clear( Color( 0, 0, 0 ) );
    
    gl::draw( mOutputSurf, Vec2f( getWindowWidth() - mOutputSurf.getWidth(), 0 ) );
    
    drawDebugInfo();
    
    mParams.draw();
}


void PixelPusherSurfaceApp::drawDebugInfo()
{
    Vec2i                       pos( 15, 25 );
    int                         lineH = 16;
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();    // get all the devices
    std::vector<PusherGroupRef> groups  = mPusherDiscoveryService->getGroups();     // or get the groups
    PixelPusherRef              pusher;
    
    mFontBig->drawString( "FPS: " + to_string( getAverageFps() ),   pos );   pos.y += lineH * 2;
    
    mFontBig->drawString( "Pusher Discovery Service",                                                                   pos );   pos.y += lineH;
    mFontSmall->drawString( "- - - - - - - - - - - - - - - - - - - - - - - - - -",                                      pos );   pos.y += lineH;
    mFontSmall->drawString( "Globabl brightness:\t\t"   + to_string( PusherDiscoveryService::getGlobalBrightness() ),   pos );   pos.y += lineH;
    mFontSmall->drawString( "Total power:\t\t\t"        + to_string( PusherDiscoveryService::getTotalPower() ),         pos );   pos.y += lineH;
    mFontSmall->drawString( "Total power limit:\t\t"    + to_string( PusherDiscoveryService::getTotalPowerLimit() ),    pos );   pos.y += lineH;
    mFontSmall->drawString( "Color correction:\t\t"     + to_string( PusherDiscoveryService::getColorCorrection() ),    pos );   pos.y += lineH;
    mFontSmall->drawString( "Frame limit:\t\t\t"        + to_string( PusherDiscoveryService::getFrameLimit() ),         pos );   pos.y += lineH;
    mFontSmall->drawString( "Total Devices:\t\t\t"      + to_string( pushers.size() ),                                  pos );   pos.y += lineH;
    mFontSmall->drawString( "Total groups:\t\t\t"       + to_string( groups.size() ),                                   pos );   pos.y += lineH;
    
    pos.y += lineH * 2;
    
    for( size_t j=0; j < pushers.size(); j++ )
    {
        pusher = pushers[j];
        
        mFontBig->drawString( "Group " + to_string( pusher->getGroupId() ),                             pos );   pos.y += lineH;
        mFontSmall->drawString( "Controller "           + to_string( pusher->getControllerId() ),       pos );   pos.y += lineH;
        mFontSmall->drawString( "- - - - - - - - - - - - - - - - - - -",                                pos );   pos.y += lineH;
        
        // Network
        mFontSmall->drawString( "IP: " + pusher->getIp() + " (" + to_string( pusher->getPort() ) + ")", pos );  pos.y += lineH;
        mFontSmall->drawString( "Mac: " + pusher->getMacAddress(), pos );                                       pos.y += lineH;
        mFontSmall->drawString( "Artnet: universe "     + to_string( pusher->getArtnetUniverse() ) + ", ch " + to_string( pusher->getArtnetChannel() ), pos ); pos.y += lineH;
        
        // Software/hardware
        mFontSmall->drawString( "Software rev: "        + to_string( pusher->getSoftwareRevision() ),   pos );  pos.y += lineH;
        mFontSmall->drawString( "Hardware rev: "        + to_string( pusher->getHardwareRevision() ),   pos );  pos.y += lineH;
        
        // strips
        mFontSmall->drawString( "Num strips: "          + to_string( pusher->getNumStrips() ),          pos );  pos.y += lineH;
        mFontSmall->drawString( "Pixels per strip: "    + to_string( pusher->getPixelsPerStrip() ),     pos );  pos.y += lineH;
        
        mFontSmall->drawString( "Total power: "         + to_string( pusher->getPowerTotal() ),         pos );  pos.y += lineH;
        mFontSmall->drawString( "Update period: "       + to_string( pusher->getUpdatePeriod() ),       pos );  pos.y += lineH;
        mFontSmall->drawString( "Thread sleep for: "    + to_string( pusher->getThreadSleepFor() ),     pos );  pos.y += lineH;
        mFontSmall->drawString( "Packet number: "       + to_string( pusher->getPacketNumber() ),       pos );  pos.y += lineH;
        
        pos.x += 250;
    }
}


CINDER_APP_NATIVE( PixelPusherSurfaceApp, RendererGl )

