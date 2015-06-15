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
 * end_station_configuration.h
 *
 */

#include <wx/string.h>
//avdecc-lib necessary headers
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <vector>
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

class end_station_configuration
{
public:
    end_station_configuration(avdecc_lib::end_station * end_station, avdecc_lib::system *sys);
    virtual ~end_station_configuration();
    
    std::vector<wxString> clock_source_descriptions;
    
	wxString get_entity_name(){ return name; }
	wxString get_entity_id(){ return entity_id; }
	wxString get_default_name(){ return default_name; }
	wxString get_mac(){ return mac; }
	wxString get_fw_ver(){ return fw_ver; }
	uint32_t get_sample_rate(){ return sample_rate; }
	uint16_t get_clock_source(){ return current_clock_source; }
    int set_sample_rate(uint32_t sampling_rate);
    int set_clock_source(uint16_t clock_source_index);
    int set_entity_name(wxString entity_name);
    
    int SetEntityName();
    int SetSamplingRate();
    int SetClockSource();

private:
    avdecc_lib::end_station * m_end_station;
    avdecc_lib::system * m_sys;
    intptr_t notification_id;
    
    wxString name;
    wxString dialog_name;
    wxString entity_id;
    wxString default_name;
    wxString mac;
    wxString fw_ver;
    uint32_t sample_rate;
    uint32_t dialog_sample_rate;
    uint16_t current_clock_source;
    uint16_t dialog_clock_source;
    
    int GetEndStationDetails();
    int cmd_set_name(std::string desc_name, uint16_t desc_index, std::string new_name);
    int cmd_set_sampling_rate(uint32_t new_sampling_rate);
    int cmd_set_clock_source(uint16_t new_clk_src_index);
    int cmd_display_desc_name(avdecc_lib::descriptor_base *desc, uint16_t name_index, bool is_entity);
    int get_current_entity_and_descriptor(avdecc_lib::end_station *end_station,
                                          avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration);
    int get_current_end_station_entity_and_descriptor(avdecc_lib::end_station **end_station,
                                                      avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration);
    uint32_t get_next_notification_id();
};