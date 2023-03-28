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
    const std::shared_ptr<ob::StreamProfileList> infrared_stream_profile_list = pipeline->getStreamProfileList( OBSensorType::OB_SENSOR_IR );
    try{
        infrared_stream_profile = infrared_stream_profile_list->getVideoStreamProfile( 1280, 720, OBFormat::OB_FORMAT_Y16, 30 );
    }
    catch( ob::Error& e ){
        infrared_stream_profile = std::const_pointer_cast<ob::StreamProfile>( infrared_stream_profile_list->getProfile( 0 ) )->as<ob::VideoStreamProfile>(); // default
    }

    // Set Stream Profile
    config = std::make_shared<ob::Config>();
    config->enableStream( infrared_stream_profile );

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

    // Update Infrared
    update_infrared();
}

// Update Frame
inline void orbbec::update_frame()
{
    // Get Frame Set
    constexpr int32_t timeout = std::chrono::milliseconds( 100 ).count();
    frameset = pipeline->waitForFrames( timeout );
}

// Update Infrared
inline void orbbec::update_infrared()
{
    if( frameset == nullptr ){
        return;
    }

    // Get Infrared Frame
    infrared_frame = frameset->irFrame();
}

// Draw
void orbbec::draw()
{
    // Draw Infrared
    draw_infrared();
}

// Draw Infrared
inline void orbbec::draw_infrared()
{
    if( infrared_frame == nullptr ){
        return;
    }

    // Get cv::Mat from ob::VideoFrame
    infrared = ob::get_mat( infrared_frame );
}

// Show
void orbbec::show()
{
    // Show Infrared
    show_infrared();
}

// Show Infrared
inline void orbbec::show_infrared()
{
    if( infrared.empty() ){
        return;
    }

    // Scaling Infrared
    infrared.convertTo( infrared, CV_8U, 0.5 );

    // Show Image
    const cv::String window_name = cv::format( "infrared (orbbec %d)", device_index );
    cv::imshow( window_name, infrared );
}
