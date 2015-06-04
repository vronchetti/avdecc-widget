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
#include <assert.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <limits.h>
#include <string.h>
#include <fstream>
#include <inttypes.h>

#include "end_station.h"
#include "controller.h"
#include "system.h"
#include "net_interface.h"
#include "entity_descriptor.h"
#include "configuration_descriptor.h"
#include "audio_unit_descriptor.h"
#include "stream_input_descriptor.h"
#include "stream_output_descriptor.h"
#include "jack_input_descriptor.h"
#include "jack_output_descriptor.h"
#include "avb_interface_descriptor.h"
#include "clock_source_descriptor.h"
#include "memory_object_descriptor.h"
#include "locale_descriptor.h"
#include "strings_descriptor.h"
#include "stream_port_input_descriptor.h"
#include "stream_port_output_descriptor.h"
#include "audio_cluster_descriptor.h"
#include "audio_map_descriptor.h"
#include "clock_domain_descriptor.h"
#include "external_port_input_descriptor.h"
#include "external_port_output_descriptor.h"
#include "descriptor_field.h"
#include "descriptor_field_flags.h"
#include "control_descriptor.h"
#include "enumeration.h"
#include "util.h"


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
    stream_configuration(avdecc_lib::end_station * end_station, avdecc_lib::system *sys);
    virtual ~stream_configuration();

    std::vector<struct stream_configuration_details> avdecc_input_stream_config;
    std::vector<struct stream_configuration_details> dialog_input_stream_config;
    std::vector<struct stream_configuration_details> avdecc_output_stream_config;
    std::vector<struct stream_configuration_details> dialog_output_stream_config;
    
    std::vector <struct audio_mapping> avdecc_stream_port_input_audio_mappings;
    std::vector <struct audio_mapping> dialog_stream_port_input_audio_mappings;
    std::vector <struct audio_mapping> avdecc_stream_port_output_audio_mappings;
    std::vector <struct audio_mapping> dialog_stream_port_output_audio_mappings;

    size_t get_stream_input_count();
    size_t get_stream_output_count();
    size_t get_stream_input_cluster_count();
    size_t get_stream_output_cluster_count();
    void set_input_output_cluster_counts(size_t input_cluster_count, size_t output_cluster_count);
    
    size_t get_avdecc_input_maps_count();
    size_t get_avdecc_output_maps_count();
    size_t get_dialog_input_maps_count();
    size_t get_dialog_output_maps_count();

    int get_avdecc_stream_input_details_by_index(unsigned int index, struct stream_configuration_details &stream_details);
    int get_avdecc_stream_output_details_by_index(unsigned int index, struct stream_configuration_details &stream_details);
    int get_dialog_stream_input_details_by_index(unsigned int index, struct stream_configuration_details &stream_details);
    int get_dialog_stream_output_details_by_index(unsigned int index, struct stream_configuration_details &stream_details);
    
    int SetStreamInfo();
    int SetAudioMappings();

private:
    avdecc_lib::end_station * m_end_station;
    avdecc_lib::system * m_sys;
    intptr_t notification_id;
    
    size_t m_stream_input_count;
    size_t m_stream_output_count;
    size_t m_stream_input_cluster_count = 0;
    size_t m_stream_output_cluster_count = 0;
    unsigned int m_input_maps_count;
    unsigned int m_output_maps_count;
    
    int GetStreamInfo();
    int GetAudioMappings();
    
    uint64_t channel_count_and_sample_rate_to_stream_format(unsigned int channel_count, uint32_t sampling_rate);
    int cmd_set_stream_format(wxString desc_name, uint16_t desc_index, uint64_t stream_format_value);
    int cmd_set_name(std::string desc_name, uint16_t desc_index, std::string new_name);
    int cmd_display_desc_name(avdecc_lib::descriptor_base *desc, uint16_t name_index, bool is_entity);
    int add_audio_mappings(uint16_t desc_type);
    int remove_audio_mappings(uint16_t desc_type);
    
    int get_current_entity_and_descriptor(avdecc_lib::end_station *end_station,
                                          avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration);
    int get_current_end_station_entity_and_descriptor(avdecc_lib::end_station **end_station,
                                                      avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration);
    uint32_t get_next_notification_id();
};

enum
{
    NO_STRING = 65535
};
