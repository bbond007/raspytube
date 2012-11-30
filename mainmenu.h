tMenuItem mainMenuItems[] =
{
    {"Help / About",	      	"HELP\nbinarybond007@gmail.com\n",      		    	  0},     	
    {"Select Format", 	       	"SET_FORMAT",			      		    	          3}, 	 
    {"Regular search", 	    	"GET /feeds/api/videos?v=2&alt=jsonc&q=%s",		          0},
    {"User Favorites",		"GET /feeds/api/users/%s/favorites?v=2&alt=jsonc" ,	    	  0},
    {"User Upload",		"GET /feeds/api/users/%s/uploads?v=2&alt=jsonc" ,	          0},
    {"Set Country", 		"SET_COUNTRY",			      		    	          1},
    {"Top Rated",		"GET /feeds/api/standardfeeds/%stop_rated?v=2&alt=jsonc",	  2},
    {"Top Favorites",		"GET /feeds/api/standardfeeds/%stop_favorites?v=2&alt=jsonc",     2},
    {"Top Viewed",		"GET /feeds/api/standardfeeds/%smost_viewed?v=2&alt=jsonc",       2},
    {"Most Popular",		"GET /feeds/api/standardfeeds/%smost_popular?v=2&alt=jsonc",      2},
    {"Most Recent",		"GET /feeds/api/standardfeeds/%smost_recent?v=2&alt=jsonc",       2},
    {"Most Discussed",		"GET /feeds/api/standardfeeds/%smost_discussed?v=2&alt=jsonc",    2},
    {"Most Linked", 		"GET /feeds/api/standardfeeds/%srecently_featured?v=2&alt=jsonc", 2},
    {"Recently Featured",	"GET /feeds/api/standardfeeds/%srecently_featured?v=2&alt=jsonc", 2},
    {"Playlist Search",		"GET /feeds/api/playlists/snippets?v=2&alt=jsonc&q=%s",           0},
    {"Play Playlist", 		"GET /feeds/api/playlists/%s?v=2&alt=jsonc" ,		          0},
    {NULL, 			NULL         				         	     	   }
};
