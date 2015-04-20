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

#include <stdint.h>

#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/utils.h>

#include "avdecc-app.h"
#include "notif_log.h"
#include "../sample.xpm"

class AVDECC_App : public wxApp
{
public:
    virtual bool OnInit() { (new AVDECC_Controller())->Show(); return true; }
};

// ----------------------------------------------------------------------------
// event table for AVDECC_Controller
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(AVDECC_Controller, wxFrame)
    EVT_TIMER(EndStationTimer, AVDECC_Controller::OnIncrementTimer)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, AVDECC_Controller::OnEndStationDClick)
    EVT_CHOICE(InterfaceSelect, AVDECC_Controller::OnInterfaceSelect)
wxEND_EVENT_TABLE()

IMPLEMENT_APP(AVDECC_App)

AVDECC_Controller::AVDECC_Controller()
: wxFrame(NULL, wxID_ANY, wxT("AVDECC-LIB Controller widget"),
          wxDefaultPosition, wxSize(600,300))
{
	notifs = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(600, 300), wxTE_MULTILINE);
    m_end_station_count = 0;
    avdecc_app_timer = new wxTimer(this, EndStationTimer);
    avdecc_app_timer->Start(200, wxTIMER_CONTINUOUS);
    notification_id = 1;

    netif = avdecc_lib::create_net_interface();
    controller_obj = avdecc_lib::create_controller(netif, notification_callback, log_callback, log_level);
    sys = avdecc_lib::create_system(avdecc_lib::system::LAYER2_MULTITHREADED_CALLBACK, netif, controller_obj);
    PrintAndSelectInterface();
    // set the frame icon
    SetIcon(wxICON(sample));

    CreateEndStationListFormat();
}

AVDECC_Controller::~AVDECC_Controller()
{
    sys->process_close();
    sys->destroy();
    controller_obj->destroy();
    netif->destroy();
    avdecc_app_timer->Stop();
    delete wxLog::SetActiveTarget(NULL);
}

void AVDECC_Controller::CreateEndStationList()
{
	wxMilliSleep(2000);

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
}

void AVDECC_Controller::PrintAndSelectInterface()
{
    char *port = NULL;

    int interface_num = 1;
    wxArrayString str;
    
    for(uint32_t i = 1; i < netif->devs_count() + 1; i++)
    {
        char *dev_desc = netif->get_dev_desc_by_index(i - 1);
        if (!port)
        {
            str.Add(dev_desc);
        }
        else
        {
            if (strcmp(dev_desc, port) == 0)
            {
                interface_num = i;
                break;
            }
        }
    }

    interface_choice = new wxChoice(this, InterfaceSelect, wxDefaultPosition, wxSize(100,25), str);
}

void AVDECC_Controller::OnInterfaceSelect(wxCommandEvent &event)
{
    unsigned int index = interface_choice->GetSelection();
    
    netif->select_interface_by_num(index + 1);
    sys->process_start();

    CreateEndStationList();
}

void AVDECC_Controller::OnEndStationDClick(wxListEvent& event)
{
    avdecc_lib::end_station *end_station = controller_obj->get_end_station_by_index(event.GetIndex());
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(end_station, &entity, &configuration);
    
    current_end_station_index = event.GetIndex();
    
    avdecc_lib::audio_unit_descriptor *audio_unit_desc = configuration->get_audio_unit_desc_by_index(0);
    avdecc_lib::audio_unit_descriptor_response *audio_unit_resp_ref = audio_unit_desc->get_audio_unit_response();
    avdecc_lib::strings_descriptor *strings_desc = configuration->get_strings_desc_by_index(0);
    avdecc_lib::strings_descriptor_response *strings_resp_ref = strings_desc->get_strings_response();

    uint16_t number_of_stream_input_ports = configuration->stream_input_desc_count();
    uint16_t number_of_stream_output_ports = configuration->stream_output_desc_count();
    
    /****************** End Station Details *****************/
    wxString entity_id = wxString::Format("0x%llx",end_station->entity_id());
    avdecc_lib::entity_descriptor_response *entity_desc_resp = entity->get_entity_response();
    wxString entity_name = entity_desc_resp->entity_name();
    wxString default_name = strings_resp_ref->get_string_by_index(1);
    wxString mac = wxString::Format("%llx",end_station->mac());
    wxString fw_ver = (const char *)entity_desc_resp->firmware_version();
    init_sample_rate = audio_unit_resp_ref->current_sampling_rate();
    
    end_station_configuration * config = new end_station_configuration(entity_name, entity_id, default_name, mac, fw_ver, init_sample_rate); //end station details class
    stream_configuration * stream_config = new stream_configuration(number_of_stream_input_ports, number_of_stream_output_ports); //new stream config class

    delete audio_unit_resp_ref;
    delete entity_desc_resp;
    delete strings_resp_ref;
    
    /****************** Stream Input Info *****************/
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
            
            input_stream_details.clk_sync_src_flag = stream_input_resp_ref->stream_flags_clock_sync_source();
            stream_config->input_stream_config.push_back(input_stream_details);

            delete stream_input_resp_ref;
        }
    }
    
    /****************** Stream Output Info *****************/
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
            
            output_stream_details.clk_sync_src_flag = stream_output_resp_ref->stream_flags_clock_sync_source();
            
            stream_config->output_stream_config.push_back(output_stream_details);
            delete stream_output_resp_ref;
        }
    }
    
    /****************** Get Audio mappings *****************/
    intptr_t cmd_notification_id = get_next_notification_id();
    sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    avdecc_lib::stream_port_input_descriptor *stream_port_input_desc_ref = configuration->get_stream_port_input_desc_by_index(0); //index 0 returns all maps
    stream_port_input_desc_ref->send_get_audio_map_cmd((void *)cmd_notification_id, 0);
    int status = sys->get_last_resp_status();
    
    if(status == avdecc_lib::AEM_STATUS_SUCCESS)
    {
        avdecc_lib::stream_port_input_get_audio_map_response *stream_port_input_resp_ref = stream_port_input_desc_ref->
        get_stream_port_input_audio_map_response();
        uint16_t nmappings = stream_port_input_resp_ref->number_of_mappings();
        
        for (int i = 0; i < (int)nmappings; i++)
        {
            struct audio_mapping m_map;
            struct avdecc_lib::stream_port_input_audio_mapping input_map;
            
            int ret = stream_port_input_resp_ref->mapping(i, input_map);
            
            if (ret == 0)
            {
                m_map.stream_channel = input_map.stream_channel;
                m_map.stream_index = input_map.stream_index;
                m_map.cluster_offset = input_map.cluster_offset;
                m_map.cluster_channel = input_map.cluster_channel;
            }
            stream_config->stream_port_input_audio_mappings.push_back(m_map); //add mapping to stream_config class
        }
        delete stream_port_input_resp_ref;
    }
    else
    {
        //cmd get audio_map error
    }
    
    cmd_notification_id = get_next_notification_id();
    sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    avdecc_lib::stream_port_output_descriptor *stream_port_output_desc_ref = configuration->get_stream_port_output_desc_by_index(0); //index 0 returns all maps
    if(stream_port_output_desc_ref)
    {
        stream_port_output_desc_ref->send_get_audio_map_cmd((void *)cmd_notification_id, 0);
        status = sys->get_last_resp_status();
    
        if(status == avdecc_lib::AEM_STATUS_SUCCESS)
        {
            avdecc_lib::stream_port_output_get_audio_map_response *stream_port_output_resp_ref = stream_port_output_desc_ref->
            get_stream_port_output_audio_map_response();
            uint16_t nmappings = stream_port_output_resp_ref->number_of_mappings();
            
            for (int i = 0; i < (int)nmappings; i++)
            {
                struct audio_mapping m_map;
                struct avdecc_lib::stream_port_output_audio_mapping output_map;
                
                int ret = stream_port_output_resp_ref->mapping(i, output_map);
                
                if (ret == 0)
                {
                    m_map.stream_channel = output_map.stream_channel;
                    m_map.stream_index = output_map.stream_index;
                    m_map.cluster_offset = output_map.cluster_offset;
                    m_map.cluster_channel = output_map.cluster_channel;
                }
                stream_config->stream_port_output_audio_mappings.push_back(m_map); //add mapping to stream_config class
            }
            delete stream_port_output_resp_ref;
        }
        else
        {
            //cmd get_audio_map error
        }
    }
    else
    {
        //get_stream_port_output_error
    }

    end_station_details * details = new end_station_details(this, config, stream_config);
    
    int retval = details->ShowModal();
    
    if (retval == wxID_CANCEL)
    {
        details->OnCancel();
        std::cout << "Cancel" << std::endl;
    }
    else if (retval == wxID_OK)
    {
        details->OnOK();
        std::cout << "Apply" << std::endl;

        if(details->m_end_station_config->get_sample_rate() != init_sample_rate)
        {
            cmd_set_sampling_rate(details->m_end_station_config->get_sample_rate());
        }
        else
        {
            //same value
        }
        
        for(unsigned int i = 0; i < details->m_stream_input_count; i++)
        {
            struct stream_configuration_details avdecc_stream_input_details;
            struct stream_configuration_details dialog_stream_input_details;
            
            stream_config->get_stream_input_details_by_index(i, avdecc_stream_input_details);
            details->m_stream_config->get_stream_input_details_by_index(i, dialog_stream_input_details);
            
            if(dialog_stream_input_details.channel_count != avdecc_stream_input_details.channel_count)
            {
                uint64_t stream_index = channel_count_and_sample_rate_to_stream_format(dialog_stream_input_details.channel_count,
                                                                                       details->m_end_station_config->get_sample_rate());
                if(stream_index != -1)
                {
                    cmd_set_stream_format("STREAM_INPUT", i, stream_index);
                }
                else
                {
                    //error, stream_format not found
                }
            }
            else
            {
                //same value
            }
        }
        
        for(unsigned int i = 0; i < details->m_stream_output_count; i++)
        {
            struct stream_configuration_details avdecc_stream_output_details;
            struct stream_configuration_details dialog_stream_output_details;
            
            stream_config->get_stream_output_details_by_index(i, avdecc_stream_output_details);
            details->m_stream_config->get_stream_output_details_by_index(i, dialog_stream_output_details);
            
            if(dialog_stream_output_details.channel_count != avdecc_stream_output_details.channel_count)
            {
                uint64_t stream_index = channel_count_and_sample_rate_to_stream_format(dialog_stream_output_details.channel_count,
                                                                                       details->m_sampling_rate);
                if(stream_index != -1)
                {
                    cmd_set_stream_format("STREAM_OUTPUT", i, stream_index);
                }
                else
                {
                    //error, stream_format not found
                }
            }
            else
            {
                //same value
            }
        }
        details->Destroy();
    }
    else
    {
        //not supported
    }
	delete config;
	delete stream_config;
}

uint64_t AVDECC_Controller::channel_count_and_sample_rate_to_stream_format(unsigned int channel_count, uint32_t sampling_rate)
{
    sampling_rate /= 1000; //extract first 2 digits

    std::ostringstream os;
    os << "IEC..." << sampling_rate << "KHZ_" << channel_count << "CH";
    std::string s = os.str();
    
    return avdecc_lib::utility::ieee1722_format_name_to_value(s.c_str());
}


int AVDECC_Controller::get_current_entity_and_descriptor(avdecc_lib::end_station *end_station,
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

int AVDECC_Controller::get_current_end_station_entity_and_descriptor(avdecc_lib::end_station **end_station,
                                                                     avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration)
{
    if (get_current_end_station(end_station))
        return 1;
    
    if (get_current_entity_and_descriptor(*end_station, entity, configuration))
    {
        atomic_cout << "Current End Station not fully enumerated" << std::endl;
        return 1;
    }
    return 0;
}

int AVDECC_Controller::get_current_end_station(avdecc_lib::end_station **end_station) const
{
    if ((size_t) current_end_station_index >= controller_obj->get_end_station_count())
    {
        atomic_cout << "No End Stations available" << std::endl;
        *end_station = NULL;
        return 1;
    }
    
    *end_station = controller_obj->get_end_station_by_index(current_end_station_index);
    return 0;
}

void AVDECC_Controller::OnIncrementTimer(wxTimerEvent& event)
{
    if(m_end_station_count < controller_obj->get_end_station_count())
    {
        details_list->DeleteAllItems();
        CreateEndStationList();
    }
    else
    {
        //no added end stations found
    }
    
    std::streambuf *sbOld = std::cout.rdbuf();
    std::cout.rdbuf(notifs);
    
    for(size_t i = 0; i < pending_notification_msgs.size(); i++)
    {
        struct notification_info notification;
        
        notification = pending_notification_msgs.at(i);
		if (notification.notification_type == avdecc_lib::COMMAND_TIMEOUT || notification.notification_type == avdecc_lib::RESPONSE_RECEIVED)
		{
			const char *cmd_name;
			const char *desc_name;
			const char *cmd_status_name;

			if (notification.cmd_type < avdecc_lib::CMD_LOOKUP)
			{
				cmd_name = avdecc_lib::utility::aem_cmd_value_to_name(notification.cmd_type);
				desc_name = avdecc_lib::utility::aem_desc_value_to_name(notification.desc_type);
				cmd_status_name = avdecc_lib::utility::aem_cmd_status_value_to_name(notification.cmd_status);
			}
			else
			{
				cmd_name = avdecc_lib::utility::acmp_cmd_value_to_name(notification.cmd_type - avdecc_lib::CMD_LOOKUP);
				desc_name = "NULL";
				cmd_status_name = avdecc_lib::utility::acmp_cmd_status_value_to_name(notification.cmd_status);
			}

			std::cout << "[NOTIFICATION] " <<
				avdecc_lib::utility::notification_value_to_name(notification.notification_type) << " " <<
				wxString::Format("0x%llx", notification.entity_id) << " " <<
				cmd_name << " " <<
				desc_name << " " <<
				notification.desc_index << " " <<
				cmd_status_name << " " <<
				notification.notification_id << std::endl;
		}
		else
		{
			std::cout << "[NOTIFICATION] " <<
				avdecc_lib::utility::notification_value_to_name(notification.notification_type) << " " <<
				wxString::Format("0x%llx", notification.entity_id) << " " <<
				notification.cmd_type << " " <<
				notification.desc_type << " " <<
				notification.desc_index << " " <<
				notification.cmd_status << " " <<
				notification.notification_id << std::endl;
		}

    }
    
    pending_notification_msgs.clear();

    std::cout.rdbuf(sbOld);
}

void AVDECC_Controller::CreateEndStationListFormat()
{
    details_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition,
                                  wxSize(600,200), wxLC_REPORT);
    
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

    wxListItem col2;
    col2.SetId(2);
    col2.SetText( _("Entity ID") );
    col2.SetWidth(150);
    details_list->InsertColumn(2, col2);

    wxListItem col3;
    col3.SetId(3);
    col3.SetText( _("Firmware Version") );
    col3.SetWidth(150);
    details_list->InsertColumn(3, col3);

    wxListItem col4;
    col4.SetId(4);
    col4.SetText( _("MAC") );
    col4.SetWidth(150);
    details_list->InsertColumn(4, col4);
    
    wxBoxSizer * sizer1 = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer *sizer4 = new wxStaticBoxSizer(wxVERTICAL, this, "Messages");
    sizer4->Add(notifs);
    wxStaticBoxSizer *sizer3 = new wxStaticBoxSizer(wxVERTICAL, this, "Select Interface");
    sizer3->Add(interface_choice);
    wxStaticBoxSizer *sizer2 = new wxStaticBoxSizer(wxVERTICAL, this, "End Station List");
    sizer2->Add(details_list, 1, wxGROW);
    sizer1->Add(sizer3);
    sizer1->Add(sizer2);
    sizer1->Add(sizer4);

    SetSizer(sizer1);
}

uint32_t AVDECC_Controller::get_next_notification_id()
{
    return (uint32_t)notification_id++;
}

int AVDECC_Controller::cmd_set_sampling_rate(uint32_t new_sampling_rate)
{
    avdecc_lib::end_station *end_station = controller_obj->get_end_station_by_index(current_end_station_index);
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(end_station, &entity, &configuration);

    intptr_t cmd_notification_id = get_next_notification_id();
    sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    avdecc_lib::audio_unit_descriptor *audio_unit_desc_ref = configuration->get_audio_unit_desc_by_index(0);
    audio_unit_desc_ref->send_set_sampling_rate_cmd((void *)cmd_notification_id, new_sampling_rate);
    int status = sys->get_last_resp_status();
    
    if(status == avdecc_lib::AEM_STATUS_SUCCESS)
    {
        avdecc_lib::audio_unit_descriptor_response *audio_unit_resp_ref = audio_unit_desc_ref->get_audio_unit_response();
        std::cout << "Sampling rate: " << std::dec << audio_unit_resp_ref->current_sampling_rate();
        delete audio_unit_resp_ref;
    }

    return 0;
}

int AVDECC_Controller::cmd_set_stream_format(wxString desc_name, uint16_t desc_index, uint64_t stream_format_value)
{
    std::string stream_format;

    uint16_t desc_type_value = avdecc_lib::utility::aem_desc_name_to_value(desc_name.c_str());
    avdecc_lib::end_station *end_station;
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    if (get_current_end_station_entity_and_descriptor(&end_station, &entity, &configuration))
        return 0;
    
    if(desc_type_value == avdecc_lib::AEM_DESC_STREAM_INPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_input_descriptor *stream_input_desc_ref = configuration->get_stream_input_desc_by_index(desc_index);
        stream_input_desc_ref->send_set_stream_format_cmd((void *)cmd_notification_id, stream_format_value);
        int status = sys->get_last_resp_status();
        
        if(status == avdecc_lib::AEM_STATUS_SUCCESS)
        {
            avdecc_lib::stream_input_descriptor_response *stream_input_resp_ref = stream_input_desc_ref->get_stream_input_response();
            stream_format = stream_input_resp_ref->current_format();
            if(stream_format == "UNKNOWN")
            {
                atomic_cout << "Stream format: 0x" << std::hex << stream_input_resp_ref->current_format() << std::endl;
            }
            else
            {
                atomic_cout << "Stream format: " << stream_format << std::endl;
            }
            delete stream_input_resp_ref;
        }
    }
    else if(desc_type_value == avdecc_lib::AEM_DESC_STREAM_OUTPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_output_descriptor *stream_output_desc_ref = configuration->get_stream_output_desc_by_index(desc_index);
        stream_output_desc_ref->send_set_stream_format_cmd((void *)cmd_notification_id, stream_format_value);
        int status = sys->get_last_resp_status();
        
        if(status == avdecc_lib::AEM_STATUS_SUCCESS)
        {
            avdecc_lib::stream_output_descriptor_response *stream_output_resp_ref = stream_output_desc_ref->get_stream_output_response();
            stream_format = stream_output_resp_ref->current_format();
            if(stream_format == "UNKNOWN")
            {
                atomic_cout << "Stream format: 0x" << std::hex << stream_output_resp_ref->current_format() << std::endl;
            }
            else
            {
                atomic_cout << "Stream format: " << stream_format << std::endl;
            }
            delete stream_output_resp_ref;
        }
    }
    else
    {
        atomic_cout << "cmd_set_stream_format error" << std::endl;
    }
    
    return 0;
}
