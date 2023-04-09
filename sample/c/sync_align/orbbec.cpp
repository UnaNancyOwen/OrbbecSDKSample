#include "orbbec.hpp"
#include "util.h"
#include "check_error.h"

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
    // Create Context
    context = ob_create_context( &error );
    CHECK_ERROR( error );

    if( device_index != -1 ){
        // Get Connected Devices
        ob_device_list* device_list = ob_query_device_list( context, &error );
        CHECK_ERROR( error );

        const uint32_t device_count = ob_device_list_device_count( device_list, &error );
        CHECK_ERROR( error );

        if( device_count == 0 ) {
            throw std::runtime_error( "[error] failed to found devices!" );
        }

        // Connect Device by Device Index (USB)
        device = ob_device_list_get_device( device_list, device_index, &error );
        CHECK_ERROR( error );

        ob_delete_device_list( device_list, &error );
        CHECK_ERROR( error );
    }
    else{
        // Connect Deveice by IP Address and Port (Ethernet)
        device = ob_create_net_device( context, address.c_str(), port, &error );
        CHECK_ERROR( error );
    }

    // Create Pipeline
    pipeline = ob_create_pipeline_with_device( device, &error );
    CHECK_ERROR( error );

    // Get Stream Profile
    ob_stream_profile_list* color_stream_profile_list = ob_pipeline_get_stream_profile_list( pipeline, ob_sensor_type::OB_SENSOR_COLOR, &error );
    CHECK_ERROR( error );

    color_stream_profile = ob_stream_profile_list_get_video_stream_profile( color_stream_profile_list, 1280, 720, ob_format::OB_FORMAT_BGR, 30, &error );
    if( ob_error_status( error ) != ob_status::OB_STATUS_OK ){
        error = NULL;
        color_stream_profile = ob_stream_profile_list_get_profile( color_stream_profile_list, 0, &error ); // default
    }
    CHECK_ERROR( error );

    ob_delete_stream_profile_list( color_stream_profile_list, &error );
    CHECK_ERROR( error );

    ob_stream_profile_list* depth_stream_profile_list = ob_pipeline_get_stream_profile_list( pipeline, ob_sensor_type::OB_SENSOR_DEPTH, &error );
    CHECK_ERROR( error );

    depth_stream_profile = ob_stream_profile_list_get_video_stream_profile( depth_stream_profile_list, 320, 288, ob_format::OB_FORMAT_Y16, 30, &error );
    if( ob_error_status( error ) != ob_status::OB_STATUS_OK ){
        error = NULL;
        depth_stream_profile = ob_stream_profile_list_get_profile( depth_stream_profile_list, 0, &error ); // default
    }
    CHECK_ERROR( error );

    ob_delete_stream_profile_list( depth_stream_profile_list, &error );
    CHECK_ERROR( error );

    // Set Stream Profile
    config = ob_create_config( &error );
    CHECK_ERROR( error );

    ob_config_enable_stream( config, color_stream_profile, &error );
    CHECK_ERROR( error );
    ob_config_enable_stream( config, depth_stream_profile, &error );
    CHECK_ERROR( error );

    // Set Align Mode
    ob_config_set_align_mode( config, ob_align_mode::ALIGN_D2C_HW_MODE, &error );
    CHECK_ERROR( error );

    // Get Depth Range
    depth_range = get_depth_range( depth_stream_profile );

    // Start Pipeline
    ob_pipeline_start_with_config( pipeline, config, &error );
    CHECK_ERROR( error );
}

// Finalize
void orbbec::finalize()
{
    // Stop Pipeline
    if( pipeline != nullptr ){
        ob_pipeline_stop( pipeline, &error );
        CHECK_ERROR( error );
    }

    // Delete Frame Set
    if( frameset != nullptr ){
        ob_delete_frame( frameset, &error );
        CHECK_ERROR( error );
    }

    // Delete Stream Profiles
    if( color_stream_profile != nullptr ){
        ob_delete_stream_profile( color_stream_profile, &error );
        CHECK_ERROR( error );
    }

    if( depth_stream_profile != nullptr )
    {
        ob_delete_stream_profile( depth_stream_profile, &error );
        CHECK_ERROR( error );
    }

    // Delete Device
    if( device != nullptr ){
        ob_delete_device( device, &error );
        CHECK_ERROR( error );
    }

    // Delete Pipeline
    if( pipeline != nullptr ){
        ob_delete_pipeline( pipeline, &error );
        CHECK_ERROR( error );
    }

    // Delete Config
    if( config != nullptr ){
        ob_delete_config( config, &error );
        CHECK_ERROR( error );
    }

    // Delete Context
    if( context != nullptr ){
        ob_delete_context( context, &error );
        CHECK_ERROR( error );
    }
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
}

// Update Frame
inline void orbbec::update_frame()
{
    // Get Frame Set
    constexpr int32_t timeout = std::chrono::milliseconds( 100 ).count();
    frameset = ob_pipeline_wait_for_frameset( pipeline, timeout, &error );
    CHECK_ERROR( error );
}

// Update Color
inline void orbbec::update_color()
{
    if( frameset == nullptr ){
        return;
    }

    // Get Color Frame
    color_frame = ob_frameset_color_frame( frameset, &error );
    CHECK_ERROR( error );
}

// Update Depth
inline void orbbec::update_depth()
{
    if( frameset == nullptr ){
        return;
    }

    // Get Depth Frame
    depth_frame = ob_frameset_depth_frame( frameset, &error );
    CHECK_ERROR( error );
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

    // Get cv::Mat from ob_frame
    color = ob_get_mat( color_frame );
}

// Draw Depth
inline void orbbec::draw_depth()
{
    if( depth_frame == nullptr ){
        return;
    }

    // Get cv::Mat from ob_frame
    depth = ob_get_mat( depth_frame );
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
inline std::tuple<double, double> orbbec::get_depth_range( ob_stream_profile* depth_stream_profile )
{
    const uint16_t width = ob_video_stream_profile_width( depth_stream_profile, &error );
    const uint16_t height = ob_video_stream_profile_height( depth_stream_profile, &error );

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

