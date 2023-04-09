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

    // Initialize Recorder
    initialize_recorder();
}

// Initialize Sensor
inline void orbbec::initialize_sensor()
{
    if( device_index != -1 ){
        // Get Connected Devices
        const std::shared_ptr<ob::DeviceList> device_list = context.queryDeviceList();
        if( device_list->deviceCount() == 0 ){
            throw std::runtime_error( "[error] failed to found devices!" );
        }

        // Connect Device by Device Index (USB)
        device = device_list->getDevice( device_index );
    }
    else{
        // Connect Deveice by IP Address and Port (Ethernet)
        device = context.createNetDevice( address.c_str(), port );
    }

    // Create Pipeline
    pipeline = std::make_shared<ob::Pipeline>( device );

    // Get Stream Profile
    const std::shared_ptr<ob::StreamProfileList> color_stream_profile_list = pipeline->getStreamProfileList( OBSensorType::OB_SENSOR_COLOR );
    try{
        color_stream_profile = color_stream_profile_list->getVideoStreamProfile( 1280, 720, OBFormat::OB_FORMAT_YUYV, 30 );
    }
    catch( ob::Error& e ){
        color_stream_profile = std::const_pointer_cast<ob::StreamProfile>( color_stream_profile_list->getProfile( 0 ) )->as<ob::VideoStreamProfile>(); // default
    }
    const std::shared_ptr<ob::StreamProfileList> depth_stream_profile_list = pipeline->getStreamProfileList( OBSensorType::OB_SENSOR_DEPTH );
    try{
        depth_stream_profile = depth_stream_profile_list->getVideoStreamProfile( 320, 288, OBFormat::OB_FORMAT_Y16, 30 );
    }
    catch( ob::Error& e ){
        depth_stream_profile = std::const_pointer_cast<ob::StreamProfile>( depth_stream_profile_list->getProfile( 0 ) )->as<ob::VideoStreamProfile>(); // default
    }

    // Set Stream Profile
    config = std::make_shared<ob::Config>();
    config->enableStream( color_stream_profile );
    config->enableStream( depth_stream_profile );

    // Set Align Mode
    config->setAlignMode( OBAlignMode::ALIGN_D2C_HW_MODE );

    // Get Depth Range
    depth_range = get_depth_range( depth_stream_profile );

    // Start Pipeline
    pipeline->start( config );
}

// Initialize Recorder
void orbbec::initialize_recorder()
{
    // Start Record
    recorder = std::make_shared<ob::Recorder>( pipeline->getDevice() );
    recorder->start( bag_file.c_str() );
}

// Finalize
void orbbec::finalize()
{
    // Stop Record
    recorder->stop();

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

    // Update Depth
    update_depth();

    // Write Frame
    write_frame();
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

// Update Depth
inline void orbbec::update_depth()
{
    if( frameset == nullptr ){
        return;
    }

    // Get Depth Frame
    depth_frame = frameset->depthFrame();
}

// Write Frame
void orbbec::write_frame()
{
    if( frameset == nullptr ){
        return;
    }

    // Write Frame
    recorder->write( frameset );
}

// Draw
void orbbec::draw()
{
    // Draw Color
    draw_color();

    // Draw Depth
    draw_depth();
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

// Draw Depth
inline void orbbec::draw_depth()
{
    if( depth_frame == nullptr ){
        return;
    }

    // Get cv::Mat from ob::VideoFrame
    depth = ob::get_mat( depth_frame );
}

// Show
void orbbec::show()
{
    // Show Color
    show_color();

    // Show Depth
    show_depth();
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

// Show Depth
inline void orbbec::show_depth()
{
    if( depth.empty() ){
        return;
    }

    // Scaling Depth
    const double max_range = std::get<1>( depth_range ) != 0.0 ? std::get<1>( depth_range ) : 5460.0;
    depth.convertTo( depth, CV_8U, -255.0 / max_range, 255.0 );

    // Show Image
    const cv::String window_name = cv::format( "depth (orbbec %d)", device_index );
    cv::imshow( window_name, depth );
}

// Get Depth Range
inline std::tuple<double, double> orbbec::get_depth_range( std::shared_ptr<ob::VideoStreamProfile> depth_stream_profile )
{
    const uint16_t width = depth_stream_profile->width();
    const uint16_t height = depth_stream_profile->height();

    if( width == 320 && height == 288 ){
        return std::make_tuple( 500.0, 5460.0 );
    }
    if( width == 512 && height == 512 ){
        return std::make_tuple( 250.0, 2880.0 );
    }
    if( width == 640 && height == 576 ){
        return std::make_tuple( 500.0, 3860.0 );
    }
    if( width == 1024 && height == 1024 ){
        return std::make_tuple( 250.0, 2210.0 );
    }

    throw std::runtime_error( "[error] unknown depth format!" );
}
