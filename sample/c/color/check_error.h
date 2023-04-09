#ifndef __CHECK_ERROR__
#define __CHECK_ERROR__

#include <sstream>
#include <stdexcept>

#include <libobsensor/ObSensor.h>

#define CHECK_ERROR( error )                                                                                              \
    if( ob_error_status( error ) != ob_status::OB_STATUS_OK ){                                                            \
        std::stringstream error_message;                                                                                  \
        const std::string exception_type = to_string( ob_error_exception_type( error ) );                                 \
        const std::string message = ob_error_message( error );                                                            \
        const std::string function = ob_error_function( error );                                                          \
        const std::string args =  ob_error_args( error );                                                                 \
        error_message << "[error] " << exception_type << " : " << message << " in " << function << " ( " << args << " )"; \
        throw std::runtime_error( error_message.str() );                                                                  \
    }                                                                                                                     \

std::string to_string( const ob_exception_type exception_type )
{
    switch( exception_type ){
        case OB_EXCEPTION_TYPE_UNKNOWN:
            return "unknown";
        case OB_EXCEPTION_TYPE_CAMERA_DISCONNECTED:
            return "camera disconnected";
        case OB_EXCEPTION_TYPE_PLATFORM:
            return "platform";
        case OB_EXCEPTION_TYPE_INVALID_VALUE:
            return "invalid value";
        case OB_EXCEPTION_TYPE_WRONG_API_CALL_SEQUENCE:
            return "api call sequence";
        case OB_EXCEPTION_TYPE_NOT_IMPLEMENTED:
            return "not implemented";
        case OB_EXCEPTION_TYPE_IO:
            return "io";
        case OB_EXCEPTION_TYPE_MEMORY:
            return "memory";
        case OB_EXCEPTION_TYPE_UNSUPPORTED_OPERATION:
            return "unsupported operation";
        default:
            throw std::runtime_error( "[error] failed to convert unknown exception type to string!" );
    }
}

#endif // __CHECK_ERROR__
