/*
* Copyright (c) 2015 AudioScience Inc.
*
* avdecc_stream_connections.h
* This module implements an AVB Stream Connection Grid.
*
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
#include "wx/tglbtn.h"
#include "wx/msgdlg.h"
#include "wx/textctrl.h"
#include "wx/dc.h"
#include "wx/defs.h"
#include "wx/wx.h"
#include "wx/icon.h"
#include "wx/timer.h"
#include "wx/scrolwin.h"
#include "wx/event.h"
#include "wx/colordlg.h"
#include "wx/numdlg.h"
#include "wx/htmllbox.h"
#include "wx/grid.h"
#include "wx/popupwin.h"
#include "wx/spinctrl.h"

#include <stdint.h>
#include <iostream>
#include <vector>
#include <wx/string.h>
#include <assert.h>
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

struct connection_state
{
	uint64_t stream_id;
	uint64_t stream_dest_mac;
	uint16_t connection_count;
	uint16_t stream_vlan_id;
};

struct connection_state_details
{
	wxString name;
	wxString type;
	int entity_index;
	int stream_index;
	struct connection_state connection_details;
};

struct end_station_stream_names
{
	wxString entity_name;
	std::vector<wxString> stream_names;
};

struct stream_index_identifiers
{
	int entity_index;
	std::vector<int> stream_indexes;
};

struct all_indexes
{
	int entity_index;
	int stream_index;
};

class avdecc_stream_connects_scrolled_canvas;

/*
* Parent frame, which passes the avdecc-lib objects
* to the child class, a derived wxScrolled window.
*/
class avdecc_stream_connections_frame : public wxFrame
{
public:
	avdecc_stream_connections_frame(wxWindow *parent,
		avdecc_lib::controller *controller,
		avdecc_lib::system *sys);
	virtual ~avdecc_stream_connections_frame();

	void OnClose(wxCloseEvent& event);

private:
	avdecc_stream_connects_scrolled_canvas * canvas;
	wxDECLARE_EVENT_TABLE();
};

/*
* This class includes all necessary methods for connecting
* and disconnect streams.
*/

class MySimplePopup;

class avdecc_stream_connects_scrolled_canvas : public wxScrolled<wxWindow>
{
public:
	avdecc_stream_connects_scrolled_canvas(wxWindow * parent,
		avdecc_lib::controller *controller,
		avdecc_lib::system *sys);

	virtual ~avdecc_stream_connects_scrolled_canvas();

	//Events
	void OnRefreshConnectionStates(wxCommandEvent& event);
	void OnToggle(wxCommandEvent& event);
	void OnGridCellClick(wxGridEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);

private:
	size_t m_end_station_count;
	size_t end_stations_shown_count;
	int num_cols_shown;
	int num_rows_shown;

	//custom colors
	const wxColour my_azure = wxColour(240, 255, 255);
	const wxColour my_silver = wxColour(156, 156, 156);
	const wxColour my_dark_grey = wxColour(97, 97, 97);

	//avdecc-lib local objects
	avdecc_lib::controller *m_controller_obj;
	avdecc_lib::end_station *m_end_station;
	avdecc_lib::system *m_sys;

	MySimplePopup * last_popup = NULL;
	MySimplePopup * tooltip_popup;
	int last_header_label_row;
	int last_header_label_col;

	//sizers
	wxBoxSizer * input_button_sizer;
	wxBoxSizer * output_button_sizer;
	wxBoxSizer * dialog_sizer;
	wxBoxSizer * main_sizer;
	wxBoxSizer *connection_sizer;

	//grid objects
	wxGrid * connection_grid;
	wxGridTableBase * connection_grid_base;

	//Control Containers
	std::vector<wxToggleButton *> input_expand_buttons;
	std::vector<wxToggleButton *> output_expand_buttons;
	std::vector<wxBoxSizer *> expand_button_sizers;
	std::vector<wxBoxSizer *> expand_output_button_sizers;

	//A vector of entity ids for finding end stations 
	std::vector<uint64_t> entity_ids;

	//Stored end station and stream names
	std::vector<struct end_station_stream_names> stream_input_names;
	std::vector<struct end_station_stream_names> stream_output_names;

	//Stored name indexes for grid access
	std::vector<int> stream_name_indexes;
	std::vector<struct stream_index_identifiers> grid_stream_indexes;
	std::vector<struct all_indexes> names_and_streams;

	//Stored rx/tx details
	std::vector<struct connection_state_details> all_tx_details;
	std::vector<struct connection_state_details> all_rx_details;

	void GetStreamInputNames();
	void GetStreamOutputNames();
	void StoreNameIndexes();
	void InitializeConnectionGrid(int num_rows_cols);
	void CreateAndStoreToggleButtons();
	void InitSizing();

	int AvdeccConnect(uint64_t talker_entity_id, int talker_stream_index,
		uint64_t listener_entity_id, int listener_stream_index);
	int AvdeccDisconnect(uint64_t talker_entity_id, int talker_stream_index,
		uint64_t listener_entity_id, int listener_stream_index);

	int ShowConnections();

	int get_current_entity_and_descriptor(avdecc_lib::end_station *end_station,
		avdecc_lib::entity_descriptor **entity,
		avdecc_lib::configuration_descriptor **configuration);

	wxDECLARE_EVENT_TABLE();
};

enum
{
	INIT_CANVAS_WIDTH = 750,
	INIT_CANVAS_HEIGHT = 750,
	CONNECTION_GRID_ID,
	TOGGLE_ID,
	REFRESH_ID,
	GRID_BUTTON = 1,
	CONNECTION_SUCCESS = 2,
	CONNECTION_FAILURE = 3,
	CONNECTION_WARNING = 4,
	INPUT_CELL_WIDTH = 150,
	INPUT_CELL_HEIGHT = 25,
	OUTPUT_CELL_WIDTH = 25,
	OUTPUT_CELL_HEIGHT = 150,
	TOP_LEFT_BOX_OFFSET = 175
};

/*
* This class draws custom wxGrid Cells:
*	GRID_BUTTON: A Blank cell button.
*	CONNECTION_SUCCESS: a green circle indicating connected streams.
*	CONNECTION_FAILURE: a red 'X' indicating a connection failure.
*	CONNECTION_WARNING: a yellow circle, which indicates (for now) a stream format mismatch
*/
class avdecc_connection_grid_renderer : public wxGridCellStringRenderer
{
public:
	avdecc_connection_grid_renderer(int type);

	virtual void Draw(wxGrid& grid,
		wxGridCellAttr& attr,
		wxDC& dc,
		const wxRect& rect,
		int row, int col,
		bool isSelected);
private:
	int m_type;
};

/*
* This class implements a tooltip, providing useful information
* about an end station or stream.
*/
class MySimplePopup : public wxPopupWindow
{
public:
	MySimplePopup(wxWindow *parent, int x, int y, struct connection_state_details &details, bool found);
	virtual ~MySimplePopup();

	bool timeout = false;
	void OnTimer(wxTimerEvent& event);

private:
	avdecc_stream_connects_scrolled_canvas * m_parent_canvas;

	unsigned int timeout_counter = 0;
	wxStaticText *text;
	wxTimer m_timer;
	wxDECLARE_EVENT_TABLE();
};

