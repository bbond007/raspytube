#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "EGL/egl.h"
#include "GLES/gl.h"
#include "gfxlib.h"
#include "ui.h"
#include "config.h"
#include "kbjs.h"
#include "term.h"

//------------------------------------------------------------------------------

typedef struct bufLink
{
    unsigned char buf[BUFSIZ];//
    unsigned int count;
    struct bufLink *next;
};

#define AFORMAT_HEIGHT 24
#define AFORMAT_WIDTH 8
#define MAIN_MENU_SPEC_REGION  7
#define MAIN_MENU_SPEC_USER    5
#define MAIN_MENU_STD_SEARCH   3
#define MAIN_MENU_SET_CATEGORY 2


extern char * supported_formats[AFORMAT_HEIGHT][AFORMAT_WIDTH];
//------------------------------------------------------------------------------

static bool youtube_search(char * searchStr);
static int create_tcp_socket();


static char *get_ip(char *host);
static char *build_youtube_query(char *host, char *searchStr, int results, int startIndex);
static char *build_file_request(char *host, char *fileName);
//static char * convert_UTF_8(char * src);
static int establish_socket_connection(char * host, int Port);
static void handle_output_jsonc(unsigned int iBracket, unsigned int iBrace, char * key, char * value);
static void parse_buffer_jsonc(char * jsoncContent, bool reset);
static bool youtube_search(char * searchStr);
static void play_video (char * url);
unsigned char *download_file(char * host, char * fileName, unsigned int * fileSize);
unsigned char *find_jpg_start(unsigned char * buf, unsigned int * bufSize);
static void do_search(char * searchStr);
static void do_user_search(char * userStr);
static bool try_move(int step, bool foward);
static void do_cur_up();
static void do_cur_down();
static void do_cur_right(char * searchStr);
static void do_cur_left(char * searchStr);
static void do_more(char * searchStr);
static void do_less(char * searchStr);
#define USERAGENT "RASPITUBE 1.0"
#define PORT 80
#define HOST "gdata.youtube.com"
#define AUTHOR binarybond007@gmail.com

static void do_main_menu(char * searchStr, char * userStr);
static void do_gui_menu();
static void do_jskb_menu();
static void do_info_menu(char * searchStr);
static void do_change_audio_dev();
static void do_change_jpeg_dec();
static void do_change_video_player();
static void do_download(char * url, char * title);

#define PICK_SEARCH_STR ((mainMenuItems[mainMenu.selectedItem].special==MAIN_MENU_SPEC_USER)?userStr:searchStr)
//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    bcm_host_init();
    initKb();
    init_ui();
    open_joystick();
    open_mouse();
    clear_output();
    redraw_results(false);
    char searchStr [100] = "";
    char userStr[100] = "";
    char txt[200];
    bool quit = false;
    int result;
    mainMenu.selectedIndex = mainMenu.selectedItem = MAIN_MENU_STD_SEARCH;
    if(argc > 1)
    {
        youtube_search(argv[1]);
        if(strlen(argv[1]) < sizeof(searchStr))
            strcpy(searchStr, argv[1]);
        else
            show_message("argv[1] too big!!!", 10, ERROR_POINT);
    }
    else
        youtube_search("raspberry+pi");

    int key;
    do
    {
        redraw_results(false);
        key = readKb_mouse(); //wait for keypress
        switch (key)
        {

        case MOUSE_1:
            switch(mouse_select(&clickXY))
            {
            case msFarLeft:
                if(state->screen_height * 0.92f < clickXY.y)
                    do_main_menu(searchStr, userStr);
                else
                    do_less(PICK_SEARCH_STR);
                break;
            case msFarRight:
                if(state->screen_height * 0.92f < clickXY.y)
                    resize_ui();
                else
                    do_more(PICK_SEARCH_STR);
                break;
            case msSameRec:
                do_info_menu(PICK_SEARCH_STR);
                break;
            case msInvalid:
                break;
            case msNewRec:
                break;
            }
            break;

        case CUR_UP :
            do_cur_up();
            break;

        case CUR_DWN:
            do_cur_down();
            break;

        case CUR_R:
            do_cur_right(PICK_SEARCH_STR);
            break;

        case CUR_L:
            do_cur_left(PICK_SEARCH_STR);
            break;

        case 'c':
        case 'C':
            redraw_results(false);
            setBGImage();
            show_menu(&categoryMenu);
            break;
        case 'g':
        case 'G':
            redraw_results(false);
            setBGImage();
            do_gui_menu();
            break;
        case 'h':
        case 'H' :
            do_change_audio_dev();
            break;

        case 'j':
            redraw_results(false);
            setBGImage();
            do_jskb_menu();
            break;
        case 'J' :
            do_joystick_test();
            break;

        case 't':
        case 'T' :
            //show_message("TEST-->\n~0|0:~0X~0|1:~1X~0|2:~2X~0|3:~3X~0|4:~4X~0|5:~5X~0|6:~6X~0|7:~7X\n~5press [ESC]", 0x55378008, ERROR_POINT);
            break;

        case 'm':
        case 'M':
            do_main_menu(searchStr, userStr);
            break;

        case 'r':
        case 'R':
            redraw_results(false);
            setBGImage();
            result =  show_menu(&regionMenu);
            if(result > -1)
            {
                snprintf(txt, sizeof(txt), "item #%d\nkey->%s\ndescription->%s",
                         result,
                         regionMenuItems[result].key,
                         regionMenuItems[result].description);
                show_message(txt, 3, ERROR_POINT);
            }
            break;

        case 'x':
        case 'X' :
            do_change_jpeg_dec();
            break;

        case 'q':
        case 'Q': resize_ui();
            break;
        case 'f':
        case 'F' :
            redraw_results(false);
            setBGImage();
            result = show_format_menu(&formatMenu);
            redraw_results(true);
            break;

        case 'p':
        case 'P':
            do_change_video_player();
            break;

        case 'i':
        case 'I':
            do_info_menu(PICK_SEARCH_STR);
            break;

        case 'n':
        case 'N': //new search -> fall through to 'S'
            searchStr[0] = 0x00;

        case 's':
        case 'S' : //modify existing search
            redraw_results(false);
            setBGImage();
            mainMenu.scrollIndex   = 0;
            mainMenu.selectedIndex = mainMenu.selectedItem = MAIN_MENU_STD_SEARCH;
            do_search(searchStr);
            break;

        case JOY_1:
        case RTN_KEY:
            if(selected_rec != NULL)
            {
                if (selected_rec->url != NULL)
                    play_video(selected_rec->url);
                else
                    show_message("Unable to play:\n\nselected_rec->url==NULL", 4, ERROR_POINT);
            }
            break;

        case MOUSE_2:
        case ESC_KEY:
            redraw_results(false);
            setBGImage();
            quit = yes_no_dialog("Quit?", false);
            if(!quit)
                break;

        default :
            break;
        }
    }
    while (!quit);
    clear_output();
    free_ui();
    restoreKb();
    return 0;
}


//------------------------------------------------------------------------------
static void do_download(char * url, char * title)
{
    char * server = NULL;
    char * page = NULL;
    char * freeMe = parse_url(url, &server, &page);
    char request_format[6] = "";
    redraw_results(false);
    tTermState ts;
    term_init(&ts, .70f, .95f, -1, -1);
    term_set_color(&ts, 7);
    term_put_str(&ts, "Calling youtube_dl...\n");
    term_set_color(&ts, 5);
    if(title != NULL)
        term_put_str(&ts, title);
    term_put_str(&ts, "\n");
    term_put_str(&ts, url);
    term_set_color(&ts, 0);
    term_put_str(&ts, "\n");
    term_show(&ts, true);
    if(server != NULL && page != NULL)
    {
        if (numFormat > 0)
            snprintf(request_format, sizeof(request_format), "-f%s", supported_formats[numFormat+1][0]);
        char youtube_dl_format[] = "youtube-dl -t %s http://%s/%s";

        size_t stCommand = strlen(server) +
                           strlen(page) +
                           strlen(request_format) +
                           strlen(youtube_dl_format);
        char * youtube_dl_command   = malloc(stCommand);
        snprintf(youtube_dl_command, stCommand, youtube_dl_format, request_format, server, page);
        term_command(&ts, youtube_dl_command);
        free(youtube_dl_command);
    }
    term_free(&ts);
    free(freeMe);
}

//------------------------------------------------------------------------------
static void do_main_menu(char * searchStr, char * userStr)
{
    int result;
    redraw_results(false);
    char txt[100];
    setBGImage();
    do
    {
        result =  show_menu(&mainMenu);
        if(result > -1)
        {
            switch(result)
            {
            case 1:
                show_format_menu(&formatMenu);
                break;
            case 2:
                show_menu(&categoryMenu);
                break;
            case 3:
                do_search(searchStr);
                break;
            case 4:
            case 5:
                do_user_search(userStr);
                break;
            case 6:
                show_menu(&regionMenu);
                break;
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
                numStart = 1;
                clear_output();
                clear_screen(true);
                youtube_search(NULL);
                if (selected_rec == NULL)
                    show_message("Region search returned 0 results!", 1, ERROR_POINT);
                break;
                //  case 14:
                //  case 15:
                //      break;
            case 17:
                do_gui_menu();
                break;
            default:
                snprintf(txt, sizeof(txt), "item #%d\nkey->%s\ndescription->%s\n~5**UNDER CONSTRUCTION**",
                         result,
                         mainMenuItems[result].key,
                         mainMenuItems[result].description);
                show_message(txt, 0xc64a1200, ERROR_POINT);
                break;
            }
        }
    }
    while ( result !=  -1 && result != 3 && 
            result !=   4 && result != 5 &&
            !(result >= 7 && result <= 14));

}
//------------------------------------------------------------------------------
static void do_jskb_menu()
{
    int result;
    do
    {
        result = show_menu(&jskbMenu);
        switch(result)
        {
        case 0:
            break;
        case 7:  mouseEnabled = !mouseEnabled;
        break;
        case 14: saveConfig();
        break;
        }
    }
    while (result != -1);
}

//------------------------------------------------------------------------------
static void do_gui_menu()
{
    int result;
    int resultFont;
    do
    {
        result = show_menu(&guiMenu);
        switch(result)
        {
        case 0:
            do_change_video_player();
            break;
        case 1:
            do_change_audio_dev();
            break;
        case 2:
            do_change_jpeg_dec();
            break;
        case 3:
            resultFont = show_menu(&fontMenu);
            if(resultFont != -1)
            {
                set_font((int) resultFont);
                redraw_results(false);
                setBGImage();
            }
            break;
        case 4:
            resultFont = show_menu(&titleFontMenu);
            if(resultFont != -1)
            {
                set_title_font((int) resultFont);
                redraw_results(false);
                setBGImage();
            }
            break;
        case 13:
            do_jskb_menu();
            break;

        case 14:
            saveConfig();
            break;
        }
    }
    while (result != -1);
}

//------------------------------------------------------------------------------
static void do_info_menu(char * searchStr)
{
    int result;
    if(selected_rec != NULL)
    {
        do
        {
            result = show_selection_info(selected_rec);
            switch(result)
            {

            case 'f':
            case 'F':
                redraw_results(false);
                setBGImage();
                show_format_menu(&formatMenu);
                break;
            case 'i':
            case 'I':
                redraw_results(false);
                setBGImage();
                if(selected_rec != NULL && selected_rec->url != NULL)
                    if(yes_no_dialog("Download?", true))
                        do_download(selected_rec->url, selected_rec->title);
                break;
            case CUR_L :
                do_cur_left(searchStr);
                break;
            case CUR_R:
                do_cur_right(searchStr);
                break;
            case CUR_UP :
                do_cur_up();
                break;
            case CUR_DWN:
                do_cur_down();
                break;
            case JOY_1:
            case RTN_KEY:
                if(selected_rec != NULL && selected_rec->url != NULL)
                    play_video(selected_rec->url);
                break;
            }
        }
        while (result != ESC_KEY);
    }
}
//------------------------------------------------------------------------------
static bool try_move(int step, bool foward)
{
    int i;
    if(selected_rec == NULL)
        return false;
    struct result_rec * temp_rec = selected_rec;
    for (i=0; i < step; i++)
    {
        if(foward)
            temp_rec = temp_rec->next;
        else
            temp_rec = temp_rec->prev;
        if(temp_rec == NULL)
            return false;
    }
    selected_rec = temp_rec;
    return true;
}

//------------------------------------------------------------------------------
static void do_cur_up()
{
    if (selected_rec != NULL)
    {
        if(selected_rec->prev != NULL)
            selected_rec = selected_rec->prev;
        else
            selected_rec = last_rec;
    }
}

//------------------------------------------------------------------------------
static void do_cur_down()
{
    if (selected_rec != NULL)
    {
        if(selected_rec->next != NULL)
            selected_rec = selected_rec->next;
        else
            selected_rec = first_rec;
    }
}

//------------------------------------------------------------------------------
static void do_more(char * searchStr)
{
    if(numStart < 500)
    {
        numStart += numResults;
        clear_output();
        clear_screen(true);
        youtube_search(searchStr);
    }
}
//------------------------------------------------------------------------------
static void do_less(char * searchStr)
{
    if (numStart > 1)
    {
        numStart -= numResults;
        if(numStart < 1)
            numStart = 1;
        clear_output();
        clear_screen(true);
        youtube_search(searchStr);
    }
}
//------------------------------------------------------------------------------
static void do_cur_right(char * searchStr)
{
    if(!try_move(numRow, true))
        do_more(searchStr);
}

//------------------------------------------------------------------------------
static void do_cur_left(char * searchStr)
{
    if(!try_move(numRow, false))
        do_less(searchStr);
}

//------------------------------------------------------------------------------
static void do_change_audio_dev()
{
    if (soundOutput == soHDMI)
        soundOutput = soLOCAL;
    else
        soundOutput = soHDMI;
}

//------------------------------------------------------------------------------
static void do_change_jpeg_dec()
{
    if (jpegDecoder == jdOMX)
        jpegDecoder = jdLibJpeg;
    else
        jpegDecoder = jdOMX;
}

//------------------------------------------------------------------------------
static void do_change_video_player()
{
    if (videoPlayer == vpOMXPlayer)
        videoPlayer = vpMPlayer;
    else
        videoPlayer = vpOMXPlayer;
}

//------------------------------------------------------------------------------
static void do_search(char * searchStr)
{
    char caption[] = "Search ";
    size_t stPrompt = ((categoryMenu.selectedItem>0)?strlen(categoryMenuItems[categoryMenu.selectedItem].description):0)  + strlen(caption)+ 5;
    char * prompt = malloc(stPrompt);
    strncpy(prompt, caption, stPrompt);
    if(categoryMenu.selectedItem > 0)
    {
        strncat(prompt, "(", stPrompt);
        strncat(prompt, categoryMenuItems[categoryMenu.selectedItem].description, stPrompt);
        strncat(prompt, ")", stPrompt);
    }
    strncat(prompt, ":", stPrompt);
    int result = input_string(prompt, searchStr, 50);
    free(prompt);
    if(result)
    {
        replace_char_str(searchStr, ' ', '+');
        clear_output();
        numStart = 1;
        clear_screen(true);
        youtube_search(searchStr);
        if (selected_rec == NULL)
            show_message("Search returned 0 results!", 5, ERROR_POINT);
    }
}

//------------------------------------------------------------------------------
static void do_user_search(char * userStr)
{
    char caption[] = "Search ";
    size_t stPrompt = strlen(mainMenuItems[mainMenu.selectedItem].description) + strlen(caption)+ 4;
    char * prompt = malloc(stPrompt);
    strncpy(prompt, caption, stPrompt);
    strncat(prompt, "(", stPrompt);
    strncat(prompt, mainMenuItems[mainMenu.selectedItem].description, stPrompt);
    strncat(prompt, "):", stPrompt);
    int result = input_string(prompt, userStr, 50);
    free(prompt);
    if(result)
    {
        clear_output();
        numStart = 1;
        clear_screen(true);
        youtube_search(userStr);
        if (selected_rec == NULL)
            show_message("User search returned 0 results!", 6, ERROR_POINT);
    }
}

//------------------------------------------------------------------------------
static void play_video (char * url)
{
    char * server = NULL;
    char * page = NULL;
    char * freeMe = parse_url(url, &server, &page);
    char url2[4096] = "";
    char request_format[6] = "";
    int status;
    redraw_results(false);
    show_message("Calling youtube_dl...", false, numPointFontMed);
    eglSwapBuffers(state->display, state->surface);
    //show_message(url, false, 20);
    if(server != NULL && page != NULL)
    {
        if (numFormat > 0)
            snprintf(request_format, sizeof(request_format), "-f%s", supported_formats[numFormat+1][0]);
        char youtube_dl_format[] = "youtube-dl %s -g http://%s/%s";

        size_t stCommand = strlen(server) +
                           strlen(page) +
                           strlen(request_format) +
                           strlen(youtube_dl_format);
        char * youtube_dl_command   = malloc(stCommand);
        snprintf(youtube_dl_command, stCommand, youtube_dl_format, request_format, server, page);
        free(freeMe);
        FILE * fp = popen(youtube_dl_command, "r");
        unsigned int i = strlen(url2);
        free(youtube_dl_command);
        do
        {
            int c = fgetc(fp);
            if (c == RTN_KEY || c == EOF)
            {
                //trash that Pesky EOF and CR.
            }
            else if(c < 0x80) // Normal char
                url2[i++] = c;
            else // Convert to UTF-8
            {
                url2[i++] = (0xc0|(c & 0xc0)>>6);
                url2[i++] = (0x80|(c & 0x3f));
            }
            if(i > sizeof(url2) - 3)
            {
                show_message("YIKES! popen(youtube-dl) failed. URL TOO LONG!", true, ERROR_POINT);
                return;
            }
        }
        while(!feof(fp));
        url2[i] = 0x00;
        fclose(fp);
        clear_screen(false);
        switch(videoPlayer)
        {
        case vpMPlayer:
            show_message("Calling youtube_dl... ~5DONE~0\nStarting Mplayer...", false, numPointFontMed);
            eglSwapBuffers(state->display, state->surface);
            break;
        case vpOMXPlayer:
            show_message("Calling youtube_dl... ~5DONE~0\nStarting OMXPlayer...", false, numPointFontMed);
            eglSwapBuffers(state->display, state->surface);
            break;
        }

        int pid = fork();

        if(pid < 0)
        {
            show_message("Fork failed", true, ERROR_POINT);
            exit(errno);
        }

        if (pid == 0)
        {
            int iArgv = 0;
            char * player_argv[6];

            switch(videoPlayer)
            {
            case vpMPlayer:
                player_argv[iArgv++]="/usr/bin/mplayer";
                //player_argv[iArgv++]="-fs";
                player_argv[iArgv++]="--";
                break;

            case vpOMXPlayer:
                player_argv[iArgv++]="/usr/bin/omxplayer";

                switch(soundOutput)
                {
                case soHDMI:
                    player_argv[iArgv++]="-ohdmi";
                    break;
                case soLOCAL:
                    player_argv[iArgv++]="-olocal";
                    break;
                default:
                    show_message("Unknown sound output enum", true, ERROR_POINT);
                }

                break;

            default:
                show_message("Unknown video player enum.", true, ERROR_POINT);

            }
            player_argv[iArgv++]=url2;
            player_argv[iArgv++]=NULL;
            execvp(player_argv[0],player_argv);
            exit(200);
        }
        int wpid;
        int key = 0x00;
        do
        {
            wpid = waitpid(pid, &status, WNOHANG);
            if (wpid == 0)
            {
                if(jsESC() || rbPressed() || key == ESC_KEY)
                {
                    system("killall omxplayer.bin");
                    switch(videoPlayer)
                    {
                    case vpOMXPlayer:
                        system("killall omxplayer.bin");
                        break;
                    case vpMPlayer:
                        system("killall mplayer");
                        break;
                    }

                }
                else
                    usleep(2000);
                
                if (bQScreen)
                   x_window_loop(&key, false);
            }
        }
        while (wpid == 0);
        redraw_results(true);
    }
    else
    {
        //yikes server or page is null
        free(freeMe);
        show_message("URL Parser SUX.", true, ERROR_POINT);
    }
}
//------------------------------------------------------------------------------
unsigned char * find_jpg_start(unsigned char * buf, unsigned int* bufSize)
{
    unsigned char * temp;
    for(temp = buf; temp < buf + *bufSize; temp++)
        if(temp[0] == 0xFF && temp[1] == 0xD8)
        {
            *bufSize = *bufSize - (temp - buf);
            return temp;
        }
    return NULL;
}

//------------------------------------------------------------------------------

static void handle_output_jsonc(unsigned int iBracket, unsigned int iBrace, char * key, char * value)
{
    if (iBracket == 3 && strcmp(key, "id") == 0)
    {
        if(first_rec == NULL)
        {
            first_rec = init_result_rec();
            last_rec  = first_rec;
        }
        else
        {
            redraw_results(true);
            struct result_rec * temp_rec = init_result_rec();
            temp_rec->prev = last_rec;
            last_rec->next = temp_rec;
            last_rec = temp_rec;
        }
    }
    char ** pColumn = get_lastrec_column(iBracket, iBrace, key);
    if(pColumn != NULL && *pColumn == NULL )
    {

        *pColumn = malloc(strlen(value)+1);
        strcpy(*pColumn, value);
    }
}

//------------------------------------------------------------------------------
static void parse_buffer_jsonc(char * jsoncContent, bool reset)
{
    char * c = jsoncContent;
    static unsigned int iBracket = 0;
    static unsigned int iBrace   = 0;
    static unsigned int iKey     = 0;
    static unsigned int iValue   = 0;
    static bool quote            = false;
    static bool findkey          = true;
    static char key[128]         = "";
    static char value[1024]      = "";
    static char lastc            = 0x00;

    if(reset)
    {
        quote    = false;
        key[0]   = 0x00;
        value[0] = 0x00;
        iKey     = 0;
        iValue   = 0;
        findkey  = false;
        iBracket = 0;
        iBrace   = 0;
    }

    while(*c != 0x00)
    {
        bool bSkip = false;
        if (*c == '"' && lastc != BSL_KEY )
        {
            quote = !quote;
            bSkip = true;
        }

        if (*c == BSL_KEY && lastc != BSL_KEY)
            bSkip = true;

        if (!quote)
        {
            switch (*c)
            {

            case '{' :
                iBracket++;
                findkey   = true;
                iKey      = 0;
                key[0]    = 0x00;
                bSkip     = true;
                break;

            case '[' :
                findkey   = true;
                iKey      = 0;
                key[0]    = 0x00;
                iBrace++;
                bSkip     = true;
                break;

            case ']' :
                iBrace--;
                bSkip     = true;
                break;

            case '}' :
                handle_output_jsonc(iBracket, iBrace, key, value);
                findkey  = true;
                iKey     = 0;
                key[0]   = 0x00;
                iValue   = 0;
                value[0] = 0x00;
                bSkip    = true;
                iBracket--;
                break;

            case ',' :

                handle_output_jsonc(iBracket, iBrace, key, value);
                findkey  = true;
                iKey     = 0;
                key[0]   = 0x00;
                iValue   = 0;
                value[0] = 0x00;
                bSkip    = true;
                break;

            case '\n' :
                bSkip    = true;
                break;

            case '\r' :
                bSkip    = true;
                break;

            case '\t':
                bSkip    = true;
                break;

            case ':' :
                findkey  = false;
                iValue   = 0;
                value[0] = 0x00;
                bSkip    = true;
                break;
            }
        }

        if(!bSkip)
        {
            if(findkey)
            {
                if (quote && iKey < sizeof(key)-1)
                {
                    key[iKey++] = *c;
                    key[iKey] = 0x00;
                }
            }
            else
            {
                if(iValue < sizeof(value)-1)
                {
                    if(*c != ' ' || quote)
                    {
                        value[iValue++] = *c;
                        value[iValue] = 0x00;
                    }
                }
            }
        }
        lastc = *c;
        c++;
    }
}

//------------------------------------------------------------------------------
int establish_socket_connection(char * host, int Port)
{
    struct sockaddr_in *remote = NULL;
    int sock = 0;
    char *ip = NULL;
    int result;

    sock = create_tcp_socket();
    ip = get_ip(host);
    if(ip == NULL)
    {
        sock = 0;
        goto cleanup;
    }

    remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
    remote->sin_family = AF_INET; result = inet_pton(AF_INET, ip, (void
    *)(&(remote->sin_addr.s_addr)));

    if(result < 0)
    {
        show_message("Can't set remote->sin_addr.s_addr", true, ERROR_POINT);
        sock = 0;
        goto cleanup;
    }

    else if(result == 0)
    {
        show_message("invalid IP address\n", true, ERROR_POINT);
        sock = 0;
        goto cleanup;
    }

    remote->sin_port = htons(PORT);
    if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0)
    {
        sock = 0;
        goto cleanup;
    }

cleanup:

    if(ip != NULL)
        free(ip);

    if(remote != NULL)
        free(remote);

    return sock;
}

//------------------------------------------------------------------------------
static bool youtube_search(char * searchStr)
{
    char buf[BUFSIZ+1];
    bool reset = true;
    int sock = 0;
    int bytesTrans;
    char * get = NULL;

    sock = establish_socket_connection(HOST, PORT);
    if(sock == 0)
        goto cleanup;

    get = build_youtube_query(HOST, searchStr, numResults, numStart);

    int sent = 0;
    while(sent < strlen(get))
    {
        bytesTrans = send(sock, get+sent, strlen(get)-sent, 0);

        if(bytesTrans == -1)
        {
            show_message("Can't send query", true, ERROR_POINT);
            goto cleanup;
        }
        sent += bytesTrans;
    }
//now it is time to receive the data
    memset(buf, 0, sizeof(buf));
    int htmlstart = 0;
    char * jsoncContent;
    while((bytesTrans = recv(sock, buf, BUFSIZ, 0)) > 0)
    {
        if(htmlstart == 0)
        {
            /* Under certain conditions this will not work.
            * If the \r\n\r\n part is splitted into two messages
            * it will fail to detect the beginning of HTML content
            */
            jsoncContent = strstr(buf, "\r\n\r\n");
            if(jsoncContent != NULL)
            {
                htmlstart = 1;
                jsoncContent += 4;
            }
        }
        else
        {
            jsoncContent = buf;
        }

        if(htmlstart)
        {
            parse_buffer_jsonc(jsoncContent, reset);
            reset=false;
        }
        memset(buf, 0, bytesTrans);
    }

    if(bytesTrans < 0)
    {
        show_message("Error receiving data", true, ERROR_POINT);
        goto cleanup;
    }

cleanup:

    if (get != NULL)
        free(get);

    if (sock > 0)
        close(sock);

    return 0;
}

//------------------------------------------------------------------------------
unsigned char * download_file(char * host, char * fileName, unsigned int * fileSize)
{
    int sock = 0;
    int bytesTrans;
    char * get = NULL;
    unsigned char * result = NULL;

    sock = establish_socket_connection(host, PORT);
    if(sock == 0)
        goto cleanup;

    get = build_file_request(host, fileName);

    int sent = 0;
    while(sent < strlen(get))
    {
        bytesTrans = send(sock, get+sent, strlen(get)-sent, 0);

        if(bytesTrans == -1)
        {
            show_message("Can't send query", true, ERROR_POINT);
            goto cleanup;
        }
        sent += bytesTrans;
    }

    struct bufLink * saveBuf = NULL;
    struct bufLink * firstBuf = NULL;
    struct bufLink * buf = NULL;
    unsigned int count = 0;
    do
    {
        if(buf != NULL)
            saveBuf= buf;
        buf = malloc(sizeof(struct bufLink));
        if(firstBuf == NULL)
            firstBuf = buf;
        buf->next = NULL;
        if(saveBuf != NULL)
            saveBuf->next = buf;
        buf->count = recv(sock, &buf->buf[0], sizeof(buf->buf), 0);
        count += buf->count;
    }
    while (buf->count > 0);


    if(buf->count < 0)
    {
        show_message("Error receiving data", true, ERROR_POINT);
        goto cleanup;
    }

    result = malloc(count);
    *fileSize = count;

    count = 0;
    buf = firstBuf;
    while (buf != NULL)
    {
        saveBuf = buf;
        memcpy(&result[count], &buf->buf[0], buf->count);
        count += buf->count;
        buf = buf->next;
        free(saveBuf);
    }

cleanup:

    if (get != NULL)
        free(get);

    if (sock > 0)
        close(sock);

    return result;
}
//------------------------------------------------------------------------------

static int create_tcp_socket()
{
    int sock;
    if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        show_message("Can't create TCP socket", true, ERROR_POINT);
        return 0;
    }
    return sock;
}

//------------------------------------------------------------------------------

static char *get_ip(char *host)
{
    struct hostent *hent;
    int iplen = 15; //XXX.XXX.XXX.XXX
    char *ip = (char *)malloc(iplen+1);
    memset(ip, 0, iplen+1);
    if((hent = gethostbyname(host)) == NULL)
    {
        show_message("Can't get IP", true, ERROR_POINT);
        return NULL;
    }

    if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
    {
        show_message("Can't resolve host", true, ERROR_POINT);
        return NULL;
    }
    return ip;
}

//------------------------------------------------------------------------------

static char *build_youtube_query(char *host, char *searchStr, int results, int startIndex)
{

    char * query;
    char tempBegin[] = "GET /feeds/api/";
    char * tempEnd = "&max-results=%d&start-index=%d HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
    char * temp;
    char * categoryFmtStr = mainMenuItems[MAIN_MENU_SET_CATEGORY].key;
    char * categoryStr = "";
    char * tempMid;
    bool bCategory = false;
        
    switch(mainMenuItems[mainMenu.selectedItem].special)
    {
        case MAIN_MENU_SPEC_REGION : tempMid = mainMenuItems[mainMenu.selectedItem].key;
            break;
        case MAIN_MENU_SPEC_USER   : tempMid = mainMenuItems[mainMenu.selectedItem].key;
            break;
        default:
            tempMid = mainMenuItems[MAIN_MENU_STD_SEARCH].key;
            bCategory = true;
            break;
    }
        
    char country[6] = "";
    size_t stTemp = strlen(tempBegin) +strlen(tempMid) + strlen(tempEnd) + 1;
    temp = malloc(stTemp);
    temp[0] = 0x00;
    strncat(temp, tempBegin, stTemp);
    strncat(temp, tempMid,   stTemp);
    strncat(temp, tempEnd,   stTemp);
    if (mainMenuItems[mainMenu.selectedItem].special == MAIN_MENU_SPEC_REGION)
    {
        if (regionMenu.selectedItem > 0)
        {
            strcat(country, regionMenuItems[regionMenu.selectedItem].key);
            strcat(country, "/");
        }
        searchStr = country;
    }

    if (bCategory && categoryMenu.selectedItem > 0)
    {
        size_t stCatFmt = strlen(categoryFmtStr) +
                          strlen(categoryMenuItems[categoryMenu.selectedItem].key);
        categoryStr = malloc(stCatFmt);
        snprintf(categoryStr, stCatFmt, categoryFmtStr,  
            categoryMenuItems[categoryMenu.selectedItem].key);
    }
     
    size_t stQuery = strlen(host) +
                     strlen(searchStr)+
                     strlen(USERAGENT)+
                     strlen(temp) + 
                     strlen(categoryStr) + 30;
                     
    query = malloc (stQuery);
    if(bCategory) // REGULAR SEARCH
        snprintf(query, stQuery, temp, searchStr, categoryStr, results, startIndex, host, USERAGENT);
    else
        snprintf(query, stQuery, temp, searchStr, results, startIndex, host, USERAGENT);
    
    free(temp);
    if(strcmp(categoryStr, "") != 0)
        free(categoryStr);
    //show_message(query, true, ERROR_POINT);
    return query;
}

//------------------------------------------------------------------------------
static char *build_file_request(char *host, char * fileName)
{
    char *query;
    char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
    if(fileName[0] == '/')
        fileName++;
    size_t stQuery = strlen(host)+strlen(fileName)+strlen(USERAGENT)+strlen(tpl);
    query = (char *)malloc(stQuery);
    snprintf(query, stQuery, tpl, fileName, host, USERAGENT);
    return query;
}
/* now convert on the fly with fgetc
//------------------------------------------------------------------------------
static char * convert_UTF_8(char * src)
{
    int length = strlen(src);
    int i;
    for(i = 0; i < strlen(src); i++)
        if(src[i] >= 80)
            length++;
    char * dst = malloc(length + 1);
    char * tmp = dst;
    for(i =0; i < strlen(src); i++)
    {
        if(src[i]<0x80)
        {
            *tmp = src[i];
            tmp++;
        }
        else
        {
            *tmp = (0xc0|(src[i]&0xc0)>>6);
            tmp++;
            *tmp = (0x80|(src[1]&0x3f));
            tmp++;
        }
    }
    *tmp=0x00;
    return dst;
}
*/
