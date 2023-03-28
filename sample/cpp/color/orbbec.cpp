#include "orbbec.hpp"
#include "util.h"

#include <vector>
#include <chrono>

// Constructor
orbbec::orbbec()
{
    // Initialize
    initialize();
}

orbbec::~orbbec()
{
    // Finalize
    finalize();
}

// Initialize
void orbbec::initialize()
{
    // Initialize Sensor
    initialize_sensor();
}

// Initialize Sensor
inline void orbbec::initialize_sensor()
{
    // Get Connected Devices
    const std::shared_ptr<ob::DeviceList> device_list = context.queryDeviceList();
    if( device_list->deviceCount() == 0 ){
        throw std::runtime_error( "[error] failed to found devices!" );
    }

    // Connect Default Device by Device Index
    device = device_list->getDevice( device_index );

    // Create Pipeline
    pipeline = std::make_shared<ob::Pipeline>( device );

    // Get Stream Profile
    const std::shared_ptr<ob::StreamProfileList> stream_profile_list = pipeline->getStreamProfileList( OBSensorType::OB_SENSOR_COLOR );
    try{
        color_stream_profile = stream_profile_list->getVideoStreamProfile( 1280, 720, OBFormat::OB_FORMAT_BGRA, 30 );
    }
    catch( ob::Error& e ){
        color_stream_profile = std::const_pointer_cast<ob::StreamProfile>( stream_profile_list->getProfile( 0 ) )->as<ob::VideoStreamProfile>(); // default
    }

    // Set Stream Profile
    config = std::make_shared<ob::Config>();
    config->enableStream( color_stream_profile );

    // Start Pipeline
    pipeline->start( config );
}

// Finalize
void orbbec::finalize()
{
    // Stop Pipeline
    pipeline->stop();
}

// Run
void orbbec::run()
{
    // Main Loop
    while( true ){
        // Update
        update();

        // Draw
        draw();

        // Show
        show();

        // Wait Key
        constexpr int32_t delay = 10;
        const int32_t key = cv::waitKey( delay );
        if( key == 'q' ){
            break;
        }
    }
}

// Update
void orbbec::update()
{
    // Update Frame
    update_frame();

    // Update Color
    update_color();
}

// Update Frame
inline void orbbec::update_frame()
{
    // Get Frame Set
    constexpr int32_t timeout = std::chrono::milliseconds( 100 ).count();
    frameset = pipeline->waitForFrames( timeout );
}

// Update Color
inline void orbbec::update_color()
{
    if( frameset == nullptr ){
        return;
    }

    // Get Color Frame
    color_frame = frameset->colorFrame();
}

// Draw
void orbbec::draw()
{
    // Draw Color
    draw_color();
}

// Draw Color
inline void orbbec::draw_color()
{
    if( color_frame == nullptr ){
        return;
    }

    // Get cv::Mat from ob::VideoFrame
    color = ob::get_mat( color_frame );
}

// Show
void orbbec::show()
{
    // Show Color
    show_color();
}

// Show Color
inline void orbbec::show_color()
{
    if( color.empty() ){
        return;
    }

    // Show Image
    const cv::String window_name = cv::format( "color (orbbec %d)", device_index );
    cv::imshow( window_name, color );
}
