

tMenuItem mainMenuItems[] =
{
    {"Help / About",	      	"HELP\n~5binarybond007@gmail.com\n",      	   0},     	
    {"Select Format", 	       	"SET_FORMAT",			      		   1}, 	 
    {"Regular search", 	    	"videos?v=2&alt=jsonc&q=%s",		           0},
    {"User Favorites",		"users/%s/favorites?v=2&alt=jsonc" ,	    	   0},
    {"User Upload",		"users/%s/uploads?v=2&alt=jsonc" ,	           0},
    {"Set Country", 		"SET_COUNTRY",			      		   2},
    {"Top Rated",		"standardfeeds/%stop_rated?v=2&alt=jsonc",	   3},
    {"Top Favorites",		"standardfeeds/%stop_favorites?v=2&alt=jsonc",     3},
    {"Top Viewed",		"standardfeeds/%smost_viewed?v=2&alt=jsonc",       3},
    {"Most Popular",		"standardfeeds/%smost_popular?v=2&alt=jsonc",      3},
    {"Most Recent",		"standardfeeds/%smost_recent?v=2&alt=jsonc",       3},
    {"Most Discussed",		"standardfeeds/%smost_discussed?v=2&alt=jsonc",    3},
    {"Most Linked", 		"standardfeeds/%srecently_featured?v=2&alt=jsonc", 3},
    {"Recently Featured",	"standardfeeds/%srecently_featured?v=2&alt=jsonc", 3},
    {"Playlist Search",		"playlists/snippets?v=2&alt=jsonc&q=%s",           0},
    {"Play Playlist", 		"playlists/%s?v=2&alt=jsonc" ,		           0},
    {"GUI Menu",		"GUI",						   0},
    {NULL, 			NULL,         				    	   0}
};


tMenuItem videoMenuItems[] =
{
    {"OMXPlayer",	      	"/usr/bin/omxplayer", 0},
    {"Mplayer",	      		"/usr/bin/mplayer",   0},
    {NULL, 			NULL, 		      0}
};
     	
     	
tMenuItem audioMenuItems[] =
{
    {"LOCAL",	      	"LOCAL", 		      0},
    {"HDMI",	      	"HDMI",  		      0},
    {NULL,		NULL, 			      0},
};
     	
tMenuItem jpegMenuItems[] =
{
    {"OMX",	      	"OMX", 			      0},
    {"LIBJPEG",	      	"LIBJPEG",                    0},
    {NULL, 		NULL, 			      0}
};
     	
tMenuItem guiMenuItems[] =
{
    {"Video Player", 	 "VP",	        	      1},
    {"Audo Device", 	 "AD",			      2},	
    {"JPEG Decoder",	 "JD",	 		      3},
    {"Font",		 "FONT",		      4},
    {"Title Font",	 "TFONT",		      5},
    {"Rows", 	 	 "ROW", 		      6},
    {"Columns", 	 "COL", 		      7},	
    {"Thumb Width",	 "TW",			      8},	 	  
    {"Tiny Font Size", 	 "TF",			      9},
    {"Small Font Size",	 "SF",			      10},
    {"Medium Font Size ","MF", 			      11},
    {"Large Font Size",  "LF",			      12},
    {"Font Vert. Space", "FS", 			      13},
    {"Input Menu",       "IM", 			      0},
    {"***Save config***","SC",			      0},
    {NULL,		NULL,			      0}
};

    			        	

tMenuItem jskbMenuItems[] =
{
    {"JS/GP X Axis",     "XA",	        	      1},
    {"JS/GP Y Axis",     "YA",			      2},	
    {"X/Y Threshold",    "TH",			      3},
    {"Info Button",      "IB",	 		      4},
    {"Menu Button",      "MB",		     	      5},
    {"Select Button",    "SB", 		      	      6},
    {"Back Button",      "BB",			      7},	 	  
    {"Mouse",		 "MM",		              8},
    {"Mouse Device",  	 "MD", 			      9}, 
    {"Joystick Device",  "JD",                       10}, 		
    {"Pointer",          "PP", 			     11},
    {"Pointer Size",     "PS",			     12},
    {"Pointer Offset X", "OX",			     13},
    {"Pointer Offset Y", "OY",			     14},
    {NULL,		 NULL,			      0}
};
     	
