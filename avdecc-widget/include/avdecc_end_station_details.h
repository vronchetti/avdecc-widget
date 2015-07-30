/*
* Copyright (c) 2015 AudioScience Inc.
*/

/**
* end_station_details.h
*/

#include <set>

#include <wx/aui/framemanager.h>
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

#include "avdecc_end_station_configuration.h"
#include "avdecc_stream_configuration.h"

class avdecc_end_station_details : public wxDialog
{
public:
	avdecc_end_station_details(wxWindow *parent, avdecc_end_station_configuration *end_station_config, avdecc_stream_configuration *stream_config);
	virtual ~avdecc_end_station_details();

	//events
	void OnChannelChange(wxCommandEvent& event);
	void OnGridChange(wxGridEvent& event);

	//public dialog return methods
	void OnOK();

	//copied class object pointers
	avdecc_stream_configuration * m_stream_config;
	avdecc_end_station_configuration * m_end_station_config;

private:
	//boolean to determine whether apply button should be disabled
	bool invalid;

	//dialog buttons
	wxButton * apply_button;
	wxButton * cancel_button;

	//custom colors
	const wxColour my_green = wxColour(180, 255, 170);
	const wxColour my_red = wxColour(255, 100, 100);
	const wxColour my_grey = wxColour(225, 225, 225);

	//end station config objects
	wxTextCtrl *name;
	wxTextCtrl *default_name;
	wxChoice *sampling_rate;
	wxTextCtrl *entity_id;
	wxTextCtrl *mac;
	wxTextCtrl *fw_ver;
	wxChoice *clock_source;

	//stream config objects
	wxGrid * input_stream_grid;
	wxGrid * input_stream_name_grid;
	wxGrid * output_stream_grid;
	wxGrid * output_stream_name_grid;
	std::vector<wxChoice *> input_channel_counts;
	std::vector<wxChoice *> output_channel_counts;

	//end station config data
	wxString m_mac;
	wxString m_default_name;
	wxString m_entity_id;
	wxString m_fw_ver;
	wxString m_entity_name;
	uint32_t m_sampling_rate;
	uint16_t m_current_clk_source;
	std::vector<wxString> m_clock_source_descriptions;

	//stream config data
	size_t m_stream_input_count;
	size_t m_stream_output_count;
	size_t m_input_cluster_count;
	size_t  m_output_cluster_count;
	unsigned int m_input_maps_count;
	unsigned int m_output_maps_count;

	//class sizers required globally
	wxStaticBoxSizer *avdecc_end_station_details_sizer;
	wxBoxSizer * input_stream_header_sizer;
	wxBoxSizer * output_stream_header_sizer;
	wxBoxSizer * input_channel_count_choice_sizer;
	wxBoxSizer * output_channel_count_choice_sizer;
	wxBoxSizer *dialog_sizer;

	//Class methods
	void CreateAndSizeEndStationDetailsList();
	void CreateAndSizeStreamGrids();
	void GetEndStationData();
	void GetStreamData();
	void SetStreamDetails();
	void CreateChannelChoices();
	void SetInputChannelCount(unsigned int stream_index, size_t channel_count);
	void SetOutputChannelCount(unsigned int stream_index, size_t channel_count);
	void SetInputMappings(struct audio_mapping &map);
	void SetOutputMappings(struct audio_mapping &map);
	void CreateInputStreamGridHeader();
	void CreateOutputStreamGridHeader();
	void SetInputChannelName(unsigned int stream_index, wxString name);
	void SetOutputChannelName(unsigned int stream_index, wxString name);
	void UpdateChannelCount();
	void CheckInputStreamGridChanges();
	void CheckValid();
	int ConvertClusterOffsetToAvdecc(uint16_t user_cluster_offset, uint16_t &avdecc_cluster_offset, wxString stream_type);
	unsigned int index_to_channel_count(unsigned int index);
	wxDECLARE_EVENT_TABLE();
};

enum
{
	DetailsTimer,
	INPUT_GRID_ID,
	OUTPUT_GRID_ID,
	CHANNEL_CHANGE
};

