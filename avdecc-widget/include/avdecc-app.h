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
 *
 * 
 */

#include "end_station_details.h"

//avdecc-lib necessary headers
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

class AVDECC_Controller : public wxFrame
{
public:
    AVDECC_Controller();
    virtual ~AVDECC_Controller();
    
    // event handlers
    void OnQuit(wxCommandEvent& event);
    
    void OnEndStationDClick(wxListEvent& event);
    void OnIncrementTimer(wxTimerEvent& event);
    
    void CreateEndStationListFormat();
    void CreateEndStationList();
    int get_current_entity_and_descriptor(avdecc_lib::end_station *end_station,
                                         avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration);
    
    int get_current_end_station_entity_and_descriptor(avdecc_lib::end_station **end_station,
                                                      avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration);
    
    int get_current_end_station(avdecc_lib::end_station **end_station) const;
private:
    //main window objects
    wxTextCtrl *notif_text;
    wxTextCtrl *log_text;
    wxListCtrl * details_list;
    wxTimer * m_timer;

    end_station_details * details;
    end_station_configuration * config;
    stream_configuration * stream_config;
    
    //avdecc-lib objects, variables
    avdecc_lib::controller *controller_obj;
    avdecc_lib::system *sys;
    avdecc_lib::net_interface *netif;
    int32_t log_level = avdecc_lib::LOGGING_LEVEL_ERROR;
    intptr_t notification_id;
    unsigned int m_end_station_count;
    long current_end_station_index;
    uint32_t init_sample_rate;
    uint32_t get_next_notification_id();
    
    int cmd_set_sampling_rate(uint32_t new_sampling_rate);
    int cmd_set_stream_format(wxString desc_name, uint16_t desc_index, unsigned int stream_format_index);
    unsigned int channel_count_and_sample_rate_to_stream_index(unsigned int channel_count, uint32_t sampling_rate);
    
    // any class wishing to process wxWidgets events must use this macro
    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    HtmlLbox_CustomBox = 1,
    HtmlLbox_SimpleBox,
    HtmlLbox_Quit,
    
    HtmlLbox_SetMargins,
    HtmlLbox_DrawSeparator,
    HtmlLbox_ToggleMulti,
    HtmlLbox_SelectAll,
    HtmlLbox_UpdateItem,
    HtmlLbox_GetItemRect,
    
    HtmlLbox_SetBgCol,
    HtmlLbox_SetSelBgCol,
    HtmlLbox_SetSelFgCol,
    
    HtmlLbox_Clear,
    EndStationTimer,
    
    
    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    HtmlLbox_About = wxID_ABOUT
};
