tMenuItem mainMenuItems[] =
{
    {"Help / About",	      	"HELP\nbinarybond007@gmail.com\n",      	   0},     	
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
    {"Video Player", 		"VP",						   4},
    {"Audo Device", 		"AD",						   5},	
    {"JPEG Decoder",		"JD",	 					   6},
    {NULL, 			NULL,         				    	   7}
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
     	
     	

     	
