/*
 * Licensed under the MIT License (MIT)
 *
 * Copyright (c) 2015 AudioScience Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * stream_configuration.cpp
 *
 */

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

size_t stream_configuration::get_input_maps_count()
{
    return stream_port_input_audio_mappings.size();
}

size_t stream_configuration::get_output_maps_count()
{
    return stream_port_output_audio_mappings.size();
}

int stream_configuration::get_stream_input_details_by_index(unsigned int index, struct stream_configuration_details &stream_details)
{
    if (index >= m_stream_input_count)
        return -1;
    
    stream_details = input_stream_config.at(index);
    return 0;
}

int stream_configuration::get_stream_output_details_by_index(unsigned int index, struct stream_configuration_details &stream_details)
{
    if (index >= m_stream_output_count)
        return -1;
    
    stream_details = output_stream_config.at(index);
    return 0;
}

