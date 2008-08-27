/*******************************(GPL)************************************
*	wxHexEditor is a hex edit tool for editing massive files in Linux   *
*	Copyright (C) 2007  Erdem U. Altinyurt                              *
*                                                                       *
*	This program is free software; you can redistribute it and/or       *
*	modify it under the terms of the GNU General Public License         *
*	as published by the Free Software Foundation; either version 2      *
*	of the License, or any later version.                               *
*                                                                       *
*	This program is distributed in the hope that it will be useful,     *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of      *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
*	GNU General Public License for more details.                        *
*                                                                       *
*	You should have received a copy of the GNU General Public License   *
*	along with this program;                                            *
*   if not, write to the Free Software	Foundation, Inc.,               *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA        *
*                                                                       *
*               home  : wxhexeditor.sourceforge.net                     *
*               email : death_knight at gamebox.net                     *
*************************************************************************/

#include "HexEditor.h"

HexEditor::HexEditor(	wxWindow* parent,
							int id,
							wxStatusBar *statbar,
							DataInterpreter *interpreter,
							wxFileName* myfilenm,
							const wxPoint& pos,
							const wxSize& size,
							long style ):
			HexEditorCtrl(parent, id, pos, size, wxTAB_TRAVERSAL)
			,statusbar(statbar),
			interpreter(interpreter)
			{
		printf("Rahman ve Rahim olan Allah'ın adıyla.\n"); // Praying to GOD
		myfile = NULL;

		if( myfilenm != NULL ){
			myfilename = *myfilenm;
			FileOpen( myfilename );
			}
		offset_scroll->Enable( true );
		selection.start_offset = selection.end_offset = 0;
		selection.state = selector::SELECTION_FALSE;

		start_offset=0;
		Dynamic_Connector();
	}
	HexEditor::~HexEditor(){
		Dynamic_Disconnector();
		FileClose();
	}

void HexEditor::Dynamic_Connector(){
    hex_ctrl 	->Connect( wxEVT_KEY_DOWN,	wxKeyEventHandler(HexEditor::OnKeyboardInput),NULL, this);
//	hex_ctrl 	->Connect( wxEVT_KEY_UP,	wxKeyEventHandler(HexEditor::OnKeyboardSelectionEnd),NULL, this);
    text_ctrl	->Connect( wxEVT_KEY_DOWN,	wxKeyEventHandler(HexEditor::OnKeyboardInput),NULL, this );
//	text_ctrl	->Connect( wxEVT_KEY_UP,	wxKeyEventHandler(HexEditor::OnKeyboardSelectionEnd),NULL, this);
	offset_ctrl->Connect( wxEVT_LEFT_DOWN,	wxMouseEventHandler(HexEditor::OnOffsetMouseFocus),NULL, this);
	hex_ctrl	->Connect( wxEVT_LEFT_DOWN,	wxMouseEventHandler(HexEditor::OnHexMouseFocus),NULL, this);
	text_ctrl	->Connect( wxEVT_LEFT_DOWN,	wxMouseEventHandler(HexEditor::OnTextMouseFocus),NULL, this);
	hex_ctrl	->Connect( wxEVT_LEFT_UP,	wxMouseEventHandler(HexEditor::OnMouseSelectionEnd),NULL, this);
	text_ctrl	->Connect( wxEVT_LEFT_UP,	wxMouseEventHandler(HexEditor::OnMouseSelectionEnd),NULL, this);
	hex_ctrl	->Connect( wxEVT_RIGHT_DOWN,wxMouseEventHandler(HexEditor::OnMouseTest),NULL, this);
	text_ctrl	->Connect( wxEVT_RIGHT_DOWN,wxMouseEventHandler(HexEditor::OnMouseTest),NULL, this);
	hex_ctrl	->Connect( wxEVT_MOTION,	wxMouseEventHandler(HexEditor::OnMouseMove),NULL, this);
	text_ctrl	->Connect( wxEVT_MOTION,	wxMouseEventHandler(HexEditor::OnMouseMove),NULL, this);
	hex_ctrl	->Connect( wxEVT_MOUSEWHEEL,wxMouseEventHandler(HexEditor::OnMouseWhell),NULL, this);
	text_ctrl	->Connect( wxEVT_MOUSEWHEEL,wxMouseEventHandler(HexEditor::OnMouseWhell),NULL, this);
	}

void HexEditor::Dynamic_Disconnector(){
    hex_ctrl ->Disconnect( wxEVT_KEY_DOWN,	wxKeyEventHandler(HexEditor::OnKeyboardInput),NULL, this);
//	hex_ctrl ->Disconnect( wxEVT_KEY_UP,	wxKeyEventHandler(HexEditor::OnKeyboardSelectionEnd),NULL, this);
    text_ctrl->Disconnect( wxEVT_KEY_DOWN,	wxKeyEventHandler(HexEditor::OnKeyboardInput),NULL, this );
//	text_ctrl->Disconnect( wxEVT_KEY_UP,	wxKeyEventHandler(HexEditor::OnKeyboardSelectionEnd),NULL, this);
	offset_ctrl->Disconnect(wxEVT_LEFT_DOWN,wxMouseEventHandler(HexEditor::OnOffsetMouseFocus),NULL, this);
	hex_ctrl ->Disconnect( wxEVT_LEFT_DOWN,	wxMouseEventHandler(HexEditor::OnHexMouseFocus),NULL, this);
	text_ctrl->Disconnect( wxEVT_LEFT_DOWN,	wxMouseEventHandler(HexEditor::OnTextMouseFocus),NULL, this);
	hex_ctrl ->Disconnect( wxEVT_LEFT_UP,	wxMouseEventHandler(HexEditor::OnMouseSelectionEnd),NULL, this);
	text_ctrl->Disconnect( wxEVT_LEFT_UP,	wxMouseEventHandler(HexEditor::OnMouseSelectionEnd),NULL, this);
	hex_ctrl ->Disconnect( wxEVT_RIGHT_DOWN,wxMouseEventHandler(HexEditor::OnMouseTest),NULL, this);
	text_ctrl->Disconnect( wxEVT_RIGHT_DOWN,wxMouseEventHandler(HexEditor::OnMouseTest),NULL, this);
	hex_ctrl ->Disconnect( wxEVT_MOTION,	wxMouseEventHandler(HexEditor::OnMouseMove),NULL, this);
	text_ctrl->Disconnect( wxEVT_MOTION,	wxMouseEventHandler(HexEditor::OnMouseMove),NULL, this);
	}

void HexEditor::FileOpen(wxFileName& myfn){
	myfilename = wxFileName(myfn);
	if(myfile!=NULL) wxLogError(_("Critical Error. File pointer is not empty!"));
	else if(myfilename.FileExists()){
		myfile = new FileDifference( myfilename );
		if(myfile->IsOpened()){
			myscroll = new scrollthread(0,this);
//			copy_mark = new copy_maker();
			LoadFromOffset(0, true);
			offset_scroll->SetRange((myfile->Length() / hex_ctrl->ByteCapacity())+1);//Adjusting slider to page size
			}
		else{
			wxDialog *dlg = new wxMessageDialog(this,_("File cannot open."),_("Error"), wxOK|wxICON_ERROR, wxDefaultPosition);
			dlg->ShowModal();dlg->Destroy();
			}
		SetHexInsertionPoint(0);
		}
	else{
		wxMessageDialog *dlg = new wxMessageDialog(this,_("File not found."),_("Error"), wxOK|wxICON_ERROR, wxDefaultPosition);
		dlg->ShowModal();dlg->Destroy();
		}
	}

bool HexEditor::FileSave( bool question ){
	wxMessageDialog *msg = new wxMessageDialog(
								this, _( "Do you want to save this file?\n")
							, _("File Save"), wxYES_NO|wxCANCEL|wxICON_QUESTION, wxDefaultPosition);
	int selection=msg->ShowModal();
	msg->Destroy();
	switch( selection ){
		case(wxID_YES):
			if( !myfile->Apply() ){
				wxMessageDialog *msg = new wxMessageDialog(
							this, _( "File cannot saved. Operation Cancelled\n")
							, _("File Save Error"), wxOK|wxICON_ERROR, wxDefaultPosition);
				msg->ShowModal();
				msg->Destroy();
				return false;
				}
		case(wxID_NO):
			return true;
		case(wxID_CANCEL):
		default: return false;
		}
	}

bool HexEditor::FileClose( void ){
// TODO (death#1#): repair here!
	if( myfile->IsChanged() ){
		wxMessageDialog *msg = new wxMessageDialog(
								this, _( "Do you want to save file?\n")
							, _("File Has Changed!"), wxYES_NO|wxCANCEL|wxICON_QUESTION, wxDefaultPosition);
		int state = msg->ShowModal();
		msg->Destroy();
		switch(state){
			case(wxID_YES):
				if( !FileSave( false ) )
					return false;
				break;
			case(wxID_NO):
				break;
			case(wxID_CANCEL):
				return false;
			}
		}
	myscroll->Exit();
	delete myscroll;
	if( myfile != NULL )
		myfile->Close();
//	delete copy_mark;
	Clear( true );
	delete myfile;
	myfile = NULL;
	myfilename.Clear();
	return true;
	}

bool HexEditor::FileUndo( void ){
	GoTo( myfile->Undo() );
	}
bool HexEditor::FileRedo( void ){
	GoTo( myfile->Redo() );
	}

void HexEditor::GoTo( int64_t goto_offset ){
	if( goto_offset == -1){	//if cursor_offset not returned
		goto_offset = CursorOffset();
		start_offset -= goto_offset % hex_ctrl->BytePerLine();	//to allign offset
		if( start_offset < 0 ) start_offset = 0;
		LoadFromOffset( start_offset );
		SetLocalHexInsertionPoint( goto_offset*2 );
		}
	else					//if cursor_offset is returned
		if(goto_offset <= CursorOffset() && goto_offset + hex_ctrl->ByteCapacity() >= CursorOffset() ){ // cursor_offset is in visible area
			//LoadFromOffset( current_offset );
			SetLocalHexInsertionPoint( CursorOffset()*2 );
			}
		else{// out of view
			start_offset = CursorOffset();
			start_offset -= static_cast<int64_t>( hex_ctrl->ByteCapacity() * 0.20 ); //make load some lines to upside
			start_offset -= CursorOffset() % hex_ctrl->BytePerLine();	//to allign offset
			if(start_offset < 0) start_offset = 0;
			LoadFromOffset( start_offset );
			SetLocalHexInsertionPoint( CursorOffset()/2 );
			}
	}

void HexEditor::OnOffsetScroll( wxScrollEvent& event ){
    start_offset = static_cast<int64_t>(offset_scroll->GetThumbPosition()) * ByteCapacity();
    LoadFromOffset( start_offset );
    UpdateCursorLocation();
    wxYieldIfNeeded();
	}

void HexEditor::LoadFromOffset(int64_t position, bool cursor_reset, bool paint){
	static wxMutex MyMutex;
	MyMutex.Lock();

    myfile->Seek(position, wxFromStart);
	char *buffer = new char[ ByteCapacity() ];
	int readedbytes = myfile->Read(buffer, ByteCapacity());
	ReadFromBuffer( position, readedbytes, buffer, cursor_reset, paint );
	delete buffer;

	MyMutex.Unlock();
	}

void HexEditor::OnResize( wxSizeEvent &event){
	HexEditorCtrl::OnResize( event );
	//event.Skip( true );
	if(myfile != NULL && 0 < ByteCapacity()){
		offset_scroll->SetScrollbar(start_offset / ByteCapacity(),
									1,
									1+(myfile->Length() / ByteCapacity()),
									1,true
									);
		LoadFromOffset(start_offset, false);
		}
    }

bool HexEditor::Select ( int64_t start_offset, int64_t end_offset ){
	if( start_offset < 0 || end_offset < 0 ||
		start_offset > myfile->Length() ||  end_offset > myfile->Length()){
		wxBell();
		return false;
		}
	selection.state	= selector::SELECTION_END;
	selection.start_offset = start_offset;
	selection.end_offset  = end_offset;
	PaintSelection( );
	return true;
	}

void inline HexEditor::ClearPaint( void ){
	hex_ctrl ->ClearSelection();
	text_ctrl->ClearSelection();
	}

bool HexEditor::Selector( bool mode ){
	static bool polarity_possitive;
	if( FindFocus() == hex_ctrl || FindFocus() == text_ctrl )
		selection.end_offset = start_offset + GetLocalHexInsertionPoint()/2;
	else{
		wxLogError(wxT("Selector without focuse captured"));
		return false;
		}

	bool first_selection = false;
	if( selection.state != selector::SELECTION_TRUE ){			// If no selection available,
		selection.state	= selector::SELECTION_TRUE;				// then selection start procedure code
		selection.start_offset	= selection.end_offset;
		polarity_possitive		= false;
		first_selection			= true;
		}

	if( statusbar != NULL ){
		int start = selection.start_offset;
		int end = selection.end_offset;
		if(start > end ){
			int temp = start;
			start = end;
			end = temp;
			}
		if( selection.state == selector::SELECTION_FALSE ){
			statusbar->SetStatusText(_("Block: \tn/a"), 3);
			statusbar->SetStatusText(_("Size: \tn/a") ,4);
			}

		else{
			statusbar->SetStatusText(wxString::Format(_("Block: \t%d -> %d"),start,end), 3);
			statusbar->SetStatusText(wxString::Format(_("Size= \t%d"), abs(start-end)+1), 4);
			}
		}
	return true;
	}

void HexEditor::OnKeyboardSelector(wxKeyEvent& event){
	if(! event.ShiftDown() ){
		if( selection.state == selector::SELECTION_TRUE )
			selection.state = selector::SELECTION_END;
			}
    else
		Selector();
	}

// TODO (death#1#): BUG: Remove Text Selection when UNDO (CTRL+SHIFT)
// TODO (death#1#): BUG: Hex-Text Selection at release shift action
// TODO (death#5#): File Name star'in * when file changed & saved
// TODO (death#1#): BUG: Remove Text Selection when UNDO (CTRL+SHIFT)
// TODO (death#1#): BUG: Hex-Text Selection at release shift action
// TODO (death#5#): File Name star'in * when file changed & saved

void HexEditor::OnKeyboardInput( wxKeyEvent& event ){
	if(myfile != NULL){
		void *myvoid = FindFocus();		//avoid reinterpret_cast for speed
		wxHexCtrl* myctrl;
		if( myvoid == text_ctrl )
			myctrl = text_ctrl;
		else if ( myvoid == hex_ctrl )
			myctrl = hex_ctrl;
		else
			{
			wxBell();
			return;
			}
		//Keyboard Selection Code
		if(	event.GetKeyCode()==WXK_UP || event.GetKeyCode()==WXK_NUMPAD_UP ||
			event.GetKeyCode()==WXK_DOWN || event.GetKeyCode()==WXK_NUMPAD_DOWN ||
			event.GetKeyCode()==WXK_LEFT || event.GetKeyCode()==WXK_NUMPAD_LEFT ||
			event.GetKeyCode()==WXK_RIGHT || event.GetKeyCode()==WXK_NUMPAD_RIGHT ||
			event.GetKeyCode()==WXK_HOME || event.GetKeyCode()==WXK_NUMPAD_HOME ||
			event.GetKeyCode()==WXK_END || event.GetKeyCode()==WXK_NUMPAD_END ||
			event.GetKeyCode()==WXK_PAGEUP || event.GetKeyCode()==WXK_NUMPAD_PAGEUP ||
			event.GetKeyCode()==WXK_PAGEDOWN || event.GetKeyCode()==WXK_NUMPAD_PAGEDOWN
			)
			OnKeyboardSelector(event);	//Selection Starter call

		switch (event.GetKeyCode()){
			case (WXK_UP):case (WXK_NUMPAD_UP):
				if (ActiveLine() == 1){	//If cursor at first line
					if( start_offset == 0 )
						wxBell();					// there is no line over up!
					else{							// Illusion code
						start_offset -= BytePerLine(); 	//offset decreasing one line
						LoadFromOffset( start_offset );	//update text with new location, makes screen slide illusion
						UpdateCursorLocation();
						}
					}
					else
						SetHexInsertionPoint( GetLocalHexInsertionPoint() - HexPerLine() );
					break;

				case (WXK_DOWN):case (WXK_NUMPAD_DOWN):
					if ( ActiveLine() == LineCount() ){			//If cursor at bottom of screen
						if(start_offset + ByteCapacity() < myfile->Length() ){//detects if another line is present or not
							int temp = GetLocalHexInsertionPoint();	//preserving cursor location
							start_offset += BytePerLine();		//offset increasing one line
							LoadFromOffset( start_offset );		//update text with new location, makes screen slide illusion
							SetHexInsertionPoint(temp);			//restoring cursor location
							}
						else{
							wxBell();											//there is no line to slide bell
							}
						}
					else if( ActiveLine() * HexPerLine() > GetLastPosition() )
						wxBell();	//If cursor at last line but not at bottom of screen
					else
						SetHexInsertionPoint( GetLocalHexInsertionPoint() + HexPerLine() );
					break;
// TODO (death#1#): bug in left & text ctrl
				case (WXK_LEFT):case (WXK_NUMPAD_LEFT):
					if( GetLocalHexInsertionPoint() == 0){
						if(start_offset == 0)
							wxBell();
						else{
							start_offset -= BytePerLine();
							LoadFromOffset( start_offset );
							SetHexInsertionPoint( HexPerLine() - 1 );
							}
						}
					else
						SetHexInsertionPoint( GetLocalHexInsertionPoint() - ((myctrl == hex_ctrl) ? 1 : 2) );
					break;
				case (WXK_RIGHT):case (WXK_NUMPAD_RIGHT):
					if( myctrl->GetInsertionPoint() >= myctrl->GetLastPosition() ){
						if(start_offset + ByteCapacity() < myfile->Length() ){	//Checks if its EOF or not
							start_offset += BytePerLine();
							LoadFromOffset( start_offset );
							SetHexInsertionPoint( (LineCount() - 1) * HexPerLine() );
							}
						else
							wxBell();
						}
					else
						SetHexInsertionPoint( GetLocalHexInsertionPoint() +  ((myctrl == hex_ctrl)? 1 : 2) );
					break;

				case (WXK_HOME):case(WXK_NUMPAD_HOME):{
					SetHexInsertionPoint( GetLocalHexInsertionPoint() - GetLocalHexInsertionPoint() % HexPerLine() );
					break;
					}
				case (WXK_END):case(WXK_NUMPAD_END):{
					SetHexInsertionPoint( GetLocalHexInsertionPoint() + HexPerLine() - (GetLocalHexInsertionPoint() % HexPerLine()) - 1  );
					break;
					}
				case (WXK_PAGEUP):case (WXK_NUMPAD_PAGEUP):
						if(start_offset - ByteCapacity() > 0){
//							int temp = GetHexInsertionPoint();
							start_offset -= ByteCapacity();
							LoadFromOffset(start_offset );
							UpdateCursorLocation();
//							SetHexInsertionPoint(temp);
							}
						else{
							int temp = GetLocalHexInsertionPoint() % HexPerLine();
							start_offset=0;
							LoadFromOffset( start_offset );
							SetHexInsertionPoint(temp);
							wxBell();
							}
					break;
// TODO (death#5#): Add last byte problem. Also text ctrl has +1 issue
				case (WXK_PAGEDOWN):case (WXK_NUMPAD_PAGEDOWN):
						if(start_offset + ByteCapacity()*2 < myfile->Length()){ //*2 for cosmetic
							int temp = GetLocalHexInsertionPoint();
							start_offset += ByteCapacity();
							LoadFromOffset( start_offset );
							SetHexInsertionPoint( temp );
							}
						else{
							int temp = ( GetLocalHexInsertionPoint() %  HexPerLine() ) + ( LineCount()-1 ) * HexPerLine();
							start_offset = myfile->Length() - ByteCapacity();
							start_offset += BytePerLine() - start_offset % BytePerLine(); //cosmetic
							LoadFromOffset( start_offset );
							SetHexInsertionPoint(temp);
							wxBell();
							}
					break;

				case (WXK_DELETE):case (WXK_NUMPAD_DELETE):
					/*if( myctrl->GetInsertionPoint() != myctrl->GetLastPosition() ){
						int hex_loc = myctrl->GetInsertionPoint();
						HexCharReplace(hex_loc,'0');
						myctrl->SetInsertionPoint(hex_loc);
						}
					else
					*/
						wxBell();
					break;

				case (WXK_BACK):
					/*
					if( myctrl->GetInsertionPoint()!=0 ){
						HexCharReplace(myctrl->GetInsertionPoint()-1,'0');
						myctrl->SetInsertionPoint(myctrl->GetInsertionPoint()-1);
						}
					else
						if(start_offset == 0)
							wxBell();
						else{	//Has to be a line over up
		// TODO (death#3#): if BytePerLine() changes, current offset gona be mad, like taking minus argument because it thinks a compleete line over up... spend caution about it.
							start_offset -= myctrl->BytePerLine();
							LoadFromOffset( start_offset );
							HexCharReplace(myctrl->HexPerLine()-1,'0');
							myctrl->SetInsertionPoint(myctrl->HexPerLine()-1);
							}
					*/
					break;

				case( 26 ):		// 26 == CTRL+Z = UNDO
					if(event.ShiftDown())
						FileRedo();	// UNDO with shift = REDO
					else
						FileUndo();
					break;
				case( 25 ):		// 25 == CTRL+Y = REDO
					FileRedo();
					break;
				case( 19 ):{	// 19 == CTRL+S = SAVE
					FileSave();
					// TODO (death#1#): File Name star'in * when file changed & saved
					}
					break;
				case(  1 ):		//  1 == CTRL+A = ALL
					//select(0, myTempFile->Length());
					break;
				case( 24 ):		// 24 == CTRL+X = CUT
					wxBell();
					break;
				case(  3 ):		//  3 == CTRL+C = COPY
					//copy( true );
					break;
				case( 22 ):		// 22 == CTRL+V = PASTE
					//replace();
					break;
				case(  6 ):		//  6 == CTRL+F = FIND
					//finddlg();
					break;
				case( 15 ):		// 15 == CTRL+O = OPEN
					wxBell();
					break;
				default:
					OnKeyboardChar( event );
					break;
				}//switch end
			if( offset_scroll->GetRange() != (myfile->Length() / ByteCapacity()))
				offset_scroll->SetRange((myfile->Length() / ByteCapacity())+1);
			if( offset_scroll->GetThumbPosition() != start_offset / ByteCapacity() )
				offset_scroll->SetThumbPosition( start_offset / ByteCapacity() );
			OnKeyboardSelector(event);
			PaintSelection( );
		}
	}

void HexEditor::OnKeyboardChar( wxKeyEvent& event ){
	if(myfile != NULL){
		wxChar chr = event.GetKeyCode();
		void *myctrl = FindFocus();		//avoid reinterpret_cast for speed
		if( myctrl == text_ctrl )
			myctrl = text_ctrl;
		else if ( myctrl == hex_ctrl )
			myctrl = hex_ctrl;
		else
			{
			wxBell();
			return;
			}

		if( myctrl == hex_ctrl )
			if( isxdigit(chr) ){
				(chr>='a'&&chr<='f')?(chr-=('a'-'A')):(chr=chr);		// Upper() for Char
				HexCharReplace( GetLocalHexInsertionPoint(), chr);		// write to screen

				char rdchr = hex_ctrl->ReadByte( GetLocalHexInsertionPoint()/2 );	// read from screen
				myfile->Add( CursorOffset(), &rdchr ,1);				// add node to file

				if( hex_ctrl->GetInsertionPoint() >= hex_ctrl->GetLastPosition() ){
					if( CursorOffset() + hex_ctrl->ByteCapacity() <= myfile->Length() ){	//Checks if its EOF or not
						start_offset += hex_ctrl->BytePerLine();
						LoadFromOffset( start_offset );
						hex_ctrl->LastLine();
						}
					else
						wxBell();
					}
                else
                	hex_ctrl->NextChar();
				}
			else
				wxBell();
		else if( myctrl == text_ctrl ){
			if( wxString((wxChar)event.GetKeyCode()).IsAscii() &&
				event.GetKeyCode()!=WXK_BACK &&
				event.GetKeyCode()!=WXK_DELETE &&
				!event.ControlDown()
				){

/*				if( text_ctrl->GetInsertionPoint() >= text_ctrl->GetLastPosition()){
					if(current_offset+hex_ctrl->ByteCapacity() <= myTempFile->Length() ){	//Checks if its EOF or not
						current_offset += hex_ctrl->BytePerLine();
						LoadFromOffset(current_offset);
						text_ctrl->LastLine();	// HexPerLine = BytePerLine()*2
						}
					}
*/
				TextCharReplace(GetLocalInsertionPoint(), event.GetKeyCode());

				char rdchr = hex_ctrl->ReadByte( GetLocalInsertionPoint() );	// read from screen
				myfile->Add( CursorOffset(), &rdchr ,1);				// add node to file

				if( text_ctrl->GetInsertionPoint() >= text_ctrl->GetLastPosition() ){
					if( start_offset + hex_ctrl->ByteCapacity() <= myfile->Length() ){	//Checks if its EOF or not
						start_offset += hex_ctrl->BytePerLine();
						LoadFromOffset(start_offset);
						text_ctrl->LastLine();
						}
					else
						wxBell();
					}
                else
                	text_ctrl->NextChar();
				}
			else
				wxBell();
			}
		}
	}

//void HexEditor::RefreshCursor( int64_t cursor_offset ){
	/*
	if(cursor_hex_offset == -1){	//if cursor_offset is not returned
		cursor_hex_offset = GetHexOffset();
		start_offset -= cursor_hex_offset % hex_ctrl->BytePerLine();	//to allign offset
		if(start_offset < 0) start_offset = 0;
			LoadFromOffset( start_offset );
		}
	else
		if(	start_offset <= cursor_hex_offset/2 &&
			start_offset+hex_ctrl->ByteCapacity() >= cursor_hex_offset/2){ // cursor_offset is in visible area
			//LoadFromOffset( start_offset );
			SetHexInsertionPoint( cursor_hex_offset/2 );
			}
		else{// out of view
			start_offset = cursor_offset;
			start_offset -= static_cast<int64_t>( hex_ctrl->ByteCapacity() * 0.20 ); //make load some lines to upside
			start_offset -= cursor_offset % hex_ctrl->BytePerLine();	//to allign offset
			if(start_offset < 0) start_offset = 0;
			LoadFromOffset( start_offset );
			SetHexInsertionPoint( cursor_hex_offset/2 );
			}
	*/
//	}

void HexEditor::OnHexMouseFocus(wxMouseEvent& event){
	selection.state=selector::SELECTION_FALSE;
	statusbar->SetStatusText(_("Block: \tn/a"), 3);
	statusbar->SetStatusText(_("Size: \tn/a") ,4);
	ClearPaint();
	hex_ctrl->SetFocus();
	SetHexInsertionPoint( hex_ctrl->PixelCoordToInternalPosition( wxPoint( event.GetX(),event.GetY() ) ) );
	}

void HexEditor::OnTextMouseFocus(wxMouseEvent& event){
	selection.state=selector::SELECTION_FALSE;
	statusbar->SetStatusText(_("Block: \tn/a"), 3);
	statusbar->SetStatusText(_("Size: \tn/a") ,4);
	ClearPaint();
	text_ctrl->SetFocus();
	SetHexInsertionPoint( 2 * text_ctrl->PixelCoordToInternalPosition( wxPoint( event.GetX(),event.GetY() ) ) );
	}

void HexEditor::OnMouseWhell( wxMouseEvent& event ){
#ifdef _DEBUG_
	std::cout << "MouseWhell Rotation = " << event.GetWheelRotation() << "\t Delta = " << event.GetWheelDelta() << std::endl;
#endif
	if(event.GetWheelRotation() > 0){
//		if (ActiveLine() == 1){	//If cursor first line
			if( start_offset == 0 )
				wxBell();					// there is no line over up!
			else{							// Illusion code
				start_offset -= BytePerLine(); 	//offset decreasing one line
				LoadFromOffset( start_offset );	//update text with new location, makes screen slide illusion
				SetHexInsertionPoint(GetLocalHexInsertionPoint() + HexPerLine());	//restoring cursor location
				}
			}
//		else
//			SetHexInsertionPoint( GetLocalHexInsertionPoint() - HexPerLine() );
//		}
	else if(event.GetWheelRotation() < 0 ){
//		if ( ActiveLine() == LineCount() ){			//If cursor at bottom of screen
			if(start_offset + ByteCapacity() < myfile->Length() ){//detects if another line is present or not
				start_offset += BytePerLine();		//offset increasing one line
				LoadFromOffset( start_offset );		//update text with new location, makes screen slide illusion
				SetHexInsertionPoint(GetLocalHexInsertionPoint() - HexPerLine());	//restoring cursor location
				}
			else{
				wxBell();							//there is no line to slide bell
				}
			}
//		else
//			SetHexInsertionPoint( GetLocalHexInsertionPoint() + HexPerLine() );
//		}
	}

void HexEditor::OnMouseMove( wxMouseEvent& event ){
#ifdef _DEBUG_
	std::cout << "MouseMove Coordinate X:Y = " << event.m_x	<< " " << event.m_y
			<< "\tLeft mouse button:" << event.m_leftDown << std::endl;
#endif
	if(event.m_leftDown){
		int spd=0;
		if( event.m_y < 0 &&start_offset != 0){
			spd = static_cast<int>(0 - pow(2, abs(event.GetY()) / 25));
			(spd < -1024) ? (spd = -1024):(spd=spd);
			}
		else if(event.m_y > hex_ctrl->GetSize().GetHeight() && start_offset + hex_ctrl->ByteCapacity() < myfile->Length()){
			int pointer_diff = event.GetY() - hex_ctrl->GetSize().GetHeight();
			spd = static_cast<int>(pow(2, pointer_diff / 25));
			(spd > 1024) ? (spd = 1024):(spd=spd);
			}
		myscroll->UpdateSpeed(spd);
#ifdef _DEBUG_
		std::cout << "Scroll Speed = " << spd << std::endl;
#endif
		int new_location;
		if( FindFocus() == hex_ctrl )
			new_location = hex_ctrl->PixelCoordToInternalPosition( wxPoint( event.GetX(), event.GetY() ) );
		else if ( FindFocus() == text_ctrl )
			new_location = 2*(text_ctrl->PixelCoordToInternalPosition( wxPoint( event.GetX(), event.GetY() ) ));

		int old_location = GetLocalHexInsertionPoint();
		if( new_location != old_location ){
			if(selection.state != selector::SELECTION_TRUE)		// At first selection
				Selector();
			SetHexInsertionPoint( new_location );
			Selector();
			PaintSelection( );
			}
		}
	}

void HexEditor::OnMouseSelectionEnd( wxMouseEvent& event ){
	if(selection.state == selector::SELECTION_TRUE)
		selection.state = selector::SELECTION_END;
	event.Skip();
	myscroll->UpdateSpeed( 0 );
	}
int64_t HexEditor::FileLenght(){
	return myfile->Length();
	}

void HexEditor::SetHexInsertionPoint( int local_hex_location){
	SetLocalHexInsertionPoint( local_hex_location );
	UpdateCursorLocation();
	}

void HexEditor::UpdateCursorLocation(){
	static wxMutex update;
	update.Lock();
	if( GetLocalHexInsertionPoint()/2+start_offset > myfile->Length() ){
		SetLocalHexInsertionPoint( (myfile->Length() - start_offset)*2 - 1 );
		}

	if( interpreter != NULL ){
		myfile->Seek( GetLocalHexInsertionPoint()/2+start_offset, wxFromStart );
		char *bfr = new char [8];
		int size=myfile->Read( bfr, 8);
		interpreter->Set( bfr, size);
		delete bfr;
		}

	if( statusbar != NULL ){
		statusbar->SetStatusText(wxString::Format(_("Page: %d"), CursorOffset()/hex_ctrl->ByteCapacity() ), 0);
		if( hex_offset )
			statusbar->SetStatusText(wxString::Format(_("Offset: 0x%llX"), CursorOffset() ), 1);
		else
			statusbar->SetStatusText(wxString::Format(_("Offset: %lld"), CursorOffset() ), 1);
		uint8_t ch;
		myfile->Seek( CursorOffset() );
		myfile->Read( reinterpret_cast<char*>(&ch), 1);
		statusbar->SetStatusText(wxString::Format(_("=\t%u"), ch), 2);
		//statbar->SetStatusText(wxString::Format(_("Block: %d-%d"),0,0), 3);
		//statbar->SetStatusText(wxString::Format(_("Size: %d"),myTempFile->Length()), 4);
		}
	update.Unlock();
	}

void HexEditor::OnOffsetMouseFocus( wxMouseEvent& event ){
	if( ! hex_offset )	// ! needed hex_offset shows current state, post state is different
		statusbar->SetStatusText(wxString::Format(_("Offset: 0x%llX"), CursorOffset() ), 1);
	else
		statusbar->SetStatusText(wxString::Format(_("Offset: %lld"), CursorOffset() ), 1);
	event.Skip();
	}

void HexEditor::OnMouseTest( wxMouseEvent& event ){
	myfile->ShowDebugState();
	}

