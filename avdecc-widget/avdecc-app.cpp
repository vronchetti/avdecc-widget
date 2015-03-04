/*
 * Licensed under the MIT License (MIT)
 *
 * Copyright (c) 2013 AudioScience Inc.
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

#include "avdecc-app.h"
#include "notif_log.h"
#include "../sample.xpm"

class AVDECC_App : public wxApp
{
public:
    virtual bool OnInit() wxOVERRIDE { (new AVDECC_Controller())->Show(); return true; }
};

class Input_Stream_Details
{
public:
    
};

// ----------------------------------------------------------------------------
// event tables and other macros for AVDECC_Controller
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(AVDECC_Controller, wxFrame)

EVT_MENU(HtmlLbox_Quit,  AVDECC_Controller::OnQuit)

EVT_TIMER(EndStationTimer, AVDECC_Controller::OnIncrementTimer)

EVT_LIST_ITEM_ACTIVATED(wxID_ANY, AVDECC_Controller::OnEndStationDClick)

wxEND_EVENT_TABLE()

IMPLEMENT_APP(AVDECC_App)

AVDECC_Controller::AVDECC_Controller()
: wxFrame(NULL, wxID_ANY, wxT("AVDECC-LIB Controller widget"),
          wxDefaultPosition, wxSize(600,300))
{
    netif = avdecc_lib::create_net_interface();
    netif->select_interface_by_num(7);
    controller_obj = avdecc_lib::create_controller(netif, notification_callback, log_callback, log_level);
    sys = avdecc_lib::create_system(avdecc_lib::system::LAYER2_MULTITHREADED_CALLBACK, netif, controller_obj);
    sys->process_start();
    m_end_station_count = 0;
    m_timer = new wxTimer(this, wxID_ANY);
    m_timer->Start(1000, wxTIMER_CONTINUOUS);

    // set the frame icon
    SetIcon(wxICON(sample));
    
#if wxUSE_MENUS
    
    // create a menu bar
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(HtmlLbox_Quit, wxT("E&xit\tAlt-X"), wxT("Quit this program"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(menuFile, wxT("&File"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS
    
#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText(wxT("Welcome to avdecc-lib controller!"));
#endif // wxUSE_STATUSBAR
    
    CreateEndStationListFormat();
    CreateEndStationList();
}

AVDECC_Controller::~AVDECC_Controller()
{
    sys->process_close();
    sys->destroy();
    controller_obj->destroy();
    netif->destroy();
    m_timer->Stop();
    delete wxLog::SetActiveTarget(NULL);
    delete config;
    delete stream_config;
}

void AVDECC_Controller::CreateEndStationList()
{
    sleep(1); //delay to process end stations (will be replaced by timer method)

    for (unsigned int i = 0; i < controller_obj->get_end_station_count(); i++)
    {
        avdecc_lib::end_station *end_station = controller_obj->get_end_station_by_index(i);
        
        if (end_station)
        {
            uint64_t end_station_entity_id = end_station->entity_id();
            avdecc_lib::entity_descriptor_response *ent_desc_resp = NULL;
            if (end_station->entity_desc_count())
            {
                uint16_t current_entity = end_station->get_current_entity_index();
                ent_desc_resp = end_station->get_entity_desc_by_index(current_entity)->get_entity_response();
            }
            const char *end_station_name = "";
            const char *fw_ver = "";
            if (ent_desc_resp)
            {
                end_station_name = (const char *)ent_desc_resp->entity_name();
                fw_ver = (const char *)ent_desc_resp->firmware_version();
            }
            uint64_t end_station_mac = end_station->mac();
            
            wxListItem item;
            item.SetId(i);
            details_list->InsertItem(item);
            details_list->SetItem(i, 0, end_station->get_connection_status());
            details_list->SetItem(i, 1, end_station_name);
            details_list->SetItem(i, 2, wxString::Format("0x%llx", end_station_entity_id));
            details_list->SetItem(i, 3, fw_ver);
            details_list->SetItem(i, 4, wxString::Format("%llx",end_station_mac));
            delete ent_desc_resp;
        }
        m_end_station_count++;
    }
#if wxUSE_STATUSBAR
    SetStatusText(wxString::Format(
                                   wxT("# end stations found = %u"),
                                   m_end_station_count
                                   ));
#endif // wxUSE_STATUSBAR
}


// ----------------------------------------------------------------------------
// menu event handlers
// ----------------------------------------------------------------------------

void AVDECC_Controller::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void AVDECC_Controller::OnEndStationDClick(wxListEvent& event)
{
    avdecc_lib::end_station *end_station = controller_obj->get_end_station_by_index(event.GetIndex());
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_decriptor(end_station, &entity, &configuration);
    avdecc_lib::audio_unit_descriptor *audio_unit_desc = configuration->get_audio_unit_desc_by_index(0);
    avdecc_lib::audio_unit_descriptor_response *audio_unit_resp_ref = audio_unit_desc->get_audio_unit_response();

    uint16_t number_of_stream_input_ports = configuration->stream_input_desc_count();
    uint16_t number_of_stream_output_ports = configuration->stream_output_desc_count();

    wxString entity_id = wxString::Format("0x%llx",end_station->entity_id());
    avdecc_lib::entity_descriptor_response *entity_desc_resp = entity->get_entity_response();
    wxString default_name = entity_desc_resp->entity_name();
    wxString mac = wxString::Format("%llx",end_station->mac());
    wxString fw_ver = (const char *)entity_desc_resp->firmware_version();
    uint32_t init_sample_rate = audio_unit_resp_ref->current_sampling_rate();
    
    config = new end_station_configuration(entity_id, default_name, mac, fw_ver, init_sample_rate);
    stream_config = new stream_configuration(number_of_stream_input_ports, number_of_stream_output_ports);

    delete audio_unit_resp_ref;
    
    for(unsigned int i = 0; i < number_of_stream_input_ports; i++)
    {
        avdecc_lib::stream_input_descriptor *stream_input_desc_ref = configuration->get_stream_input_desc_by_index(i);
        if(stream_input_desc_ref)
        {
            struct stream_configuration_details input_stream_details;
            
            avdecc_lib::stream_input_descriptor_response *stream_input_resp_ref = stream_input_desc_ref->get_stream_input_response();
            const uint8_t * object_name = stream_input_resp_ref->object_name();
            const uint8_t * stream_input_name;
            if(object_name[0] == '\0')
            {
                stream_input_name = configuration->get_strings_desc_string_by_reference(stream_input_resp_ref->localized_description());
            }
            else
            {
                stream_input_name = object_name;
            }
            
            input_stream_details.stream_name = stream_input_name;
            
            const char * current_format = stream_input_resp_ref->current_format();
            uint64_t value = avdecc_lib::utility::ieee1722_format_name_to_value(current_format);
            value = value << 44;
            value = value >> 52;
        
            if(value == 1) //channel count 1
            {
                input_stream_details.channel_count = 1;
            }
            else if(value == 2) //channel count 2
            {
                input_stream_details.channel_count = 2;
            }
            else //channel count 2
            {
                input_stream_details.channel_count = 8;
            }
            stream_config->input_stream_config.push_back(input_stream_details);
            delete stream_input_resp_ref;
        }
    }
    
    for(unsigned int i = 0; i < number_of_stream_output_ports; i++)
    {
        avdecc_lib::stream_output_descriptor *stream_output_desc_ref = configuration->get_stream_output_desc_by_index(i);
        if(stream_output_desc_ref)
        {
            struct stream_configuration_details output_stream_details;

            avdecc_lib::stream_output_descriptor_response *stream_output_resp_ref = stream_output_desc_ref->get_stream_output_response();
            const uint8_t * object_name = stream_output_resp_ref->object_name();
            const uint8_t * stream_output_name;
            if(object_name[0] == '\0')
            {
                stream_output_name = configuration->get_strings_desc_string_by_reference(stream_output_resp_ref->localized_description());
            }
            else
            {
                stream_output_name = object_name;
            }
            
            output_stream_details.stream_name = stream_output_name;

            const char * current_format = stream_output_resp_ref->current_format();
            uint64_t value = avdecc_lib::utility::ieee1722_format_name_to_value(current_format);
            value = value << 44;
            value = value >> 52;
            
            if(value == 1) //channel count 1
            {
                output_stream_details.channel_count = 1;
            }
            else if(value == 2) //channel count 2
            {
                output_stream_details.channel_count = 2;
            }
            else //channel count 8
            {
                output_stream_details.channel_count = 8;
            }
            stream_config->output_stream_config.push_back(output_stream_details);
            delete stream_output_resp_ref;
        }
    }
    
    details = new end_station_details(config, stream_config);
}

int AVDECC_Controller::get_current_entity_and_decriptor(avdecc_lib::end_station *end_station,
                                              avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration)
{
    *entity = NULL;
    *configuration = NULL;
    
    uint16_t current_entity = end_station->get_current_entity_index();
    if (current_entity >= end_station->entity_desc_count())
    {
        atomic_cout << "Current entity not available" << std::endl;
        return 1;
    }
    
    *entity = end_station->get_entity_desc_by_index(current_entity);
    
    uint16_t current_config = end_station->get_current_config_index();
    if (current_config >= (*entity)->config_desc_count())
    {
        atomic_cout << "Current configuration not available" << std::endl;
        return 1;
    }
    
    *configuration = (*entity)->get_config_desc_by_index(current_config);
    
    return 0;
}

void AVDECC_Controller::OnIncrementTimer(wxTimerEvent& event)
{
    if(m_end_station_count < controller_obj->get_end_station_count())
    {
        details_list->DeleteAllItems();
        CreateEndStationList();
    }
}

void AVDECC_Controller::CreateEndStationListFormat()
{
    // create the child controls
    wxNotebook *notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(700,200), wxGROW);
    wxPanel * window1 = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxSize(700, 200), wxGROW);
    
    notebook->AddPage(window1, wxT("End Stations"), true, 0);
    details_list = new wxListCtrl(window1, wxID_ANY, wxDefaultPosition,
                                  wxSize(700,200), wxLC_REPORT);
    
    wxListItem col0;
    col0.SetId(0);
    col0.SetText( _("") );
    col0.SetWidth(25);
    details_list->InsertColumn(0, col0);
    
    wxListItem col1;
    col1.SetId(1);
    col1.SetText( _("Name") );
    col1.SetWidth(100);
    details_list->InsertColumn(1, col1);
    
    // Add second column
    wxListItem col2;
    col2.SetId(2);
    col2.SetText( _("Entity ID") );
    col2.SetWidth(150);
    details_list->InsertColumn(2, col2);
    
    // Add third column
    wxListItem col3;
    col3.SetId(3);
    col3.SetText( _("Firmware Version") );
    col3.SetWidth(150);
    details_list->InsertColumn(3, col3);
    
    // Add fourth column
    wxListItem col4;
    col4.SetId(4);
    col4.SetText( _("MAC") );
    col4.SetWidth(150);
    details_list->InsertColumn(4, col4);
    
    wxSizer *sizer2 = new wxBoxSizer(wxVERTICAL);
    sizer2->Add(notebook, 1, wxGROW);
    
    SetSizer(sizer2);
}
