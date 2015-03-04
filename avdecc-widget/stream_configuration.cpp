#include <iostream>
#include "stream_configuration.h"


stream_configuration::stream_configuration(unsigned int stream_input_count, unsigned int stream_output_count)
{
    m_stream_input_count = stream_input_count;
    m_stream_output_count = stream_output_count;
}
stream_configuration::~stream_configuration() {}

unsigned int stream_configuration::get_stream_input_count()
{
    return m_stream_input_count;
}

unsigned int stream_configuration::get_stream_output_count()
{
    return m_stream_output_count;
}

int stream_configuration::get_stream_input_name_by_index(unsigned int index, struct stream_configuration_details &stream_details)
{
    if (index >= m_stream_input_count)
        return -1;
    
    stream_details = input_stream_config.at(index);
        return 0;
}

int stream_configuration::get_stream_output_name_by_index(unsigned int index, struct stream_configuration_details &stream_details)
{
    if (index >= m_stream_output_count)
        return -1;
    
    stream_details = output_stream_config.at(index);
    return 0;
}

