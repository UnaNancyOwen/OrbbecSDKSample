#include "orbbec.hpp"
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

    // Initialize Point Cloud
    initialize_pointcloud();
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

    color_stream_profile = ob_stream_profile_list_get_video_stream_profile( color_stream_profile_list, 1280, 720, ob_format::OB_FORMAT_YUYV, 30, &error );
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
    if( format == ob_format::OB_FORMAT_POINT ){
        ob_config_set_align_mode( config, ob_align_mode::ALIGN_DISABLE, &error );
    }
    CHECK_ERROR( error );

    // Start Pipeline
    ob_pipeline_start_with_config( pipeline, config, &error );
    CHECK_ERROR( error );

    // Create Point Cloud Filter
    pointcloud_filter = ob_create_pointcloud_filter( &error );
    CHECK_ERROR( error );

    const ob_camera_param camera_parameter = ob_pipeline_get_camera_param( pipeline, &error );
    CHECK_ERROR( error );

    ob_pointcloud_filter_set_camera_param( pointcloud_filter, camera_parameter, &error );
    CHECK_ERROR( error );

    ob_pointcloud_filter_set_point_format( pointcloud_filter, format, &error );
    CHECK_ERROR( error );

    ob_pointcloud_filter_set_color_data_normalization( pointcloud_filter, true, &error );
    CHECK_ERROR( error );
}

// Initialize Point Cloud
void orbbec::initialize_pointcloud()
{
    // Create Point Cloud
    pointcloud = std::make_shared<open3d::geometry::PointCloud>();

    // Create Visualize Window
    const int32_t width = 1280;
    const int32_t height = 720;
    visualizer.CreateVisualizerWindow( "Open3D", width, height );
    visualizer.RegisterKeyCallback( GLFW_KEY_ESCAPE,
        [&]( open3d::visualization::Visualizer* visualizer ){
            is_run = false;
            return true;
        }
    );
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
    while( is_run ){
        // Update
        update();

        // Draw
        draw();

        // Show
        show();
    }
}

// Update
void orbbec::update()
{
    // Update Frame
    update_frame();

    // Update Point Cloud
    update_pointclod();
}

// Update Frame
inline void orbbec::update_frame()
{
    // Get Frame Set
    constexpr int32_t timeout = std::chrono::milliseconds( 100 ).count();
    frameset = ob_pipeline_wait_for_frameset( pipeline, timeout, &error );
    CHECK_ERROR( error );
}

// Update Point Cloud
inline void orbbec::update_pointclod()
{
    if( frameset == nullptr ){
        return;
    }

    if( ob_frameset_color_frame( frameset, &error ) == nullptr || ob_frameset_color_frame( frameset, &error ) == nullptr ){
        return;
    }
    CHECK_ERROR( error );

    // Create Point Cloud from Frame Set
    pointcloud_frame = ob_filter_process( pointcloud_filter, frameset, &error );
    CHECK_ERROR( error );
}

// Draw
void orbbec::draw()
{
    // Draw Point Cloud
    draw_pointcloud();
}

// Draw Point Cloud
inline void orbbec::draw_pointcloud()
{
    if( pointcloud_frame == nullptr ){
        return;
    }

    // Create Point Cloud for Open3D
    if( format == ob_format::OB_FORMAT_RGB_POINT ){
        const int32_t num_points = ob_frame_data_size( pointcloud_frame, &error ) / sizeof( ob_color_point );
        CHECK_ERROR( error );

        ob_color_point* data = reinterpret_cast<ob_color_point*>( ob_frame_data( pointcloud_frame, &error ) );
        CHECK_ERROR( error );

        std::vector<Eigen::Vector3d> points = std::vector<Eigen::Vector3d>( num_points );
        std::vector<Eigen::Vector3d> colors = std::vector<Eigen::Vector3d>( num_points );

        #pragma omp parallel for
        for( int32_t i = 0; i < num_points; i++ ){
            points[i] = Eigen::Vector3d( data[i].x, data[i].y, data[i].z );
            colors[i] = Eigen::Vector3d( data[i].r, data[i].g, data[i].b );
        }

        pointcloud->points_ = points;
        pointcloud->colors_ = colors;
    }
    else{
        const int32_t num_points = ob_frame_data_size( pointcloud_frame, &error ) / sizeof( ob_point );
        CHECK_ERROR( error );

        ob_point* data = reinterpret_cast<ob_point*>( ob_frame_data( pointcloud_frame, &error ) );
        CHECK_ERROR( error );

        std::vector<Eigen::Vector3d> points = std::vector<Eigen::Vector3d>( num_points );

        #pragma omp parallel for
        for( int32_t i = 0; i < num_points; i++ ){
            points[i] = Eigen::Vector3d( data[i].x, data[i].y, data[i].z );
        }

        pointcloud->points_ = points;
    }

    ob_delete_frame( pointcloud_frame, &error );
    CHECK_ERROR( error );
}

// Show
void orbbec::show()
{
    // Show Point Cloud
    show_pointcloud();
}

// Show Point Cloud
inline void orbbec::show_pointcloud()
{
    if( pointcloud_frame == nullptr ){
        return;
    }

    static bool is_added = false;
    if( !is_added ){
        visualizer.AddGeometry( pointcloud );
        is_added = true;
    }

    visualizer.UpdateGeometry();
    visualizer.PollEvents();
    visualizer.UpdateRender();
}
