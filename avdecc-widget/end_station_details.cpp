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
 * end_station_details.cpp
 *
 */

#include "end_station_details.h"

wxBEGIN_EVENT_TABLE(end_station_details, wxFrame)
    EVT_TIMER(DetailsTimer, end_station_details::OnIncrementTimer)
wxEND_EVENT_TABLE()

end_station_details::end_station_details(wxWindow *parent, end_station_configuration *config, stream_configuration *stream_config)
{
    EndStation_Details_Dialog = new wxDialog(this, wxID_ANY, wxT("End Station Configuration"),
                                            wxDefaultPosition,
                                            wxSize(500, 700), wxRESIZE_BORDER);
    wxTimer *details_timer = new wxTimer(this, DetailsTimer);
    details_timer->Start(500, wxTIMER_CONTINUOUS);
    
    m_stream_input_count = stream_config->get_stream_input_count();
    m_stream_output_count = stream_config->get_stream_output_count();

    m_end_station_config = config;
    m_stream_config = new stream_configuration(m_stream_input_count, m_stream_output_count);

    m_entity_name = config->get_entity_name();
    m_default_name = config->get_default_name();
    m_entity_id = config->get_entity_id();
    m_mac = config->get_mac();
    m_fw_ver = config->get_fw_ver();
    m_sampling_rate = config->get_sample_rate();
    m_clk_source = config->get_clock_source();
    uint16_t m_clk_source_count = config->get_clock_source_count();
    
    for(int i = 0; i < m_clk_source_count; i++)
    {
        if(i <= config->clock_source_descriptions.size())
        {
            wxString clock_src_description = config->clock_source_descriptions.at(i);
            m_clock_source_descriptions.push_back(clock_src_description);
        }
        else
        {
            std::cout << "out of bounds" << std::endl;
        }
    }

    CreateEndStationDetailsPanel(m_entity_name, m_default_name,
                                 m_sampling_rate, m_entity_id,
                                 m_mac, m_fw_ver, m_clk_source, m_clk_source_count);

    CreateAndSizeGrid(m_stream_input_count, m_stream_output_count);

    for(unsigned int i = 0; i < m_stream_input_count; i++)
    {
        struct stream_configuration_details m_stream_details;
        
        stream_config->get_stream_input_details_by_index(i, m_stream_details);
        SetInputChannelName(i, m_stream_details.stream_name);
        SetInputChannelCount(i, m_stream_details.channel_count, m_stream_input_count);
    }
    
    for(unsigned int i = 0; i < m_stream_output_count; i++)
    {
        struct stream_configuration_details m_stream_details;
        
        stream_config->get_stream_output_details_by_index(i, m_stream_details);
        SetOutputChannelName(i, m_stream_details.stream_name);
        SetOutputChannelCount(i, m_stream_details.channel_count, m_stream_output_count);
    }
    
    m_input_maps_count = (unsigned int) stream_config->get_input_maps_count();
    m_output_maps_count = (unsigned int) stream_config->get_output_maps_count();
    
    for(unsigned int i = 0; i < m_output_maps_count; i++)
    {
        SetInputMappings(stream_config->stream_port_input_audio_mappings.at(i));
    }
    
    for(unsigned int i = 0; i < m_output_maps_count; i++)
    {
        SetOutputMappings(stream_config->stream_port_output_audio_mappings.at(i));
    }
    
    for(unsigned int i = 0; i < m_stream_input_count; i++)
    {
        struct stream_configuration_details m_stream_details;
        
        stream_config->get_stream_input_details_by_index(i, m_stream_details);
        if(m_stream_details.clk_sync_src_flag)
        {
            input_stream_grid->HideRow(i); //Hide Media Clock Input
        }
    }
    
    for(unsigned int i = 0; i < m_stream_output_count; i++)
    {
        struct stream_configuration_details m_stream_details;
        
        stream_config->get_stream_output_details_by_index(i, m_stream_details);
        if(m_stream_details.clk_sync_src_flag)
        {
            output_stream_grid->HideRow(i); //Hide Media Clock Output
        }
    }
    
    EndStation_Details_Dialog->Show();
}

end_station_details::~end_station_details()
{
    delete m_stream_config;
}

void end_station_details::CreateEndStationDetailsPanel(wxString Entity_Name, wxString Default_Name,
                                                       uint32_t Sampling_Rate, wxString Entity_ID,
                                                       wxString Mac, wxString Fw_version,
                                                       uint16_t clk_source, uint16_t clk_source_count)
{
    wxBoxSizer* Sizer1  = new wxBoxSizer(wxHORIZONTAL);
    Sizer1->Add(new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "End Station Name: ", wxDefaultPosition, wxSize(125,25)));
    
    wxBoxSizer* Sizer2  = new wxBoxSizer(wxHORIZONTAL);
    Sizer2->Add(new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "Default Name: ", wxDefaultPosition, wxSize(125,25)));

    wxBoxSizer *Sizer3 = new wxBoxSizer(wxHORIZONTAL);
    Sizer3->Add(new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "Sampling Rate:", wxDefaultPosition, wxSize(125,25)));
    
    wxBoxSizer* Sizer4  = new wxBoxSizer(wxHORIZONTAL);
    Sizer4->Add(new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "Entity ID: ", wxDefaultPosition, wxSize(125,25)));
    
    wxBoxSizer* Sizer5  = new wxBoxSizer(wxHORIZONTAL);
    Sizer5->Add(new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "MAC: ", wxDefaultPosition, wxSize(125,25)));
    
    wxBoxSizer* Sizer6  = new wxBoxSizer(wxHORIZONTAL);
    Sizer6->Add(new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "Firmware Version: ", wxDefaultPosition, wxSize(125,25)));
    
    wxBoxSizer* Sizer7  = new wxBoxSizer(wxHORIZONTAL);
    Sizer7->Add(new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "Clock Source: ", wxDefaultPosition, wxSize(125,25)));

    name = new wxTextCtrl(EndStation_Details_Dialog, wxID_ANY, Entity_Name, wxDefaultPosition, wxSize(150,25));
    Sizer1->Add(name);
    
    default_name = new wxTextCtrl(EndStation_Details_Dialog, wxID_ANY, Default_Name,
                                  wxDefaultPosition, wxSize(150,25), wxTE_READONLY);
    default_name->SetBackgroundColour(*wxLIGHT_GREY);
    Sizer2->Add(default_name);
    
    wxArrayString str;
    str.Add("48000 Hz");
    str.Add("96000 Hz");
    
    sampling_rate = new wxChoice(EndStation_Details_Dialog, wxID_ANY, wxDefaultPosition, wxSize(150,25), str);
    switch(Sampling_Rate)
    {
        case 48000:
            sampling_rate->SetSelection(0);
            break;
        case 96000:
            sampling_rate->SetSelection(1);
            break;
        default:
            //not implemented
            break;
    }

    Sizer3->Add(sampling_rate);
    
    wxArrayString str2;
    for(unsigned int i = 0; i < clk_source_count; i++)
    {
        if(i <= m_clock_source_descriptions.size())
        {
            str2.Add(m_clock_source_descriptions.at(i));
        }
        else
        {
            //errorr
        }
    }

    clock_source = new wxChoice(EndStation_Details_Dialog, wxID_ANY, wxDefaultPosition, wxSize(150,25), str2);
    clock_source->SetSelection(clk_source);
    
    Sizer7->Add(clock_source);
    
    entity_id = new wxTextCtrl(EndStation_Details_Dialog, wxID_ANY, Entity_ID, wxDefaultPosition, wxSize(150,25));
    entity_id->SetBackgroundColour(*wxLIGHT_GREY);
    Sizer4->Add(entity_id);
    
    mac = new wxTextCtrl(EndStation_Details_Dialog, wxID_ANY, Mac,
                         wxDefaultPosition, wxSize(150,25), wxTE_READONLY);
    mac->SetBackgroundColour(*wxLIGHT_GREY);
    Sizer5->Add(mac);
    
    fw_ver = new wxTextCtrl(EndStation_Details_Dialog, wxID_ANY, Fw_version,
                            wxDefaultPosition, wxSize(150,25), wxTE_READONLY);
    fw_ver->SetBackgroundColour(*wxLIGHT_GREY);
    Sizer6->Add(fw_ver);

    Details_Sizer = new wxStaticBoxSizer(wxVERTICAL, EndStation_Details_Dialog, "End Station Details");
    Details_Sizer->Add(Sizer1);
    Details_Sizer->Add(Sizer2);
    Details_Sizer->Add(Sizer3);
    Details_Sizer->Add(Sizer4);
    Details_Sizer->Add(Sizer5);
    Details_Sizer->Add(Sizer6);
    Details_Sizer->Add(Sizer7);
}

void end_station_details::SetChannelChoice(size_t stream_input_count, size_t stream_output_count)
                                            
{
    wxArrayString str;
    str.Add("1-Channel");
    str.Add("2-Channel");
    str.Add("8-Channel");
    
    for(unsigned int j = 0; j < stream_input_count; j++)
    {
        input_channel_choice = new wxGridCellChoiceEditor(str);
        input_stream_grid->SetCellEditor(j, 1, input_channel_choice);
    }
    
    for(unsigned int j = 0; j < stream_output_count; j++)
    {
        output_channel_choice = new wxGridCellChoiceEditor(str);
        output_stream_grid->SetCellEditor(j, 1, output_channel_choice);
    }
}

void end_station_details::SetInputMappings(struct audio_mapping &map)
{
    int16_t stream_index = 0;
    int16_t stream_channel = 0;
    int16_t cluster_offset = 0;
    int16_t cluster_channel = 0;
    
    stream_index = map.stream_index;
    stream_channel = map.stream_channel + 2; //to skip the columns containing stream name/channel count
    cluster_offset = map.cluster_offset;
    cluster_channel = map.cluster_channel;
    
    wxString cluster_offset_str = wxString::Format(wxT("%i"), cluster_offset);
    
    input_stream_grid->SetCellValue(stream_index, stream_channel, cluster_offset_str);
}

void end_station_details::SetOutputMappings(struct audio_mapping &map)
{
    int16_t stream_index = 0;
    int16_t stream_channel = 0;
    int16_t cluster_offset = 0;
    int16_t cluster_channel = 0;
    
    stream_index = map.stream_index;
    stream_channel = map.stream_channel + 2;
    cluster_offset = map.cluster_offset;
    cluster_channel = map.cluster_channel;
    
    wxString cluster_offset_str = wxString::Format(wxT("%i"), cluster_offset);
    
    output_stream_grid->SetCellValue(stream_index, stream_channel, cluster_offset_str);
}

void end_station_details::SetInputChannelName(unsigned int stream_index, wxString name)
{
    input_stream_grid->SetCellValue(stream_index, 0, name);
}

void end_station_details::SetOutputChannelName(unsigned int stream_index, wxString name)
{
    output_stream_grid->SetCellValue(stream_index, 0, name);
}

void end_station_details::SetInputChannelCount(unsigned int stream_index, size_t channel_count,
                                               size_t stream_input_count)
{
    input_stream_grid->SetCellValue(stream_index, 1, wxString::Format("%u-Channel", (int) channel_count));
    
    for(unsigned int i = 0; i < stream_input_count; i++)
    {
        input_stream_grid->SetRowSize(i, 25);
        
        for(unsigned int j = 2; j < 10; j++)
        {
            input_stream_grid->SetColSize(j, 25);
        }
        
        if(input_stream_grid->GetCellValue(i, 1) == "1-Channel")
        {
            for(unsigned int k = 3; k < 10; k++)
            {
                input_stream_grid->SetReadOnly(i, k);
                input_stream_grid->SetCellBackgroundColour(i, k, *wxLIGHT_GREY);
            }
        }
        else if(input_stream_grid->GetCellValue(i, 1) == "2-Channel")
        {
            input_stream_grid->SetReadOnly(i, 3, false); //writable
            input_stream_grid->SetCellBackgroundColour(i, 3, *wxWHITE);
            for(unsigned int k = 4; k < 10; k++)
            {
                input_stream_grid->SetReadOnly(i, k);
                input_stream_grid->SetCellBackgroundColour(i, k, *wxLIGHT_GREY);
            }
        }
        else
        {
            for(unsigned int k = 3; k < 10; k++)
            {
                input_stream_grid->SetReadOnly(i, k, false);
                input_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
            }
        }
    }
}

void end_station_details::SetOutputChannelCount(unsigned int stream_index, size_t channel_count,
                                                size_t stream_output_count)
{
    output_stream_grid->SetCellValue(stream_index, 1, wxString::Format("%u-Channel", (int) channel_count));
    
    for(unsigned int i = 0; i < stream_output_count; i++)
    {
        output_stream_grid->SetRowSize(i, 25);
        
        for(unsigned int j = 2; j < 10; j++)
        {
            output_stream_grid->SetColSize(j, 25);
        }
        
        if(output_stream_grid->GetCellValue(i, 1) == "1-Channel")
        {
            for(unsigned int k = 3; k < 10; k++)
            {
                output_stream_grid->SetReadOnly(i, k);
                output_stream_grid->SetCellBackgroundColour(i, k, *wxLIGHT_GREY);
            }
        }
        else if(output_stream_grid->GetCellValue(i, 1) == "2-Channel")
        {
            output_stream_grid->SetReadOnly(i, 3, false); //writable
            output_stream_grid->SetCellBackgroundColour(i, 3, *wxWHITE);
            for(unsigned int k = 4; k < 10; k++)
            {
                output_stream_grid->SetReadOnly(i, k);
                output_stream_grid->SetCellBackgroundColour(i, k, *wxLIGHT_GREY);
            }
        }
        else
        {
            for(unsigned int k = 3; k < 10; k++)
            {
                output_stream_grid->SetReadOnly(i, k, false);
                output_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
            }
        }
    }
}

void end_station_details::UpdateChannelCount()
{
    for(unsigned int i = 0; i < m_stream_input_count; i++)
    {
        if(input_stream_grid->GetCellValue(i, 1) == "1-Channel")
        {
            for(unsigned int k = 3; k < 10; k++)
            {
                input_stream_grid->SetReadOnly(i, k);
                input_stream_grid->SetCellBackgroundColour(i, k, *wxLIGHT_GREY);
            }
        }
        else if(input_stream_grid->GetCellValue(i, 1) == "2-Channel")
        {
            input_stream_grid->SetReadOnly(i, 3, false); //writable
            input_stream_grid->SetCellBackgroundColour(i, 3, *wxWHITE);
            for(unsigned int k = 4; k < 10; k++)
            {
                input_stream_grid->SetReadOnly(i, k);
                input_stream_grid->SetCellBackgroundColour(i, k, *wxLIGHT_GREY);
            }
        }
        else
        {
            for(unsigned int k = 3; k < 10; k++)
            {
                input_stream_grid->SetReadOnly(i, k, false);
                input_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
            }
        }
    }
    
    for(unsigned int i = 0; i < m_stream_output_count; i++)
    {
        if(output_stream_grid->GetCellValue(i, 1) == "1-Channel")
        {
            for(unsigned int k = 3; k < 10; k++)
            {
                output_stream_grid->SetReadOnly(i, k);
                output_stream_grid->SetCellBackgroundColour(i, k, *wxLIGHT_GREY);
            }
        }
        else if(output_stream_grid->GetCellValue(i, 1) == "2-Channel")
        {
            output_stream_grid->SetReadOnly(i, 3, false); //writable
            output_stream_grid->SetCellBackgroundColour(i, 3, *wxWHITE);
            for(unsigned int k = 4; k < 10; k++)
            {
                output_stream_grid->SetReadOnly(i, k);
                output_stream_grid->SetCellBackgroundColour(i, k, *wxLIGHT_GREY);
            }
        }
        else
        {
            for(unsigned int k = 3; k < 10; k++)
            {
                output_stream_grid->SetReadOnly(i, k, false);
                output_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
            }
        }
    }
}

void end_station_details::CreateInputStreamGridHeader()
{
    input_stream_header_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *Name_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "Stream Input", wxDefaultPosition, wxSize(125,25));
    input_stream_header_sizer->Add(Name_label);
    wxStaticText *Channel_type = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, " Channels", wxDefaultPosition, wxSize(85,25));
    input_stream_header_sizer->Add(Channel_type, wxCENTER);
    
    wxStaticText * channel1_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "1", wxDefaultPosition, wxSize(25,25));
    input_stream_header_sizer->Add(channel1_label);
    wxStaticText * channel2_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "2", wxDefaultPosition, wxSize(25,25));
    input_stream_header_sizer->Add(channel2_label);
    wxStaticText * channel3_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "3", wxDefaultPosition, wxSize(25,25));
    input_stream_header_sizer->Add(channel3_label);
    wxStaticText * channel4_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "4", wxDefaultPosition, wxSize(25,25));
    input_stream_header_sizer->Add(channel4_label);
    wxStaticText * channel5_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "5", wxDefaultPosition, wxSize(25,25));
    input_stream_header_sizer->Add(channel5_label);
    wxStaticText * channel6_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "6", wxDefaultPosition, wxSize(25,25));
    input_stream_header_sizer->Add(channel6_label);
    wxStaticText * channel7_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "7", wxDefaultPosition, wxSize(25,25));
    input_stream_header_sizer->Add(channel7_label);
    wxStaticText * channel8_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "8", wxDefaultPosition, wxSize(25,25));
    input_stream_header_sizer->Add(channel8_label);
}

void end_station_details::CreateOutputStreamGridHeader()
{
    output_stream_header_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *Name_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "Stream Output", wxDefaultPosition, wxSize(125,25));
    output_stream_header_sizer->Add(Name_label);
    wxStaticText *Channel_type = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, " Channels", wxDefaultPosition, wxSize(85,25));
    output_stream_header_sizer->Add(Channel_type, wxCENTER);
    
    wxStaticText * channel1_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "1", wxDefaultPosition, wxSize(25,25));
    output_stream_header_sizer->Add(channel1_label);
    wxStaticText * channel2_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "2", wxDefaultPosition, wxSize(25,25));
    output_stream_header_sizer->Add(channel2_label);
    wxStaticText * channel3_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "3", wxDefaultPosition, wxSize(25,25));
    output_stream_header_sizer->Add(channel3_label);
    wxStaticText * channel4_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "4", wxDefaultPosition, wxSize(25,25));
    output_stream_header_sizer->Add(channel4_label);
    wxStaticText * channel5_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "5", wxDefaultPosition, wxSize(25,25));
    output_stream_header_sizer->Add(channel5_label);
    wxStaticText * channel6_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "6", wxDefaultPosition, wxSize(25,25));
    output_stream_header_sizer->Add(channel6_label);
    wxStaticText * channel7_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "7", wxDefaultPosition, wxSize(25,25));
    output_stream_header_sizer->Add(channel7_label);
    wxStaticText * channel8_label = new wxStaticText(EndStation_Details_Dialog, wxID_ANY, "8", wxDefaultPosition, wxSize(25,25));
    output_stream_header_sizer->Add(channel8_label);
}

void end_station_details::CreateAndSizeGrid(size_t stream_input_count, size_t stream_output_count)
{
    apply_button = new wxButton(EndStation_Details_Dialog, wxID_OK, wxT("Apply"));
    cancel_button = new wxButton(EndStation_Details_Dialog, wxID_CANCEL, wxT("Cancel"));

    input_stream_grid = new wxGrid(EndStation_Details_Dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    output_stream_grid = new wxGrid(EndStation_Details_Dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    
    Input_Stream_Sizer = new wxStaticBoxSizer(wxVERTICAL,
                                              EndStation_Details_Dialog, "Input Streams");
    Output_Stream_Sizer = new wxStaticBoxSizer(wxVERTICAL,
                                              EndStation_Details_Dialog, "Output Streams");
    
    CreateInputStreamGridHeader();
    CreateOutputStreamGridHeader();
    
    input_stream_grid->EnableGridLines();
    input_stream_grid->SetRowLabelSize(0);
    input_stream_grid->SetColLabelSize(0);
    
    output_stream_grid->EnableGridLines();
    output_stream_grid->SetRowLabelSize(0);
    output_stream_grid->SetColLabelSize(0);

    grid_base = new wxGridStringTable((int) stream_input_count, 10);
    grid_base2 = new wxGridStringTable((int)stream_output_count, 10);

    input_stream_grid->SetTable(grid_base);
    output_stream_grid->SetTable(grid_base2);
    SetChannelChoice(stream_input_count, stream_output_count);

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);

    sizer->Add(Details_Sizer);
    Input_Stream_Sizer->Add(input_stream_header_sizer);
    Input_Stream_Sizer->Add(input_stream_grid);
    sizer->Add(Input_Stream_Sizer);
    
    Output_Stream_Sizer->Add(output_stream_header_sizer);
    Output_Stream_Sizer->Add(output_stream_grid);
    sizer->Add(Output_Stream_Sizer);
    
    button_sizer->Add(apply_button);
    button_sizer->Add(cancel_button);
    sizer->Add(button_sizer);

    EndStation_Details_Dialog->SetSizer(sizer, true);
    
    input_stream_grid->SetColSize(0, 130);
    output_stream_grid->SetColSize(0, 130);
}

void end_station_details::OnOK()
{
    if(name->GetNumberOfLines() == 1)
    {
        wxString set_entity_name = name->GetLineText(0);
        m_end_station_config->set_entity_name(set_entity_name);
    }
    else
    {
        //Set name error
    }

    int sample_rate_index = sampling_rate->GetSelection(); //return sample rate choice index
    uint32_t set_sampling_rate_value = atoi(sampling_rate->GetString(sample_rate_index)); //return dialog sampling_rate selection
    m_end_station_config->set_sample_rate(set_sampling_rate_value);
    
    uint16_t clock_source_index = (uint16_t) clock_source->GetSelection(); //return clock source choice index
    m_end_station_config->set_clock_source(clock_source_index); //return dialog clock_source index selection

    for(unsigned int i = 0; i < m_stream_input_count; i++)
    {
        struct stream_configuration_details input_stream_details;
        struct audio_mapping input_audio_mapping;
        unsigned int channel_count;
    
        input_stream_details.stream_name = input_stream_grid->GetCellValue(i, 0);
        channel_count = wxAtoi(input_stream_grid->GetCellValue(i, 1));
        input_stream_details.channel_count = channel_count;
        
        m_stream_config->input_stream_config.push_back(input_stream_details);
        
        switch(channel_count)
        {
            case 1:
                input_audio_mapping.cluster_offset = wxAtoi(input_stream_grid->GetCellValue(i, 2));
                input_audio_mapping.cluster_channel = 0; //cluster only has 1 channel
                input_audio_mapping.stream_index = (uint16_t) i;
                input_audio_mapping.stream_channel = 1;
                m_stream_config->stream_port_input_audio_mappings.push_back(input_audio_mapping);
                break;
            case 2:
                for(unsigned int j = 2; j < 3; j++)
                {
                    input_audio_mapping.cluster_offset = wxAtoi(input_stream_grid->GetCellValue(i, j));
                    input_audio_mapping.cluster_channel = 0; //cluster only has 1 channel
                    input_audio_mapping.stream_index = (uint16_t) i;
                    input_audio_mapping.stream_channel = (j - 1);
                    m_stream_config->stream_port_input_audio_mappings.push_back(input_audio_mapping);
                }
                break;
            case 8:
                for(unsigned int j = 2; j < 9; j++)
                {
                    input_audio_mapping.cluster_offset = wxAtoi(input_stream_grid->GetCellValue(i, j));
                    input_audio_mapping.cluster_channel = 0; //cluster only has 1 channel
                    input_audio_mapping.stream_index = (uint16_t) i;
                    input_audio_mapping.stream_channel = (j - 1);
                    m_stream_config->stream_port_input_audio_mappings.push_back(input_audio_mapping);
                }
                break;
            default:
                //unsupported channel count
                break;
        }
    }
    
    for(unsigned int i = 0; i < m_stream_output_count; i++)
    {
        struct stream_configuration_details output_stream_details;
        struct audio_mapping output_audio_mapping;
        unsigned int channel_count;

        output_stream_details.stream_name = output_stream_grid->GetCellValue(i, 0);
        channel_count = wxAtoi(output_stream_grid->GetCellValue(i, 1));
        output_stream_details.channel_count = channel_count;

        m_stream_config->output_stream_config.push_back(output_stream_details);
        
        switch(channel_count)
        {
            case 1:
                output_audio_mapping.cluster_offset = wxAtoi(output_stream_grid->GetCellValue(i, 2));
                output_audio_mapping.cluster_channel = 0; //cluster only has 1 channel
                output_audio_mapping.stream_index = (uint16_t) i;
                output_audio_mapping.stream_channel = 1;
                m_stream_config->stream_port_output_audio_mappings.push_back(output_audio_mapping);
                break;
            case 2:
                for(unsigned int j = 2; j < 3; j++)
                {
                    output_audio_mapping.cluster_offset = wxAtoi(output_stream_grid->GetCellValue(i, j));
                    output_audio_mapping.cluster_channel = 0; //cluster only has 1 channel
                    output_audio_mapping.stream_index = (uint16_t) i;
                    output_audio_mapping.stream_channel = (j - 1);
                    m_stream_config->stream_port_output_audio_mappings.push_back(output_audio_mapping);
                }
                break;
            case 8:
                for(unsigned int j = 2; j < 9; j++)
                {
                    output_audio_mapping.cluster_offset = wxAtoi(output_stream_grid->GetCellValue(i, j));
                    output_audio_mapping.cluster_channel = 0; //cluster only has 1 channel
                    output_audio_mapping.stream_index = (uint16_t) i;
                    output_audio_mapping.stream_channel = (j - 1);
                    m_stream_config->stream_port_output_audio_mappings.push_back(output_audio_mapping);
                }
                break;
            default:
                //unsupported channel count
                break;
        }
    }
}

void end_station_details::OnCancel()
{
    //Destroy();
}

int end_station_details::ShowModal()
{
    return EndStation_Details_Dialog->ShowModal();
}

void end_station_details::OnIncrementTimer(wxTimerEvent &event)
{
    UpdateChannelCount();
}

