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
 * avdecc-app.h
 */

#include "end_station_details.h"

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

class AtomicOut : public std::ostream
{
public:
	AtomicOut() : std::ostream(0), buffer()
	{
		this->init(buffer.rdbuf());
	}

	~AtomicOut()
	{
		// Use printf as cout seems to still be interleaved
		printf("%s", buffer.str().c_str());
	}

private:
	std::ostringstream buffer;
};

#define atomic_cout AtomicOut()

class AVDECC_Controller : public wxFrame
{
public:
    AVDECC_Controller();
    virtual ~AVDECC_Controller();
    
    //avdecc-app event handlers
    void OnEndStationDClick(wxListEvent& event);
    void OnInterfaceSelect(wxCommandEvent& event);
    void OnIncrementTimer(wxTimerEvent& event);

private:
    //avdecc-lib objects
    avdecc_lib::controller *controller_obj;
    avdecc_lib::system *sys;
    avdecc_lib::net_interface *netif;
    
    //avdecc-app controls
    wxListCtrl * details_list;
    wxTimer * avdecc_app_timer;
    wxChoice * interface_choice;
    wxTextCtrl * logs_notifs;
    
    wxBoxSizer * app_sizer;
    
    //child class objects
    end_station_configuration * config;
    stream_configuration * stream_config;
    end_station_details * details;
    
    //avdecc-app methods
    void SetTimer();
    void CreateController();
    void CreateLogging();
    uint64_t channel_count_and_sample_rate_to_stream_format(unsigned int channel_count, uint32_t sampling_rate);
    void PrintAndSelectInterface();
    void CreateEndStationListFormat();
    void CreateEndStationList();

    //avdecc-lib variables
    int32_t log_level = avdecc_lib::LOGGING_LEVEL_ERROR;
    intptr_t notification_id;
    size_t m_end_station_count;
    long current_end_station_index;
    
    //avdecc-lib methods
    uint32_t get_next_notification_id();
    int get_current_entity_and_descriptor(avdecc_lib::end_station *end_station,
                                          avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration);

    wxDECLARE_EVENT_TABLE();
};

enum
{
    AVDECC_GUI_Quit,
    EndStationTimer,
    InterfaceSelect,
    TIMER_INCREMENT = 200, //200 ms
    END_STATION_PROCESS_DELAY = 2000,
};
