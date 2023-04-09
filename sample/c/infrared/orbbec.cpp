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
    ob_stream_profile_list* stream_profile_list = ob_pipeline_get_stream_profile_list( pipeline, ob_sensor_type::OB_SENSOR_IR, &error );
    CHECK_ERROR( error );

    infrared_stream_profile = ob_stream_profile_list_get_video_stream_profile( stream_profile_list, 320, 288, ob_format::OB_FORMAT_Y16, 30, &error );
    if( ob_error_status( error ) != ob_status::OB_STATUS_OK ){
        error = NULL;
        infrared_stream_profile = ob_stream_profile_list_get_profile( stream_profile_list, 0, &error ); // default
    }
    CHECK_ERROR( error );

    ob_delete_stream_profile_list( stream_profile_list, &error );
    CHECK_ERROR( error );

    // Set Stream Profile
    config = ob_create_config( &error );
    CHECK_ERROR( error );

    ob_config_enable_stream( config, infrared_stream_profile, &error );
    CHECK_ERROR( error );

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
    if( infrared_stream_profile != nullptr ){
        ob_delete_stream_profile( infrared_stream_profile, &error );
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

    // Update Infrared
    update_infrared();
}

// Update Frame
inline void orbbec::update_frame()
{
    // Get Frame Set
    constexpr int32_t timeout = std::chrono::milliseconds( 100 ).count();
    frameset = ob_pipeline_wait_for_frameset( pipeline, timeout, &error );
    CHECK_ERROR( error );
}

// Update Infrared
inline void orbbec::update_infrared()
{
    if( frameset == nullptr ){
        return;
    }

    // Get Infrared Frame
    infrared_frame = ob_frameset_ir_frame( frameset, &error );
    CHECK_ERROR( error );
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

    // Get cv::Mat from ob_frame
    infrared = ob_get_mat( infrared_frame );
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
