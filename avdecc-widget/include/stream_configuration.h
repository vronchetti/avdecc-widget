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
 * stream_configuration_details.h
 *
 */

#include <stdint.h>
#include <iostream>
#include <vector>
#include <wx/string.h>

struct stream_configuration_details {
    wxString stream_name;
    unsigned int channel_count;
    bool clk_sync_src_flag;
};

struct audio_mapping {
    uint16_t stream_index;
    uint16_t stream_channel;
    uint16_t cluster_offset;
    uint16_t cluster_channel;
};

class stream_configuration
{
public:
    stream_configuration(size_t stream_input_count, size_t stream_output_count);
    virtual ~stream_configuration();

    std::vector<struct stream_configuration_details> input_stream_config;
    std::vector<struct stream_configuration_details> output_stream_config;
    
    std::vector <struct audio_mapping> stream_port_input_audio_mappings;
    std::vector <struct audio_mapping> stream_port_output_audio_mappings;

    size_t get_stream_input_count();
    size_t get_stream_output_count();
    size_t get_input_maps_count();
    size_t get_output_maps_count();

    int get_stream_input_details_by_index(unsigned int index, struct stream_configuration_details &stream_details);
    int get_stream_output_details_by_index(unsigned int index, struct stream_configuration_details &stream_details);

private:
    size_t m_stream_input_count;
    size_t m_stream_output_count;
    unsigned int m_input_maps_count;
    unsigned int m_output_maps_count;
};
