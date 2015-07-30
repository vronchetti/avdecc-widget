/*
* Copyright (c) 2015 AudioScience Inc.
*/

/**
* avdecc_stream_connections.cpp
*/

#include "avdecc_stream_connections.h"

wxBEGIN_EVENT_TABLE(avdecc_stream_connections_frame, wxFrame)
EVT_CLOSE(avdecc_stream_connections_frame::OnClose)
wxEND_EVENT_TABLE()

avdecc_stream_connections_frame::avdecc_stream_connections_frame(wxWindow *parent, avdecc_lib::controller *controller, avdecc_lib::system *sys)
: wxFrame(parent, wxID_ANY, wxT("Stream Connections"), wxDefaultPosition, wxDefaultSize)
{
	canvas = new avdecc_stream_connects_scrolled_canvas(this, controller, sys);
	SetClientSize(INIT_CANVAS_WIDTH / 2, INIT_CANVAS_HEIGHT / 2);
	Show();
}

avdecc_stream_connections_frame::~avdecc_stream_connections_frame() {}

void avdecc_stream_connections_frame::OnClose(wxCloseEvent& event)
{
	this->Destroy();
}

wxBEGIN_EVENT_TABLE(avdecc_stream_connects_scrolled_canvas, wxScrolled)
EVT_BUTTON(REFRESH_ID, avdecc_stream_connects_scrolled_canvas::OnRefreshConnectionStates)
EVT_TOGGLEBUTTON(wxID_ANY, avdecc_stream_connects_scrolled_canvas::OnToggle)
EVT_GRID_CELL_LEFT_CLICK(avdecc_stream_connects_scrolled_canvas::OnGridCellClick)
EVT_MOTION(avdecc_stream_connects_scrolled_canvas::OnMouseMove)
EVT_PAINT(avdecc_stream_connects_scrolled_canvas::OnPaint)
wxEND_EVENT_TABLE()

avdecc_stream_connects_scrolled_canvas::avdecc_stream_connects_scrolled_canvas(wxWindow *parent,
avdecc_lib::controller *controller,
avdecc_lib::system *sys)
: wxScrolled<wxWindow>(parent, wxID_ANY)
{
	//Scrolled window initial settings
	SetScrollRate(10, 10);
	SetVirtualSize(INIT_CANVAS_WIDTH, INIT_CANVAS_HEIGHT);
	SetBackgroundColour(*wxLIGHT_GREY);

	//Copy avdecc-lib objects
	m_controller_obj = controller;
	m_sys = sys;

	//Initialize Globals
	m_end_station_count = m_controller_obj->get_end_station_count();
	end_stations_shown_count = 0;
	num_rows_shown = 0;
	num_cols_shown = 0;

	//Get Stream names, store indexes
	GetStreamInputNames();
	GetStreamOutputNames();
	StoreNameIndexes();

	//Fill Grid with connections
	ShowConnections();

	//Setup buttons for minimizing grid
	CreateAndStoreToggleButtons();
	InitSizing();
}

avdecc_stream_connects_scrolled_canvas::~avdecc_stream_connects_scrolled_canvas() {}

/*
* This function loops through the vector of stored
* end station and stream names, assigning and storing
* an index for each end station name/stream name pair
* to be used to access the Grid.  The total
* number of names is calculated to create the wxGrid.
*
*/
void avdecc_stream_connects_scrolled_canvas::StoreNameIndexes()
{
	int total_streams = 0;
	int total_rows_cols = 0;
	for (size_t i = 0; i < stream_input_names.size(); i++)
	{
		struct end_station_stream_names names = stream_input_names.at(i);
		total_streams += names.stream_names.size();
		struct all_indexes entity;
		entity.entity_index = i;
		entity.stream_index = -1;
		names_and_streams.push_back(entity);

		for (size_t j = 0; j < names.stream_names.size(); j++)
		{
			struct all_indexes indexes;
			indexes.entity_index = i;
			indexes.stream_index = j;
			names_and_streams.push_back(indexes);
		}
	}

	total_rows_cols = total_streams + (int)stream_input_names.size();
	InitializeConnectionGrid(total_rows_cols); //create grid
}

/*
* This function creates and initializes the connection grid.
* Initial conditions are showing only the end station names
* (hiding the stream names).  Blank connection buttons are
* rendered in the applicable cell connection location.
*
* Each Grid cell is assigned a value to keep track of its status:
*	GRID_BUTTON = 1,
*	CONNECTION_SUCCESS = 2,
*	CONNECTION_FAILURE = 3,
*
*
* See OnGridCellClick()
*/
void avdecc_stream_connects_scrolled_canvas::InitializeConnectionGrid(int num_rows_cols)
{
	connection_grid = new wxGrid(this, CONNECTION_GRID_ID,
		wxDefaultPosition, wxDefaultSize);
	//We want to pick up mouse events
	connection_grid->GetGridWindow()->Connect(wxID_ANY, wxEVT_MOTION,
		wxMouseEventHandler(avdecc_stream_connects_scrolled_canvas::OnMouseMove),
		NULL, this);
	connection_grid_base = new wxGridStringTable((int)num_rows_cols, num_rows_cols);
	connection_grid->SetTable(connection_grid_base, true);
	connection_grid->SetDefaultColSize(INPUT_CELL_HEIGHT, true);
	connection_grid->SetDefaultRowSize(OUTPUT_CELL_WIDTH, true);
	connection_grid->SetDefaultCellTextColour(*wxWHITE);
	connection_grid->HideRowLabels();
	connection_grid->HideColLabels();
	connection_grid->EnableGridLines(false);
	connection_grid->SetCellHighlightPenWidth(0);

	int index = 0;
	for (size_t it = 0; it < stream_input_names.size(); it++)
	{
		struct stream_index_identifiers indexes;
		struct end_station_stream_names names = stream_input_names.at(it);
		stream_name_indexes.push_back(index);
		indexes.entity_index = it;

		++index;

		for (size_t i = 0; i < names.stream_names.size(); i++)
		{
			indexes.stream_indexes.push_back(index);
			connection_grid->HideRow(index);
			connection_grid->HideCol(index);
			++index;
		}
		grid_stream_indexes.push_back(indexes);
	}

	//draw connection buttons
	for (size_t i = 0; i < grid_stream_indexes.size(); i++)
	{
		struct stream_index_identifiers indexes = grid_stream_indexes.at(i);
		int name_index = indexes.entity_index;

		for (size_t j = 0; j < grid_stream_indexes.size(); j++)
		{
			struct stream_index_identifiers indexes_compare = grid_stream_indexes.at(j);
			int name_index_compare = indexes_compare.entity_index;
			if (name_index_compare != name_index)
			{
				for (size_t k = 0; k < indexes.stream_indexes.size(); k++)
				{
					for (size_t m = 0; m < indexes_compare.stream_indexes.size(); m++)
					{
						int x_coord = indexes.stream_indexes.at(k);
						int y_coord = indexes_compare.stream_indexes.at(m);
						wxString grid_button_type_string;

						connection_grid->SetCellRenderer(x_coord, y_coord,
							new avdecc_connection_grid_renderer(GRID_BUTTON));
						//Assign a value to a cell to keep track of its status
						connection_grid->SetCellValue(x_coord, y_coord,
							grid_button_type_string << GRID_BUTTON);
					}
				}
			}
		}
	}
}

/*
* This function creates and stores toggle buttons
* for hiding and showing an end station's streams.
*
* See OnToggle()
*/
void avdecc_stream_connects_scrolled_canvas::CreateAndStoreToggleButtons()
{
	int id = -1;
	for (size_t i = 0; i < end_stations_shown_count; i++)
	{
		input_expand_buttons.push_back(
			new wxToggleButton(this, ++id, "+", wxDefaultPosition,
			wxSize(OUTPUT_CELL_WIDTH, INPUT_CELL_HEIGHT)));
	}

	for (size_t i = 0; i < end_stations_shown_count; i++)
	{
		output_expand_buttons.push_back(
			new wxToggleButton(this, ++id, "+", wxDefaultPosition,
			wxSize(OUTPUT_CELL_WIDTH, INPUT_CELL_HEIGHT)));
	}
}

void avdecc_stream_connects_scrolled_canvas::InitSizing()
{
	input_button_sizer = new wxBoxSizer(wxVERTICAL);
	output_button_sizer = new wxBoxSizer(wxHORIZONTAL);
	connection_sizer = new wxBoxSizer(wxHORIZONTAL);
	main_sizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer * button_sizer = new wxBoxSizer(wxVERTICAL);
	button_sizer->AddSpacer(63.5);

	output_button_sizer->AddSpacer(25);
	//refresh button currently unimplemented
	wxButton *refresh_button = new wxButton(this, REFRESH_ID, "Refresh Connections",
		wxDefaultPosition, wxSize(150, 25));
	button_sizer->Add(refresh_button);

	for (size_t i = 0; i < end_stations_shown_count; i++)
	{
		input_button_sizer->Add(input_expand_buttons.at(i));

		struct end_station_stream_names input_names = stream_input_names.at(i);
		wxPanel * button_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
			wxSize(25, 25 * (int)input_names.stream_names.size()));
		wxBoxSizer * button_sizer = new wxBoxSizer(wxVERTICAL);
		button_sizer->Add(button_panel);
		input_button_sizer->Add(button_sizer);
		button_sizer->Show(false);
		expand_button_sizers.push_back(button_sizer);
	}

	for (size_t i = 0; i < end_stations_shown_count; i++)
	{
		output_button_sizer->Add(output_expand_buttons.at(i));

		struct end_station_stream_names output_names = stream_output_names.at(i);
		wxPanel * button_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
			wxSize(25 * (int)output_names.stream_names.size(), 25));
		wxBoxSizer * button_sizer = new wxBoxSizer(wxHORIZONTAL);
		button_sizer->Add(button_panel);
		output_button_sizer->Add(button_sizer);
		button_sizer->Show(false);
		expand_output_button_sizers.push_back(button_sizer);
	}

	dialog_sizer = new wxBoxSizer(wxHORIZONTAL);
	dialog_sizer->Add(button_sizer);
	dialog_sizer->Add(main_sizer);
	main_sizer->AddSpacer(150);
	main_sizer->Add(output_button_sizer);
	main_sizer->Add(connection_sizer);
	connection_sizer->Add(input_button_sizer);
	connection_sizer->Add(connection_grid);

	SetSizer(dialog_sizer);
}

/*
* This function draws grid headers.
* It is called whenever Layout()/Refresh() is called, or whenever
* the window is resized or scrolled.
*
* The status of each toggle buttom determines whether or
* not the stream names are painted.  The Layout() call from
* the OnToggle() event triggers a repaint.
*/
void avdecc_stream_connects_scrolled_canvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC paintDC(this);
	wxFont my_bold_font(wxFontInfo(11).FaceName("Helvetica").Bold());
	wxFont my_regular_font(wxFontInfo(11).FaceName("Helvetica"));

	static const wxCoord INPUT_WIDTH = INPUT_CELL_WIDTH;
	static const wxCoord OUTPUT_WIDTH = OUTPUT_CELL_WIDTH;
	static const wxCoord INPUT_HEIGHT = INPUT_CELL_HEIGHT;
	static const wxCoord OUTPUT_HEIGHT = OUTPUT_CELL_HEIGHT;

	PrepareDC(paintDC);

	//Draw top left corner
	wxRect refresh_rect(0, 0, INPUT_WIDTH, OUTPUT_HEIGHT);
	paintDC.SetBrush(my_dark_grey);
	paintDC.SetPen(*wxWHITE);
	paintDC.SetTextForeground(*wxWHITE);
	paintDC.DrawRectangle(refresh_rect);

	//Draw labels
	wxCoord t_x = 55, t_y = 155;
	wxCoord l_x = 155, l_y = 100;

	wxCoord t_rect_x = 0, t_rect_y = OUTPUT_CELL_HEIGHT;
	wxCoord r_rect_x = INPUT_CELL_WIDTH, r_rect_y = 0;
	paintDC.DrawRectangle(t_rect_x, t_rect_y, INPUT_WIDTH, INPUT_HEIGHT);
	paintDC.DrawRectangle(r_rect_x, r_rect_y, OUTPUT_WIDTH, OUTPUT_HEIGHT);
	paintDC.SetPen(*wxWHITE);
	paintDC.DrawText(wxT("Talker"), t_x, t_y);
	paintDC.DrawRotatedText(wxT("Listener"), l_x, l_y, 90); //90 degrees vertical

	int input_index = 0;
	for (size_t it = 0; it < stream_input_names.size(); it++)
	{
		//Draw End Station Names
		wxCoord x = 0, y = TOP_LEFT_BOX_OFFSET + input_index * INPUT_HEIGHT;
		wxCoord x1 = 6, y1 = TOP_LEFT_BOX_OFFSET + (input_index * INPUT_HEIGHT) + 5;
		wxPoint point1(x1, y1);
		struct end_station_stream_names names = stream_output_names.at(it);
		wxRect name_rect(x, y, INPUT_WIDTH, INPUT_HEIGHT);

		if (it % 2)
		{
			paintDC.SetBrush(my_dark_grey);
		}
		else
		{
			paintDC.SetBrush(my_silver);
		}

		paintDC.DrawRectangle(name_rect);
		paintDC.SetFont(my_bold_font);
		wxString max_string_fit = names.entity_name.Mid(0, 12);
		if (paintDC.GetTextExtent(names.entity_name).GetWidth() < INPUT_WIDTH - 23)
		{
			paintDC.DrawText(names.entity_name, point1);
		}
		else
		{
			paintDC.DrawText(max_string_fit << wxT("..."), point1);
		}

		++input_index;

		wxToggleButton * expand_button = input_expand_buttons.at(it);

		//If corresponding toggle button is pressed, draw stream names
		if (expand_button->GetValue())
		{
			for (size_t i = 0; i < names.stream_names.size(); i++)
			{
				wxCoord x = 0, y = TOP_LEFT_BOX_OFFSET + input_index * OUTPUT_CELL_WIDTH;
				wxCoord x2 = 14, y2 = TOP_LEFT_BOX_OFFSET + (input_index * OUTPUT_CELL_WIDTH) + 5;
				wxPoint point2(x2, y2);
				paintDC.DrawRectangle(x, y, INPUT_WIDTH + 23, OUTPUT_HEIGHT);
				paintDC.SetFont(my_regular_font);
				wxString max_string_fit = names.stream_names.at(i).Mid(0, 12);
				if (paintDC.GetTextExtent(names.stream_names.at(i)).GetWidth() < INPUT_WIDTH - 23)
				{
					paintDC.DrawText(names.stream_names.at(i), point2);
				}
				else
				{
					paintDC.DrawText(max_string_fit << wxT("..."), point2);
				}
				++input_index;
			}
		}
	}

	int output_index = 0;
	for (size_t it = 0; it < stream_output_names.size(); it++)
	{
		if (it % 2)
		{
			paintDC.SetBrush(*wxGREY_BRUSH);
		}
		else
		{
			paintDC.SetBrush(my_silver);
		}

		wxCoord x = TOP_LEFT_BOX_OFFSET + output_index * INPUT_CELL_HEIGHT, y = 0;
		wxCoord x1 = TOP_LEFT_BOX_OFFSET + (output_index * INPUT_CELL_HEIGHT) + 5, y1 = 140;
		wxPoint point1(x1, y1);
		struct end_station_stream_names names = stream_input_names.at(it);
		wxRect rect(x, y, OUTPUT_WIDTH, OUTPUT_HEIGHT);
		paintDC.DrawRectangle(rect);
		paintDC.SetFont(my_bold_font);
		wxString max_string_fit = names.entity_name.Mid(0, 12);
		if (paintDC.GetTextExtent(names.entity_name).GetWidth() < OUTPUT_HEIGHT)
		{
			paintDC.DrawRotatedText(names.entity_name, point1, 90);
		}
		else
		{
			paintDC.DrawRotatedText(max_string_fit << wxT("..."), point1, 90);
		}

		++output_index;

		wxToggleButton * expand_button = output_expand_buttons.at(it);

		if (expand_button->GetValue())//is pressed (expanded)
		{
			for (size_t i = 0; i < names.stream_names.size(); i++)
			{
				wxCoord x = TOP_LEFT_BOX_OFFSET + output_index * INPUT_CELL_HEIGHT, y = 0;
				wxCoord x2 = TOP_LEFT_BOX_OFFSET + (output_index * INPUT_CELL_HEIGHT) + 5, y2 = 132;
				wxPoint point2(x2, y2);
				paintDC.DrawRectangle(x, y, OUTPUT_WIDTH, OUTPUT_HEIGHT + 23);
				paintDC.SetFont(my_regular_font);
				wxString max_string_fit = names.stream_names.at(i).Mid(0, 12);
				if (paintDC.GetTextExtent(names.stream_names.at(i)).GetWidth() < OUTPUT_HEIGHT)
				{
					paintDC.DrawRotatedText(names.stream_names.at(i), point2, 90);
				}
				else
				{
					paintDC.DrawRotatedText(max_string_fit << wxT("..."), point2, 90);
				}
				++output_index;
			}
		}
	}
}

void avdecc_stream_connects_scrolled_canvas::OnRefreshConnectionStates(wxCommandEvent& event)
{
	all_tx_details.clear();
	all_rx_details.clear();
	ShowConnections();
}

/*
* Find and store AVDECC end station and stream input names.
*/

void avdecc_stream_connects_scrolled_canvas::GetStreamInputNames()
{
	for (size_t i = 0; i < m_end_station_count; i++)
	{
		m_end_station = m_controller_obj->get_end_station_by_index(i);
		if (m_end_station) //if end station is valid
		{
			if (m_end_station->entity_desc_count()) //if entity is valid
			{
				++end_stations_shown_count; //keep track of valid end stations

				struct end_station_stream_names inputs;

				avdecc_lib::entity_descriptor *entity;
				avdecc_lib::configuration_descriptor *configuration;
				get_current_entity_and_descriptor
					(m_end_station, &entity, &configuration);

				entity_ids.push_back(m_end_station->entity_id());

				avdecc_lib::entity_descriptor_response
					*entity_desc_resp = entity->get_entity_response();
				if (entity_desc_resp)
				{
					const uint8_t * end_station_set_name = entity_desc_resp->entity_name();
					wxString end_station_name;
					if (end_station_set_name[0] == '\0')
					{
						size_t string_desc_index;
						size_t string_index;
						int ret = configuration->get_strings_desc_string_by_reference(
							entity->localized_description(),
							string_desc_index, string_index);

						if (ret == 0)
						{
							avdecc_lib::strings_descriptor
								*strings_desc = configuration->get_strings_desc_by_index(string_desc_index);
							if (strings_desc)
							{
								avdecc_lib::strings_descriptor_response
									*strings_resp_ref = strings_desc->get_strings_response();

								const uint8_t * default_name = strings_resp_ref->get_string_by_index(
									string_index);
								end_station_name = default_name;
								inputs.entity_name = end_station_name;
								delete strings_resp_ref;
							}
							else
							{
								inputs.entity_name = "UNKNOWN";
							}
						}
					}
					else
					{
						end_station_name = end_station_set_name;
						inputs.entity_name = end_station_name;
					}
				}
				delete entity_desc_resp;

				/****************** Stream Input Info *****************/
				for (unsigned int i = 0; i < configuration->stream_input_desc_count(); i++)
				{
					avdecc_lib::stream_input_descriptor
						*stream_input_desc_ref = configuration->get_stream_input_desc_by_index(i);
					if (stream_input_desc_ref)
					{
						avdecc_lib::stream_input_descriptor_response
							*stream_input_resp_ref = stream_input_desc_ref->get_stream_input_response();
						const uint8_t * object_name = stream_input_resp_ref->object_name();
						wxString stream_input_name;
						if (object_name[0] == '\0')
						{
							size_t string_desc_index;
							size_t string_index;
							int ret = configuration->get_strings_desc_string_by_reference(
								stream_input_resp_ref->localized_description(),
								string_desc_index, string_index);

							if (ret == 0)
							{
								avdecc_lib::strings_descriptor
									*strings_desc = configuration->get_strings_desc_by_index(string_desc_index);
								if (strings_desc)
								{
									avdecc_lib::strings_descriptor_response
										*strings_resp_ref = strings_desc->get_strings_response();

									stream_input_name = strings_resp_ref->get_string_by_index(string_index);
									inputs.stream_names.push_back(stream_input_name);
									delete strings_resp_ref;
								}
								else
								{
									inputs.stream_names.push_back("UNKNOWN");
								}
							}
						}
						else
						{
							stream_input_name = object_name;
							inputs.stream_names.push_back(stream_input_name);
						}

						delete stream_input_resp_ref;
					}
					else
					{
						wxLogMessage("AVDECC get stream_input descriptor [%i] error", i);
					}
				}
				stream_input_names.push_back(inputs);
			}
		}
	}

	num_rows_shown = end_stations_shown_count;
	num_cols_shown = end_stations_shown_count;
}

/*
* Find and store AVDECC end station and stream output names.
*/
void avdecc_stream_connects_scrolled_canvas::GetStreamOutputNames()
{
	for (size_t i = 0; i < m_end_station_count; i++)
	{
		m_end_station = m_controller_obj->get_end_station_by_index(i);
		if (m_end_station)
		{
			if (m_end_station->entity_desc_count())
			{
				struct end_station_stream_names outputs;

				avdecc_lib::entity_descriptor *entity;
				avdecc_lib::configuration_descriptor *configuration;
				get_current_entity_and_descriptor
					(m_end_station, &entity, &configuration);

				avdecc_lib::entity_descriptor_response
					*entity_desc_resp = entity->get_entity_response();
				if (entity_desc_resp)
				{
					const uint8_t * end_station_set_name = entity_desc_resp->entity_name();
					wxString end_station_name;
					if (end_station_set_name[0] == '\0')
					{
						size_t string_desc_index;
						size_t string_index;
						int ret = configuration->get_strings_desc_string_by_reference(
							entity->localized_description(),
							string_desc_index, string_index);
						if (ret == 0)
						{
							avdecc_lib::strings_descriptor
								*strings_desc = configuration->get_strings_desc_by_index(string_desc_index);
							avdecc_lib::strings_descriptor_response
								*strings_resp_ref = strings_desc->get_strings_response();

							const uint8_t * default_name = strings_resp_ref->get_string_by_index(string_index);
							end_station_name = default_name;
							outputs.entity_name = end_station_name;
							delete strings_resp_ref;
						}
					}
					else
					{
						end_station_name = end_station_set_name;
						outputs.entity_name = end_station_name;
					}
				}
				delete entity_desc_resp;

				/****************** Stream Output Info *****************/
				for (unsigned int i = 0; i < configuration->stream_output_desc_count(); i++)
				{
					avdecc_lib::stream_output_descriptor
						*stream_output_desc_ref = configuration->get_stream_output_desc_by_index(i);
					if (stream_output_desc_ref)
					{
						avdecc_lib::stream_output_descriptor_response
							*stream_output_resp_ref = stream_output_desc_ref->get_stream_output_response();
						const uint8_t * object_name = stream_output_resp_ref->object_name();
						wxString stream_output_name;
						if (object_name[0] == '\0')
						{
							size_t string_desc_index;
							size_t string_index;
							int ret = configuration->get_strings_desc_string_by_reference(
								stream_output_resp_ref->localized_description(),
								string_desc_index, string_index);
							if (ret == 0)
							{
								avdecc_lib::strings_descriptor
									*strings_desc = configuration->get_strings_desc_by_index(string_desc_index);
								if (strings_desc)
								{
									avdecc_lib::strings_descriptor_response
										*strings_resp_ref = strings_desc->get_strings_response();

									stream_output_name = strings_resp_ref->get_string_by_index(string_index);
									outputs.stream_names.push_back(stream_output_name);
									delete strings_resp_ref;
								}
								else
								{
									outputs.stream_names.push_back("UNKNOWN");
								}
							}
						}
						else
						{
							stream_output_name = object_name;
							outputs.stream_names.push_back(stream_output_name);
						}

						delete stream_output_resp_ref;
					}
					else
					{
						wxLogMessage("(%s) AVDECC get stream_output descriptor [%i] error", __FUNCTION__, i);
					}
				}
				stream_output_names.push_back(outputs);
			}
		}
	}
}
/*
* This event function handles all mouse events within this window.
*
* Currently implemented : Tracking the mouse movement and highlighting
*	the corresponding coordinates when hovering over a valid connection cell.
* To be implemented: A tooltip providing end station and stream details.
*/
void avdecc_stream_connects_scrolled_canvas::OnMouseMove(wxMouseEvent &event)
{
	wxWindow * event_window = (wxWindow *)event.GetEventObject();

	int mouse_x = -1;
	int mouse_y = -1;

	int scrolled_x = -1;
	int scrolled_y = -1;

	int scroll_offset_x = 0;
	int scroll_offset_y = 0;
	int cell_size = OUTPUT_CELL_WIDTH;

	int last_row = -1;
	int last_col = -1;
	int last_header_row = -1;
	int last_header_col = -1;

	event.GetPosition(&mouse_x, &mouse_y);
	CalcUnscrolledPosition(mouse_x, mouse_y, &scrolled_x, &scrolled_y);
	scroll_offset_x = scrolled_x - mouse_x;
	scroll_offset_y = scrolled_y - mouse_y;

	if (event_window->GetParent() == connection_grid)
	{
		if (last_popup)
		{
			last_popup->Hide();
		}

		wxClientDC cdc(this); //use clientDC outside of wxPaintEvent

		//find grid coords from mouse pos
		int row = connection_grid->YToRow(mouse_y);
		int col = connection_grid->XToCol(mouse_x);

		//if user moves around within a cell, do nothing
		if (row == last_row && col == last_col)
		{
			event.Skip();
		}
		else
		{
			if (row >= 0 && col >= 0)
			{
				//clear previous highlight(s)
				wxPen default_pen(*wxWHITE, 2, wxSOLID);
				cdc.SetPen(default_pen);

				for (int i = 0; i < num_cols_shown; i++)
				{
					wxCoord xx1 = TOP_LEFT_BOX_OFFSET + (cell_size * i) - scroll_offset_x,
						xy1 = 0 - scroll_offset_y; //floor x
					wxCoord xx2 = TOP_LEFT_BOX_OFFSET + (cell_size * i) - scroll_offset_x,
						xy2 = OUTPUT_CELL_HEIGHT - scroll_offset_y;
					cdc.DrawLine(xx1, xy1, xx2, xy2);

					wxCoord xx3 = 200 + (cell_size * i) - scroll_offset_x,
						xy3 = 0 - scroll_offset_y; //ceiling x
					wxCoord xx4 = 200 + (cell_size * i) - scroll_offset_x,
						xy4 = OUTPUT_CELL_HEIGHT - scroll_offset_y;
					cdc.DrawLine(xx3, xy3, xx4, xy4);
				}

				for (int i = 0; i < num_rows_shown; i++)
				{
					wxCoord x1 = 0 - scroll_offset_x,
						y1 = TOP_LEFT_BOX_OFFSET + (cell_size * i) - scroll_offset_y; //floor y
					wxCoord x2 = INPUT_CELL_WIDTH - scroll_offset_x,
						y2 = TOP_LEFT_BOX_OFFSET + (cell_size * i) - scroll_offset_y;
					cdc.DrawLine(x1, y1, x2, y2);

					wxCoord x3 = 0 - scroll_offset_x,
						y3 = 200 + (cell_size * i) - scroll_offset_y; //ceiling y
					wxCoord x4 = INPUT_CELL_WIDTH - scroll_offset_x,
						y4 = 200 + (cell_size * i) - scroll_offset_y;
					cdc.DrawLine(x3, y3, x4, y4);
				}

				//if hovered grid cell is valid 
				if (connection_grid->GetCellValue(row, col) != wxEmptyString)
				{
					wxPen my_pen(*wxBLUE, 2, wxSOLID);
					cdc.SetPen(my_pen);

					for (int i = 0; i < num_cols_shown; i++) //clear previous
					{
						wxCoord xx1 = mouse_x - (mouse_x % cell_size) - scroll_offset_x,
							xy1 = 0 - scroll_offset_y; //floor x
						wxCoord xx2 = mouse_x - (mouse_x % cell_size) - scroll_offset_x,
							xy2 = INPUT_CELL_WIDTH - scroll_offset_y;
						cdc.DrawLine(xx1 + TOP_LEFT_BOX_OFFSET,
							xy1, xx2 + TOP_LEFT_BOX_OFFSET, xy2);

						wxCoord xx3 = mouse_x + cell_size - (mouse_x % cell_size) - scroll_offset_x,
							xy3 = 0 - scroll_offset_y; //ceiling x
						wxCoord xx4 = mouse_x + cell_size - (mouse_x % cell_size) - scroll_offset_x,
							xy4 = INPUT_CELL_WIDTH - scroll_offset_y;
						cdc.DrawLine(xx3 + TOP_LEFT_BOX_OFFSET, xy3,
							xx4 + TOP_LEFT_BOX_OFFSET, xy4);
					}

					for (int i = 0; i < num_rows_shown; i++)
					{
						wxCoord x1 = 0 - scroll_offset_x,
							y1 = mouse_y - (mouse_y % cell_size) - scroll_offset_y; //floor y
						wxCoord x2 = INPUT_CELL_WIDTH - scroll_offset_x,
							y2 = mouse_y - (mouse_y % cell_size) - scroll_offset_y;
						cdc.DrawLine(x1, y1 + TOP_LEFT_BOX_OFFSET,
							x2, y2 + TOP_LEFT_BOX_OFFSET);

						wxCoord x3 = 0 - scroll_offset_x,
							y3 = mouse_y + cell_size - (mouse_y % cell_size) - scroll_offset_y; //ceiling y
						wxCoord x4 = INPUT_CELL_WIDTH - scroll_offset_x,
							y4 = mouse_y + cell_size - (mouse_y % cell_size) - scroll_offset_y;
						cdc.DrawLine(x3, y3 + TOP_LEFT_BOX_OFFSET,
							x4, y4 + TOP_LEFT_BOX_OFFSET);
					}
				}
			}
		}
		last_row = row;
		last_col = col;
	}
	else //tooltip
	{
		bool IsValid = true;
		bool IsSame = false;

		int header_label_col = connection_grid->XToCol(mouse_x + scroll_offset_x - TOP_LEFT_BOX_OFFSET);
		int header_label_row = connection_grid->YToRow(mouse_y + scroll_offset_y - TOP_LEFT_BOX_OFFSET);
		if (header_label_col == -1 &&
			header_label_row == -1)
		{
			IsValid = false;
		}

		if (header_label_col == last_header_label_col &&
			header_label_row == last_header_label_row)
		{
			IsSame = true;
		}

		if (IsValid && !IsSame)
		{
			bool found = false;
			struct connection_state_details selected_details;
			memset(&selected_details, 0, sizeof(connection_state_details));

			if (header_label_col != -1)
			{
				struct all_indexes listener_indexes = names_and_streams.at(header_label_col);

				if (listener_indexes.stream_index == -1) //end station label (no stream index)
				{
					int entity_index = listener_indexes.entity_index;
					struct end_station_stream_names names = stream_input_names.at(entity_index);

					wxString end_station_name = names.entity_name;
					uint64_t entity_id = entity_ids.at(entity_index);

					selected_details.name = end_station_name;
					selected_details.connection_details.stream_id = entity_id;
					found = true;
				}
				else //stream name label
				{
					for (size_t i = 0; i < all_rx_details.size(); i++)
					{
						struct connection_state_details rx_details = all_rx_details.at(i);
						if (listener_indexes.entity_index == rx_details.entity_index &&
							listener_indexes.stream_index == rx_details.stream_index)
						{
							found = true;
							selected_details = rx_details;
						}
					}
				}
			}
			else if (header_label_row != -1)
			{
				struct all_indexes talker_indexes = names_and_streams.at(header_label_row);

				if (talker_indexes.stream_index == -1) //end station label (no stream index)
				{
					int entity_index = talker_indexes.entity_index;
					struct end_station_stream_names names = stream_input_names.at(entity_index);

					wxString end_station_name = names.entity_name;
					uint64_t entity_id = entity_ids.at(entity_index);

					selected_details.name = end_station_name;
					selected_details.connection_details.stream_id = entity_id;
					found = true;
				}
				else //stream name label
				{
					for (size_t i = 0; i < all_tx_details.size(); i++)
					{
						struct connection_state_details tx_details = all_tx_details.at(i);
						if (talker_indexes.entity_index == tx_details.entity_index &&
							talker_indexes.stream_index == tx_details.stream_index)
						{
							found = true;
							selected_details = tx_details;
						}
					}
				}
			}

			ClientToScreen(&mouse_x, &mouse_y); //relative window coords to screen coords (so User can move frame)

			if (last_popup != NULL)
			{
				//if (!last_popup->timeout)
				last_popup->Destroy();
			}

			tooltip_popup = new MySimplePopup(this, mouse_x, mouse_y, selected_details, found);

			last_popup = tooltip_popup;
			last_header_label_row = header_label_row;
			last_header_label_col = header_label_col;
		}
	}
	event.Skip();
}
/*
* This event function is triggered whenever one of the toggle buttons is pressed.
* The toggle button is identified by id, then the corresponding grid row/col
* is shown or hidden.  Also, a repaint is triggered, showing or hiding the
* corresponding header labels.  Finally, the canvas is resized to accommodate
* for the change.
*/
void avdecc_stream_connects_scrolled_canvas::OnToggle(wxCommandEvent& event)
{
	int id = event.GetId();

	if (id < (int)end_stations_shown_count) //inputs
	{
		wxToggleButton * selected_button = input_expand_buttons.at(id);
		wxBoxSizer * selected_button_sizer = expand_button_sizers.at(id);
		struct end_station_stream_names inputs = stream_input_names.at(id);
		int name_row = stream_name_indexes.at(id);

		if (selected_button->GetValue()) //Toggle On
		{
			selected_button->SetLabel("-");
			selected_button_sizer->Show(true);
			selected_button_sizer->Layout();
			for (size_t it = 1; it <= inputs.stream_names.size(); it++)
			{
				connection_grid->ShowRow(name_row + it); //name row + stream row index
				num_rows_shown++;
			}
			//std::cout << "toggle on" << std::endl;
		}
		else
		{
			selected_button->SetLabel("+"); //Toggle Off
			selected_button_sizer->Show(false);
			selected_button_sizer->Layout();
			for (size_t it = 1; it <= inputs.stream_names.size(); it++)
			{
				connection_grid->HideRow(name_row + it);
				num_rows_shown--;
			}
			//std::cout << "toggle off" << std::endl;
		}
	}
	else
	{
		wxToggleButton * selected_button = output_expand_buttons.at
			(id - end_stations_shown_count);
		wxBoxSizer * selected_button_sizer = expand_output_button_sizers.at
			(id - end_stations_shown_count);
		struct end_station_stream_names outputs = stream_output_names.at
			(id - end_stations_shown_count);
		int name_col = stream_name_indexes.at(id - end_stations_shown_count);

		if (selected_button->GetValue()) //Toggle On
		{
			selected_button->SetLabel("-");
			selected_button_sizer->Show(true);
			selected_button_sizer->Layout();
			for (size_t it = 1; it <= outputs.stream_names.size(); it++)
			{
				connection_grid->ShowCol(name_col + it); //name row + stream row index
				num_cols_shown++;
			}
			//std::cout << "toggle on" << std::endl;
		}
		else
		{
			selected_button->SetLabel("+"); //Toggle Off
			selected_button_sizer->Show(false);
			selected_button_sizer->Layout();
			for (size_t it = 1; it <= outputs.stream_names.size(); it++)
			{
				connection_grid->HideCol(name_col + it);
				num_cols_shown--;
			}
			//std::cout << "toggle off" << std::endl;
		}
	}

	this->Refresh();
	this->Layout();
	this->FitInside();
}

/*
* This event function is triggered when a connection_grid cell is clicked.
* An Empty Button clicked will try to cmd_connect the corresponding stream output row
* and stream input col.  A button with CONNECTION_SUCCESS or CONNECTION_FAILURE
* status will try to cmd_disconnect the row/col.
*/
void avdecc_stream_connects_scrolled_canvas::OnGridCellClick(wxGridEvent &event)
{
	int row = event.GetRow();
	int col = event.GetCol();
	wxString type_val = connection_grid->GetCellValue(row, col);

	struct all_indexes talker_indexes = names_and_streams.at(row);
	struct all_indexes listener_indexes = names_and_streams.at(col);

	int talker_entity_index = talker_indexes.entity_index;
	uint64_t talker_entity_id = entity_ids.at(talker_entity_index);
	int talker_stream_index = talker_indexes.stream_index;

	int listener_entity_index = listener_indexes.entity_index;
	uint64_t listener_entity_id = entity_ids.at(listener_entity_index);
	int listener_stream_index = listener_indexes.stream_index;

	if (wxAtoi(type_val) == GRID_BUTTON)
	{
		//cmd connect
		int ret = AvdeccConnect(talker_entity_id, talker_stream_index,
			listener_entity_id, listener_stream_index);

		if (ret == 1)
		{
			connection_grid->SetCellRenderer(
				row, col,
				new avdecc_connection_grid_renderer(CONNECTION_FAILURE));
			connection_grid->SetCellValue(row, col, wxT("3"));
		}
		else if (ret == 2)
		{
			connection_grid->SetCellRenderer(
				row, col,
				new avdecc_connection_grid_renderer(CONNECTION_WARNING));
			connection_grid->SetCellValue(row, col, wxT("4"));
		}
		else
		{
			connection_grid->SetCellRenderer(
				row, col,
				new avdecc_connection_grid_renderer(CONNECTION_SUCCESS));
			connection_grid->SetCellValue(row, col, wxT("2"));
		}
	}

	else if (wxAtoi(type_val) == CONNECTION_SUCCESS || wxAtoi(type_val) == CONNECTION_WARNING)
	{
		//cmd disconnect
		int ret = AvdeccDisconnect(talker_entity_id, talker_stream_index,
			listener_entity_id, listener_stream_index);

		if (ret)
		{
			connection_grid->SetCellRenderer(
				row, col,
				new avdecc_connection_grid_renderer(CONNECTION_FAILURE));
			connection_grid->SetCellValue(row, col, wxT("3"));
		}
		else
		{
			connection_grid->SetCellRenderer(
				row, col,
				new avdecc_connection_grid_renderer(GRID_BUTTON));
			connection_grid->SetCellValue(row, col, wxT("1"));
		}

	}
	else if (wxAtoi(type_val) == CONNECTION_FAILURE)
	{
		connection_grid->SetCellRenderer(
			row, col,
			new avdecc_connection_grid_renderer(GRID_BUTTON));
		connection_grid->SetCellValue(row, col, wxT("1"));
	}
	else
	{
		//unsupported
	}
}

int avdecc_stream_connects_scrolled_canvas::get_current_entity_and_descriptor(
	avdecc_lib::end_station *end_station,
	avdecc_lib::entity_descriptor **entity,
	avdecc_lib::configuration_descriptor **configuration)
{
	*entity = NULL;
	*configuration = NULL;

	uint16_t current_entity = end_station->get_current_entity_index();
	if (current_entity >= end_station->entity_desc_count())
	{
		//std::cout << "Current entity not available" << std::endl;
		return 1;
	}

	*entity = end_station->get_entity_desc_by_index(current_entity);

	uint16_t current_config = end_station->get_current_config_index();
	if (current_config >= (*entity)->config_desc_count())
	{
		//std::cout << "Current configuration not available" << std::endl;
		return 1;
	}

	*configuration = (*entity)->get_config_desc_by_index(current_config);

	return 0;
}
/*
* Send a 1722.1 send_connect_rx command to connect talker to listener.
*/
int avdecc_stream_connects_scrolled_canvas::AvdeccConnect(
	uint64_t talker_ent_id, int talker_stream_index,
	uint64_t listener_ent_id, int listener_stream_index)
{
	//outstream
	uint32_t outstream_end_station_index;
	m_controller_obj->is_end_station_found_by_entity_id(
		talker_ent_id, outstream_end_station_index);
	uint16_t outstream_desc_index = talker_stream_index;

	//instream
	uint32_t instream_end_station_index;
	m_controller_obj->is_end_station_found_by_entity_id(
		listener_ent_id, instream_end_station_index);
	uint16_t instream_desc_index = listener_stream_index;
	uint16_t connection_flags = 0;

	avdecc_lib::configuration_descriptor
		*in_descriptor = m_controller_obj->get_current_config_desc(instream_end_station_index,
		false);
	avdecc_lib::configuration_descriptor
		*out_descriptor = m_controller_obj->get_current_config_desc(outstream_end_station_index,
		false);

	bool test_mode = false;
	bool is_valid = (in_descriptor && out_descriptor &&
		(test_mode || (instream_end_station_index != outstream_end_station_index)) &&
		(instream_end_station_index < m_controller_obj->get_end_station_count()) &&
		(outstream_end_station_index < m_controller_obj->get_end_station_count()) &&
		(instream_desc_index < in_descriptor->stream_input_desc_count()) &&
		(outstream_desc_index < out_descriptor->stream_output_desc_count()));

	if (is_valid)
	{
		intptr_t cmd_notification_id = 0;
		uint64_t talker_entity_id;
		bool check_stream_format;

		cmd_notification_id = 1; //change this
		m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);

		//Stream Input
		avdecc_lib::stream_input_descriptor
			*instream = in_descriptor->get_stream_input_desc_by_index(instream_desc_index);
		avdecc_lib::stream_input_descriptor_response
			*stream_input_resp_ref = instream->get_stream_input_response();

		//Stream Output
		avdecc_lib::stream_output_descriptor
			*outstream = out_descriptor->get_stream_output_desc_by_index(outstream_desc_index);
		avdecc_lib::stream_output_descriptor_response
			*stream_output_resp_ref = outstream->get_stream_output_response();

		check_stream_format = (strcmp(stream_input_resp_ref->current_format(),
			stream_output_resp_ref->current_format()) == 0);
		if (!check_stream_format)
		{
			//what do we do here???
			wxString ws;
			ws << "AVDECC WARNING : Stream Formats don't match! "
				<< "instream format: " << stream_input_resp_ref->current_format()
				<< " != outstream format: " << stream_output_resp_ref->current_format();
			wxLogMessage(ws);

			return 2;
		}

		avdecc_lib::end_station
			*outstream_end_station = m_controller_obj->get_end_station_by_index(
			outstream_end_station_index);
		uint16_t outstream_current_entity = outstream_end_station->get_current_entity_index();

		avdecc_lib::entity_descriptor_response
			*entity_resp_ref = outstream_end_station->get_entity_desc_by_index(
			outstream_current_entity)->get_entity_response();

		talker_entity_id = entity_resp_ref->entity_id();
		delete(entity_resp_ref);

		//cmd connect
		instream->send_connect_rx_cmd((void *)cmd_notification_id, talker_entity_id,
			outstream_desc_index, connection_flags);
		delete(stream_input_resp_ref);

		if (m_sys->get_last_resp_status() != 0)
		{
			return 1;
		}
	}
	else
	{
		wxLogMessage("(%s) Invalid ACMP Connection", __FUNCTION__);
	}

	return 0;
}
/*
* Send a 1722.1 send_connect_rx command to disconnect talker to listener.
*/
int avdecc_stream_connects_scrolled_canvas::AvdeccDisconnect(
	uint64_t talker_ent_id, int talker_stream_index,
	uint64_t listener_ent_id, int listener_stream_index)
{
	//outstream
	uint32_t outstream_end_station_index;
	m_controller_obj->is_end_station_found_by_entity_id(talker_ent_id,
		outstream_end_station_index);
	uint16_t outstream_desc_index = talker_stream_index;

	//instream
	uint32_t instream_end_station_index;
	m_controller_obj->is_end_station_found_by_entity_id(listener_ent_id,
		instream_end_station_index);
	uint16_t instream_desc_index = listener_stream_index;

	bool test_mode = false;

	avdecc_lib::configuration_descriptor
		*in_descriptor = m_controller_obj->get_current_config_desc(instream_end_station_index,
		false);
	avdecc_lib::configuration_descriptor
		*out_descriptor = m_controller_obj->get_current_config_desc(outstream_end_station_index,
		false);
	bool is_valid = (in_descriptor && out_descriptor &&
		(test_mode || (instream_end_station_index != outstream_end_station_index)) &&
		(instream_end_station_index < m_controller_obj->get_end_station_count()) &&
		(outstream_end_station_index < m_controller_obj->get_end_station_count()) &&
		(instream_desc_index < in_descriptor->stream_input_desc_count()) &&
		(outstream_desc_index < out_descriptor->stream_output_desc_count()));

	if (is_valid)
	{
		intptr_t cmd_notification_id = 0;
		uint64_t talker_entity_id;

		cmd_notification_id = 2; //change
		m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
		avdecc_lib::stream_input_descriptor
			*instream = in_descriptor->get_stream_input_desc_by_index(instream_desc_index);

		avdecc_lib::end_station
			*outstream_end_station = m_controller_obj->get_end_station_by_index(outstream_end_station_index);
		uint16_t current_entity = outstream_end_station->get_current_entity_index();
		avdecc_lib::entity_descriptor_response *entity_resp_ref =
			outstream_end_station->get_entity_desc_by_index(current_entity)->get_entity_response();
		talker_entity_id = entity_resp_ref->entity_id();
		delete(entity_resp_ref);

		instream->send_disconnect_rx_cmd((void *)cmd_notification_id, talker_entity_id, outstream_desc_index);
		if (m_sys->get_last_resp_status() != 0)
		{
			return 1;
		}
	}
	else
	{
		wxLogMessage(wxT("Invalid ACMP Disconnection"));
	}

	return 0;
}

/*
* This function sends 1722.1 get_rx_state() and get_tx_state()
* to determine which stream inputs and outputs are currently
* connected.
*/
int avdecc_stream_connects_scrolled_canvas::ShowConnections()
{
	// Use the same notification ID for all the read commands
	intptr_t cmd_notification_id = 3; //temporary

	for (uint32_t i = 0; i < m_controller_obj->get_end_station_count(); i++)
	{
		avdecc_lib::end_station *end_station = m_controller_obj->get_end_station_by_index(i);
		avdecc_lib::entity_descriptor *entity;
		avdecc_lib::configuration_descriptor *configuration;
		if (get_current_entity_and_descriptor(end_station, &entity, &configuration))
			continue;

		size_t stream_input_desc_count = configuration->stream_input_desc_count();
		for (uint32_t j = 0; j < stream_input_desc_count; j++)
		{
			avdecc_lib::stream_input_descriptor
				*instream = configuration->get_stream_input_desc_by_index(j);
			instream->send_get_rx_state_cmd((void *)cmd_notification_id);
		}

		size_t stream_output_desc_count = configuration->stream_output_desc_count();

		for (uint32_t j = 0; j < stream_output_desc_count; j++)
		{
			// Only wait when issuing the last packet
			const bool last_command = (i == m_controller_obj->get_end_station_count() - 1) &&
				(j == stream_output_desc_count - 1);
			if (last_command)
				m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
			avdecc_lib::stream_output_descriptor
				*outstream = configuration->get_stream_output_desc_by_index(j);
			outstream->send_get_tx_state_cmd((void *)cmd_notification_id);
			if (last_command)
				m_sys->get_last_resp_status();
		}
	}

	for (uint32_t in_index = 0; in_index < m_controller_obj->get_end_station_count(); in_index++)
	{
		avdecc_lib::end_station
			*in_end_station = m_controller_obj->get_end_station_by_index(in_index);
		avdecc_lib::entity_descriptor *in_entity;
		avdecc_lib::configuration_descriptor *in_descriptor;
		if (get_current_entity_and_descriptor(in_end_station, &in_entity, &in_descriptor))
			continue;

		size_t stream_input_desc_count = in_descriptor->stream_input_desc_count();
		for (uint32_t in_stream_index = 0; in_stream_index < stream_input_desc_count; in_stream_index++)
		{
			avdecc_lib::stream_input_descriptor
				*instream = in_descriptor->get_stream_input_desc_by_index(in_stream_index);
			avdecc_lib::stream_input_get_rx_state_response
				*stream_input_resp_ref = instream->get_stream_input_get_rx_state_response();
			if (!stream_input_resp_ref->get_rx_state_connection_count())
				continue;
			delete stream_input_resp_ref;

			for (uint32_t out_index = 0; out_index < m_controller_obj->get_end_station_count(); out_index++)
			{
				avdecc_lib::end_station
					*out_end_station = m_controller_obj->get_end_station_by_index(out_index);
				avdecc_lib::entity_descriptor *out_entity;
				avdecc_lib::configuration_descriptor *out_descriptor;
				if (get_current_entity_and_descriptor(out_end_station, &out_entity, &out_descriptor))
					continue;

				size_t stream_output_desc_count = out_descriptor->stream_output_desc_count();
				for (uint32_t out_stream_index = 0; out_stream_index < stream_output_desc_count; out_stream_index++)
				{
					struct connection_state_details rx_details;
					struct connection_state_details tx_details;

					avdecc_lib::stream_input_get_rx_state_response
						*stream_input_resp_ref = instream->get_stream_input_get_rx_state_response();
					avdecc_lib::stream_input_descriptor_response
						*stream_input_desc_resp = instream->get_stream_input_response();
					const uint8_t * input_stream_name = stream_input_desc_resp->object_name();
					wxString stream_name;
					if (input_stream_name[0] == '\0')
					{
						size_t string_desc_index;
						size_t string_index;
						int ret = out_descriptor->get_strings_desc_string_by_reference(
							stream_input_desc_resp->localized_description(),
							string_desc_index, string_index);
						if (ret == 0)
						{
							avdecc_lib::strings_descriptor
								*strings_desc = in_descriptor->get_strings_desc_by_index(string_desc_index);
							avdecc_lib::strings_descriptor_response
								*strings_resp_ref = strings_desc->get_strings_response();

							const uint8_t * default_name = strings_resp_ref->get_string_by_index(string_index);

							stream_name = default_name;
							rx_details.name = stream_name;
							delete strings_resp_ref;
						}
					}
					else
					{
						rx_details.name = input_stream_name;
					}

					std::vector<uint64_t>::iterator input_entity_it = std::find(entity_ids.begin(), entity_ids.end(),
						in_end_station->entity_id());

					if (input_entity_it != entity_ids.end())
					{
						rx_details.entity_index = input_entity_it - entity_ids.begin();
					}

					rx_details.stream_index = instream->descriptor_index();
					rx_details.type = wxT("INSTREAM");

					delete stream_input_desc_resp;

					avdecc_lib::stream_output_descriptor
						*outstream = out_descriptor->get_stream_output_desc_by_index(out_stream_index);
					avdecc_lib::stream_output_get_tx_state_response
						*stream_output_resp_ref = outstream->get_stream_output_get_tx_state_response();
					if (!stream_output_resp_ref->get_tx_state_connection_count() ||
						(stream_input_resp_ref->get_rx_state_stream_id() != stream_output_resp_ref->get_tx_state_stream_id()))
					{
						continue;
					}

					avdecc_lib::stream_output_descriptor_response
						*stream_output_desc_resp = outstream->get_stream_output_response();
					const uint8_t * output_stream_name = stream_output_desc_resp->object_name();
					wxString stream_output_name;
					if (output_stream_name[0] == '\0')
					{
						size_t string_desc_index;
						size_t string_index;
						int ret = out_descriptor->get_strings_desc_string_by_reference(
							stream_output_desc_resp->localized_description(),
							string_desc_index, string_index);
						if (ret == 0)
						{
							avdecc_lib::strings_descriptor
								*strings_desc = out_descriptor->get_strings_desc_by_index(string_desc_index);
							avdecc_lib::strings_descriptor_response
								*strings_resp_ref = strings_desc->get_strings_response();

							const uint8_t * default_name = strings_resp_ref->get_string_by_index(string_index);

							stream_output_name = default_name;
							tx_details.name = stream_output_name;
							delete strings_resp_ref;
						}
					}
					else
					{
						tx_details.name = output_stream_name;
					}

					std::vector<uint64_t>::iterator output_entity_it = std::find(entity_ids.begin(), entity_ids.end(),
						out_end_station->entity_id());

					if (output_entity_it != entity_ids.end())
					{
						tx_details.entity_index = output_entity_it - entity_ids.begin();
					}

					tx_details.stream_index = outstream->descriptor_index();
					tx_details.type = wxT("OUTSTREAM");

					delete stream_output_desc_resp;

					rx_details.connection_details.connection_count =
						stream_input_resp_ref->get_rx_state_connection_count();
					rx_details.connection_details.stream_dest_mac =
						stream_input_resp_ref->get_rx_state_stream_dest_mac();
					rx_details.connection_details.stream_id =
						stream_input_resp_ref->get_rx_state_stream_id();
					rx_details.connection_details.stream_vlan_id =
						stream_input_resp_ref->get_rx_state_stream_vlan_id();
					all_rx_details.push_back(rx_details);

					tx_details.connection_details.connection_count =
						stream_output_resp_ref->get_tx_state_connection_count();
					tx_details.connection_details.stream_dest_mac =
						stream_output_resp_ref->get_tx_state_stream_dest_mac();
					tx_details.connection_details.stream_id =
						stream_output_resp_ref->get_tx_state_stream_id();
					tx_details.connection_details.stream_vlan_id =
						stream_output_resp_ref->get_tx_state_stream_vlan_id();
					all_tx_details.push_back(tx_details);

					/*
					atomic_cout << "0x" << std::setw(16) << std::hex << std::setfill('0') << out_end_station->entity_id()
					<< "[" << in_stream_index << "] -> "
					<< "0x" << std::setw(16) << std::hex << std::setfill('0') << in_end_station->entity_id()
					<< "[" << out_stream_index << "]" << std::endl;
					*/

					std::vector<uint64_t>::iterator out_entity_it = std::find(entity_ids.begin(), entity_ids.end(),
						out_end_station->entity_id());
					std::vector<uint64_t>::iterator in_entity_it = std::find(entity_ids.begin(), entity_ids.end(),
						in_end_station->entity_id());

					if (out_entity_it != entity_ids.end() && in_entity_it != entity_ids.end())
					{
						int output_entity_index = out_entity_it - entity_ids.begin();
						int output_stream_index = out_stream_index;
						int input_entity_index = in_entity_it - entity_ids.begin();
						int input_stream_index = in_stream_index;

						int output_grid_index = -1;
						int input_grid_index = -1;

						for (size_t it = 0; it < names_and_streams.size(); it++)
						{
							struct all_indexes indexes = names_and_streams.at(it);
							if (indexes.entity_index == output_entity_index &&
								indexes.stream_index == output_stream_index)
							{
								output_grid_index = it;
							}

							if (indexes.entity_index == input_entity_index &&
								indexes.stream_index == input_stream_index)
							{
								input_grid_index = it;
							}
						}

						if (output_grid_index != -1 && input_grid_index != -1)
						{
							connection_grid->SetCellRenderer(output_grid_index, input_grid_index,
								new avdecc_connection_grid_renderer(CONNECTION_SUCCESS));
							connection_grid->SetCellValue(output_grid_index, input_grid_index,
								wxT("2"));
						}

					}

					delete(stream_input_resp_ref);
					delete(stream_output_resp_ref);
				}
			}
		}
	}
	return 0;
}

avdecc_connection_grid_renderer::avdecc_connection_grid_renderer(int type)
{
	m_type = type;
}

void avdecc_connection_grid_renderer::Draw(wxGrid& grid,
	wxGridCellAttr& attr,
	wxDC& dc,
	const wxRect& rect,
	int row, int col,
	bool isSelected)
{
	if (m_type == GRID_BUTTON) //draw a blank button
	{
		wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

		wxRect border(rect.x, rect.y, 25, 25);
		wxRect button(rect.x + 4, rect.y + 4, 18, 18);
		wxRect button_bottom(rect.x + 5, rect.y + 13, 16, 8);

		dc.SetPen(*wxLIGHT_GREY);
		dc.DrawRectangle(border);
		dc.SetPen(*wxBLACK);

		dc.DrawRoundedRectangle(button, 1);
		dc.GradientFillLinear(button, *wxWHITE, wxColour(240, 240, 240), wxDOWN);

		dc.SetPen(*wxWHITE);
		dc.DrawRoundedRectangle(button_bottom, 1);
		dc.GradientFillLinear(button_bottom, wxColour(224, 224, 224), *wxWHITE, wxUP);
	}
	else if (m_type == CONNECTION_SUCCESS) //draw a green connection circle
	{
		wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

		wxRect border(rect.x, rect.y, 25, 25);

		dc.SetPen(*wxLIGHT_GREY);
		dc.DrawRectangle(border);
		dc.SetPen(*wxBLACK);

		dc.SetBrush(*wxBLACK); //outer circle
		dc.DrawCircle(rect.x + 12.5, rect.y + 12.5, 8);

		dc.SetBrush(*wxGREEN);
		dc.DrawCircle(rect.x + 12.5, rect.y + 12.5, 6);
	}
	else if (m_type == CONNECTION_FAILURE) //draw a red 'x'
	{
		wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

		wxRect border(rect.x, rect.y, 25, 25);

		dc.SetPen(*wxLIGHT_GREY);
		dc.DrawRectangle(border);
		dc.SetPen(*wxBLACK);

		wxPen my_pen(*wxRED, 3, wxSOLID);
		dc.SetPen(my_pen);

		wxCoord x = rect.x, y = rect.y;
		wxCoord x2 = rect.x + 25, y2 = rect.y + 25;
		dc.DrawLine(x, y, x2, y2);

		wxCoord x3 = rect.x + 25, y3 = rect.y;
		wxCoord x4 = rect.x, y4 = rect.y + 25;
		dc.DrawLine(x3, y3, x4, y4);
	}
	else if (m_type == CONNECTION_WARNING) //draw a yellow connection circle
	{
		wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

		wxRect border(rect.x, rect.y, 25, 25);

		dc.SetPen(*wxLIGHT_GREY);
		dc.DrawRectangle(border);
		dc.SetPen(*wxBLACK);

		dc.SetBrush(*wxBLACK); //outer circle
		dc.DrawCircle(rect.x + 12.5, rect.y + 12.5, 8);

		dc.SetBrush(*wxYELLOW);
		dc.DrawCircle(rect.x + 12.5, rect.y + 12.5, 6);
	}
	else
	{
		//potentially more
	}
}

wxBEGIN_EVENT_TABLE(MySimplePopup, wxPopupWindow)
EVT_TIMER(wxID_ANY, MySimplePopup::OnTimer)
wxEND_EVENT_TABLE()

MySimplePopup::MySimplePopup(wxWindow *parent, int x, int y, struct connection_state_details &details, bool found)
: wxPopupWindow(parent)
{
	wxPopupWindow::Position(wxPoint(x + 10, y + 10), wxDefaultSize);
	wxPopupWindow::SetBackgroundColour(*wxWHITE);

	wxString ws;
	if (found)
	{
		if (details.type.IsSameAs("INSTREAM")) //rx details
		{
			ws << "RX Name: " << details.name << "\n"
				//<< "Entity index: " << details.entity_index << "\n"
				//<< "Stream index: " << details.stream_index << "\n"
				<< "Stream Id: " << wxString::Format("0x%llx\n", details.connection_details.stream_id)
				<< "Stream Dest Mac: " << wxString::Format("0x%llx\n", details.connection_details.stream_dest_mac)
				<< "Connection Count: " << wxString::Format("%i\n", details.connection_details.connection_count)
				<< "Stream Vlan ID: " << wxString::Format("%i\n", details.connection_details.stream_vlan_id);
		}
		else if (details.type.IsSameAs("OUTSTREAM")) //tx details
		{
			ws << "TX Name: " << details.name << "\n"
				//<< "Entity index: " << details.entity_index << "\n"
				//<< "Stream index: " << details.stream_index << "\n"
				<< "Stream Id: " << wxString::Format("0x%llx\n", details.connection_details.stream_id)
				<< "Stream Dest Mac: " << wxString::Format("0x%llx\n", details.connection_details.stream_dest_mac)
				<< "Connection Count: " << wxString::Format("%i\n", details.connection_details.connection_count)
				<< "Stream Vlan ID: " << wxString::Format("%i\n", details.connection_details.stream_vlan_id);
		}
		else //end station details
		{
			ws << "End Station Name: " << details.name << "\n"
				<< "Entity ID: " << wxString::Format("0x%llx", details.connection_details.stream_id);
		}
	}
	else
	{
		ws << "Unconnected";
	}

	text = new wxStaticText(this, wxID_ANY,
		ws, wxDefaultPosition, wxDefaultSize);

	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(text, 0, wxALL, 5);
	wxPopupWindow::SetSizerAndFit(topSizer);

	m_timer.SetOwner(this);
	m_timer.Start(500);
}

MySimplePopup::~MySimplePopup()
{
	m_timer.Stop();
}

void MySimplePopup::OnTimer(wxTimerEvent& event)
{
	if (timeout_counter == 0) //Delay half a second before showing the popup
	{
		Show();
	}

	if (timeout_counter == 6) //Hide Popup after 3 seconds
	{
		timeout = true;
		this->Hide();
	}

	timeout_counter++;
}




