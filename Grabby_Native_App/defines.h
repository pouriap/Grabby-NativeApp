#pragma once

//TODO: make types integer?

//general defines
#define GRB_ADDON_ID "grabby.pouriap"
#define CMD_MAX_LEN 4096
#define NATIVE_MESSAGE_MAX_LEN 1000000

#define YTDL_CANCEL_CODE 3221225786

//message types
#define MSGTYP_GET_VERSION "get_version"
#define MSGTYP_GET_AVAIL_DMS "get_available_dms"
#define MSGTYP_AVAIL_DMS "available_dms"
#define MSGTYP_DOWNLOAD "download"
#define MSGTYP_USER_CMD "user_cmd"

#define MSGTYP_YTDL_INFO "ytdl_info"
#define MSGTYP_YTDL_INFO_YTPL "ytdl_info_ytpl"
#define MSGTYP_YTDL_GET "ytdl_get"
#define YTDLTYP_VID "ytdl_video"
#define YTDLTYP_AUD "ytdl_audio"
#define YTDLTYP_PLVID "ytdl_video_playlist"
#define YTDLTYP_PLAUD "ytdl_audio_playlist"
#define MSGTYP_YTDLPROG "ytdl_progress"
#define MSGTYP_YTDL_COMP "ytdl_comp"
#define MSGTYP_YTDL_FAIL "ytdl_fail"
#define MSGTYP_YTDL_KILL "ytdl_kill"

#define MSGTYP_ERR "app_error"
#define MSGTYP_ERR_GUI "app_error_gui"
#define MSGTYP_MSG "app_message"
#define MSGTYP_UNSUPP "unsupported"

#define FILETYPE_MKV 0
#define FILETYPE_MP3 1
#define FILETYPE_ALL 2