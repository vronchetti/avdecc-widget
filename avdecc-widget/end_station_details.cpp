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

wxBEGIN_EVENT_TABLE(end_station_details, wxDialog)
    EVT_GRID_CELL_CHANGED(end_station_details::OnGridChange)
    EVT_CHOICE(CHANNEL_CHANGE, end_station_details::OnChannelChange)
wxEND_EVENT_TABLE()

end_station_details::end_station_details(wxWindow *parent, end_station_configuration *config, stream_configuration *stream_config) :
wxDialog(parent, wxID_ANY, wxT("End Station Configuration"),
        wxDefaultPosition,
        wxSize(500, 700), wxRESIZE_BORDER)
{
	/*
	* Copy pointers to end station details and stream details class objects.  Values will be updated
	* in these classes when this Dialog window closes.
	*/
	m_end_station_config = config;
	m_stream_config = stream_config;

	GetEndStationData();
	GetStreamData();

	//End station details list
    CreateAndSizeEndStationDetailsList();

	//Create Stream details grids and channel count choices
	CreateAndSizeStreamGrids();
	CreateChannelChoices(); 
	
	//Set Initial Stream Details values
	SetStreamDetails();

    SetSizerAndFit(dialog_sizer); //Fit the main sizer
    Show();
}

end_station_details::~end_station_details() {}

void end_station_details::GetEndStationData()
{
	//get end station details info
	m_entity_name = m_end_station_config->get_entity_name();
	m_default_name = m_end_station_config->get_default_name();
	m_entity_id = m_end_station_config->get_entity_id();
	m_mac = m_end_station_config->get_mac();
	m_fw_ver = m_end_station_config->get_fw_ver();
	m_sampling_rate = m_end_station_config->get_sample_rate();
	m_current_clk_source = m_end_station_config->get_clock_source();

	for (size_t i = 0; i < m_end_station_config->clock_source_descriptions.size(); i++)
	{
		if (m_end_station_config->clock_source_descriptions.size() > i)
		{
			wxString clock_src_description = m_end_station_config->clock_source_descriptions.at(i);
			m_clock_source_descriptions.push_back(clock_src_description);
		}
	}
}

void end_station_details::GetStreamData()
{
	//get stream details info
	m_stream_input_count = m_stream_config->get_stream_input_count();
	m_stream_output_count = m_stream_config->get_stream_output_count();
	m_input_cluster_count = m_stream_config->get_stream_input_cluster_count();
	m_output_cluster_count = m_stream_config->get_stream_output_cluster_count();
	m_input_maps_count = (unsigned int)m_stream_config->get_avdecc_input_maps_count();
	m_output_maps_count = (unsigned int)m_stream_config->get_avdecc_output_maps_count();
}

void end_station_details::SetStreamDetails()
{
	for (unsigned int i = 0; i < m_stream_input_count; i++) //initial stream names and channel counts
	{
		struct stream_configuration_details m_stream_details;

		m_stream_config->get_avdecc_stream_input_details_by_index(i, m_stream_details);
		SetInputChannelName(i, m_stream_details.stream_name);
		SetInputChannelCount(i, m_stream_details.channel_count);
	}

	for (unsigned int i = 0; i < m_stream_output_count; i++)
	{
		struct stream_configuration_details m_stream_details;

		m_stream_config->get_avdecc_stream_output_details_by_index(i, m_stream_details);
		SetOutputChannelName(i, m_stream_details.stream_name);
		SetOutputChannelCount(i, m_stream_details.channel_count);
	}

	for (unsigned int i = 0; i < m_input_maps_count; i++) //initial audio mappings
	{
		SetInputMappings(m_stream_config->avdecc_stream_port_input_audio_mappings.at(i));
	}

	for (unsigned int i = 0; i < m_output_maps_count; i++)
	{
		SetOutputMappings(m_stream_config->avdecc_stream_port_output_audio_mappings.at(i));
	}

	for (unsigned int i = 0; i < m_stream_input_count; i++) //Hide Media Clock Input and Output
	{
		struct stream_configuration_details m_stream_details;

		m_stream_config->get_avdecc_stream_input_details_by_index(i, m_stream_details);
		if (m_stream_details.clk_sync_src_flag)
		{
			input_stream_grid->HideRow(i);
			input_stream_name_grid->HideRow(i);
			wxChoice * channel_count = input_channel_counts.at(i);
			channel_count->Hide();
		}
	}

	for (unsigned int i = 0; i < m_stream_output_count; i++)
	{
		struct stream_configuration_details m_stream_details;

		m_stream_config->get_avdecc_stream_output_details_by_index(i, m_stream_details);
		if (m_stream_details.clk_sync_src_flag)
		{
			output_stream_grid->HideRow(i);
			output_stream_name_grid->HideRow(i);
			wxChoice * channel_count = output_channel_counts.at(i);
			channel_count->Hide();
		}
	}
}

void end_station_details::CreateAndSizeEndStationDetailsList()
{
    wxBoxSizer* Sizer1  = new wxBoxSizer(wxHORIZONTAL);
    Sizer1->Add(new wxStaticText(this, wxID_ANY, "End Station Name: ", wxDefaultPosition, wxSize(125,25)));
    
    wxBoxSizer* Sizer2  = new wxBoxSizer(wxHORIZONTAL);
    Sizer2->Add(new wxStaticText(this, wxID_ANY, "Default Name: ", wxDefaultPosition, wxSize(125,25)));

    wxBoxSizer *Sizer3 = new wxBoxSizer(wxHORIZONTAL);
    Sizer3->Add(new wxStaticText(this, wxID_ANY, "Sampling Rate:", wxDefaultPosition, wxSize(125,25)));
    
    wxBoxSizer* Sizer4  = new wxBoxSizer(wxHORIZONTAL);
    Sizer4->Add(new wxStaticText(this, wxID_ANY, "Entity ID: ", wxDefaultPosition, wxSize(125,25)));
    
    wxBoxSizer* Sizer5  = new wxBoxSizer(wxHORIZONTAL);
    Sizer5->Add(new wxStaticText(this, wxID_ANY, "MAC: ", wxDefaultPosition, wxSize(125,25)));
    
    wxBoxSizer* Sizer6  = new wxBoxSizer(wxHORIZONTAL);
    Sizer6->Add(new wxStaticText(this, wxID_ANY, "Firmware Version: ", wxDefaultPosition, wxSize(125,25)));
    
    wxBoxSizer* Sizer7  = new wxBoxSizer(wxHORIZONTAL);
    Sizer7->Add(new wxStaticText(this, wxID_ANY, "Clock Source: ", wxDefaultPosition, wxSize(125,25)));

    name = new wxTextCtrl(this, wxID_ANY, m_entity_name, wxDefaultPosition, wxSize(150,25));
    Sizer1->Add(name);
    
    default_name = new wxTextCtrl(this, wxID_ANY, m_default_name,
                                  wxDefaultPosition, wxSize(150,25), wxTE_READONLY);
    default_name->SetBackgroundColour(my_grey);
    Sizer2->Add(default_name);
    
    wxArrayString str;
    str.Add("48000 Hz");
    str.Add("96000 Hz");
    
    sampling_rate = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(150,25), str);
    switch(m_sampling_rate)
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
    for(unsigned int i = 0; i < m_clock_source_descriptions.size(); i++)
    {
        str2.Add(m_clock_source_descriptions.at(i));
    }

    clock_source = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(150,25), str2);
    clock_source->SetSelection(m_current_clk_source);
    
    Sizer7->Add(clock_source);
    
    entity_id = new wxTextCtrl(this, wxID_ANY, m_entity_id, wxDefaultPosition, wxSize(150,25));
    entity_id->SetBackgroundColour(my_grey);
    Sizer4->Add(entity_id);
    
    mac = new wxTextCtrl(this, wxID_ANY, m_mac,
                         wxDefaultPosition, wxSize(150,25), wxTE_READONLY);
    mac->SetBackgroundColour(my_grey);
    Sizer5->Add(mac);
    
    fw_ver = new wxTextCtrl(this, wxID_ANY, m_fw_ver,
                            wxDefaultPosition, wxSize(150,25), wxTE_READONLY);
    fw_ver->SetBackgroundColour(my_grey);
    Sizer6->Add(fw_ver);

    end_station_details_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "End Station Details");
    end_station_details_sizer->Add(Sizer1);
    end_station_details_sizer->Add(Sizer2);
    end_station_details_sizer->Add(Sizer3);
    end_station_details_sizer->Add(Sizer4);
    end_station_details_sizer->Add(Sizer5);
    end_station_details_sizer->Add(Sizer6);
    end_station_details_sizer->Add(Sizer7);
}

void end_station_details::CreateChannelChoices()
                                            
{
    wxArrayString str;
    str.Add("1-Channel");
    str.Add("2-Channel");
	str.Add("4-Channel");
    str.Add("8-Channel");

    for(size_t i = 0; i < m_stream_input_count; i++)
    {
        wxChoice * channel_count;
        
        channel_count = new wxChoice(this, CHANNEL_CHANGE, wxDefaultPosition,
                                     wxSize(100, 25), str);
        
        input_channel_count_choice_sizer->Add(channel_count);
        input_channel_counts.push_back(channel_count);
    }
    
    for(size_t i = 0; i < m_stream_output_count; i++)
    {
        wxChoice * channel_count;
        
        channel_count = new wxChoice(this, CHANNEL_CHANGE, wxDefaultPosition,
                                     wxSize(100, 25), str);
        
        output_channel_count_choice_sizer->Add(channel_count);
        output_channel_counts.push_back(channel_count);
    }
}

void end_station_details::SetInputMappings(struct audio_mapping &map)
{
    int16_t stream_index = 0;
    int16_t stream_channel = 0;
    int16_t cluster_offset = 0;
    int16_t cluster_channel = 0;
    
    stream_index = map.stream_index;
    stream_channel = map.stream_channel;
    cluster_offset = map.cluster_offset + 1; //avdecc mappings start at index 0
    cluster_channel = map.cluster_channel;
    
    wxString cluster_offset_str = wxString::Format(wxT("%i"), cluster_offset);
    input_stream_grid->SetCellValue(stream_index, stream_channel, cluster_offset_str);
	input_stream_grid->SetCellBackgroundColour(stream_index, stream_channel, my_green);
}

void end_station_details::SetOutputMappings(struct audio_mapping &map)
{
    int16_t stream_index = 0;
    int16_t stream_channel = 0;
    int16_t cluster_offset = 0;
    int16_t cluster_channel = 0;
    
    stream_index = map.stream_index;
    stream_channel = map.stream_channel;
    cluster_offset = map.cluster_offset - m_input_cluster_count + 1;
    cluster_channel = map.cluster_channel;
    
    wxString cluster_offset_str = wxString::Format(wxT("%i"), cluster_offset);
    output_stream_grid->SetCellValue(stream_index, stream_channel, cluster_offset_str);
	output_stream_grid->SetCellBackgroundColour(stream_index, stream_channel, my_green);
}

void end_station_details::SetInputChannelName(unsigned int stream_index, wxString name)
{
    input_stream_name_grid->SetCellValue(stream_index, 0, name);
}

void end_station_details::SetOutputChannelName(unsigned int stream_index, wxString name)
{
    output_stream_name_grid->SetCellValue(stream_index, 0, name);
}

void end_station_details::SetInputChannelCount(unsigned int stream_index, size_t channel_count)
{
	switch (channel_count)
	{
		case 1:
			input_channel_counts.at(stream_index)->SetSelection(0);
			break;
		case 2:
			input_channel_counts.at(stream_index)->SetSelection(1);
			break;
		case 4:
			input_channel_counts.at(stream_index)->SetSelection(2);
			break;
		case 8:
			input_channel_counts.at(stream_index)->SetSelection(3);
			break;
		default:
			//unsupported
			break;
	}

    for(unsigned int i = 0; i < m_stream_input_count; i++)
    {
        input_stream_grid->SetRowSize(i, 25);
        
        for(unsigned int j = 0; j < 8; j++)
        {
            input_stream_grid->SetColSize(j, 25);
        }
        
        if(input_channel_counts.at(i)->GetSelection() == 0) //1 channel
        {
			input_stream_grid->SetCellValue(i, 0, wxT("0"));

            for(unsigned int k = 1; k < 8; k++)
            {
                input_stream_grid->SetReadOnly(i, k);
                input_stream_grid->SetCellBackgroundColour(i, k, my_grey);
            }
        }
        else if(input_channel_counts.at(i)->GetSelection() == 1) //2 channel
        {
            input_stream_grid->SetReadOnly(i, 1, false); //writable
            input_stream_grid->SetCellBackgroundColour(i, 1, *wxWHITE);

			for (unsigned int k = 0; k < 2; k++)
			{
				input_stream_grid->SetCellValue(i, k, wxT("0"));
			}

            for(unsigned int k = 2; k < 8; k++)
            {
                input_stream_grid->SetReadOnly(i, k);
                input_stream_grid->SetCellBackgroundColour(i, k, my_grey);
            }
        }
		else if(input_channel_counts.at(i)->GetSelection() == 2) //4 channel
        {
			for (unsigned int k = 0; k < 4; k++)
			{
				input_stream_grid->SetCellValue(i, k, wxT("0"));
			}

            for(unsigned int k = 4; k < 8; k++)
            {
                input_stream_grid->SetReadOnly(i, k, false);
                input_stream_grid->SetCellBackgroundColour(i, k, my_grey);
            }
        }
		else if (input_channel_counts.at(i)->GetSelection() == 3) //8 channel
		{
			for (unsigned int k = 0; k < 8; k++)
			{
				input_stream_grid->SetCellValue(i, k, wxT("0"));
				input_stream_grid->SetReadOnly(i, k, false);
				input_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
			}
		}
		else
		{
			//unsupported
		}
    }
}

void end_station_details::SetOutputChannelCount(unsigned int stream_index, size_t channel_count)
{
	switch (channel_count)
	{
	case 1:
		output_channel_counts.at(stream_index)->SetSelection(0);
		break;
	case 2:
		output_channel_counts.at(stream_index)->SetSelection(1);
		break;
	case 4:
		output_channel_counts.at(stream_index)->SetSelection(2);
		break;
	case 8:
		output_channel_counts.at(stream_index)->SetSelection(3);
		break;
	default:
		//unsupported
		break;
	}
    
    for(unsigned int i = 0; i < m_stream_output_count; i++)
    {
        output_stream_grid->SetRowSize(i, 25);
        
        for(unsigned int j = 0; j < 8; j++)
        {
            output_stream_grid->SetColSize(j, 25);
        }
        
        if(output_channel_counts.at(i)->GetSelection() == 0) //1 channel
        {
			output_stream_grid->SetCellValue(i, 0, wxT("0"));

            for(unsigned int k = 1; k < 8; k++)
            {
                output_stream_grid->SetReadOnly(i, k);
                output_stream_grid->SetCellBackgroundColour(i, k, my_grey);
            }
        }
        else if(output_channel_counts.at(i)->GetSelection() == 1) //2 channel
        {
            output_stream_grid->SetReadOnly(i, 3, false); //writable
            output_stream_grid->SetCellBackgroundColour(i, 3, *wxWHITE);

			for (unsigned int k = 0; k < 2; k++)
			{
				output_stream_grid->SetCellValue(i, k, wxT("0"));
			}

            for(unsigned int k = 2; k < 8; k++)
            {
                output_stream_grid->SetReadOnly(i, k);
                output_stream_grid->SetCellBackgroundColour(i, k, my_grey);
            }
        }
		else if(output_channel_counts.at(i)->GetSelection() == 2) //4 channel
        {
			for (unsigned int k = 0; k < 4; k++)
			{
				output_stream_grid->SetCellValue(i, k, wxT("0"));
			}

            for(unsigned int k = 4; k < 8; k++)
            {
                output_stream_grid->SetReadOnly(i, k, false);
                output_stream_grid->SetCellBackgroundColour(i, k, my_grey);
            }
        }
		else if (output_channel_counts.at(i)->GetSelection() == 3) //8 channel
		{
			for (unsigned int k = 0; k < 8; k++)
			{
				output_stream_grid->SetCellValue(i, k, wxT("0"));
				output_stream_grid->SetReadOnly(i, k, false);
				output_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
			}
		}
		else
		{
			//not supported
		}
    }
}

void end_station_details::UpdateChannelCount()
{
	/*
	unsigned int current_input_cluster_count = 0;
	unsigned int current_output_cluster_count = 0;
	
	for(unsigned int i = 0; i < m_stream_input_count; i++)
	{
        if(!input_stream_grid->IsRowShown(i))
        {
            //skip media clock
        }
        else
        {
            for (unsigned int j = 0; j < 8; j++)
            {
                int val = wxAtoi(input_stream_grid->GetCellValue(i, j));
                if (val != 0)
                {
                    ++current_input_cluster_count;
                }
            }
        }
	}

	for (unsigned int i = 0; i < m_stream_output_count; i++)
	{
        if(!output_stream_grid->IsRowShown(i))
        {
            //skip media clock
        }
        else
        {
            for (unsigned int j = 0; j < 8; j++)
            {
                int val = wxAtoi(output_stream_grid->GetCellValue(i, j));
                if (val != 0)
                {
                    ++current_output_cluster_count;
                }
            }
        }
	}
	*/

    for(unsigned int i = 0; i < m_stream_input_count; i++)
    {
        if(input_channel_counts.at(i)->GetSelection() == 0) //1 channel
        {
            for(unsigned int k = 1; k < 8; k++)
            {
				input_stream_grid->SetCellValue(i, k, wxEmptyString);
                input_stream_grid->SetReadOnly(i, k);
                input_stream_grid->SetCellBackgroundColour(i, k, my_grey);
            }
        }
		else if (input_channel_counts.at(i)->GetSelection() == 1) //2 channel
		{
			input_stream_grid->SetReadOnly(i, 1, false); //writable

			for (unsigned int k = 2; k < 8; k++) //clear other channel data
			{
				input_stream_grid->SetCellValue(i, k, wxEmptyString);
				input_stream_grid->SetReadOnly(i, k);
				input_stream_grid->SetCellBackgroundColour(i, k, my_grey);
			}

			for (unsigned int k = 0; k < 2; k++) //initialize 2 channels if empty
			{
				int val = wxAtoi(input_stream_grid->GetCellValue(i, k));
				if (val == 0)
				{
                    input_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
					input_stream_grid->SetCellValue(i, k, wxT("0"));
				}
			}
		}
		else if (input_channel_counts.at(i)->GetSelection() == 2) //4 channel
		{
			for (unsigned int k = 4; k < 8; k++) //clear other channel data
			{
				input_stream_grid->SetCellValue(i, k, wxEmptyString);
				input_stream_grid->SetReadOnly(i, k);
				input_stream_grid->SetCellBackgroundColour(i, k, my_grey);
			}

			for (unsigned int k = 0; k < 4; k++) //initialize 4 channels if empty
			{
				input_stream_grid->SetReadOnly(i, k, false); //writable
				int val = wxAtoi(input_stream_grid->GetCellValue(i, k));
				if (val == 0)
				{
					input_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
					input_stream_grid->SetCellValue(i, k, wxT("0"));
				}
			}
		}
		else if (input_channel_counts.at(i)->GetSelection() == 3) //8 channel
        {
            for(unsigned int k = 0; k < 8; k++) //initialize 8 channels if empty
            {
				int val = wxAtoi(input_stream_grid->GetCellValue(i, k));
                input_stream_grid->SetReadOnly(i, k, false);

				if (val == 0)
				{
                    input_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
                    input_stream_grid->SetCellValue(i, k, wxT("0"));
				}
            }
        }
		else
		{
			//not supported
		}
    }
    
    for(unsigned int i = 0; i < m_stream_output_count; i++)
    {
		for (unsigned int i = 0; i < m_stream_output_count; i++)
		{
			if (output_channel_counts.at(i)->GetSelection() == 0) //1 channel
			{
				for (unsigned int k = 1; k < 8; k++)
				{
					output_stream_grid->SetCellValue(i, k, wxEmptyString);
					output_stream_grid->SetReadOnly(i, k);
					output_stream_grid->SetCellBackgroundColour(i, k, my_grey);
				}
			}
			else if (output_channel_counts.at(i)->GetSelection() == 1) //2 channel
			{
				output_stream_grid->SetReadOnly(i, 1, false); //writable

				for (unsigned int k = 2; k < 8; k++) //clear other channel data
				{
					output_stream_grid->SetCellValue(i, k, wxEmptyString);
					output_stream_grid->SetReadOnly(i, k);
					output_stream_grid->SetCellBackgroundColour(i, k, my_grey);
				}

				for (unsigned int k = 0; k < 2; k++) //initialize 2 channels if empty
				{
					int val = wxAtoi(output_stream_grid->GetCellValue(i, k));
					if (val == 0)
					{
						output_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
						output_stream_grid->SetCellValue(i, k, wxT("0"));
					}
				}
			}
			else if (output_channel_counts.at(i)->GetSelection() == 2) //4 channel
			{
				for (unsigned int k = 4; k < 8; k++) //clear other channel data
				{
					output_stream_grid->SetCellValue(i, k, wxEmptyString);
					output_stream_grid->SetReadOnly(i, k);
					output_stream_grid->SetCellBackgroundColour(i, k, my_grey);
				}

				for (unsigned int k = 0; k < 4; k++) //initialize 4 channels if empty
				{
					output_stream_grid->SetReadOnly(i, k, false); //writable
					int val = wxAtoi(output_stream_grid->GetCellValue(i, k));
					if (val == 0)
					{
						output_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
						output_stream_grid->SetCellValue(i, k, wxT("0"));
					}
				}
			}
			else if (output_channel_counts.at(i)->GetSelection() == 3) //8 channel
			{
				for (unsigned int k = 0; k < 8; k++) //initialize 8 channels if empty
				{
					output_stream_grid->SetReadOnly(i, k, false);
					int val = wxAtoi(output_stream_grid->GetCellValue(i, k));
					if (val == 0)
					{
						output_stream_grid->SetCellBackgroundColour(i, k, *wxWHITE);
						output_stream_grid->SetCellValue(i, k, wxT("0"));
					}
				}
			}
			else
			{
				//not supported
			}
		}
    }
    
    input_stream_grid->ForceRefresh();
    output_stream_grid->ForceRefresh();
	CheckValid();
}

void end_station_details::CreateInputStreamGridHeader()
{
    input_stream_header_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *Name_label = new wxStaticText(this, wxID_ANY, "Stream Input", wxDefaultPosition, wxSize(125,25), wxALIGN_CENTER_HORIZONTAL);
    input_stream_header_sizer->Add(Name_label);
	wxStaticText *Channel_type = new wxStaticText(this, wxID_ANY, "Channels", wxDefaultPosition, wxSize(100, 25), wxALIGN_CENTER_HORIZONTAL);
    input_stream_header_sizer->Add(Channel_type, wxCENTER);
    
	wxStaticText * channel1_label = new wxStaticText(this, wxID_ANY, "1", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    input_stream_header_sizer->Add(channel1_label);
	wxStaticText * channel2_label = new wxStaticText(this, wxID_ANY, "2", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    input_stream_header_sizer->Add(channel2_label);
	wxStaticText * channel3_label = new wxStaticText(this, wxID_ANY, "3", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    input_stream_header_sizer->Add(channel3_label);
	wxStaticText * channel4_label = new wxStaticText(this, wxID_ANY, "4", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    input_stream_header_sizer->Add(channel4_label);
	wxStaticText * channel5_label = new wxStaticText(this, wxID_ANY, "5", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    input_stream_header_sizer->Add(channel5_label);
	wxStaticText * channel6_label = new wxStaticText(this, wxID_ANY, "6", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    input_stream_header_sizer->Add(channel6_label);
	wxStaticText * channel7_label = new wxStaticText(this, wxID_ANY, "7", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    input_stream_header_sizer->Add(channel7_label);
	wxStaticText * channel8_label = new wxStaticText(this, wxID_ANY, "8", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    input_stream_header_sizer->Add(channel8_label);
}

void end_station_details::CreateOutputStreamGridHeader()
{
    output_stream_header_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *Name_label = new wxStaticText(this, wxID_ANY, "Stream Output", wxDefaultPosition, wxSize(125, 25), wxALIGN_CENTER_HORIZONTAL);
    output_stream_header_sizer->Add(Name_label);
	wxStaticText *Channel_type = new wxStaticText(this, wxID_ANY, "Channels", wxDefaultPosition, wxSize(100, 25), wxALIGN_CENTER_HORIZONTAL);
    output_stream_header_sizer->Add(Channel_type, wxCENTER);
    
	wxStaticText * channel1_label = new wxStaticText(this, wxID_ANY, "1", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    output_stream_header_sizer->Add(channel1_label);
	wxStaticText * channel2_label = new wxStaticText(this, wxID_ANY, "2", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    output_stream_header_sizer->Add(channel2_label);
	wxStaticText * channel3_label = new wxStaticText(this, wxID_ANY, "3", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    output_stream_header_sizer->Add(channel3_label);
	wxStaticText * channel4_label = new wxStaticText(this, wxID_ANY, "4", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    output_stream_header_sizer->Add(channel4_label);
	wxStaticText * channel5_label = new wxStaticText(this, wxID_ANY, "5", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    output_stream_header_sizer->Add(channel5_label);
	wxStaticText * channel6_label = new wxStaticText(this, wxID_ANY, "6", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    output_stream_header_sizer->Add(channel6_label);
	wxStaticText * channel7_label = new wxStaticText(this, wxID_ANY, "7", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    output_stream_header_sizer->Add(channel7_label);
	wxStaticText * channel8_label = new wxStaticText(this, wxID_ANY, "8", wxDefaultPosition, wxSize(25, 25), wxALIGN_CENTER_HORIZONTAL);
    output_stream_header_sizer->Add(channel8_label);
}

void end_station_details::CreateAndSizeStreamGrids()
{
	CreateInputStreamGridHeader();
	CreateOutputStreamGridHeader();

    apply_button = new wxButton(this, wxID_OK, wxT("Apply"));
    cancel_button = new wxButton(this, wxID_CANCEL, wxT("Cancel"));

	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	button_sizer->Add(apply_button);
	button_sizer->Add(cancel_button);

	input_stream_name_grid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	output_stream_name_grid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    input_stream_grid = new wxGrid(this, INPUT_GRID_ID, wxDefaultPosition, wxDefaultSize);
    output_stream_grid = new wxGrid(this, OUTPUT_GRID_ID, wxDefaultPosition, wxDefaultSize);

	wxGridStringTable *input_stream_grid_base = new wxGridStringTable((int)m_stream_input_count, 8);
	wxGridStringTable *output_stream_grid_base = new wxGridStringTable((int)m_stream_output_count, 8);
	wxGridStringTable *input_name_grid_base = new wxGridStringTable((int)m_stream_input_count, 1);
	wxGridStringTable *output_name_grid_base = new wxGridStringTable((int)m_stream_output_count, 1);

	input_stream_grid->SetTable(input_stream_grid_base, true); // take ownership (delete both grid and base in destructor)
	output_stream_grid->SetTable(output_stream_grid_base, true);
	input_stream_name_grid->SetTable(input_name_grid_base, true);
	output_stream_name_grid->SetTable(output_name_grid_base, true);

	input_stream_grid->EnableGridLines();
	input_stream_name_grid->EnableGridLines();
	input_stream_grid->SetRowLabelSize(0);
	input_stream_name_grid->SetRowLabelSize(0);
	input_stream_grid->SetColLabelSize(0);
	input_stream_name_grid->SetColLabelSize(0);
	input_stream_grid->SetColSize(0, 130);
	input_stream_name_grid->SetColSize(0, 130);
	input_stream_name_grid->SetDefaultRowSize(25);

	output_stream_grid->EnableGridLines();
	output_stream_name_grid->EnableGridLines();
	output_stream_grid->SetRowLabelSize(0);
	output_stream_name_grid->SetRowLabelSize(0);
	output_stream_grid->SetColLabelSize(0);
	output_stream_name_grid->SetColLabelSize(0);
	output_stream_grid->SetColSize(0, 130);
	output_stream_name_grid->SetColSize(0, 130);
	output_stream_name_grid->SetDefaultRowSize(25);

    wxBoxSizer *input_stream_details_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *output_stream_details_sizer = new wxBoxSizer(wxHORIZONTAL);
	input_channel_count_choice_sizer = new wxBoxSizer(wxVERTICAL);
	output_channel_count_choice_sizer = new wxBoxSizer(wxVERTICAL);

    input_stream_details_sizer->Add(input_stream_name_grid);
	input_stream_details_sizer->Add(input_channel_count_choice_sizer);
	input_stream_details_sizer->Add(input_stream_grid);
	output_stream_details_sizer->Add(output_stream_name_grid);
	output_stream_details_sizer->Add(output_channel_count_choice_sizer);
	output_stream_details_sizer->Add(output_stream_grid);

	wxStaticBoxSizer *input_stream_label_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "Input Stream Details");
	input_stream_label_sizer->Add(input_stream_header_sizer);
	input_stream_label_sizer->Add(input_stream_details_sizer);

	wxStaticBoxSizer *output_stream_label_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "Output Stream Details");
	output_stream_label_sizer->Add(output_stream_header_sizer);
	output_stream_label_sizer->Add(output_stream_details_sizer);

    dialog_sizer = new wxBoxSizer(wxVERTICAL);
    dialog_sizer->Add(end_station_details_sizer);
	dialog_sizer->Add(input_stream_label_sizer);
	dialog_sizer->Add(output_stream_label_sizer);
	dialog_sizer->Add(button_sizer);
}

unsigned int end_station_details::index_to_channel_count(unsigned int index)
{
    switch(index)
    {
        case 0:
            return 1;
        case 1:
            return 2;
		case 2:
			return 4;
        case 3:
            return 8;
        default:
            return 0;
    }
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
    
    m_end_station_config->SetEntityName();
    m_end_station_config->SetSamplingRate();
    m_end_station_config->SetClockSource();

	for (unsigned int i = 0; i < m_stream_input_count; i++)
	{
		struct stream_configuration_details input_stream_details;
		struct audio_mapping input_audio_mapping;
		unsigned int channel_count;

		input_stream_details.stream_name = input_stream_name_grid->GetCellValue(i, 0);
		channel_count = index_to_channel_count(input_channel_counts.at(i)->GetSelection());
		input_stream_details.channel_count = channel_count;
		m_stream_config->dialog_input_stream_config.push_back(input_stream_details);

		if (!input_stream_grid->IsRowShown(i))
		{
			//skip media clock
		}
		else
		{
			for (unsigned int j = 0; j < 8; j++)
			{
				wxString ret = input_stream_grid->GetCellValue(i, j);
				if (wxAtoi(ret) == 0)
				{
					//skip
				}
				else if (input_stream_grid->GetCellBackgroundColour(i, j) == my_red)
				{
					std::cout << "entered invalid cluster" << std::endl;
				}
				else
				{
					ConvertClusterOffsetToAvdecc(wxAtoi(input_stream_grid->GetCellValue(i, j)),
						input_audio_mapping.cluster_offset, "STREAM_INPUT");
					input_audio_mapping.cluster_channel = 0; //cluster only has 1 channel
					input_audio_mapping.stream_index = (uint16_t)i;
					input_audio_mapping.stream_channel = j;
					m_stream_config->dialog_stream_port_input_audio_mappings.push_back(input_audio_mapping);
				}
			}
		}
	}
               
    for(unsigned int i = 0; i < m_stream_output_count; i++)
    {
        struct stream_configuration_details output_stream_details;
        struct audio_mapping output_audio_mapping;
        unsigned int channel_count;

        output_stream_details.stream_name = output_stream_name_grid->GetCellValue(i, 0);
        channel_count = index_to_channel_count(output_channel_counts.at(i)->GetSelection());
        output_stream_details.channel_count = channel_count;
        m_stream_config->dialog_output_stream_config.push_back(output_stream_details);
        
		if (!output_stream_grid->IsRowShown(i))
		{
			//skip media clock
		}
		else
		{
			for (unsigned int j = 0; j < 8; j++)
			{
				wxString ret = output_stream_grid->GetCellValue(i, j);
				if (wxAtoi(ret) == 0)
				{
					//skip
				}
				else if (output_stream_grid->GetCellBackgroundColour(i, j) == my_red)
				{
					std::cout << "entered invalid cluster" << std::endl;
				}
				else
				{
					ConvertClusterOffsetToAvdecc(wxAtoi(output_stream_grid->GetCellValue(i, j)),
						output_audio_mapping.cluster_offset, "STREAM_OUTPUT");
					output_audio_mapping.cluster_channel = 0; //cluster only has 1 channel
					output_audio_mapping.stream_index = (uint16_t)i;
					output_audio_mapping.stream_channel = j;
					m_stream_config->dialog_stream_port_output_audio_mappings.push_back(output_audio_mapping);
				}
			}
		}
    }

	m_stream_config->SetStreamInfo();
	m_stream_config->SetAudioMappings();

}

void end_station_details::OnGridChange(wxGridEvent &event)
{
    int selected_row, selected_col, id;
    std::cout << "change" << std::endl;

    id = event.GetId();
    selected_row = event.GetRow();
    selected_col = event.GetCol();

	int num_input_rows = input_stream_grid->GetNumberRows();
	int num_input_cols = input_stream_grid->GetNumberCols();

	if (id == INPUT_GRID_ID)
	{
		std::set<int> cluster_offsets;

		int val = wxAtoi(input_stream_grid->GetCellValue(selected_row, selected_col));

		if (val == 0)
		{
			input_stream_grid->SetCellBackgroundColour(selected_row, selected_col, *wxWHITE);
			input_stream_grid->SetCellValue(selected_row, selected_col, wxT("0"));
		}
		else if (val > (int) m_input_cluster_count)
		{
			input_stream_grid->SetCellBackgroundColour(selected_row, selected_col, my_red);
		}
		else
		{
			for (int row = 0; row < num_input_rows; row++)
			{
				for (int col = 0; col < num_input_cols; col++)
				{
					if (wxAtoi(input_stream_grid->GetCellValue(row, col)) != 0)
					{
						int val = wxAtoi(input_stream_grid->GetCellValue(row, col));

						std::set<int>::iterator it = cluster_offsets.find(val);
						if (it == cluster_offsets.end() && val <= (int)m_input_cluster_count)
						{
							cluster_offsets.insert(val);
							input_stream_grid->SetCellBackgroundColour(row, col, my_green);
						}
						else
						{
							input_stream_grid->SetCellBackgroundColour(row, col, my_red);
						}
					}
				}
			}
		}
	}
    else if(id == OUTPUT_GRID_ID)
    {
		int val = wxAtoi(output_stream_grid->GetCellValue(selected_row, selected_col));

		if (val == 0)
		{
			output_stream_grid->SetCellBackgroundColour(selected_row, selected_col, *wxWHITE);
			output_stream_grid->SetCellValue(selected_row, selected_col, wxT("0"));
		}
        else if(val > (int) m_output_cluster_count)
        {
			output_stream_grid->SetCellBackgroundColour(selected_row, selected_col, my_red);
        }
        else
        {
			output_stream_grid->SetCellBackgroundColour(selected_row, selected_col, my_green);
        }
    }
    else
    {
        //unsupported
    }

	input_stream_grid->ForceRefresh();
	output_stream_grid->ForceRefresh();

	CheckValid();
}

int end_station_details::ConvertClusterOffsetToAvdecc(uint16_t user_cluster_offset, uint16_t &avdecc_cluster_offset, wxString stream_type)
{
    if(stream_type.IsSameAs("STREAM_INPUT"))
    {
        if(user_cluster_offset > 0)
        {
            avdecc_cluster_offset = user_cluster_offset - 1; //avdecc mappings start at index 0
        }
        else
        {
            std::cout << "user offset out of bounds" << std::endl;
        }
    }
    else
    {
        if(user_cluster_offset > 0)
        {
            avdecc_cluster_offset = user_cluster_offset + m_input_cluster_count - 1;
        }
        else
        {
            std::cout << "user offset out of bounds" << std::endl;
        }
    }
    
    return 0;
}

void end_station_details::CheckValid()
{
	invalid = false;
	int num_input_rows = output_stream_grid->GetNumberRows();
	int num_input_cols = output_stream_grid->GetNumberCols();
	int num_output_rows = output_stream_grid->GetNumberRows();
	int num_output_cols = output_stream_grid->GetNumberCols();

	for (int row = 0; row < num_input_rows; row++)
	{
		for (int col = 0; col < num_input_cols; col++)
		{
			if (input_stream_grid->GetCellBackgroundColour(row, col) == my_red)
			{
				invalid = true;
			}
		}
	}

	for (int row = 0; row < num_output_rows; row++)
	{
		for (int col = 0; col < num_output_cols; col++)
		{
			if (output_stream_grid->GetCellBackgroundColour(row, col) == my_red)
			{
				invalid = true;
			}
		}
	}

	if (invalid)
	{
		apply_button->Disable();
	}
	else
	{
		apply_button->Enable();
	}
}

void end_station_details::OnChannelChange(wxCommandEvent &event)
{
    UpdateChannelCount();
}
