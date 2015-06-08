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
 * end_station_configuration.cpp
 *
 */

#include "end_station_configuration.h"

end_station_configuration::end_station_configuration(avdecc_lib::end_station * end_station, avdecc_lib::system * sys)
{
    m_end_station = end_station;
    m_sys = sys;
    FillEndStationDetails();
}

end_station_configuration::~end_station_configuration() {}

int end_station_configuration::FillEndStationDetails()
{
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(m_end_station, &entity, &configuration);
    
    entity_id = wxString::Format("0x%llx",m_end_station->entity_id());
    mac = wxString::Format("%llx",m_end_station->mac());
    
    if(entity)
    {
        avdecc_lib::entity_descriptor_response *entity_desc_resp = entity->get_entity_response();
        name = entity_desc_resp->entity_name();
        fw_ver = (const char *)entity_desc_resp->firmware_version();
        delete entity_desc_resp;
    }
    
    avdecc_lib::strings_descriptor *strings_desc = configuration->get_strings_desc_by_index(0);
    if(strings_desc)
    {
        avdecc_lib::strings_descriptor_response *strings_resp_ref = strings_desc->get_strings_response();
        default_name = strings_resp_ref->get_string_by_index(1);
        delete strings_resp_ref;
    }
    
    avdecc_lib::audio_unit_descriptor *audio_unit_desc = configuration->get_audio_unit_desc_by_index(0);
    if(audio_unit_desc)
    {
        avdecc_lib::audio_unit_descriptor_response *audio_unit_resp_ref = audio_unit_desc->get_audio_unit_response();
        sample_rate = audio_unit_resp_ref->current_sampling_rate();
        delete audio_unit_resp_ref;
    }
    
    avdecc_lib::clock_domain_descriptor *clk_domain_desc = configuration->get_clock_domain_desc_by_index(0);
    if(clk_domain_desc)
    {
        avdecc_lib::clock_domain_descriptor_response *clk_domain_resp_ref = clk_domain_desc->get_clock_domain_response();
        clock_source = clk_domain_resp_ref->get_clock_source_by_index(clk_domain_resp_ref->clock_source_index());
        clock_source_count = clk_domain_resp_ref->clock_sources_count();
        delete clk_domain_resp_ref;
    }
    
    for(int i = 0; i < (int) clock_source_count; i++)
    {
        avdecc_lib::clock_source_descriptor *clk_src_desc = configuration->get_clock_source_desc_by_index(i);
        if(clk_src_desc)
        {
            avdecc_lib::clock_source_descriptor_response *clk_src_resp_ref = clk_src_desc->get_clock_source_response();
            uint8_t * clk_src_name = clk_src_resp_ref->object_name();
            uint8_t * clk_src_description;
            size_t string_desc_index;
            size_t string_index;
            if(clk_src_name[0] == '\0')
            {
                int ret = configuration->get_strings_desc_string_by_reference(clk_src_resp_ref->localized_description(),
                                                                              string_desc_index, string_index);
                if(ret == 0)
                {
                    avdecc_lib::strings_descriptor * desc = configuration->get_strings_desc_by_index(string_desc_index);
                    avdecc_lib::strings_descriptor_response *strings_resp_ref = desc->get_strings_response();
                    clk_src_description = strings_resp_ref->get_string_by_index(string_index);
                    delete strings_resp_ref;
                }
            }
            else
            {
                clk_src_description = clk_src_name;
            }
            
            clock_source_descriptions.push_back(clk_src_description);
            delete clk_src_resp_ref;
        }
        else
        {
            std::cout << "get clock_source desc error" << std::endl;
        }
    }
    return 0;
}

wxString end_station_configuration::get_entity_id()
{
    return entity_id;
}

wxString end_station_configuration::get_entity_name()
{
    return name;
}

wxString end_station_configuration::get_default_name()
{
    return default_name;
}

wxString end_station_configuration::get_mac()
{
    return mac;
}

wxString end_station_configuration::get_fw_ver()
{
    return fw_ver;
}

uint32_t end_station_configuration::get_sample_rate()
{
    return sample_rate;
}

uint16_t end_station_configuration::get_clock_source()
{
    return clock_source;
}

uint16_t end_station_configuration::get_clock_source_count()
{
    return clock_source_count;
}

int end_station_configuration::set_sample_rate(uint32_t sampling_rate)
{
    dialog_sample_rate = sampling_rate;
    return 0;
}

int end_station_configuration::set_clock_source(uint16_t clock_source_index)
{
    dialog_clock_source = clock_source_index;
    return 0;
}

int end_station_configuration::set_entity_name(wxString entity_name)
{
    dialog_name = entity_name;
    return 0;
}

int end_station_configuration::SetSamplingRate()
{
    if(sample_rate != dialog_sample_rate)
    {
        cmd_set_sampling_rate(dialog_sample_rate);
    }
    else
    {
        std::cout << "sampling rate unchanged" << std::endl;
    }
    return 0;
}

int end_station_configuration::cmd_set_sampling_rate(uint32_t new_sampling_rate)
{
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(m_end_station, &entity, &configuration);
    
    intptr_t cmd_notification_id = get_next_notification_id();
    m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    avdecc_lib::audio_unit_descriptor *audio_unit_desc_ref = configuration->get_audio_unit_desc_by_index(0);
    audio_unit_desc_ref->send_set_sampling_rate_cmd((void *)cmd_notification_id, new_sampling_rate);
    int status = m_sys->get_last_resp_status();
    
    if(status == avdecc_lib::AEM_STATUS_SUCCESS)
    {
        avdecc_lib::audio_unit_descriptor_response *audio_unit_resp_ref = audio_unit_desc_ref->get_audio_unit_response();
        std::cout << "Sampling rate: " << std::dec << audio_unit_resp_ref->current_sampling_rate();
        delete audio_unit_resp_ref;
    }
    
    return 0;
}

int end_station_configuration::SetEntityName()
{
    if(name.IsSameAs(dialog_name))
    {
        std::cout << "entity name unchanged" << std::endl;
    }
    else
    {
        cmd_set_name("ENTITY", 0, std::string(dialog_name));
    }
    return 0;
}

int end_station_configuration::cmd_set_name(std::string desc_name, uint16_t desc_index, std::string new_name)
{
    int status = avdecc_lib::AEM_STATUS_NOT_IMPLEMENTED;
    bool is_entity = false;
    
    uint16_t desc_type = avdecc_lib::utility::aem_desc_name_to_value(desc_name.c_str());

    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    avdecc_lib::descriptor_base *desc_base = NULL;
    
    if (get_current_end_station_entity_and_descriptor(&m_end_station, &entity,
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
        std::cout << "cmd_set_name cannot lookup descriptor" << std::endl;
        return 0;
    }
    
    uint16_t name_index = 0;
    struct avdecc_lib::avdecc_lib_name_string64 new_name64 = {{0}};
    strncpy((char *)new_name64.value, new_name.c_str(),
            sizeof(new_name64.value));
    
    intptr_t cmd_notification_id = get_next_notification_id();
    m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    desc_base->send_set_name_cmd((void *)cmd_notification_id, name_index, 0,
                                 + &new_name64);
    status = m_sys->get_last_resp_status();
    
    if(status == avdecc_lib::AEM_STATUS_SUCCESS)
    {
        cmd_display_desc_name(desc_base, name_index, is_entity);
    }
    else
    {
        std::cout << "cmd_set_name failed with AEM status: " <<
        avdecc_lib::utility::aem_cmd_status_value_to_name(status) <<
        std::endl;
    }
    
    return 0;
}

int end_station_configuration::SetClockSource()
{
    if(clock_source != dialog_clock_source)
    {
        cmd_set_clock_source(dialog_clock_source);
    }
    else
    {
        std::cout << "clock source unchanged" << std::endl;
    }
    return 0;
}

int end_station_configuration::cmd_set_clock_source(uint16_t new_clk_src_index)
{
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    if (get_current_end_station_entity_and_descriptor(&m_end_station, &entity, &configuration))
        return 0;
    
    intptr_t cmd_notification_id = get_next_notification_id();
    m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    avdecc_lib::clock_domain_descriptor *clk_domain_desc_ref = configuration->get_clock_domain_desc_by_index(0);
    if (!clk_domain_desc_ref)
        return 0;
    
    clk_domain_desc_ref->send_set_clock_source_cmd((void *)cmd_notification_id, new_clk_src_index);
    int status = m_sys->get_last_resp_status();
    
    if(status == avdecc_lib::AEM_STATUS_SUCCESS)
    {
        avdecc_lib::clock_domain_descriptor_response *clk_domain_resp_ref = clk_domain_desc_ref->get_clock_domain_response();
        std::cout << "Set clock source index : " << std::dec << clk_domain_resp_ref->clock_source_index() << std::endl;
        delete clk_domain_resp_ref;
    }
    
    return 0;
}

int end_station_configuration::cmd_display_desc_name(avdecc_lib::descriptor_base *desc, uint16_t name_index, bool is_entity)
{
    uint8_t * name;
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    
    if (get_current_end_station_entity_and_descriptor(&m_end_station, &entity,
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
        std::cout << "cmd_set_name() failed" << std::endl;
    }
    else
    {
        std::cout << "Descriptor " <<
        avdecc_lib::utility::aem_desc_value_to_name(desc->descriptor_type()) <<
        "." << desc->descriptor_index() <<
        " name at index " << name_index << ": " <<
        std::dec << name << std::endl;
    }
    
    return 0;
}

int end_station_configuration::get_current_entity_and_descriptor(avdecc_lib::end_station *end_station,
                                                                 avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration)
{
    *entity = NULL;
    *configuration = NULL;
    
    uint16_t current_entity = end_station->get_current_entity_index();
    if (current_entity >= end_station->entity_desc_count())
    {
        std::cout << "Current entity not available" << std::endl;
        return 1;
    }
    
    *entity = end_station->get_entity_desc_by_index(current_entity);
    
    uint16_t current_config = end_station->get_current_config_index();
    if (current_config >= (*entity)->config_desc_count())
    {
        std::cout << "Current configuration not available" << std::endl;
        return 1;
    }
    
    *configuration = (*entity)->get_config_desc_by_index(current_config);
    
    return 0;
}

uint32_t end_station_configuration::get_next_notification_id()
{
    return (uint32_t)notification_id++;
}

int end_station_configuration::get_current_end_station_entity_and_descriptor(avdecc_lib::end_station **end_station,
                                                                     avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration)
{
    if (m_end_station == NULL)
        return 1;
    
    if (get_current_entity_and_descriptor(*end_station, entity, configuration))
    {
        std::cout << "Current End Station not fully enumerated" << std::endl;
        return 1;
    }
    return 0;
}
