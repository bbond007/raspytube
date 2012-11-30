tMenuItem mainMenuItems[] =
{
    {"Help / About",	      	"HELP\nbinarybond007@gmail.com\n",      		     0},     	
    {"Select Format", 	       	"SET_FORMAT",			      		    	     3}, 	 
    {"Regular search", 	    	"videos?v=2&alt=jsonc&q=%s",		          	     0},
    {"User Favorites",		"users/%s/favorites?v=2&alt=jsonc" ,	    	  	     0},
    {"User Upload",		"users/%s/uploads?v=2&alt=jsonc" ,	          	     0},
    {"Set Country", 		"SET_COUNTRY",			      		    	     1},
    {"Top Rated",		"feeds/api/standardfeeds/%stop_rated?v=2&alt=jsonc",	     2},
    {"Top Favorites",		"feeds/api/standardfeeds/%stop_favorites?v=2&alt=jsonc",     2},
    {"Top Viewed",		"feeds/api/standardfeeds/%smost_viewed?v=2&alt=jsonc",       2},
    {"Most Popular",		"feeds/api/standardfeeds/%smost_popular?v=2&alt=jsonc",      2},
    {"Most Recent",		"feeds/api/standardfeeds/%smost_recent?v=2&alt=jsonc",       2},
    {"Most Discussed",		"feeds/api/standardfeeds/%smost_discussed?v=2&alt=jsonc",    2},
    {"Most Linked", 		"feeds/api/standardfeeds/%srecently_featured?v=2&alt=jsonc", 2},
    {"Recently Featured",	"feeds/api/standardfeeds/%srecently_featured?v=2&alt=jsonc", 2},
    {"Playlist Search",		"feeds/api/playlists/snippets?v=2&alt=jsonc&q=%s",           0},
    {"Play Playlist", 		"feeds/api/playlists/%s?v=2&alt=jsonc" ,		     0},
    {NULL, 			NULL         				         	      }
};
