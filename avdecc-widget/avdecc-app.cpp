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
    std::streambuf *sbOld = std::cout.rdbuf();
    std::cout.rdbuf(notifs);

    int get_end_station_details_ret = GetEndStationDetails();
    if(get_end_station_details_ret)
    {
        std::cout << "Get End Station Details Error" << std::endl;
    }

    config = new end_station_configuration(init_entity_name, entity_id, default_name,
                                           mac, fw_ver, init_sample_rate,
                                           init_clock_source, clock_source_count); //end station details class

    int get_clock_source_ret = GetClockSource();
    if(get_clock_source_ret)
    {
        std::cout << "Get Clock Source Error" << std::endl;
    }

    stream_config = new stream_configuration(configuration->stream_input_desc_count(),
                                             configuration->stream_output_desc_count()); //new stream config class
    int get_stream_info_ret = GetStreamInfo();
    if(get_stream_info_ret)
    {
        std::cout << "Get Stream Info Error" << std::endl;
    }
    
    int get_audio_mappings_ret = GetAudioMappings();
    if(get_audio_mappings_ret)
    {
        std::cout << "Get Audio Mappings Eror" << std::endl;
    }

    details = new end_station_details(this, config, stream_config);
    
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
        
        int set_entity_name_ret = SetEntityName();
        if(set_entity_name_ret)
        {
            std::cout << "Set Entity Name Error" << std::endl;
        }
        
        int set_sampling_rate_ret = SetSamplingRate();
        if(set_sampling_rate_ret)
        {
            std::cout << "Set Sampling Rate Error" << std::endl;
        }
        
        int set_clock_source_ret = SetClockSource();
        if(set_clock_source_ret)
        {
            std::cout << "Set Clock Source Error" << std::endl;
        }
        
        int set_stream_format_ret = SetStreamFormatAndName();
        if(set_stream_format_ret)
        {
            std::cout << "Set Stream Format Error" << std::endl;
        }
        
        int set_audio_mappings_ret = SetAudioMappings();
        if(set_audio_mappings_ret)
        {
            std::cout << "Set Audio Mappings Error" << std::endl;
        }
    }
    else
    {
        //not supported
    }

    std::cout.rdbuf(sbOld);
	delete config;
	delete stream_config;
}

int AVDECC_Controller::GetEndStationDetails()
{
    avdecc_lib::end_station *end_station = controller_obj->get_end_station_by_index(current_end_station_index);
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(end_station, &entity, &configuration);

    entity_id = wxString::Format("0x%llx",end_station->entity_id());
    mac = wxString::Format("%llx",end_station->mac());

    if(entity)
    {
        avdecc_lib::entity_descriptor_response *entity_desc_resp = entity->get_entity_response();
        init_entity_name = entity_desc_resp->entity_name();
        fw_ver = (const char *)entity_desc_resp->firmware_version();
        delete entity_desc_resp;
    }
    else
    {
        return 1;
    }

    avdecc_lib::strings_descriptor *strings_desc = configuration->get_strings_desc_by_index(0);
    if(strings_desc)
    {
        avdecc_lib::strings_descriptor_response *strings_resp_ref = strings_desc->get_strings_response();
        default_name = strings_resp_ref->get_string_by_index(1);
        delete strings_resp_ref;
    }
    else
    {
        return 1;
    }

    avdecc_lib::audio_unit_descriptor *audio_unit_desc = configuration->get_audio_unit_desc_by_index(0);
    if(audio_unit_desc)
    {
        avdecc_lib::audio_unit_descriptor_response *audio_unit_resp_ref = audio_unit_desc->get_audio_unit_response();
        init_sample_rate = audio_unit_resp_ref->current_sampling_rate();
        delete audio_unit_resp_ref;
    }
    else
    {
        return 1;
    }
    
    avdecc_lib::clock_domain_descriptor *clk_domain_desc = configuration->get_clock_domain_desc_by_index(0);
    if(clk_domain_desc)
    {
        avdecc_lib::clock_domain_descriptor_response *clk_domain_resp_ref = clk_domain_desc->get_clock_domain_response();
        init_clock_source = clk_domain_resp_ref->get_clock_source_by_index(clk_domain_resp_ref->clock_source_index());
        clock_source_count = clk_domain_resp_ref->clock_sources_count();
        delete clk_domain_resp_ref;
    }
    else
    {
        return 1;
    }

    return 0;
}

int AVDECC_Controller::GetClockSource()
{
    avdecc_lib::end_station *end_station = controller_obj->get_end_station_by_index(current_end_station_index);
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(end_station, &entity, &configuration);
    
    for(int i = 0; i < (int) clock_source_count; i++)
    {
        avdecc_lib::clock_source_descriptor *clk_src_desc = configuration->get_clock_source_desc_by_index(i);
        if(clk_src_desc)
        {
            avdecc_lib::clock_source_descriptor_response *clk_src_resp_ref = clk_src_desc->get_clock_source_response();
            
            wxString clock_source_description = configuration->get_strings_desc_string_by_reference(clk_src_resp_ref->localized_description());
            config->clock_source_descriptions.push_back(clock_source_description);
            delete clk_src_resp_ref;
        }
        else
        {
            std::cout << "get clock_source desc error" << std::endl;
            return 1;
        }
    }
    return 0;
}

int AVDECC_Controller::add_audio_mappings(uint16_t desc_type)
{
    uint16_t desc_index = 0; //1 stream_port?
    avdecc_lib::end_station *end_station;
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    if (get_current_end_station_entity_and_descriptor(&end_station, &entity, &configuration))
        return 0;
    
    if(desc_type == avdecc_lib::AEM_DESC_STREAM_PORT_INPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_port_input_descriptor *stream_port_input_desc_ref= configuration->get_stream_port_input_desc_by_index(desc_index);
        stream_port_input_desc_ref->send_add_audio_mappings_cmd((void *)cmd_notification_id);
        sys->get_last_resp_status();
    }
    else if(desc_type == avdecc_lib::AEM_DESC_STREAM_PORT_OUTPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_port_output_descriptor *stream_port_output_desc_ref= configuration->get_stream_port_output_desc_by_index(desc_index);
        stream_port_output_desc_ref->send_add_audio_mappings_cmd((void *)cmd_notification_id);
        sys->get_last_resp_status();
    }
    else
    {
        atomic_cout << "Invalid Descriptor" << std::endl;
    }
    
    return 0;
}

int AVDECC_Controller::remove_audio_mappings(uint16_t desc_type)
{
    avdecc_lib::end_station *end_station;
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    if (get_current_end_station_entity_and_descriptor(&end_station, &entity, &configuration))
        return 0;
    
    if(desc_type == avdecc_lib::AEM_DESC_STREAM_PORT_INPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_port_input_descriptor *stream_port_input_desc_ref= configuration->get_stream_port_input_desc_by_index(0);
        stream_port_input_desc_ref->send_remove_audio_mappings_cmd((void *)cmd_notification_id);
        sys->get_last_resp_status();
    }
    else if(desc_type == avdecc_lib::AEM_DESC_STREAM_PORT_OUTPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_port_output_descriptor *stream_port_output_desc_ref= configuration->get_stream_port_output_desc_by_index(0);
        stream_port_output_desc_ref->send_remove_audio_mappings_cmd((void *)cmd_notification_id);
        sys->get_last_resp_status();
    }
    else
    {
        atomic_cout << "Invalid Descriptor" << std::endl;
    }
    
    return 0;
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

int AVDECC_Controller::GetAudioMappings()
{
    avdecc_lib::end_station *end_station = controller_obj->get_end_station_by_index(current_end_station_index);
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(end_station, &entity, &configuration);

    intptr_t cmd_notification_id = get_next_notification_id();
    sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    avdecc_lib::stream_port_input_descriptor *stream_port_input_desc_ref = configuration->get_stream_port_input_desc_by_index(0); //index 0 returns all maps
    if(stream_port_input_desc_ref)
    {
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
            std::cout << "get audio map error" << std::endl;
            return 1;
        }
    }
    else
    {
        std::cout << "get stream port input error" << std::endl;
        return 1;
    }
    
    cmd_notification_id = get_next_notification_id();
    sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    avdecc_lib::stream_port_output_descriptor *stream_port_output_desc_ref = configuration->get_stream_port_output_desc_by_index(0); //index 0 returns all maps
    if(stream_port_output_desc_ref)
    {
        stream_port_output_desc_ref->send_get_audio_map_cmd((void *)cmd_notification_id, 0);
        int status = sys->get_last_resp_status();
        
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
            std::cout << "get audio map error" << std::endl;
            return 1;
        }
    }
    else
    {
        std::cout << "get stream port output error" << std::endl;
        return 1;
    }
    return 0;
}

int AVDECC_Controller::SetSamplingRate()
{
    if(details->m_end_station_config->get_sample_rate() != init_sample_rate)
    {
        cmd_set_sampling_rate(details->m_end_station_config->get_sample_rate());
    }
    else
    {
        std::cout << "sampling rate unchanged" << std::endl;
        return 1;
    }
    return 0;
}

int AVDECC_Controller::SetEntityName()
{
    if(details->m_end_station_config->get_entity_name().IsSameAs(init_entity_name))
    {
        std::cout << "entity name unchanged" << std::endl;
        return 1;
    }
    else
    {
        cmd_set_name("ENTITY", 0, std::string(details->m_end_station_config->get_entity_name()));
    }
    return 0;
}

int AVDECC_Controller::SetClockSource()
{
    if(details->m_end_station_config->get_clock_source() != init_clock_source)
    {
        cmd_set_clock_source(details->m_end_station_config->get_clock_source());
    }
    else
    {
        std::cout << "clock source unchanged" << std::endl;
        return 1;
    }
    
    return 0;
}

int AVDECC_Controller::SetStreamFormatAndName()
{
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
                return 1;
            }
        }
        if(dialog_stream_input_details.stream_name.IsSameAs(avdecc_stream_input_details.stream_name))
        {
            //same
        }
        else
        {
            cmd_set_name("STREAM_INPUT", i, std::string(dialog_stream_input_details.stream_name));
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
                return 1;
            }
        }

        if(dialog_stream_output_details.stream_name.IsSameAs(avdecc_stream_output_details.stream_name))
        {
            //same
        }
        else
        {
            cmd_set_name("STREAM_INPUT", i, std::string(dialog_stream_output_details.stream_name.mb_str()));
        }
    }
    return 0;
}

int AVDECC_Controller::SetAudioMappings()
{
    avdecc_lib::end_station *end_station = controller_obj->get_end_station_by_index(current_end_station_index);
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(end_station, &entity, &configuration);
    
    avdecc_lib::stream_port_input_descriptor *stream_port_input_desc_ref = configuration->get_stream_port_input_desc_by_index(0); //index 0 returns all maps
    avdecc_lib::stream_port_output_descriptor *stream_port_output_desc_ref = configuration->get_stream_port_output_desc_by_index(0); //index 0 returns all maps
    if(stream_port_input_desc_ref && stream_port_output_desc_ref)
    {
        bool input_mapping_found = false;
        
        for(size_t it = 0; it < details->m_stream_config->stream_port_input_audio_mappings.size(); it++)
        {
            struct audio_mapping dialog_mapping;
            
            dialog_mapping = details->m_stream_config->stream_port_input_audio_mappings.at(it);
            
            for(size_t j = 0; j < stream_config->stream_port_input_audio_mappings.size(); j++)
            {
                struct audio_mapping avdecc_mapping;
                
                avdecc_mapping = stream_config->stream_port_input_audio_mappings.at(j);
                
                if(dialog_mapping.stream_index == avdecc_mapping.stream_index &&
                   dialog_mapping.stream_channel == avdecc_mapping.stream_channel &&
                   dialog_mapping.cluster_offset == avdecc_mapping.cluster_offset &&
                   dialog_mapping.cluster_channel == avdecc_mapping.cluster_channel)
                {
                    input_mapping_found = true;
                }
            }
            
            if(!input_mapping_found) //add
            {
                struct avdecc_lib::audio_map_mapping input_audio_map;
                
                input_audio_map.stream_channel = dialog_mapping.stream_channel;
                input_audio_map.stream_index = dialog_mapping.stream_index;
                input_audio_map.cluster_offset = dialog_mapping.cluster_offset;
                input_audio_map.cluster_channel = dialog_mapping.cluster_channel;
                
                stream_port_input_desc_ref->store_pending_map(input_audio_map);
            }
        }
        
        bool output_mapping_found = false;
        
        for(size_t it = 0; it < details->m_stream_config->stream_port_output_audio_mappings.size(); it++)
        {
            struct audio_mapping dialog_mapping;
            
            dialog_mapping = details->m_stream_config->stream_port_output_audio_mappings.at(it);
            
            for(size_t j = 0; j < stream_config->stream_port_output_audio_mappings.size(); j++)
            {
                struct audio_mapping avdecc_mapping;
                
                avdecc_mapping = stream_config->stream_port_output_audio_mappings.at(j);
                
                if(dialog_mapping.stream_index == avdecc_mapping.stream_index &&
                   dialog_mapping.stream_channel == avdecc_mapping.stream_channel &&
                   dialog_mapping.cluster_offset == avdecc_mapping.cluster_offset &&
                   dialog_mapping.cluster_channel == avdecc_mapping.cluster_channel)
                {
                    output_mapping_found = true;
                }
            }
            
            if(!output_mapping_found) //add
            {
                struct avdecc_lib::audio_map_mapping output_audio_map;
                
                output_audio_map.stream_channel = dialog_mapping.stream_channel;
                output_audio_map.stream_index = dialog_mapping.stream_index;
                output_audio_map.cluster_offset = dialog_mapping.cluster_offset;
                output_audio_map.cluster_channel = dialog_mapping.cluster_channel;
                
                stream_port_output_desc_ref->store_pending_map(output_audio_map);
            }
        }
        
        if(!input_mapping_found)
        {
            add_audio_mappings(avdecc_lib::AEM_DESC_STREAM_PORT_INPUT);
        }
        else
        {
            std::cout << "No added input mappings" << std::endl;
        }
        
        if(!output_mapping_found)
        {
            add_audio_mappings(avdecc_lib::AEM_DESC_STREAM_PORT_OUTPUT);
        }
        else
        {
            std::cout << "No added output mappings" << std::endl;
        }
        
        bool input_mapping_not_found = false;
        
        for(size_t it = 0; it < stream_config->stream_port_input_audio_mappings.size(); it++)
        {
            struct audio_mapping avdecc_mapping;
            
            avdecc_mapping = stream_config->stream_port_input_audio_mappings.at(it);
            
            for(size_t j = 0; j < details->m_stream_config->stream_port_input_audio_mappings.size(); j++)
            {
                struct audio_mapping dialog_mapping;
                
                dialog_mapping = details->m_stream_config->stream_port_input_audio_mappings.at(j);
                
                if(dialog_mapping.stream_index == avdecc_mapping.stream_index &&
                   dialog_mapping.stream_channel == avdecc_mapping.stream_channel &&
                   dialog_mapping.cluster_offset == avdecc_mapping.cluster_offset &&
                   dialog_mapping.cluster_channel == avdecc_mapping.cluster_channel)
                {
                    input_mapping_not_found = true;
                }
            }
            
            if(!input_mapping_not_found) //remove
            {
                struct avdecc_lib::audio_map_mapping input_audio_map;
                
                input_audio_map.stream_channel = avdecc_mapping.stream_channel;
                input_audio_map.stream_index = avdecc_mapping.stream_index;
                input_audio_map.cluster_offset = avdecc_mapping.cluster_offset;
                input_audio_map.cluster_channel = avdecc_mapping.cluster_channel;
                
                stream_port_input_desc_ref->store_pending_map(input_audio_map);
            }
        }
        
        bool output_mapping_not_found = false;
        
        for(size_t it = 0; it < stream_config->stream_port_output_audio_mappings.size(); it++)
        {
            struct audio_mapping avdecc_mapping;
            
            avdecc_mapping = stream_config->stream_port_output_audio_mappings.at(it);
            
            for(size_t j = 0; j < details->m_stream_config->stream_port_output_audio_mappings.size(); j++)
            {
                struct audio_mapping dialog_mapping;
                
                dialog_mapping = details->m_stream_config->stream_port_output_audio_mappings.at(j);
                
                if(dialog_mapping.stream_index == avdecc_mapping.stream_index &&
                   dialog_mapping.stream_channel == avdecc_mapping.stream_channel &&
                   dialog_mapping.cluster_offset == avdecc_mapping.cluster_offset &&
                   dialog_mapping.cluster_channel == avdecc_mapping.cluster_channel)
                {
                    output_mapping_not_found = true;
                }
            }
            
            if(!output_mapping_not_found) //remove
            {
                struct avdecc_lib::audio_map_mapping output_audio_map;
                
                output_audio_map.stream_channel = avdecc_mapping.stream_channel;
                output_audio_map.stream_index = avdecc_mapping.stream_index;
                output_audio_map.cluster_offset = avdecc_mapping.cluster_offset;
                output_audio_map.cluster_channel = avdecc_mapping.cluster_channel;
                
                stream_port_output_desc_ref->store_pending_map(output_audio_map);
            }
        }
        
        if(!input_mapping_not_found)
        {
            remove_audio_mappings(avdecc_lib::AEM_DESC_STREAM_PORT_INPUT);
        }
        else
        {
            std::cout << "No removed input mappings" << std::endl;
        }
        
        if(!output_mapping_not_found)
        {
            remove_audio_mappings(avdecc_lib::AEM_DESC_STREAM_PORT_OUTPUT);
        }
        else
        {
            std::cout << "No removed output mappings" << std::endl;
        }
    }
    else
    {
        std::cout << "get stream port error" << std::endl;
    }
    return 0;
}

int AVDECC_Controller::GetStreamInfo()
{
    avdecc_lib::end_station *end_station = controller_obj->get_end_station_by_index(current_end_station_index);
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(end_station, &entity, &configuration);
    
    uint16_t number_of_stream_input_ports = configuration->stream_input_desc_count();
    uint16_t number_of_stream_output_ports = configuration->stream_output_desc_count();

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
        else
        {
            std::cout << "get stream_input error" << std::endl;
            return 1;
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
        else
        {
            std::cout << "get stream_output error" << std::endl;
            return 1;
        }
    }
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

int AVDECC_Controller::cmd_set_clock_source(uint16_t new_clk_src_index)
{
    avdecc_lib::end_station *end_station;
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    if (get_current_end_station_entity_and_descriptor(&end_station, &entity, &configuration))
        return 0;
    
    intptr_t cmd_notification_id = get_next_notification_id();
    sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    avdecc_lib::clock_domain_descriptor *clk_domain_desc_ref = configuration->get_clock_domain_desc_by_index(0);
    if (!clk_domain_desc_ref)
        return 0;
    
    clk_domain_desc_ref->send_set_clock_source_cmd((void *)cmd_notification_id, new_clk_src_index);
    int status = sys->get_last_resp_status();
    
    if(status == avdecc_lib::AEM_STATUS_SUCCESS)
    {
        avdecc_lib::clock_domain_descriptor_response *clk_domain_resp_ref = clk_domain_desc_ref->get_clock_domain_response();
        atomic_cout << "Set clock source index : " << std::dec << clk_domain_resp_ref->clock_source_index() << std::endl;
        delete clk_domain_resp_ref;
    }
    
    return 0;
}

int AVDECC_Controller::cmd_set_name(std::string desc_name, uint16_t desc_index, std::string new_name)
{
    int status = avdecc_lib::AEM_STATUS_NOT_IMPLEMENTED;
    bool is_entity = false;
    
    uint16_t desc_type = avdecc_lib::utility::aem_desc_name_to_value(desc_name.c_str());
    
    avdecc_lib::end_station *end_station;
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    avdecc_lib::descriptor_base *desc_base = NULL;
    
    if (get_current_end_station_entity_and_descriptor(&end_station, &entity,
                                                      &configuration))
        return 0;
    
    if(desc_type == avdecc_lib::AEM_DESC_ENTITY)
    {
        desc_base = entity;
        is_entity = true;
    }
    else if(desc_type == avdecc_lib::AEM_DESC_CONFIGURATION)
    {
        desc_base = dynamic_cast<avdecc_lib::descriptor_base *>(configuration);
    }
    else
    {
        desc_base = configuration->lookup_desc(desc_type, desc_index);
    }
    
    if(!desc_base)
    {
        atomic_cout << "cmd_set_name cannot lookup descriptor" << std::endl;
        return 0;
    }
    
    uint16_t name_index = 0;
    struct avdecc_lib::avdecc_lib_name_string64 new_name64 = {{0}};
    strncpy((char *)new_name64.value, new_name.c_str(),
            sizeof(new_name64.value));
    
    intptr_t cmd_notification_id = get_next_notification_id();
    sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    desc_base->send_set_name_cmd((void *)cmd_notification_id, name_index, 0,
                                 + &new_name64);
    status = sys->get_last_resp_status();
    
    if(status == avdecc_lib::AEM_STATUS_SUCCESS)
    {
        cmd_display_desc_name(desc_base, name_index, is_entity);
    }
    else
    {
        atomic_cout << "cmd_set_name failed with AEM status: " <<
        avdecc_lib::utility::aem_cmd_status_value_to_name(status) <<
        std::endl;
    }
    
    return 0;
}

int AVDECC_Controller::cmd_display_desc_name(avdecc_lib::descriptor_base *desc, uint16_t name_index, bool is_entity)
{
    uint8_t * name;
    avdecc_lib::end_station *end_station;
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    
    if (get_current_end_station_entity_and_descriptor(&end_station, &entity,
                                                      &configuration))
        return 0;
    
    if(is_entity == true)
    {
        avdecc_lib::entity_descriptor_response *entity_resp_ref = entity->get_entity_response();
        if(name_index == 0)
        {
            name = entity_resp_ref->entity_name();
        }
        else
        {
            name = entity_resp_ref->group_name();
        }
        delete entity_resp_ref;
    }
    else
    {
        avdecc_lib::descriptor_response_base *desc_resp_base = desc->get_descriptor_response();
        name = desc_resp_base->object_name();
        delete desc_resp_base;
    }
    
    if(!name)
    {
        atomic_cout << "cmd_set_name() failed" << std::endl;
    }
    else
    {
        atomic_cout << "Descriptor " <<
        avdecc_lib::utility::aem_desc_value_to_name(desc->descriptor_type()) <<
        "." << desc->descriptor_index() <<
        " name at index " << name_index << ": " <<
        std::dec << name << std::endl;
    }
    
    return 0;
}
