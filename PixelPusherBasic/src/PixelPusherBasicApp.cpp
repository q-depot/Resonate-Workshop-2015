/*
 *  PixelPusherBasicApp.cpp
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


class PixelPusherBasicApp : public AppNative {
    
public:
    
    void setup();
    void keyDown( KeyEvent event );
    void update();
    void updatePushers();
    void draw();
    void drawStrips();
    void drawDebugInfo();
    
    PusherDiscoveryServiceRef   mPusherDiscoveryService;
    gl::TextureFontRef          mFontBig, mFontSmall;
    
    params::InterfaceGl         mParams;
    float                       mSpeed;
    int                         mOffset;
    Color                       mColor;
};


void PixelPusherBasicApp::setup()
{
    setWindowSize( 800, 600 );

    // Create a instance of the Discovery Service
    mPusherDiscoveryService = PusherDiscoveryService::create( io_service() );
    PusherDiscoveryService::enableColorCorrection(true);

    mFontBig        = gl::TextureFont::create( Font( "Arial", 16 ) );
    mFontSmall      = gl::TextureFont::create( Font( "Arial", 12 ) );
    mSpeed          = 1.0f;
    mOffset         = 0;
    mColor          = Color::white();
    
    // Initialise the GUI
    mParams         = params::InterfaceGl( "params", Vec2i( 260, 150 ) );
    mParams.setPosition( Vec2i( 200, 150 ) );
    mParams.addParam( "Speed",  &mSpeed ).min(0.0f).max(25.0f).step(0.1f);
    mParams.addParam( "Offset", &mOffset );
    mParams.addParam( "Color",  &mColor );
}


void PixelPusherBasicApp::keyDown( KeyEvent event )
{
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
    if ( pushers.empty() )
        return;
    
    int code = event.getCode();
    
    if ( code == KeyEvent::KEY_r )
        pushers.front()->reset();
}


void PixelPusherBasicApp::update()
{

    updatePushers();
}


void PixelPusherBasicApp::updatePushers()
{
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();        // get connected devices
    std::vector<StripRef>       strips;
    std::vector<PixelRef>       pixels;
    uint8_t                     red, blue, green;

    int c = 0;
    for( auto& pusher : pushers )
    {
        for( auto& strip : pusher->getStrips() )                                  // for each device get the Strips
        {
            for( size_t k=0; k < strip->getNumPixels(); k++ )                               // get the pixels
            {
                red     = mColor.r * 255 * ( sin( c * mOffset + getElapsedSeconds() * mSpeed ) + 1.0f ) * 0.5f;
                blue    = mColor.b * 255;
                green   = mColor.g * 255;

                strip->setPixel( k,  red, green, blue );
                c++;
            }
        }
    }
}


void PixelPusherBasicApp::draw()
{
    gl::enableAlphaBlending();
    gl::clear( Color::gray( 0.1f ) );
    
    drawStrips();
    
    gl::color( Color::white() );
    drawDebugInfo();
    
    mParams.draw();
}


void PixelPusherBasicApp::drawStrips()
{
    Vec2f                       size    = Vec2f( 2, 3 );
    Vec2f                       margin  = Vec2f( 10, 2 );
    Vec2f                       topLeft = Vec2f( 200, 15 );
    Vec2f                       pos;
    
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();        // get connected devices
    std::vector<StripRef>       strips;
    std::vector<PixelRef>       pixels;
    
    glBegin( GL_QUADS );
    
    for( const auto& pusher : pushers )
    {
        strips = pusher->getStrips();
        
        for( size_t stripN = 0; stripN < strips.size(); stripN ++ )                                  // for each device get the Strips
        {
            pixels = strips[stripN]->getPixels();
            
            for( size_t pixelN = 0; pixelN < pixels.size(); pixelN ++ )                              // get the pixels
            {
                pos = topLeft + ( Vec2f( pixelN, stripN ) * ( size ) );
                gl::color( pixels[pixelN]->getColorRgb() );

                gl::vertex( pos );
                gl::vertex( pos + Vec2f( size.x, 0 ) );
                gl::vertex( pos + size );
                gl::vertex( pos + Vec2f( 0, size.y ) );
            }
        }
    }
    
    glEnd();
}

void PixelPusherBasicApp::drawDebugInfo()
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


CINDER_APP_NATIVE( PixelPusherBasicApp, RendererGl )

