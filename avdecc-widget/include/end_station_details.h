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
 * end_station_details.h
 */

#include "wx/wxprec.h"
#include "wx/app.h"
#include "wx/frame.h"
#include "wx/log.h"
#include "wx/textdlg.h"
#include "wx/sizer.h"
#include "wx/button.h"
#include "wx/notebook.h"
#include "wx/statbox.h"
#include "wx/stattext.h"
#include "wx/listctrl.h"
#include "wx/choice.h"
#include "wx/menu.h"
#include "wx/msgdlg.h"
#include "wx/textctrl.h"
#include "wx/dc.h"
#include "wx/icon.h"
#include "wx/timer.h"
#include "wx/scrolwin.h"
#include "wx/event.h"
#include "wx/colordlg.h"
#include "wx/numdlg.h"
#include "wx/htmllbox.h"
#include "wx/grid.h"

#include "end_station_configuration.h"
#include "stream_configuration.h"

class end_station_details : public wxFrame
{
public:
    end_station_details(wxWindow *parent, end_station_configuration *config, stream_configuration *stream_config);
    virtual ~end_station_details();

    uint32_t m_sampling_rate;
    unsigned int m_stream_input_count;
    unsigned int m_stream_output_count;
    unsigned int m_input_maps_count;
    unsigned int m_output_maps_count;

    //public dialog return methods
    void OnOK();
    void OnCancel();
    int ShowModal();

    stream_configuration * m_stream_config;
    end_station_configuration * m_end_station_config;

private:
    //Dialog object
    wxDialog *EndStation_Details_Dialog;
    
    //End Station Details objects
    wxTextCtrl *name;
    wxTextCtrl *default_name;
    wxChoice *sampling_rate;
    wxTextCtrl *entity_id;
    wxTextCtrl *mac;
    wxTextCtrl *fw_ver;
    
    wxButton * apply_button;
    wxButton * cancel_button;

    wxGrid * input_stream_grid;
    wxGrid * output_stream_grid;
    wxGridCellChoiceEditor *input_channel_choice;
    wxGridCellChoiceEditor *output_channel_choice;
    wxGridStringTable *grid_base;
    wxGridStringTable *grid_base2;
    
    //End Station Details member variables
    wxString m_mac;
    wxString m_default_name;
    wxString m_entity_id;
    wxString m_fw_ver;
    wxString m_entity_name;
    uint64_t channel_count;

    //End Station Details sizers
    wxStaticBoxSizer *Details_Sizer;
    wxStaticBoxSizer *Input_Stream_Sizer;
    wxBoxSizer * input_stream_header_sizer;
    wxStaticBoxSizer *Output_Stream_Sizer;
    wxBoxSizer * output_stream_header_sizer;
    
    //Class methods
    void CreateEndStationDetailsPanel(wxString Entity_Name, wxString Default_Name,
                                      uint32_t Init_Sampling_Rate, wxString Entity_ID,
                                      wxString Mac, wxString fw_ver);
    
    void CreateAndSizeGrid(unsigned int stream_input_count, unsigned int stream_output_count);
    void OnGridCellChange(wxGridEvent& event);
    void SetChannelChoice(unsigned int stream_input_count, unsigned int stream_output_count);
    void SetInputChannelCount(unsigned int stream_index, unsigned int channel_count,
                              unsigned int stream_input_count);
    void SetOutputChannelCount(unsigned int stream_index, unsigned int channel_count,
                               unsigned int stream_input_count);
    
    void SetInputMappings(struct audio_mapping &map);
    void SetOutputMappings(struct audio_mapping &map);
    
    void CreateInputStreamGridHeader();
    void CreateOutputStreamGridHeader();
    void SetInputChannelName(unsigned int stream_index, wxString name);
    void SetOutputChannelName(unsigned int stream_index, wxString name);
};
