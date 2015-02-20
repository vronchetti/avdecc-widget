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

/**
 * end_station_details.cpp
 *
 */

#include "end_station_details.h"

wxBEGIN_EVENT_TABLE(end_station_details, wxFrame)
    EVT_BUTTON(10, end_station_details::OnApply)
wxEND_EVENT_TABLE()

end_station_details::end_station_details()
{

   

    EndStation_Details_Dialog = new wxFrame(NULL, wxID_ANY, wxT("End Station Configuration"),
                                            wxDefaultPosition,
                                            wxSize(500,750));
    
    
    
    EndStation_Details_Dialog->Show();
}

end_station_details::~end_station_details()
{
    //delete button;
    delete input_stream_grid;
    delete output_stream_grid;
}

void end_station_details::CreateEndStationDetailsPanel(wxString Default_Name, wxString Init_Sampling_Rate,
                                                       wxString Entity_ID, wxString Mac, wxString fw_version)
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

    name = new wxTextCtrl(EndStation_Details_Dialog, wxID_ANY, "", wxDefaultPosition, wxSize(150,25));
    Sizer1->Add(name);
    
    default_name = new wxTextCtrl(EndStation_Details_Dialog, wxID_ANY, Default_Name,
                                  wxDefaultPosition, wxSize(150,25), wxTE_READONLY);
    default_name->SetBackgroundColour(*wxLIGHT_GREY);
    Sizer2->Add(default_name);
    
    wxArrayString str;
    str.Add("48000 Hz");
    str.Add("96000 Hz");
    
    sampling_rate = new wxChoice(EndStation_Details_Dialog, wxID_ANY, wxDefaultPosition, wxSize(150,25), str);
    Sizer3->Add(sampling_rate);
    
    entity_id = new wxTextCtrl(EndStation_Details_Dialog, wxID_ANY, Entity_ID, wxDefaultPosition, wxSize(150,25));
    entity_id->SetBackgroundColour(*wxLIGHT_GREY);
    Sizer4->Add(entity_id);
    
    mac = new wxTextCtrl(EndStation_Details_Dialog, wxID_ANY, Mac,
                         wxDefaultPosition, wxSize(150,25), wxTE_READONLY);
    mac->SetBackgroundColour(*wxLIGHT_GREY);
    Sizer5->Add(mac);
    
    fw_ver = new wxTextCtrl(EndStation_Details_Dialog, wxID_ANY, fw_version,
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
}

void end_station_details::SetChannelChoice(unsigned int stream_input_count, unsigned int stream_output_count)
                                            
{
    wxArrayString str;
    str.Add("1-Channel");
    str.Add("2-Channel");
    str.Add("8-Channel");
    
    for(unsigned int j = 0; j < stream_input_count; j++)
    {
        wxGridCellChoiceEditor *channel_choice = new wxGridCellChoiceEditor(str);
        input_stream_grid->SetCellEditor(j, 1, channel_choice);
    }
    
    for(unsigned int j = 0; j < stream_output_count; j++)
    {
        output_stream_grid->SetCellEditor(j, 1, new wxGridCellChoiceEditor(str));
        output_stream_grid->SetCellValue(j, 1, "1-Channel");
    }
}

void end_station_details::SetInputChannelName(unsigned int stream_index, wxString name)
{
    input_stream_grid->SetCellValue(stream_index, 0, name);
}

void end_station_details::SetOutputChannelName(unsigned int stream_index, wxString name)
{
    output_stream_grid->SetCellValue(stream_index, 0, name);
}

void end_station_details::SetInputChannelCount(unsigned int stream_index, unsigned int channel_count,
                                               unsigned int stream_input_count)
{
    input_stream_grid->SetCellValue(stream_index, 1, wxString::Format("%u-Channel", channel_count));
    
    for(unsigned int j = 0; j < stream_input_count; j++)
    {
        input_stream_grid->SetRowSize(j, 25);
        
        for(unsigned int j = 2; j < 10; j++)
        {
            input_stream_grid->SetColSize(j, 25);
        }
        
        if(input_stream_grid->GetCellValue(j, 1) == "1-Channel")
        {
            for(unsigned int k = 3; k < 10; k++)
            {
                input_stream_grid->SetReadOnly(j, k);
                input_stream_grid->SetCellBackgroundColour(j, k, *wxLIGHT_GREY);
            }
        }
        else if(input_stream_grid->GetCellValue(j, 1) == "2-Channel")
        {
            input_stream_grid->SetCellBackgroundColour(2, 0, *wxWHITE);
            
            for(unsigned int k = 3; k < 10; k++)
            {
                input_stream_grid->SetReadOnly(j, k);
                input_stream_grid->SetCellBackgroundColour(j, k, *wxLIGHT_GREY);
            }
        }
        else
        {
            //8 channel
        }
    }
}

void end_station_details::SetOutputChannelCount(unsigned int stream_index, unsigned int channel_count,
                                                unsigned int stream_output_count)
{
    output_stream_grid->SetCellValue(stream_index, 1, wxString::Format("%u-Channel", channel_count));
    
    for(unsigned int j = 0; j < stream_output_count; j++)
    {
        output_stream_grid->SetRowSize(j, 25);
        
        for(unsigned int j = 2; j < 10; j++)
        {
            output_stream_grid->SetColSize(j, 25);
        }
        
        if(output_stream_grid->GetCellValue(j, 1) == "1-Channel")
        {
            for(unsigned int k = 3; k < 10; k++)
            {
                output_stream_grid->SetReadOnly(j, k);
                output_stream_grid->SetCellBackgroundColour(j, k, *wxLIGHT_GREY);
            }
        }
        else if(output_stream_grid->GetCellValue(j, 1) == "2-Channel")
        {
            output_stream_grid->SetCellBackgroundColour(2, 0, *wxWHITE);
            
            for(unsigned int k = 3; k < 10; k++)
            {
                output_stream_grid->SetReadOnly(j, k);
                output_stream_grid->SetCellBackgroundColour(j, k, *wxLIGHT_GREY);
            }
        }
        else
        {
            //8 channel
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

void end_station_details::CreateAndSizeGrid(unsigned int stream_input_count, unsigned int stream_output_count)
{
    wxButton * button_test = new wxButton(EndStation_Details_Dialog, 10, wxT("Apply"));
    button_test->Connect(
                         wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(end_station_details::OnApply),
                         NULL, EndStation_Details_Dialog);
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

    grid_base = new wxGridStringTable(stream_input_count, 10);
    grid_base2 = new wxGridStringTable(stream_output_count, 10);

    input_stream_grid->SetTable(grid_base);
    output_stream_grid->SetTable(grid_base2);
    SetChannelChoice(stream_input_count, stream_output_count);

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(Details_Sizer);
    Input_Stream_Sizer->Add(input_stream_header_sizer);
    Input_Stream_Sizer->Add(input_stream_grid);
    sizer->Add(Input_Stream_Sizer);
    
    Output_Stream_Sizer->Add(output_stream_header_sizer);
    Output_Stream_Sizer->Add(output_stream_grid);
    sizer->Add(Output_Stream_Sizer);
    sizer->Add(button_test);

    EndStation_Details_Dialog->SetSizer(sizer, true);
    
    input_stream_grid->SetColSize(0, 130);
    output_stream_grid->SetColSize(0, 130);
    
}

void end_station_details::OnApply(wxCommandEvent& event)
{
    std::cout << "working";
}
