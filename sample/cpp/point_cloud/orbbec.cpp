#include "orbbec.hpp"

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
    if( format == OBFormat::OB_FORMAT_POINT ){
        config->setAlignMode( OBAlignMode::ALIGN_DISABLE );
    }

    // Start Pipeline
    pipeline->start( config );

    // Create Point Cloud Filter
    pointcloud_filter = std::make_shared< ob::PointCloudFilter>();
    const OBCameraParam camera_parameter = pipeline->getCameraParam();
    pointcloud_filter->setCameraParam( camera_parameter );
    pointcloud_filter->setCreatePointFormat( format );
    pointcloud_filter->setColorDataNormalization( true );
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
    pipeline->stop();
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
    frameset = pipeline->waitForFrames( timeout );
}

// Update Point Cloud
inline void orbbec::update_pointclod()
{
    if( frameset == nullptr ){
        return;
    }

    if( frameset->colorFrame() == nullptr || frameset->depthFrame() == nullptr ) {
        return;
    }

    // Create Point Cloud from Frame Set
    pointcloud_frame = pointcloud_filter->process( frameset );
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
    if( format == OBFormat::OB_FORMAT_RGB_POINT ){
        const int32_t num_points = pointcloud_frame->dataSize() / sizeof( OBColorPoint );
        OBColorPoint* data = reinterpret_cast<OBColorPoint*>( pointcloud_frame->data() );

        std::vector<Eigen::Vector3d> points = std::vector<Eigen::Vector3d>( num_points );
        std::vector<Eigen::Vector3d> colors = std::vector<Eigen::Vector3d>( num_points );

        #pragma omp parallel for
        for( int32_t i = 0; i < num_points; i++ ){
            points[i] = Eigen::Vector3d( data[i].x, data[i].y, data[i].z);
            colors[i] = Eigen::Vector3d( data[i].r, data[i].g, data[i].b );
        }

        pointcloud->points_ = points;
        pointcloud->colors_ = colors;
    }
    else{
        const int32_t num_points = pointcloud_frame->dataSize() / sizeof( OBPoint );
        OBPoint* data = reinterpret_cast<OBPoint*>( pointcloud_frame->data() );

        std::vector<Eigen::Vector3d> points = std::vector<Eigen::Vector3d>( num_points );

        #pragma omp parallel for
        for( int32_t i = 0; i < num_points; i++ ){
            points[i] = Eigen::Vector3d( data[i].x, data[i].y, data[i].z );
        }

        pointcloud->points_ = points;
    }
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
