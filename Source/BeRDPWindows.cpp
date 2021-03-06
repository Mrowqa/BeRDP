/*

BeRDPWindow

Author: Sikosis (phil@sikosis.com)

(C)2003-2004 Shell Created using BRIE (http://brie.sf.net/)

*/

// Includes ------------------------------------------------------------------------------------------ //
#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Menu.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <Screen.h>
#include <ScrollView.h>
#include <Slider.h>
#include <stdio.h>
#include <stdlib.h>
#include <String.h>
#include <StringView.h>
#include <TabView.h>
#include <TextView.h>
#include <TextControl.h>
#include <Window.h>
#include <View.h>
#include <iostream.h>
#include <fstream.h>

#include "BeRDP.h"
#include "BeRDPWindows.h"
#include "BeRDPViews.h"
#include "BeRDPConstants.h"
// -------------------------------------------------------------------------------------------------- //



// CenterWindowOnScreen -- Centers the BWindow to the Current Screen
static void CenterWindowOnScreen(BWindow* w)
{
	BRect	screenFrame = (BScreen(B_MAIN_SCREEN_ID).Frame());	BPoint	pt;
	pt.x = screenFrame.Width()/2 - w->Bounds().Width()/2;
	pt.y = screenFrame.Height()/2 - w->Bounds().Height()/2;

	if (screenFrame.Contains(pt))
		w->MoveTo(pt);
}
// -------------------------------------------------------------------------------------------------- //

// BeRDPWindow - Constructor
BeRDPWindow::BeRDPWindow(BRect frame) : BWindow (frame, "BeRDP v0.5", B_TITLED_WINDOW, B_NORMAL_WINDOW_FEEL , 0)
{
	InitWindow();
	CenterWindowOnScreen(this);

	// Load User Settings 
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY,&path);
	path.Append("BeRDP/BeRDP_Settings",true);
	BFile file(path.Path(),B_READ_ONLY);
	BMessage msg;
	msg.Unflatten(&file);
	LoadSettings (&msg);
    
    // Set Window Limits
	SetSizeLimits(315,315,205,230);
    
    // Update from our Loaded Settings
    UpdateDisplaySlider();
    
	Show();
}
// -------------------------------------------------------------------------------------------------- //

// BeRDPWindow - Destructor
BeRDPWindow::~BeRDPWindow()
{
	exit(0);
}
// -------------------------------------------------------------------------------------------------- //

// BeRDPWindow::InitWindow -- Initialization Commands here
void BeRDPWindow::InitWindow(void)
{
	BRect r;
	BRect rtab;
	BRect rlist;
	r = Bounds();
    rtab = Bounds();
    rtab.top += 10;
    rlist = Bounds();
    rlist.top += 44;
    rlist.left += 12;
    rlist.right -= 12;
    rlist.bottom -= 12;
    
    // Create Views for our Tabs
    ptrGeneralView = new GeneralView(r);
    ptrDisplayView = new DisplayView(r);
    ptrAboutView = new AboutView(r);
    
    // Create the Buttons for GeneralView
	btnConnect = new BButton(BRect (133,150,213,180), 
					"btnConnect","Connect", new BMessage(BTN_CONNECT),
					B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
	btnConnect->MakeDefault(true);
	btnConnect->MakeFocus(true);					
	btnClose = new BButton(BRect (225,150,305,180), 
					"btnClose","Close", new BMessage(BTN_CLOSE),
					B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);	
	ptrGeneralView->AddChild(btnConnect);
    ptrGeneralView->AddChild(btnClose);
    
    
    // Create Connection Menu for GeneralView
    BString DefaultIP;
    DefaultIP.SetTo("Default"); // debug
    
    pmnConnection = new BPopUpMenu("", true, true);
    pmnConnection->AddItem(new BMenuItem(" ... ", new BMessage(MENU_CONNECTION_ELLIPSIS)));
    pmnConnection->AddItem(new BMenuItem(DefaultIP.String(), new BMessage(MENU_CONNECTION_DEFAULT)));
    pmnConnection->AddSeparatorItem();
    pmnConnection->AddItem(new BMenuItem("New Connection", new BMessage(MENU_NEW_CONNECTION)));
    pmnConnection->AddItem(mniConnectionDelete = new BMenuItem("Delete Current", new BMessage(MENU_CON_DELETE_CURRENT)));
    mniConnectionDelete->SetEnabled(false);

    mnfConnection = new BMenuField(BRect(5,25,220,40), "connection_menu", "Connection Name:", pmnConnection, true);
	mnfConnection->SetDivider(110);
	mnfConnection->SetFont(be_bold_font);
	mnfConnection->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
    /*conmenufield = new BMenu(" ... ");
    conmenufield->AddItem(new BMenuItem(DefaultIP.String(), new BMessage(MENU_CONNECTION_DEFAULT)));
    conmenufield->AddSeparatorItem();
    conmenufield->AddItem(new BMenuItem("New Connection", new BMessage(MENU_NEW_CONNECTION)));
    conmenufield->AddItem(menucondelete = new BMenuItem("Delete Current", new BMessage(MENU_CON_DELETE_CURRENT)));
    menucondelete->SetEnabled(false);
    connectionmenufield = new BMenuField(BRect (5,25,220,40),"connection_menu","Connection Name:",conmenufield,B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    connectionmenufield->SetDivider(110);	
    connectionmenufield->SetFont(be_bold_font);
    connectionmenufield->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));*/
    
    // Create TextControls for GeneralView
    txtComputer = new BTextControl(BRect (30,50,220,65), "txtComputer",
      					"Computer: ", "", new BMessage (TXT_COMPUTER),
						B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
	txtComputer->SetDivider(55);
	txtUsername = new BTextControl(BRect (30,75,250,90), "txtUsername",
      					"Username: ", "", new BMessage (TXT_USERNAME),
						B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
	txtUsername->SetDivider(55);					
    
    txtPassword = new BTextControl(BRect (30,100,250,115), "txtPassword",
      					"Password: ", "", new BMessage (TXT_PASSWORD),
						B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
	txtPassword->SetDivider(55);					
    txtPassword->SetEnabled(false);
    
    txtDomain = new BTextControl(BRect (30,125,250,140), "txtDomain",
      					"Domain: ", "", new BMessage (TXT_DOMAIN),
						B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
	txtDomain->SetDivider(55);

	ptrGeneralView->AddChild(mnfConnection);
	ptrGeneralView->AddChild(txtComputer);
	ptrGeneralView->AddChild(txtUsername);					
    ptrGeneralView->AddChild(txtPassword);					
    ptrGeneralView->AddChild(txtDomain);
    
    // Create CheckBox for Display
    chkForceBitmapUpdates = new BCheckBox(BRect (30,69,300,79), "chkForceBitmapUpdates",
      					"Force Bitmap Updates", new BMessage (CHK_FORCE_BITMAP_UPDATES),
      					B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    
    // Create Slider for Display
    sldDisplaySize = new BSlider(BRect (20,15,r.right - 20,30), "sldDisplaySize",
      					"Full Screen", new BMessage (SLD_DISPLAYSIZE), 0, 3,
      					B_BLOCK_THUMB, B_FOLLOW_LEFT | B_FOLLOW_TOP,
						B_FRAME_EVENTS|B_WILL_DRAW | B_NAVIGABLE);
	sldDisplaySize->SetHashMarkCount(1);
	sldDisplaySize->SetKeyIncrementValue(1);					
    sldDisplaySize->SetHashMarks(B_HASH_MARKS_BOTH);
    sldDisplaySize->SetLimitLabels("Less", "More");
    sldDisplaySize->SetValue(3);  
    
   // int wsleft = 175;
    
    // Add them all to Display Tab
    ptrDisplayView->AddChild(sldDisplaySize);
    ptrDisplayView->AddChild(chkForceBitmapUpdates);
          
    // Create StringViews for AboutView
    float fLeftMargin = 8;
    float fRightMargin = 310;
    float fDescTop = 60;
    stvTitle = new BStringView(BRect (fLeftMargin,10,fRightMargin,25), "BeRDP", "BeRDP - BeOS Remote Desktop Client",
      				B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
    stvAuthor1 = new BStringView(BRect (fLeftMargin,30,fRightMargin,45), "Author 1", "Coded by Sikosis",
      				B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
      				
    
	txvDescription = new BTextView(BRect(fLeftMargin, fDescTop, fRightMargin, fDescTop+80),
						 "Description", BRect(0,0,fRightMargin-fLeftMargin,80), B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	txvDescription->SetWordWrap(true);
	txvDescription->MakeEditable(true);
	txvDescription->SetStylable(true);
	txvDescription->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	txvDescription->Insert("BeRDP is a Remote Desktop Protocol Client which is used to connect to Microsoft Windows 2000/2003 Servers and Windows XP Professional workstations.\n\nThis version requires mmu_man\'s native rdesktop command line app. Respect!");
      				
    stvURL = new BStringView(BRect (fLeftMargin,153,r.right,168), "URL", "Web Site: http://berdp.sf.net/",
      				B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
    ptrAboutView->AddChild(stvAuthor1);
    ptrAboutView->AddChild(stvTitle);
    ptrAboutView->AddChild(txvDescription); 
    ptrAboutView->AddChild(stvURL); 				
        
	// Create the TabView and Tabs
	tabView = new BTabView(rtab,"berdp_tabview");
	tabView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	rtab = tabView->Bounds();
	rtab.InsetBy(5,5);
	tab = new BTab();
	tabView->AddTab(ptrGeneralView, tab);
	tab->SetLabel("General");
	tab = new BTab();
	tabView->AddTab(ptrDisplayView, tab);
	tab->SetLabel("Display");
	tab = new BTab();
	tabView->AddTab(ptrAboutView, tab);
	tab->SetLabel("About BeRDP");
	
	// Create the Views
	AddChild(ptrBeRDPView = new BeRDPView(r));
	ptrBeRDPView->AddChild(tabView);
}
// -------------------------------------------------------------------------------------------------- //


// BeRDPWindow::QuitRequested -- Post a message to the app to quit
bool BeRDPWindow::QuitRequested()
{

	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
// -------------------------------------------------------------------------------------------------- //


// BeRDPWindow::LoadSettings -- Loads your current settings
void BeRDPWindow::LoadSettings(BMessage *msg)
{
	BRect     frame;
	BString   tmpC;
	BString   tmpU;
	BString   tmpP;
	BString   tmpD;
	BString   tmpWSP;
	long 	  FBU;
	long      slide;

	if (B_OK == msg->FindRect("windowframe",&frame)) {
		MoveTo(frame.left,frame.top);
		ResizeTo(frame.right-frame.left,frame.bottom-frame.top);
	}
	
	if (B_OK == msg->FindString("txtComputer",&tmpC)) {
		txtComputer->SetText(tmpC.String());
	}
	
	if (B_OK == msg->FindString("txtUsername",&tmpU)) {
		txtUsername->SetText(tmpU.String());
	}
	
	if (B_OK == msg->FindString("txtPassword",&tmpP)) {
		txtPassword->SetText(tmpP.String());
	}
	
	if (B_OK == msg->FindString("txtDomain",&tmpD)) {
		txtDomain->SetText(tmpD.String());
	}
	
	if (B_OK == msg->FindInt32("chkForceBitmapUpdates",&FBU)) {
		if (&FBU != 0) {
			chkForceBitmapUpdates->SetValue(FBU);
    	}	
	}
	if (B_OK == msg->FindInt32("sldDisplaySize",&slide)) {
		sldDisplaySize->SetValue(slide);
	}
	
}
// -------------------------------------------------------------------------------------------------- //


// BeRDPWindow::SaveConnectionList -- Saves the List of Conection Names from conmenufield
void BeRDPWindow::SaveConnectionList(void)
{
	BMessage msg;
	int TotalMenuItems, Counter, NumberOfConnections;
	
	TotalMenuItems = pmnConnection->CountItems();
	printf("Number of Items: %d\n",TotalMenuItems); // debug
	
	if (TotalMenuItems > 5) {
		NumberOfConnections =  TotalMenuItems - 5;
		msg.AddInt32("NumberOfConnections",NumberOfConnections);
		BString OneItem;
		Counter=0;
		while(Counter<TotalMenuItems) {
			OneItem.SetTo(pmnConnection->ItemAt(Counter)->Label());
			printf ("Item #%d - %s\n",Counter,OneItem.String()); // debug
			Counter++;
		}
		// Save Connection List
		BPath path;
		status_t result = find_directory(B_USER_SETTINGS_DIRECTORY,&path);
		if (result == B_OK) {
			path.Append("BeRDP/BeRDP_Connection_List",true);
			BFile file(path.Path(),B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
			msg.Flatten(&file);
		}
	} else {
		// Default Connection List - No Need to Save
	}
}
// -------------------------------------------------------------------------------------------------- //


// BeRDPWindow::SaveConnectionDetails -- Saves the Users settings
void BeRDPWindow::SaveConnectionDetails(const char *cnxname)
{
	BMessage msg;
	msg.AddString("txtComputer",txtComputer->Text());
	msg.AddString("txtUsername",txtUsername->Text());
	msg.AddString("txtPassword",txtPassword->Text());
	msg.AddString("txtDomain",txtDomain->Text());
	msg.AddInt32("chkForceBitmapUpdates",chkForceBitmapUpdates->Value());
	msg.AddInt32("sldDisplaySize",sldDisplaySize->Value());
	
	BPath path;
	status_t result = find_directory(B_USER_SETTINGS_DIRECTORY,&path);
	if (result == B_OK) {
		// Check for BeRDP Dir
		status_t err;
		BString cnxfilename;
		cnxfilename.SetTo(path.Path());
		cnxfilename.Append("/BeRDP");
		BEntry entry(cnxfilename.String());
		if (entry.Exists() == true) {
			printf("BeRDP Directory Exists.\n");
		} else {
			printf("BeRDP Directory does not exist.\n");
			// Create Directory
			path.SetTo(cnxfilename.String());
			err = create_directory(path.Path(),777);
			if (result == B_OK) {
				printf("Directory Created - %d\n",err);
			} else {
				printf("Sorry - Could not Create Directory &i\n",err);
			}
		}
		cnxfilename.Append("/");
		cnxfilename.Append(cnxname);
		cnxfilename.Append(".berdp");
		
		//BMenuItem *CurrentItem = new BMenuItem(conmenufield);
		//CurrentItem = conmenufield->FindMarked();
		//printf("Save Connection Details Filename: %s / Total Items: %d\n\n",CurrentItem->Label(),
		//	conmenufield->CountItems());
		printf("Save Connection Details Filename: %s\n\n",cnxfilename.String());
		path.SetTo(cnxfilename.String());
		BFile file(path.Path(),B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
		msg.Flatten(&file);
	}
}
// -------------------------------------------------------------------------------------------------- //


// BeRDPWindow::SaveSettings -- Saves the Users settings
void BeRDPWindow::SaveSettings(const char *cnxname)
{
	BMessage msg;
	msg.AddRect("windowframe",Frame());
	msg.AddString("cnxname",cnxname);
	
	printf("Connection Name is %s\n\n",cnxname);
	
	// these will be moved to the function above shortly
	//msg.AddString("txtComputer",txtComputer->Text());
	//msg.AddString("txtUsername",txtUsername->Text());
	//msg.AddString("txtPassword",txtPassword->Text());
	//msg.AddString("txtDomain",txtDomain->Text());
	//msg.AddInt32("chkForceBitmapUpdates",chkForceBitmapUpdates->Value());
	//msg.AddInt32("sldDisplaySize",sldDisplaySize->Value());
	
	BPath path;
	status_t result = find_directory(B_USER_SETTINGS_DIRECTORY,&path);
	if (result == B_OK) {
		path.Append("BeRDP/BeRDP_Settings",true);
		BFile file(path.Path(),B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
		msg.Flatten(&file);
	}
}
// -------------------------------------------------------------------------------------------------- //


void BeRDPWindow::UpdateDisplaySlider()
{
	switch (sldDisplaySize->Value())
	{
		case 0:
			{
				sldDisplaySize->SetLabel("640 by 480 pixels");
			}
			break;
		case 1:
			{
				sldDisplaySize->SetLabel("800 by 600 pixels");
			}
			break;
		case 2:
			{
				sldDisplaySize->SetLabel("1024 by 768 pixels");
			}
			break;
		case 3:
			{
				sldDisplaySize->SetLabel("Full Screen");
			}
			break;			
	}
	//printf("Slider Value: %d\n",sldDisplaySize->Value());
}
// -------------------------------------------------------------------------------------------------- //


// BeRDPWindow::MessageReceived -- receives messages
void BeRDPWindow::MessageReceived (BMessage *message)
{
	BString cnxname;
	int index;

	BMenuItem *CurrentItem = new BMenuItem(pmnConnection);
	CurrentItem = pmnConnection->FindMarked();
	index = pmnConnection->IndexOf(CurrentItem);
	if (index > -1) {
		printf("CurrentItem - %s // index - %d\n\n",CurrentItem->Label(),index); // debug
		cnxname.SetTo(pmnConnection->ItemAt(index)->Label()); // debug
	} else {
		cnxname.SetTo("");
	}	
	
	switch(message->what)
	{
		case BTN_CONNECT:
			{
				SaveConnectionDetails(cnxname.String()); // debug
				SaveConnectionList();
				SaveSettings(cnxname.String());
				
				// Remove the Script (as we no longer need it)
				char tmp[256];
				sprintf(tmp,"rm -r %s/berdp.sh","/boot/home");
				system(tmp);
				
				BString cmdline;
				cmdline.SetTo("rdesktop ");
				switch (sldDisplaySize->Value())
				{
					case 0:
						{
							cmdline.Append("-g640x480 ");
						}
						break;
					case 1:
						{
							cmdline.Append("-g800x600 ");
						}
						break;
					case 2:
						{
							cmdline.Append("-g1024x768 ");
						}
						break;
					case 3:
						{
							cmdline.Append("-f ");
						}
						break;			
				}
				
				BString tmpUsername;
				tmpUsername.SetTo(txtUsername->Text());
				
				if (tmpUsername.CountChars() > 0) {
					cmdline.Append("-u");
					cmdline.Append(txtUsername->Text());
					cmdline.Append(" ");
				}
				cmdline.Append("-d");
				cmdline.Append(txtDomain->Text());
				cmdline.Append(" ");
				if (chkForceBitmapUpdates->Value() == B_CONTROL_ON) {
					cmdline.Append("-b ");
				}
				cmdline.Append(txtComputer->Text());
				printf("%s\n\n",cmdline.String());
				
				// hold up wait a minute ...
				//
				// thx to some version fine work by mmu_man or tha man as i like to call him ;)
				// has released a native beos rdesktop - so we're removing 
				// all other options except for native.
		
				// Create our Script and Launch rdesktop with our params
				int x;
				FILE *f;
				char FileName[256];
				
				sprintf(FileName,"%s/berdp.sh","/boot/home");
				f = fopen(FileName,"w");
				x = fputs("#!/bin/sh\n\n",f);
				x = fputs("# BeRDP Connect String\n",f);
				x = fputs("# Coded by Sikosis\n",f);
				x = fputs("# Native RDesktop Command Line by mmu_man\n\n",f);
				x = fputs("# RDesktop Command Line Options\n",f);
				sprintf(tmp,"RDTP=\"%s\"\n\n",cmdline.String());
				x = fputs(tmp,f);
				x = fputs("$RDTP\n\n",f);
				fclose(f);
				
				// Minimize the Window
				Minimize(true);
				
				// Now Execute the Script
				system(FileName);
				
				// Remove the Script (as we no longer need it)
				sprintf(tmp,"rm -r %s/berdp.sh","/boot/home/");
				system(tmp);
				
				// Now Show it Again
				Minimize(false);
			}
			break;
		case MENU_NEW_CONNECTION:
			{
				// Launch Window - Maybe my InputBox Class ?
				
				BMenuItem *marked = pmnConnection->FindItem("New Connection");
				marked->SetMarked(true);
			}
			break;
		case MENU_CONNECTION_ELLIPSIS:
			{
				// debug info for testing 
				// this will eventually stored the last known config
				txtComputer->SetText("61.88.23.139");
				txtUsername->SetText("admin");
				txtDomain->SetText("WENCK");
				sldDisplaySize->SetValue(3);
				chkForceBitmapUpdates->SetValue(B_CONTROL_ON);
			}
			break;		
		case MENU_CONNECTION_DEFAULT:
			{
				// Set the Default Connections Settings
				printf("Set Default Settings\n\n");
				txtComputer->SetText("");
				txtUsername->SetText("");
				txtPassword->SetText("");
				txtDomain->SetText("");
				sldDisplaySize->SetValue(3);
				chkForceBitmapUpdates->SetValue(B_CONTROL_OFF);
				
				BMenuItem *marked = pmnConnection->FindItem("Default");
				marked->SetMarked(true);
			}
			break;
		case BTN_CLOSE:
			{
				//BString cnxname;
				// debug - should be the current selected item
				//cnxname.SetTo(conmenufield->ItemAt(conmenufield->CountItems())->Label()); // debug
				printf("Closing ... Connection Name is %s\n\n",cnxname.String());
				SaveSettings(cnxname.String());
				Quit();
			}
			break;
		case SLD_DISPLAYSIZE:
			{
				UpdateDisplaySlider();
			}
			break;		
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
// -------------------------------------------------------------------------------------------------- //

